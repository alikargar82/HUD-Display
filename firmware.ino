//#include <lvgl.h>
//
//#if LV_USE_TFT_ESPI
//#include <TFT_eSPI.h>
//TFT_eSPI tft = TFT_eSPI(); // Create TFT instance
//#endif
//
//#define TFT_HOR_RES   320
//#define TFT_VER_RES   480
//#define TFT_ROTATION  LV_DISPLAY_ROTATION_0
//
//#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
//uint32_t draw_buf[DRAW_BUF_SIZE / 4];
//
//lv_display_t * disp;
//bool mirrorEnabled = true; // set true for mirrored mode
//
//#if LV_USE_LOG != 0
//void my_print(lv_log_level_t level, const char * buf) {
//    LV_UNUSED(level);
//    Serial.println(buf);
//    Serial.flush();
//}
//#endif
//
///* Custom flush function that mirrors horizontally */
//void my_disp_flush(lv_display_t *disp_drv, const lv_area_t *area, uint8_t * px_map) {
//    uint32_t w = lv_area_get_width(area);
//    uint32_t h = lv_area_get_height(area);
//    
//#if LV_USE_TFT_ESPI
//    tft.startWrite();
//    
//    if (!mirrorEnabled) {
//        // Normal mode - fast rendering
//        tft.setAddrWindow(area->x1, area->y1, w, h);
//        tft.pushPixels((uint16_t *)px_map, w * h);
//    } else {
//        // Mirrored mode - flip horizontally
//        tft.setAddrWindow(area->x1, area->y1, w, h);
//        
//        uint16_t *pixels = (uint16_t *)px_map;
//        
//        // Process each row
//        for (uint32_t y = 0; y < h; y++) {
//            uint32_t row_offset = y * w;
//            
//            // Push pixels in reverse order for this row
//            for (int32_t x = w - 1; x >= 0; x--) {
//                tft.pushPixels(&pixels[row_offset + x], 1);
//            }
//        }
//    }
//    
//    tft.endWrite();
//#endif
//    
//    lv_display_flush_ready(disp_drv);
//}
//
///*Read the touchpad*/
//void my_touchpad_read(lv_indev_t * indev, lv_indev_data_t * data) {
//    // Touchpad implementation if needed
//    data->state = LV_INDEV_STATE_RELEASED;
//}
//
//static uint32_t my_tick(void) {
//    return millis();
//}
//
//void setup() {
//    Serial.begin(115200);
//    Serial.println("Starting LVGL with Mirror Support");
//    
//    lv_init();
//    lv_tick_set_cb(my_tick);
//    
//#if LV_USE_LOG != 0
//    lv_log_register_print_cb(my_print);
//#endif
//
//#if LV_USE_TFT_ESPI
//    // Initialize TFT
//    tft.begin();
//    tft.setRotation(1); // Adjust rotation as needed (0, 1, 2, or 3)
//    tft.fillScreen(TFT_BLACK);
//    
//    // Create LVGL display
//    disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
//    lv_display_set_flush_cb(disp, my_disp_flush);
//    lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
//#else
//    disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
//    lv_display_set_flush_cb(disp, my_disp_flush);
//    lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
//#endif
//
//    // Create input device
//    lv_indev_t * indev = lv_indev_create();
//    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
//    lv_indev_set_read_cb(indev, my_touchpad_read);
//    
//    // Create UI elements
//    lv_obj_t *label = lv_label_create(lv_screen_active());
//    lv_label_set_text(label, "HUD Mirror Test\nMirror: ON");
//    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
//    
//    Serial.println("Setup complete");
//    Serial.print("Mirror mode: ");
//    Serial.println(mirrorEnabled ? "ENABLED" : "DISABLED");
//}
//
//void loop() {
//    lv_timer_handler();
//    delay(5);
//}
//

/*

 Example sketch for TFT_eSPI library.

 No fonts are needed.
 
 Draws a 3d rotating cube on the TFT screen.
 
 Original code was found at http://forum.freetronics.com/viewtopic.php?f=37&t=5495
 
 */

