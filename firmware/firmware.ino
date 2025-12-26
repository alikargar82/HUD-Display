#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <lvgl.h>

#if LV_USE_TFT_ESPI
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
#endif

// ===== تنظیمات فونت =====
// نکته مهم: اگر فونت فارسی تبدیل شده ندارید، خط زیر را کامنت کنید و از فونت پیش‌فرض استفاده کنید
// LV_FONT_DECLARE(lv_font_persian_20);
#define HUD_FONT_BIG &lv_font_montserrat_48
#define HUD_FONT_SMALL &lv_font_montserrat_20 // جایگزین کنید با &lv_font_persian_20 برای فارسی

// ===== تنظیمات BLE =====
#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-90ab-cdef-1234567890ab"

// ===== تنظیمات نمایشگر =====
#define TFT_HOR_RES 480
#define TFT_VER_RES 320
#define DRAW_BUF_SIZE (TFT_HOR_RES * 20)
uint32_t draw_buf[DRAW_BUF_SIZE / 4];
lv_display_t * disp;

// ===== آبجکت‌های UI =====
lv_obj_t * lbl_clock; // ساعت HH:MM:SS
lv_obj_t * icon_ble; // آیکون بلوتوث
lv_obj_t * panel_popup; // پنل اصلی پاپ‌آپ
lv_obj_t * lbl_app_name; // نام برنامه (Telegram)
lv_obj_t * lbl_sender; // نام فرستنده
lv_obj_t * lbl_message; // متن پیام

// ===== متغیرهای وضعیت =====
bool mirrorEnabled = false;
bool bleConnected = false;
unsigned long popupTimer = 0;
bool isPopupVisible = false;

// متغیرهای ساعت ماک
int mockHour = 12;
int mockMinute = 59;
int mockSecond = 45;

// ===== رنگ‌ها =====
#define COLOR_BG lv_color_hex(0x000000)
#define COLOR_ACCENT lv_color_hex(0x00FFFF)
#define COLOR_CONNECTED lv_color_hex(0x00FF00) // سبز
#define COLOR_DISCONNECTED lv_color_hex(0xFF0000) // قرمز
#define COLOR_POPUP_BG lv_color_hex(0x222222)

// ==========================================
// لاجیک پارس کردن پیام
// ==========================================
void show_notification(String fullMsg) {
// فرمت ورودی: app|sender|message
// مثال: Telegram|Ali|Salam Chetori?

String appName = "System";
String sender = "Unknown";
String msgBody = fullMsg;

int firstPipe = fullMsg.indexOf('|');
if (firstPipe != -1) {
appName = fullMsg.substring(0, firstPipe);
int secondPipe = fullMsg.indexOf('|', firstPipe + 1);
if (secondPipe != -1) {
sender = fullMsg.substring(firstPipe + 1, secondPipe);
msgBody = fullMsg.substring(secondPipe + 1);
} else {
// اگر فقط یک پایپ بود: App|Message
msgBody = fullMsg.substring(firstPipe + 1);
}
}

// تنظیم متن‌ها
lv_label_set_text(lbl_app_name, appName.c_str());
lv_label_set_text(lbl_sender, sender.c_str());
lv_label_set_text(lbl_message, msgBody.c_str());

// نمایش پاپ‌آپ
lv_obj_clear_flag(panel_popup, LV_OBJ_FLAG_HIDDEN);

// انیمیشن ساده (اختیاری: تغییر شفافیت)
lv_obj_set_style_opa(panel_popup, LV_OPA_COVER, 0);

isPopupVisible = true;
popupTimer = millis();
}

// ==========================================
// مدیریت BLE
// ==========================================
class MyServerCallbacks : public BLEServerCallbacks {
void onConnect(BLEServer* pServer) override {
bleConnected = true;
lv_obj_set_style_text_color(icon_ble, COLOR_CONNECTED, 0); // سبز
}
void onDisconnect(BLEServer* pServer) override {
bleConnected = false;
lv_obj_set_style_text_color(icon_ble, COLOR_DISCONNECTED, 0); // قرمز
pServer->getAdvertising()->start();
}
};

