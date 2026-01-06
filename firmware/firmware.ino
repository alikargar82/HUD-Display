#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <lvgl.h>
#include <WiFi.h>
#include <time.h>
#include <DHT.h>



//wifi setting
const char* ssid = "starter";
const char* password = "12345676";

// NTP Server settings
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 12605;  // Iran is UTC+3:30 = 3.5 * 3600
const int daylightOffset_sec = 0;

// --- SENSOR CONFIG ---
#define PIN_LM35 34       // ADC1 Pin (Must use ADC1 with WiFi)
#define PIN_DHT  27       // Digital Pin
#define DHT_TYPE DHT11
DHT dht(PIN_DHT, DHT_TYPE);

#if LV_USE_TFT_ESPI
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
#endif

// ===== تنظیمات فونت =====
// اگر فونت فارسی دارید، اینجا آن را جایگزین کنید.
// فعلاً از فونت‌های استاندارد بزرگ استفاده می‌کنیم.

LV_FONT_DECLARE(vazir_20);
LV_FONT_DECLARE(vazir_28);
// LV_FONT_DECLARE(vazir_48);
LV_FONT_DECLARE(vazir_56);

#define HUD_FONT_STATUS &lv_font_montserrat_20  // Original font with symbols

#define HUD_FONT_EXTRA_HUGE   &vazir_56  // برای ساعت
#define HUD_FONT_LARGE   &vazir_28  // برای ساعت
#define HUD_FONT_NORMAL   &vazir_20  // برای ساعت

// #define HUD_FONT_HUGE   &vazir_48  // برای ساعت
#define HUD_FONT_CLOCK  &vazir_56  // برای نام برنامه/فرستنده
#define HUD_FONT_VALUE  &vazir_28  // برای نام برنامه/فرستنده

// #define HUD_FONT_NORMAL &vazir_20  // برای متن پیام و استاتوس بار



LV_IMG_DECLARE(telegram);
LV_IMG_DECLARE(setting);
LV_IMG_DECLARE(message);
LV_IMG_DECLARE(phone);
LV_IMG_DECLARE(notification);
LV_IMG_DECLARE(instagram);
LV_IMG_DECLARE(whatsapp);

// ===== رنگ‌های نئونی (HUD Theme) =====
// #define COLOR_BG            lv_color_hex(0x000000) // مشکی مطلق
// #define COLOR_ACCENT        lv_color_hex(0x00FFFF) // آبی سایان
// #define COLOR_CONNECTED     lv_color_hex(   ) // سبز
// #define COLOR_DISCONNECTED  lv_color_hex(0xFF0000) // قرمز
// // *** تغییرات جدید ***
// #define COLOR_POPUP_BG      lv_color_hex(0x000000) // پاپ‌آپ هم کاملا مشکی شد
// #define COLOR_POPUP_BORDER  lv_color_hex(0xFFFFFF) // حاشیه سفید برای دیده شدن روی سیاهی
// #define COLOR_TEXT_LABEL    lv_color_hex(0xAAAAAA) // رنگ طوسی برای متن‌های ثابت (App Name:)
// #define COLOR_TEXT_VALUE    lv_color_hex(0xFFFFFF) // رنگ سفید برای مقادیر اصلی
// ===== رنگ‌ها با فرمت BGR =====
// نکته: در فرمت BGR جای دو رقم اول و آخر عوض می‌شود (BBGGRR)

#define COLOR_BG            lv_color_hex(0x000000) // مشکی (تغییر نمی‌کند)
#define COLOR_ACCENT        lv_color_hex(0xfcca03) // آبی سایان (RGB: 00FFFF -> BGR: FFFF00)
#define COLOR_CONNECTED     lv_color_hex(0x26ff00) // سبز (RGB: 00FF00 -> BGR: 00FF00 - چون قرمز و آبی هر دو صفرند)
#define COLOR_DISCONNECTED  lv_color_hex(0xff0000) // قرمز (RGB: FF0000 -> BGR: 0000FF) 

