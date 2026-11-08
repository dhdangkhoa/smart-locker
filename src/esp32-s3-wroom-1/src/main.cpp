#include <Arduino.h>
#include "esp_camera.h"
//#include <WiFi.h>
#include "quirc.h"

// ===========================
// Select camera model in board_config.h
// ===========================
#include "board_config.h"



// ===========================
// Enter your WiFi credentials
// ===========================
// const char *ssid = "********";
// const char *password = "********";


void startCameraServer();
void setupLedFlash();

bool initCamera();
void configureSensor();
//void connectWiFi();

void qrScannerTask(void *pvParameters);


void setup() {
  Serial.begin(115200);
  delay(2000);
  //Serial.setDebugOutput(true);
  Serial.println();
  
  if (!initCamera()) {
    Serial.println("Camera init failed!");
    return;
  }
  // connectWiFi();

  configureSensor();
  //startCameraServer();

  xTaskCreatePinnedToCore(
    qrScannerTask,    // Task function
    "qrScannerTask",  // Name of task
    1024 * 24,         // Stack size (6KB is plenty for grayscale processing)
    NULL,             // Parameter passed to task
    5,                // Priority
    NULL,             // Task handle
    1                 // Pin to Core 1
  );
}

void loop() {
  // Do nothing. Everything is done in another task by the web server
  delay(10000);
}

// write function here
bool initCamera() {
  camera_config_t config = {};

  // Map pin definitions loaded from board_config.h
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;


  config.xclk_freq_hz = 20000000;
  config.frame_size   = FRAMESIZE_QVGA;
  config.pixel_format = PIXFORMAT_GRAYSCALE;
  config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;
  config.jpeg_quality = 12;                 
  config.fb_location  = CAMERA_FB_IN_PSRAM;
  config.fb_count     = 2;                  

  esp_err_t err = esp_camera_init(&config);
  return (err == ESP_OK);
}


void configureSensor() {
  sensor_t *s = esp_camera_sensor_get();
  if (!s) return;
  s->set_framesize(s, FRAMESIZE_QVGA); 
}

// void connectWiFi() {
//   WiFi.begin(ssid, password);
//   WiFi.setSleep(false);

//   Serial.print("WiFi connecting");
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("");
//   Serial.println("WiFi connected");

//   Serial.print("Camera Ready! Use 'http://");
//   Serial.print(WiFi.localIP());
//   Serial.println("' to connect");
// }

void qrScannerTask(void *pvParameters) {
  struct quirc *q = quirc_new();
  
  if (!q) {
    Serial.println("Quirc allocation failed.");
    vTaskDelete(NULL);
    return;
  }

  // Set quirc size boundaries to match FRAMESIZE_QVGA (320 x 240)
  if (quirc_resize(q, 320, 240) < 0) {
    Serial.println("Quirc buffer resize failure.");
    quirc_destroy(q);
    vTaskDelete(NULL);
    return;
  }

  Serial.println("Direct Grayscale QR Scanning Engine Loop Active.");

  while (1) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      vTaskDelay(pdMS_TO_TICKS(50));
      continue;
    }

    int q_w, q_h;
    uint8_t *q_buf = quirc_begin(q, &q_w, &q_h);

    // Verify incoming lens buffer dimensions match up safely with configured engine structure
    if (fb->width == q_w && fb->height == q_h) {
      
      // Because it's already grayscale, direct block memory copy is incredibly quick
      memcpy(q_buf, fb->buf, q_w * q_h);
      quirc_end(q);

      int count = quirc_count(q);
      for (int i = 0; i < count; i++) {
        struct quirc_code code;
        struct quirc_data data;
        quirc_extract(q, i, &code);
        quirc_decode_error_t err = quirc_decode(&code, &data);

        if (err == QUIRC_SUCCESS) {
          Serial.printf("[QR CODE DETECTED]: %s\n", data.payload);
        } else {
          Serial.printf("Decoding failed pattern index: %s\n", quirc_strerror(err));
        }
      }
    }
    
    esp_camera_fb_return(fb);

    vTaskDelay(pdMS_TO_TICKS(30));
  }

  quirc_destroy(q);
  vTaskDelete(NULL);
}