#include "qrcode_reader.h"



void qrScannerTask(void *pvParameters) {
  LCDInit();
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
          
          static String lastPayload = "";
          String currentPayload = String((const char*)data.payload);

          if (currentPayload != lastPayload) {
            lastPayload = currentPayload;


            LCDPrintQR((const char*)data.payload, data.payload_len);

            QRCode_Data read_data = processReadData(data.payload, data.payload_len);
            writePackageData(read_data);
          }
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
  delay(5000);
}