// *** رنگ‌های پاپ‌آپ ***
#define COLOR_POPUP_BG      lv_color_hex(0x000000) // مشکی
#define COLOR_POPUP_BORDER  lv_color_hex(0xFFFFFF) // سفید (تغییر نمی‌کند چون همه F هستند)
#define COLOR_TEXT_LABEL    lv_color_hex(0xAAAAAA) // طوسی (تغییر نمی‌کند چون همه برابرند)
#define COLOR_TEXT_VALUE    lv_color_hex(0xFFFFFF) // سفید

#define COLOR_TEMP lv_color_hex(0xFF4500) // Orange
#define COLOR_HUM  lv_color_hex(0x00BFFF) // Blue

// ===== تنظیمات BLE =====
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-90ab-cdef-1234567890ab"

// ===== بافر نمایشگر =====
#define TFT_HOR_RES   480
#define TFT_VER_RES   320
#define DRAW_BUF_SIZE (TFT_HOR_RES * 20)
uint32_t draw_buf[DRAW_BUF_SIZE / 4];
// uint32_t* draw_buf = nullptr;

lv_display_t * disp;


// Function to detect if text contains Persian/Arabic characters
bool isPersianText(const char* text) {
    while (*text) {
        unsigned char c = *text;
        // Check for UTF-8 Persian/Arabic range (0x0600-0x06FF)
        if (c == 0xD8 || c == 0xD9 || c == 0xDA || c == 0xDB) {
            return true;
        }
        text++;
    }
    return false;
}


// Function to get icon based on app name
const void* getAppIcon(String appName) {
    appName.toLowerCase();  // Convert to lowercase for matching
    
    // Map app names to icons
    if (appName.indexOf("telegram") >= 0) return &telegram;
    if (appName.indexOf("whatsapp") >= 0) return &whatsapp;
    if (appName.indexOf("instagram") >= 0) return &instagram;
    if (appName.indexOf("messages") >= 0) return &message;
    if (appName.indexOf("phone") >= 0) return &phone;
    if (appName.indexOf("android system") >= 0 || appName.indexOf("settings") >= 0) return &setting;

    return &notification;
}



// ===== آبجکت‌های UI =====
lv_obj_t * lbl_clock;       
lv_obj_t * lbl_ble_status;  // متن/آیکون وضعیت بلوتوث

// المان‌های پاپ‌آپ
lv_obj_t * panel_popup;     
lv_obj_t * lbl_app_name;    
lv_obj_t * lbl_sender;      
lv_obj_t * lbl_message;     
lv_obj_t * bar_timer; // متغیر سراسری جدید برای پراگرس بار
lv_obj_t * img_app_icon;

// UI Objects for Sensors
lv_obj_t * lbl_temp_val;
lv_obj_t * lbl_hum_val;
lv_obj_t * arc_temp;  // Circular gauge for temp
lv_obj_t * bar_hum;   // Horizontal bar for humidity


volatile bool doUpdateBLE = false; // Flag to trigger UI update
volatile bool bleStatusToDisplay = false; // State to show (Connected/Disconnected)

volatile bool doShowNotification = false; // Flag
String pendingNotificationMessage = "";   // Buffer to hold the message

volatile bool doToggleMirror = false;
volatile bool targetMirrorState = false;

// ===== متغیرهای وضعیت =====
bool mirrorEnabled = false; // فعال بودن حالت آینه‌ای HUD
bool bleConnected = false;
unsigned long popupTimer = 0;
bool isPopupVisible = false;

// متغیرهای ساعت
int mockHour = 13;
int mockMinute = 00;
int mockSecond = 00;

bool syncTimeFromNTP() {
    struct tm timeinfo;
    // Try to get time with a 500ms timeout per attempt
    // We don't need a long timeout because we check repeatedly
    if(!getLocalTime(&timeinfo, 500)){
        return false;
    }
    
    // Check if the time is valid (year > 2020)
    // tm_year is years since 1900, so 120 = 2020
    if (timeinfo.tm_year < 124) {
        return false;
    }

    // Update your clock variables
    mockHour = timeinfo.tm_hour;
    mockMinute = timeinfo.tm_min;
    mockSecond = timeinfo.tm_sec;
    
    Serial.println("Time successfully synced from NTP!");
    return true;
}