///***********************************************************************************************************************************/
//// line segments to draw a cube. basically p0 to p1. p1 to p2. p2 to p3 so on.
//void cube(void)
//{
//  // Front Face.
//
//  Lines[0].p0.x = -50;
//  Lines[0].p0.y = -50;
//  Lines[0].p0.z = 50;
//  Lines[0].p1.x = 50;
//  Lines[0].p1.y = -50;
//  Lines[0].p1.z = 50;
//
//  Lines[1].p0.x = 50;
//  Lines[1].p0.y = -50;
//  Lines[1].p0.z = 50;
//  Lines[1].p1.x = 50;
//  Lines[1].p1.y = 50;
//  Lines[1].p1.z = 50;
//
//  Lines[2].p0.x = 50;
//  Lines[2].p0.y = 50;
//  Lines[2].p0.z = 50;
//  Lines[2].p1.x = -50;
//  Lines[2].p1.y = 50;
//  Lines[2].p1.z = 50;
//
//  Lines[3].p0.x = -50;
//  Lines[3].p0.y = 50;
//  Lines[3].p0.z = 50;
//  Lines[3].p1.x = -50;
//  Lines[3].p1.y = -50;
//  Lines[3].p1.z = 50;
//
//
//  //back face.
//
//  Lines[4].p0.x = -50;
//  Lines[4].p0.y = -50;
//  Lines[4].p0.z = -50;
//  Lines[4].p1.x = 50;
//  Lines[4].p1.y = -50;
//  Lines[4].p1.z = -50;
//
//  Lines[5].p0.x = 50;
//  Lines[5].p0.y = -50;
//  Lines[5].p0.z = -50;
//  Lines[5].p1.x = 50;
//  Lines[5].p1.y = 50;
//  Lines[5].p1.z = -50;
//
//  Lines[6].p0.x = 50;
//  Lines[6].p0.y = 50;
//  Lines[6].p0.z = -50;
//  Lines[6].p1.x = -50;
//  Lines[6].p1.y = 50;
//  Lines[6].p1.z = -50;
//
//  Lines[7].p0.x = -50;
//  Lines[7].p0.y = 50;
//  Lines[7].p0.z = -50;
//  Lines[7].p1.x = -50;
//  Lines[7].p1.y = -50;
//  Lines[7].p1.z = -50;
//
//
//  // now the 4 edge lines.
//
//  Lines[8].p0.x = -50;
//  Lines[8].p0.y = -50;
//  Lines[8].p0.z = 50;
//  Lines[8].p1.x = -50;
//  Lines[8].p1.y = -50;
//  Lines[8].p1.z = -50;
//
//  Lines[9].p0.x = 50;
//  Lines[9].p0.y = -50;
//  Lines[9].p0.z = 50;
//  Lines[9].p1.x = 50;
//  Lines[9].p1.y = -50;
//  Lines[9].p1.z = -50;
//
//  Lines[10].p0.x = -50;
//  Lines[10].p0.y = 50;
//  Lines[10].p0.z = 50;
//  Lines[10].p1.x = -50;
//  Lines[10].p1.y = 50;
//  Lines[10].p1.z = -50;
//
//  Lines[11].p0.x = 50;
//  Lines[11].p0.y = 50;
//  Lines[11].p0.z = 50;
//  Lines[11].p1.x = 50;
//  Lines[11].p1.y = 50;
//  Lines[11].p1.z = -50;
//
//  LinestoRender = 12;
//  OldLinestoRender = LinestoRender;
//
//}

