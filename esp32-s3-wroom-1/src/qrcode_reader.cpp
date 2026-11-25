#include "qrcode_reader.h"

static volatile bool s_qr_available = false;
static QRCode_Data s_latest_qr_data;
static volatile bool s_scanning_enabled = true;

bool QR_HasNewCode(QRCode_Data &outData) {
    if (s_qr_available) {
        outData = s_latest_qr_data;
        s_qr_available = false;
        return true;
    }
    return false;
}

void QR_ClearCode(void) {
    s_qr_available = false;
}

void QR_SetScanningEnabled(bool enabled) {
    s_scanning_enabled = enabled;
}

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
    // Nếu đang ngủ hoặc quét bị tạm dừng, tạm nghỉ task
    if (!isAwake || !s_scanning_enabled) {
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      vTaskDelay(pdMS_TO_TICKS(50));
      continue;
    }

    int q_w, q_h;
    uint8_t *q_buf = quirc_begin(q, &q_w, &q_h);

    if (fb->width == q_w && fb->height == q_h) {
      // Sao chép nhanh bộ đệm ảnh grayscale
      memcpy(q_buf, fb->buf, q_w * q_h);

      // Trả ngay frame buffer cho driver camera để tránh EV-VSYNC-OVF
      esp_camera_fb_return(fb);
      fb = NULL;

      quirc_end(q);

      int count = quirc_count(q);
      for (int i = 0; i < count; i++) {
        struct quirc_code code;
        struct quirc_data data;
        quirc_extract(q, i, &code);
        quirc_decode_error_t err = quirc_decode(&code, &data);

        if (err == QUIRC_SUCCESS) {
          Serial.printf("[QR CODE DETECTED]: %s\n", data.payload);

          // Reset bộ đếm timeout
          wakeUpTimer = millis();

          QRCode_Data read_data = processReadData(data.payload, data.payload_len);

          if (strlen(read_data.MaDH) > 0) {
            s_latest_qr_data = read_data;
            s_qr_available = true; // Báo cho State Machine chính xử lý S3
          }
        }
      }
    }
    
    if (fb) {
      esp_camera_fb_return(fb);
      fb = NULL;
    }

    vTaskDelay(pdMS_TO_TICKS(40));
  }

  quirc_destroy(q);
  vTaskDelete(NULL);
}