// ==========================================
//               توابع کمکی UI
// ==========================================

// تابع ایجاد خط جداکننده افقی
lv_obj_t* create_divider(lv_obj_t* parent) {
    lv_obj_t* line = lv_obj_create(parent);
    lv_obj_set_size(line, LV_PCT(100), 3); // ارتفاع 2 پیکسل
    lv_obj_set_style_bg_color(line, COLOR_ACCENT, 0);
    lv_obj_set_style_border_width(line, 0, 0);
    return line;
}

// تابع به‌روزرسانی وضعیت بلوتوث
void update_ble_ui(bool connected) {
    if(connected) {
        // سبز + متن Connected
        lv_label_set_text(lbl_ble_status, LV_SYMBOL_BLUETOOTH " BLE: ON");
        lv_obj_set_style_text_color(lbl_ble_status, COLOR_CONNECTED, 0);
        // اضافه کردن سایه سبز (اختیاری برای زیبایی)
        lv_obj_set_style_shadow_width(lbl_ble_status, 15, 0);
        lv_obj_set_style_shadow_color(lbl_ble_status, COLOR_CONNECTED, 0);
    } else {
        // قرمز + متن Disconnected
        lv_label_set_text(lbl_ble_status, LV_SYMBOL_CLOSE " BLE: OFF");
        lv_obj_set_style_text_color(lbl_ble_status, COLOR_DISCONNECTED, 0);
        lv_obj_set_style_shadow_width(lbl_ble_status, 0, 0);
    }
}

// ==========================================
//               لاجیک پارس کردن پیام
// ==========================================
void show_notification(String fullMsg) {
    String appName = "NOTIFICATION";
    String sender = "System";
    String msgBody = fullMsg;

    int firstPipe = fullMsg.indexOf('|');
    if (firstPipe != -1) {
        appName = fullMsg.substring(0, firstPipe);
        int secondPipe = fullMsg.indexOf('|', firstPipe + 1);
        if (secondPipe != -1) {
            sender = fullMsg.substring(firstPipe + 1, secondPipe);
            msgBody = fullMsg.substring(secondPipe + 1);
        } else {
            msgBody = fullMsg.substring(firstPipe + 1);
        }
    }

    const void* icon = getAppIcon(appName);
    lv_img_set_src(img_app_icon, icon);

    // Set text direction based on content
    if (isPersianText(appName.c_str())) {
        lv_obj_set_style_base_dir(lbl_app_name, LV_BASE_DIR_RTL, 0);
    } else {
        lv_obj_set_style_base_dir(lbl_app_name, LV_BASE_DIR_LTR, 0);
    }
    
    if (isPersianText(sender.c_str())) {
        lv_obj_set_style_base_dir(lbl_sender, LV_BASE_DIR_RTL, 0);
    } else {
        lv_obj_set_style_base_dir(lbl_sender, LV_BASE_DIR_LTR, 0);
    }
    
    if (isPersianText(msgBody.c_str())) {
        lv_obj_set_style_base_dir(lbl_message, LV_BASE_DIR_RTL, 0);
    } else {
        lv_obj_set_style_base_dir(lbl_message, LV_BASE_DIR_LTR, 0);
    }

    // Set text
    lv_label_set_text(lbl_app_name, appName.c_str());
    lv_label_set_text(lbl_sender, sender.c_str());
    lv_label_set_text(lbl_message, msgBody.c_str());

    // Show popup
    lv_obj_clear_flag(panel_popup, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(panel_popup);

    isPopupVisible = true;
    popupTimer = millis();
    lv_bar_set_value(bar_timer, 100, LV_ANIM_OFF);
}


// ==========================================
//               مدیریت BLE
// ==========================================
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    bleConnected = true;
    bleStatusToDisplay = true; // Save the state we want
    doUpdateBLE = true;        // Tell the main loop to update UI
  }
  void onDisconnect(BLEServer* pServer) override {
    bleConnected = false;
    bleStatusToDisplay = false;
    doUpdateBLE = true;        // Tell the main loop to update UI
    pServer->getAdvertising()->start();
  }
};