//
//#include <BLEDevice.h>
//#include <BLEUtils.h>
//#include <BLEServer.h>
//
//#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
//#define CHARACTERISTIC_UUID "abcd1234-5678-90ab-cdef-1234567890ab"
//
//void setup() {
//  Serial.begin(115200);
//  Serial.println("Starting BLE test...");
//
//  // Give the ESP32 a visible BLE name
//  BLEDevice::init("ESP32_TestBLE");
//
//  // Create a BLE server
//  BLEServer *pServer = BLEDevice::createServer();
//
//  // Create a BLE service
//  BLEService *pService = pServer->createService(SERVICE_UUID);
//
//  // Create a BLE characteristic (does nothing yet, just needs to exist)
//  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
//      CHARACTERISTIC_UUID,
//      BLECharacteristic::PROPERTY_READ |
//      BLECharacteristic::PROPERTY_WRITE
//  );
//
//  // Optional: give it some initial value so you see something in nRF Connect
//  pCharacteristic->setValue("Hello BLE");
//
//  // Start the service
//  pService->start();
//
//  // Start BLE advertising
//  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
//  pAdvertising->addServiceUUID(SERVICE_UUID);
//  pAdvertising->setScanResponse(true);
//  pAdvertising->start();
//
//  Serial.println("BLE device is now advertising!");
//}
//
//void loop() {
//  // Nothing needed here
//  delay(2000);
//}
//
//#include <TFT_eSPI.h>
//#include <BLEDevice.h>
//#include <BLEUtils.h>
//#include <BLEServer.h>
//#include <lvgl.h>
//
//// ---------------------------------------------------------------------------
//// BLE UUIDs
//// ---------------------------------------------------------------------------
//#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
//#define CHARACTERISTIC_UUID "abcd1234-5678-90ab-cdef-1234567890ab"
//
//// ---------------------------------------------------------------------------
//// Screen size
//// ---------------------------------------------------------------------------
//#define SCREEN_WIDTH  320
//#define SCREEN_HEIGHT 480
//
//TFT_eSPI tft = TFT_eSPI();
//
//// ---------------------------------------------------------------------------
//// LVGL objects
//// ---------------------------------------------------------------------------
//static lv_obj_t *ble_label;
//static lv_draw_buf_t *draw_buf;          
//static lv_display_t *disp;
//
//// ---------------------------------------------------------------------------
//// LVGL FLUSH CALLBACK
//// ---------------------------------------------------------------------------
//static void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
//{
//    uint32_t w = lv_area_get_width(area);
//    uint32_t h = lv_area_get_height(area);
//
//    tft.startWrite();
//    tft.setAddrWindow(area->x1, area->y1, w, h);
//    tft.pushPixelsDMA((uint16_t *)px_map, w * h);
//    tft.endWrite();
//
//    lv_display_flush_ready(disp);   // tell LVGL the area is flushed
//}
//
//// ---------------------------------------------------------------------------
//// DISPLAY INITIALISATION (LVGL 9)
//// ---------------------------------------------------------------------------
//void init_display()
//{
//    tft.begin();
//    tft.setRotation(0);
//    tft.fillScreen(TFT_BLACK);
//
//    // 1. Create the display
//    disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
//
//    // 2. Set colour format (TFT_eSPI uses 16-bit RGB565)
//    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
//
//    // 3. Allocate a draw buffer (40 lines is enough for smooth scrolling)
//    draw_buf = lv_draw_buf_create(SCREEN_WIDTH, 40, LV_COLOR_FORMAT_RGB565, SCREEN_WIDTH * 2);
//    lv_display_set_draw_buffers(disp, draw_buf, NULL);   // single buffer mode
//
//    // 4. Register the flush callback
//    lv_display_set_flush_cb(disp, my_disp_flush);
//}
//
//// ---------------------------------------------------------------------------
//// BLE CALLBACK
//// ---------------------------------------------------------------------------
//class MyCallback : public BLECharacteristicCallbacks {
//    void onWrite(BLECharacteristic *pChar) override {
//        string val = pChar->getValue().c_str();
//        if (!val.length() > 0 ) {
//            Serial.print("BLE received: ");
//            Serial.println(val);
//
//            lv_label_set_text(ble_label, val.c_str());
//        }
//    }
//};
//
//// ---------------------------------------------------------------------------
//// SETUP
//// ---------------------------------------------------------------------------
//void setup()
//{
//    Serial.begin(115200);
//    Serial.println("ESP32 + LVGL 9 + BLE");
//
//    lv_init();
//    init_display();
//
//    // ------------------- UI -------------------
//    lv_obj_t *scr = lv_scr_act();
//    lv_obj_set_style_bg_color(scr, lv_color_hex(0x003366), LV_PART_MAIN);
//
//    lv_obj_t *title = lv_label_create(scr);
//    lv_label_set_text(title, "ESP32 BLE Display");
//    lv_obj_set_style_text_color(title, lv_color_white(), LV_PART_MAIN);
//    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
//
//    ble_label = lv_label_create(scr);
//    lv_label_set_text(ble_label, "Waiting for BLE data...");
//    lv_obj_set_style_text_color(ble_label, lv_color_hex(0x00FF00), LV_PART_MAIN);
//    lv_label_set_long_mode(ble_label, LV_LABEL_LONG_WRAP);
//    lv_obj_set_width(ble_label, SCREEN_WIDTH - 40);
//    lv_obj_align(ble_label, LV_ALIGN_CENTER, 0, 0);
//
//    lv_obj_t *status = lv_label_create(scr);
//    lv_label_set_text(status,
//        "Connect via nRF Connect\n"
//        "Write to characteristic");
//    lv_obj_set_style_text_color(status, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
//    lv_label_set_long_mode(status, LV_LABEL_LONG_WRAP);
//    lv_obj_set_width(status, SCREEN_WIDTH - 40);
//    lv_obj_align(status, LV_ALIGN_BOTTOM_MID, 0, -20);
//    lv_obj_set_style_text_align(status, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
//
//    // ------------------- BLE -------------------
//    BLEDevice::init("ESP32_LVGL_BLE");
//    BLEServer *pServer = BLEDevice::createServer();
//    BLEService *pService = pServer->createService(SERVICE_UUID);
//
//    BLECharacteristic *pChar = pService->createCharacteristic(
//        CHARACTERISTIC_UUID,
//        BLECharacteristic::PROPERTY_READ |
//        BLECharacteristic::PROPERTY_WRITE
//    );
//    pChar->setCallbacks(new MyCallback());
//    pChar->setValue("Ready");
//
//    pService->start();
//
//    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
//    pAdvertising->addServiceUUID(SERVICE_UUID);
//    pAdvertising->setScanResponse(true);
//    pAdvertising->start();
//
//    Serial.println("BLE ready");
//}
//
//// ---------------------------------------------------------------------------
//// LOOP
//// ---------------------------------------------------------------------------
//void loop()
//{
//    lv_timer_handler();   // LVGL tick & drawing
//    delay(5);
//}
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <lvgl.h>