class MyCallbacks : public BLECharacteristicCallbacks {
void onWrite(BLECharacteristic *pCharacteristic) override {
String value = pCharacteristic->getValue(); // تبدیل خودکار به String آردوینو
if (value.length() > 0) {
show_notification(value);
}
}
};

// ==========================================
// UI Design
// ==========================================
void my_disp_flush(lv_display_t * disp_drv, const lv_area_t * area, uint8_t * px_map) {
uint32_t w = lv_area_get_width(area);
uint32_t h = lv_area_get_height(area);

#if LV_USE_TFT_ESPI
tft.startWrite();
tft.setAddrWindow(area->x1, area->y1, w, h);
if (!mirrorEnabled) {tft.pushPixels((uint16_t *)px_map, w * h);
} else {
uint16_t * pixels = (uint16_t *)px_map;
uint16_t * line_buf = (uint16_t *)malloc(w * sizeof(uint16_t));
if(line_buf) {
for (uint32_t y = 0; y < h; y++) {
uint32_t row_offset = y * w;
for (uint32_t x = 0; x < w; x++) {
line_buf[x] = pixels[row_offset + (w - 1 - x)];
}
tft.pushPixels(line_buf, w);
}
free(line_buf);
} else {
tft.pushPixels((uint16_t *)px_map, w * h);
}
}
tft.endWrite();
#endif
lv_display_flush_ready(disp_drv);
}