class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) override {
        String value = pCharacteristic->getValue(); 
        if (value.length() > 0) {
            // Check for mirror mode commands
            if (value == "|||enable-mirror-mode|||") {
                targetMirrorState = true;
                doToggleMirror = true;
                Serial.println("Mirror mode enable requested");
            }
            else if (value == "|||disable-mirror-mode|||") {
                targetMirrorState = false;
                doToggleMirror = true;
                Serial.println("Mirror mode disable requested");
            }
            else {
                // Regular notification
                pendingNotificationMessage = value;
                doShowNotification = true;
            }
        }
    }
};



// ==========================================
//               UI Design (Main)
// ==========================================
// ==========================================
//               UI Design (Main)
// ==========================================
void build_ui() {
      lv_obj_t * scr = lv_screen_active();
    // 1. Background: Pitch Black or Very Dark Grey
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0); 

    // --- HEADER (Clock + BLE) ---
    // Clock - Huge Font
    lbl_clock = lv_label_create(scr);
    lv_label_set_text_fmt(lbl_clock, "%02d:%02d:%02d", mockHour, mockMinute, mockSecond);
    lv_obj_set_style_text_font(lbl_clock, HUD_FONT_CLOCK, 0); 
    lv_obj_set_style_text_color(lbl_clock, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(lbl_clock, LV_ALIGN_TOP_LEFT, 20, 10);

    // BLE Status
    lbl_ble_status = lv_label_create(scr);
    lv_obj_set_style_text_font(lbl_ble_status, HUD_FONT_STATUS, 0);
    lv_obj_align(lbl_ble_status, LV_ALIGN_TOP_RIGHT, -20, 30); 
    update_ble_ui(false);

    // --- MAIN CONTAINER ---
    lv_obj_t * main_cont = lv_obj_create(scr);
    lv_obj_set_size(main_cont, LV_PCT(100), 220);
    lv_obj_align(main_cont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_opa(main_cont, LV_OPA_TRANSP, 0); 
    lv_obj_set_style_border_width(main_cont, 0, 0);
    lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(main_cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_SCROLLABLE); // <--- ADD THIS

    // === TEMPERATURE CARD (Transparent + Border) ===
    lv_obj_t * card_temp = lv_obj_create(main_cont);
    lv_obj_set_size(card_temp, 200, 190);
    
    // Transparent BG
    lv_obj_set_style_bg_opa(card_temp, LV_OPA_TRANSP, 0); 
    
    // Add Border (Pastel Red/Orange to match data)
    lv_obj_set_style_border_color(card_temp, lv_color_hex(0xff6f00), 0);
    lv_obj_set_style_border_width(card_temp, 2, 0);
    lv_obj_set_style_radius(card_temp, 20, 0);
    
    lv_obj_clear_flag(card_temp, LV_OBJ_FLAG_SCROLLABLE);

    // Circular Arc
    arc_temp = lv_arc_create(card_temp);
    lv_obj_set_size(arc_temp, 140, 140);
    lv_arc_set_rotation(arc_temp, 135);
    lv_arc_set_bg_angles(arc_temp, 0, 270);
    lv_arc_set_value(arc_temp, 0);
    lv_arc_set_range(arc_temp, 0, 60); 
    lv_obj_center(arc_temp);
    
    // Arc Style
    lv_obj_set_style_arc_width(arc_temp, 12, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_temp, lv_color_hex(0xff8c00), LV_PART_INDICATOR); 
    lv_obj_set_style_arc_rounded(arc_temp, true, LV_PART_INDICATOR);

    // Arc Background (Dark Grey to see it on black)
    lv_obj_set_style_arc_width(arc_temp, 12, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_temp, lv_color_hex(0x4c3259), LV_PART_MAIN);
    lv_obj_remove_style(arc_temp, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc_temp, LV_OBJ_FLAG_CLICKABLE);

    // Temp Value
    lbl_temp_val = lv_label_create(card_temp);
    lv_label_set_text(lbl_temp_val, "--°");
    lv_obj_set_style_text_font(lbl_temp_val, HUD_FONT_EXTRA_HUGE, 0); 
    lv_obj_set_style_text_color(lbl_temp_val, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(lbl_temp_val);
    

    // Label "Temp"
    lv_obj_t * lbl_t_title = lv_label_create(card_temp);
    lv_label_set_text(lbl_t_title, "Temp");
    lv_obj_set_style_text_font(lbl_t_title, &vazir_20, 0); 
    lv_obj_set_style_text_color(lbl_t_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(lbl_t_title, LV_ALIGN_BOTTOM_MID, 0, -5);


    // === HUMIDITY CARD (Transparent + Border) ===
    lv_obj_t * card_hum = lv_obj_create(main_cont);
    lv_obj_set_size(card_hum, 200, 190);
    
    // Transparent BG
    lv_obj_set_style_bg_opa(card_hum, LV_OPA_TRANSP, 0);
    
    // Add Border (Pastel Mint to match data)
    lv_obj_set_style_border_color(card_hum, lv_color_hex(0xB5EAD7), 0);
    lv_obj_set_style_border_width(card_hum, 2, 0);
    lv_obj_set_style_radius(card_hum, 20, 0);
    
    lv_obj_clear_flag(card_hum, LV_OBJ_FLAG_SCROLLABLE);

    // Title
    lv_obj_t * lbl_h_title = lv_label_create(card_hum);
    lv_label_set_text(lbl_h_title, "Humidity");
    lv_obj_set_style_text_font(lbl_h_title, HUD_FONT_VALUE, 0);
    lv_obj_set_style_text_color(lbl_h_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(lbl_h_title, LV_ALIGN_TOP_LEFT, 10, 5);

    // Value Text
    lbl_hum_val = lv_label_create(card_hum);
    lv_label_set_text(lbl_hum_val, "--%");
    lv_obj_set_style_text_font(lbl_hum_val, HUD_FONT_EXTRA_HUGE, 0);
    lv_obj_set_style_text_color(lbl_hum_val, lv_color_hex(0xB5EAD7), 0);
    lv_obj_align(lbl_hum_val, LV_ALIGN_LEFT_MID, 10, -5);

    // Horizontal Bar
    bar_hum = lv_bar_create(card_hum);
    lv_obj_set_size(bar_hum, 160, 15);
    lv_obj_align(bar_hum, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_bar_set_value(bar_hum, 0, LV_ANIM_ON);
    
    // Bar Style
    lv_obj_set_style_bg_color(bar_hum, lv_color_hex(0xB5EAD7), LV_PART_INDICATOR);
    lv_obj_set_style_bg_grad_color(bar_hum, lv_color_hex(0x8FD3FE), LV_PART_INDICATOR);
    lv_obj_set_style_bg_grad_dir(bar_hum, LV_GRAD_DIR_HOR, LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar_hum, 8, LV_PART_INDICATOR);
    
    // Bar Background
    lv_obj_set_style_bg_color(bar_hum, lv_color_hex(0x4c3259), LV_PART_MAIN);


    // 3. طراحی پاپ‌آپ
    panel_popup = lv_obj_create(scr);
    lv_obj_set_size(panel_popup, TFT_HOR_RES - 10, TFT_VER_RES - 10);
    lv_obj_set_style_bg_color(panel_popup, lv_color_hex(0x000000), 0);
    lv_obj_align(panel_popup, LV_ALIGN_CENTER, 0, 0); 
    lv_obj_set_style_bg_color(panel_popup, COLOR_POPUP_BG, 0); 
    lv_obj_set_style_border_color(panel_popup, COLOR_POPUP_BORDER, 0); 
    lv_obj_set_style_border_width(panel_popup, 2, 0);
    lv_obj_set_style_radius(panel_popup, 12, 0);
    lv_obj_set_style_pad_all(panel_popup, 15, 0);
    lv_obj_set_flex_flow(panel_popup, LV_FLEX_FLOW_COLUMN);

    // --- سطر اول: APP with ICON ---
    lv_obj_t* row_app = lv_obj_create(panel_popup);
    lv_obj_set_size(row_app, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row_app, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row_app, 0, 0);
    lv_obj_set_style_pad_all(row_app, 0, 0);
    lv_obj_set_flex_flow(row_app, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row_app, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // "APP :" label
    lv_obj_t* lbl_static_app = lv_label_create(row_app);
    lv_label_set_text(lbl_static_app, "APP : ");
    lv_obj_set_style_text_font(lbl_static_app, HUD_FONT_LARGE, 0);
    lv_obj_set_style_text_color(lbl_static_app, COLOR_ACCENT, 0);

    // App icon (NEW!)
    img_app_icon = lv_img_create(row_app); 
    lv_img_set_src(img_app_icon, &notification);  // Default icon initially
    lv_obj_set_style_pad_right(img_app_icon, 10, 0);  // Space between icon and text

    // App name
    lbl_app_name = lv_label_create(row_app);
    lv_label_set_text(lbl_app_name, "Telegram");
    lv_obj_set_style_text_font(lbl_app_name, HUD_FONT_LARGE, 0);
    lv_obj_set_style_text_color(lbl_app_name, COLOR_TEXT_VALUE, 0);
    lv_obj_set_style_base_dir(lbl_app_name, LV_BASE_DIR_AUTO, 0);

    create_divider(panel_popup);

    // --- سطر دوم: SUBJECT ---
    lv_obj_t* row_sender = lv_obj_create(panel_popup);
    lv_obj_set_size(row_sender, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row_sender, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row_sender, 0, 0);
    lv_obj_set_style_pad_all(row_sender, 0, 0);
    lv_obj_set_flex_flow(row_sender, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row_sender, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* lbl_static_sender = lv_label_create(row_sender);
    lv_label_set_text(lbl_static_sender, "SUBJECT : ");
    lv_obj_set_style_text_font(lbl_static_sender, HUD_FONT_LARGE, 0);
    lv_obj_set_style_text_color(lbl_static_sender, COLOR_ACCENT, 0);

    lbl_sender = lv_label_create(row_sender);
    lv_label_set_text(lbl_sender, "Alert");
    lv_obj_set_style_text_font(lbl_sender, HUD_FONT_LARGE, 0);
    lv_obj_set_style_text_color(lbl_sender, COLOR_TEXT_VALUE, 0);
    lv_obj_set_style_base_dir(lbl_sender, LV_BASE_DIR_AUTO, 0);

    create_divider(panel_popup);

    // --- سطر سوم: TEXT (اصلاح شده: در یک خط) ---
    lv_obj_t* row_msg = lv_obj_create(panel_popup);
    lv_obj_set_size(row_msg, LV_PCT(100), LV_SIZE_CONTENT); // ارتفاع محتوا
    lv_obj_set_style_bg_opa(row_msg, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row_msg, 0, 0);
    lv_obj_set_style_pad_all(row_msg, 0, 0);
    lv_obj_set_flex_flow(row_msg, LV_FLEX_FLOW_ROW); // چیدمان افقی
    // تراز بالا (Start) تا اگر متن چند خط شد، لیبل ثابت بالا بماند
    lv_obj_set_flex_align(row_msg, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // لیبل ثابت
    lv_obj_t* lbl_static_msg = lv_label_create(row_msg);
    lv_label_set_text(lbl_static_msg, "TEXT : ");
    lv_obj_set_style_text_font(lbl_static_msg, HUD_FONT_LARGE, 0);
    lv_obj_set_style_text_color(lbl_static_msg, COLOR_ACCENT, 0);
    // مانع از فشرده شدن لیبل ثابت می‌شویم
    lv_obj_set_flex_grow(lbl_static_msg, 0); 

    // متن متغیر
    lbl_message = lv_label_create(row_msg);
    lv_label_set_text(lbl_message, "Waiting...");
    lv_obj_set_style_text_font(lbl_message, HUD_FONT_NORMAL, 0);
    lv_obj_set_style_text_color(lbl_message, COLOR_TEXT_VALUE, 0);
    lv_label_set_long_mode(lbl_message, LV_LABEL_LONG_WRAP); // شکستن خط
    lv_obj_set_width(lbl_message, lv_pct(100)); // and let the flex container constrain it
    lv_obj_set_flex_grow(lbl_message, 1);
    lv_obj_set_style_base_dir(lbl_message, LV_BASE_DIR_AUTO, 0);
    // Add this after creating lbl_message
    lv_obj_set_style_pad_right(lbl_message, 5, 0); 
    lv_obj_set_style_pad_left(lbl_message, 5, 0);

    // --- پراگرس بار (پایین پاپ‌آپ) ---
    // فضای خالی برای هل دادن بار به پایین
    lv_obj_t* spacer = lv_obj_create(panel_popup);
    lv_obj_set_flex_grow(spacer, 1); // پر کردن فضای باقی‌مانده
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(spacer, 0, 0);

    bar_timer = lv_bar_create(panel_popup);
    lv_obj_set_size(bar_timer, LV_PCT(100), 10); // ارتفاع 5 پیکسل
    lv_bar_set_value(bar_timer, 100, LV_ANIM_OFF); // پر بودن اولیه
    lv_obj_set_style_bg_color(bar_timer, lv_color_hex(0x333333), LV_PART_MAIN); // رنگ پس‌زمینه بار
    lv_obj_set_style_bg_color(bar_timer, COLOR_ACCENT, LV_PART_INDICATOR); // رنگ خود بار (آبی/زرد)

    lv_obj_add_flag(panel_popup, LV_OBJ_FLAG_HIDDEN);
}


// ==========================================
//               FLUSH & SETUP
// ==========================================
void my_disp_flush(lv_display_t * disp_drv, const lv_area_t * area, uint8_t * px_map) {
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

#if LV_USE_TFT_ESPI
    tft.startWrite();

    if (!mirrorEnabled) {
        // حالت عادی: همان‌جا که LVGL می‌گوید چاپ کن
        tft.setAddrWindow(area->x1, area->y1, w, h);
        tft.pushPixels((uint16_t *)px_map, w * h);
    } else {
        // === حالت آینه‌ای (HUD) ===
        
        // 1. محاسبه مختصات قرینه شده
        // در حالت میرور، سمت راستِ محدوده فعلی (x2) می‌شود سمت چپِ محدوده فیزیکی روی ال‌سی‌دی
        int32_t mirrored_x1 = TFT_HOR_RES - 1 - area->x2;

        // تنظیم پنجره در مختصات جدید (قرینه)
        tft.setAddrWindow(mirrored_x1, area->y1, w, h);

        // بافر استاتیک برای سرعت بالا (جلوگیری از پر شدن رم)
        static uint16_t line_buf[TFT_HOR_RES]; 
        uint16_t * pixels = (uint16_t *)px_map;

        for (uint32_t y = 0; y < h; y++) {
            uint32_t row_offset = y * w;
            
            // 2. معکوس کردن چیدمان پیکسل‌های هر خط
            for (uint32_t x = 0; x < w; x++) {
                line_buf[x] = pixels[row_offset + (w - 1 - x)];
            }
            
            // ارسال به مختصات قرینه شده
            tft.pushPixels(line_buf, w);
        }
    }
    tft.endWrite();
#endif
    lv_display_flush_ready(disp_drv);
}



void setup() {
    Serial.begin(115200);

    // draw_buf = (uint32_t*)malloc(sizeof(uint32_t) * (DRAW_BUF_SIZE / 4));

    // // Check if allocation succeeded
    // if (draw_buf == NULL) {
    //     while(1) { delay(1000); }
    // }
        
    // Connect to WiFi
    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.println(" CONNECTED");

    // --- Hardware Init ---
    analogReadResolution(12);
    analogSetPinAttenuation(PIN_LM35, ADC_2_5db);
    pinMode(PIN_LM35, INPUT);
    dht.begin();
    // Initialize NTP
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
 
    // === NEW: Force Sync Loop ===
    Serial.print("Syncing time");
    int retry = 0;
    while (!syncTimeFromNTP() && retry < 20) { // Try for 10 seconds (20 * 500ms)
        Serial.print(".");
        delay(100);
        retry++;
    }
    Serial.println();
    
    if (retry >= 20) {
        Serial.println("NTP Sync Failed - Using Mock Time");
    }    
    delay(2000); // Give it time to sync
    
    // 2. DISCONNECT WiFi to free up Radio for BLE
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("WiFi OFF - Starting BLE");

    lv_init();
    lv_tick_set_cb([]() { return millis(); });

#if LV_USE_TFT_ESPI
    tft.begin();
    tft.setRotation(1); // چرخش افقی
    tft.setSwapBytes(true); 
    tft.fillScreen(TFT_BLACK);
    
    disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
#endif
    
    // درایور ورودی (برای جلوگیری از کرش احتمالی، حتی اگر تاچ ندارید)
    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, [](lv_indev_t * i, lv_indev_data_t * d){ d->state = LV_INDEV_STATE_RELEASED; });

    build_ui();
   // راه‌اندازی BLE
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


    // --- SAFE UI UPDATE ---
    if (doUpdateBLE) {
        update_ble_ui(bleStatusToDisplay); // Call LVGL function safely here
        doUpdateBLE = false; // Reset flag
    }

    // --- MIRROR MODE TOGGLE (NEW) ---
    if (doToggleMirror) {
        mirrorEnabled = targetMirrorState;
        doToggleMirror = false;
        
        // Force full screen redraw to apply mirroring
        lv_obj_invalidate(lv_screen_active());

    }

    if (doShowNotification) {
        // Now it is safe to call LVGL functions
        show_notification(pendingNotificationMessage); 
        doShowNotification = false; 
    }

    // --- مدیریت ساعت (بروزرسانی هر ثانیه) ---
    static unsigned long lastClockTick = 0;
    if (millis() - lastClockTick >= 1000) {
        lastClockTick = millis();
        mockSecond++;
        if (mockSecond >= 60) { 
            mockSecond = 0; 
            mockMinute++; 
        }
        if (mockMinute >= 60) { 
            mockMinute = 0; 
            mockHour++; 
        }
        if (mockHour >= 24) { 
            mockHour = 0; 
        }

        lv_label_set_text_fmt(lbl_clock, "%02d:%02d:%02d", mockHour, mockMinute, mockSecond);
    }

    // --- SENSOR UPDATE (Every 2 seconds) ---
    static unsigned long lastSensorRead = 0;
    if (millis() - lastSensorRead >= 2000) {
        lastSensorRead = millis();

        // 1. LM35 FIX: Use analogReadMilliVolts
        long sum = 0;
        for(int i=0; i<10; i++) { 
            sum += analogReadMilliVolts(PIN_LM35); 
            delay(5); 
        }
        float avgMv = sum / 10.0;
        // LM35 Formula: 10mV = 1 Degree Celsius
        int tempC = (int)(avgMv / 10.0);

        // 2. DHT11
        float h = dht.readHumidity();
         
        // --- UPDATE UI ---
        
        // Update Circular Gauge
        lv_arc_set_value(arc_temp, tempC);
        lv_label_set_text_fmt(lbl_temp_val, "%d°", tempC);

        // Update Humidity Bar
        if (!isnan(h)) {
            lv_bar_set_value(bar_hum, (int)h, LV_ANIM_ON);
            lv_label_set_text_fmt(lbl_hum_val, "%d%%", (int)h); 
        } else {
            lv_label_set_text(lbl_hum_val, "--%");
        } 
     
    
        }

    // --- مدیریت بسته شدن پاپ‌آپ (تایمر ۱۵ ثانیه) ---
    // --- مدیریت پاپ‌آپ و پراگرس بار ---
    if (isPopupVisible) {
        unsigned long elapsed = millis() - popupTimer;
        if (elapsed > 15000) {
            lv_obj_add_flag(panel_popup, LV_OBJ_FLAG_HIDDEN);
            isPopupVisible = false;
        } else {
            // محاسبه درصد باقی‌مانده (15000 میلی‌ثانیه کل زمان است)
            // 100 - (elapsed * 100 / 15000)
            int32_t percent = 100 - (elapsed * 100 / 15000);
            if (percent < 0) percent = 0;
            lv_bar_set_value(bar_timer, percent, LV_ANIM_OFF);
        }
    }

}