#if LV_USE_TFT_ESPI
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
#endif

#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-90ab-cdef-1234567890ab"

#define TFT_HOR_RES   320
#define TFT_VER_RES   480
#define TFT_ROTATION  LV_DISPLAY_ROTATION_0

//#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
//uint32_t draw_buf[DRAW_BUF_SIZE / 4];
#define DRAW_BUF_SIZE (TFT_HOR_RES * 40)  // حدود یک ردیف 40 پیکسل
uint32_t draw_buf[DRAW_BUF_SIZE / 4];
lv_display_t * disp;
lv_obj_t *label;  // label جهانی برای تغییر متن

bool mirrorEnabled = true;
bool deviceConnected = false; 
// BLE Callback
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    Serial.println("Device connected!");
    deviceConnected = true;
  }

  void onDisconnect(BLEServer* pServer) override {
    Serial.println("Device disconnected!");
    deviceConnected = false;

    // دوباره شروع تبلیغ BLE تا دستگاه‌های دیگر بتوانند وصل شوند
    pServer->getAdvertising()->start();
    Serial.println("BLE Advertising restarted");
  }
};

// LVGL flush function
void my_disp_flush(lv_display_t *disp_drv, const lv_area_t *area, uint8_t * px_map) {
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);
    
#if LV_USE_TFT_ESPI
    tft.startWrite();
    
    if (!mirrorEnabled) {
        tft.setAddrWindow(area->x1, area->y1, w, h);
        tft.pushPixels((uint16_t *)px_map, w * h);
    } else {
        tft.setAddrWindow(area->x1, area->y1, w, h);
        uint16_t *pixels = (uint16_t *)px_map;
        for (uint32_t y = 0; y < h; y++) {
            uint32_t row_offset = y * w;
            for (int32_t x = w - 1; x >= 0; x--) {
                tft.pushPixels(&pixels[row_offset + x], 1);
            }
        }
    }
    
    tft.endWrite();
#endif
    
    lv_display_flush_ready(disp_drv);
}

void my_touchpad_read(lv_indev_t * indev, lv_indev_data_t * data) {
    data->state = LV_INDEV_STATE_RELEASED;
}

static uint32_t my_tick(void) {
    return millis();
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE + LVGL");

    // ===== BLE setup =====
    BLEDevice::init("ESP32_TestBLE");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
      BLECharacteristic::PROPERTY_WRITE
    );
    pCharacteristic->setValue("Hello BLE");
    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->start();

    // ===== LVGL setup =====
    lv_init();
    lv_tick_set_cb(my_tick);

#if LV_USE_TFT_ESPI
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
#else
    disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
#endif

    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, my_touchpad_read);

    // ===== Label اولیه =====
    label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Waiting for BLE...");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    delay(1000);
}

void loop() {
    lv_timer_handler();
    delay(5);

    // بررسی وضعیت اتصال و بروزرسانی متن label
    static bool lastState = false;  // وضعیت قبلی
    if (deviceConnected != lastState) {
        if (deviceConnected) {
            lv_label_set_text(label, "Device Connected!");
        } else {
            lv_label_set_text(label, "Device DisConnected");
            delay(3000);
        }
        lastState = deviceConnected;
    }
    else
    {
      if(deviceConnected == false )
      {
        lv_label_set_text(label, "Waiting for connection");

      }
    }
}