void build_ui() {
lv_obj_t * scr = lv_screen_active();
lv_obj_set_style_bg_color(scr, COLOR_BG, 0);

// 1. آیکون BLE (بالا راست)
icon_ble = lv_label_create(scr);
lv_label_set_text(icon_ble, LV_SYMBOL_BLUETOOTH);
lv_obj_set_style_text_font(icon_ble, &lv_font_montserrat_20, 0);
lv_obj_set_style_text_color(icon_ble, COLOR_DISCONNECTED, 0); // پیش‌فرض قرمز (قطع)
lv_obj_align(icon_ble, LV_ALIGN_TOP_RIGHT, -10, 10);

// 2. ساعت دیجیتال کامل (وسط)
lbl_clock = lv_label_create(scr);
lv_label_set_text(lbl_clock, "12:59:45");
lv_obj_set_style_text_font(lbl_clock, HUD_FONT_BIG, 0);
lv_obj_set_style_text_color(lbl_clock, COLOR_ACCENT, 0);
lv_obj_align(lbl_clock, LV_ALIGN_CENTER, 0, 0);

// استایل درخشش (Glow)
lv_obj_set_style_shadow_width(lbl_clock, 20, 0);
lv_obj_set_style_shadow_color(lbl_clock, COLOR_ACCENT, 0);

// 3. طراحی پاپ‌آپ (لایه رویی)
// ایجاد یک پنل که 80% صفحه را میگیرد
panel_popup = lv_obj_create(scr);
lv_obj_set_size(panel_popup, 300, 200);
lv_obj_align(panel_popup, LV_ALIGN_CENTER, 0, 0);
lv_obj_set_style_bg_color(panel_popup, COLOR_POPUP_BG, 0);
lv_obj_set_style_border_color(panel_popup, lv_color_hex(0xFFFFFF), 0);
lv_obj_set_style_border_width(panel_popup, 2, 0);
lv_obj_set_style_radius(panel_popup, 15, 0);
lv_obj_add_flag(panel_popup, LV_OBJ_FLAG_HIDDEN); // در ابتدا مخفی است

// تنظیمات Flex برای چیدمان عمودی تمیز
lv_obj_set_flex_flow(panel_popup, LV_FLEX_FLOW_COLUMN);
lv_obj_set_flex_align(panel_popup, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

// نام اپلیکیشن (هدر کوچک)
lbl_app_name = lv_label_create(panel_popup);
lv_label_set_text(lbl_app_name, "APP");
lv_obj_set_style_text_font(lbl_app_name, HUD_FONT_SMALL, 0);
lv_obj_set_style_text_color(lbl_app_name, lv_color_hex(0xAAAAAA), 0);

// نام فرستنده (بولد و رنگی)
lbl_sender = lv_label_create(panel_popup);
lv_label_set_text(lbl_sender, "Sender");
lv_obj_set_style_text_font(lbl_sender, HUD_FONT_SMALL, 0); // اگر فونت بولد دارید اینجا بگذارید
lv_obj_set_style_text_color(lbl_sender, lv_color_hex(0xFFA500), 0); // نارنجی
lv_obj_set_style_pad_bottom(lbl_sender, 10, 0); // فاصله با متن پیام

// متن پیام (پشتیبانی از چند خط و فارسی)
lbl_message = lv_label_create(panel_popup);
lv_label_set_text(lbl_message, "Message Body");
lv_obj_set_style_text_font(lbl_message, HUD_FONT_SMALL, 0);
lv_obj_set_width(lbl_message, 260); // محدود کردن عرض برای شکستن خط
lv_label_set_long_mode(lbl_message, LV_LABEL_LONG_WRAP); // شکستن خط خودکار
lv_obj_set_style_text_align(lbl_message, LV_TEXT_ALIGN_CENTER, 0);
// تنظیم جهت متن برای فارسی (اگر LV_USE_BIDI فعال باشد خودکار است اما صریح هم میشود)
lv_obj_set_style_base_dir(lbl_message, LV_BASE_DIR_AUTO, 0);
}

// ==========================================
// SETUP & LOOP
// ==========================================
void setup() {
Serial.begin(115200);
lv_init();
lv_tick_set_cb([]() { return millis(); });

#if LV_USE_TFT_ESPI
tft.begin();
tft.setRotation(1);
tft.fillScreen(TFT_BLACK);
disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
lv_display_set_flush_cb(disp, my_disp_flush);
lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
#endif

lv_indev_t * indev = lv_indev_create();
lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
lv_indev_set_read_cb(indev, [](lv_indev_t * i, lv_indev_data_t * d){ d->state = LV_INDEV_STATE_RELEASED; });

build_ui();

// BLE Setup
BLEDevice::init("ESP32_HUD_PRO");
BLEServer *pServer = BLEDevice::createServer();
pServer->setCallbacks(new MyServerCallbacks());
BLEService *pService = pServer->createService(SERVICE_UUID);
BLECharacteristic *pCharacteristic = pService->createCharacteristic(
CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
);
pCharacteristic->setCallbacks(new MyCallbacks());
pService->start();
BLEDevice::startAdvertising();
}

void loop() {
lv_timer_handler();
delay(5);

// --- مدیریت ساعت ماک (HH:MM:SS) ---
static unsigned long lastClockTick = 0;
if (millis() - lastClockTick >= 1000) {
lastClockTick = millis();
mockSecond++;
if (mockSecond >= 60) { mockSecond = 0; mockMinute++; }
if (mockMinute >= 60) { mockMinute = 0; mockHour++; }
if (mockHour >= 24) { mockHour = 0; }

// نمایش کامل به صورت HH:MM:SS
lv_label_set_text_fmt(lbl_clock, "%02d:%02d:%02d", mockHour, mockMinute, mockSecond);
}

// --- مدیریت پاپ‌آپ (تایمر 15 ثانیه) ---
if (isPopupVisible && (millis() - popupTimer > 15000)) {
lv_obj_add_flag(panel_popup, LV_OBJ_FLAG_HIDDEN); // مخفی کردن پاپ‌آپ
isPopupVisible = false;

// پاک کردن متن‌ها برای امنیت حافظه (اختیاری)
lv_label_set_text(lbl_message, "");
}
}