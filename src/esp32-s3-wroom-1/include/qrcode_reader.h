#include <quirc.h>
#include <Arduino.h>
#include "esp_camera.h"
#include "rw_to_flash.h"
#include "lcd.h"

void qrScannerTask(void *pvParameters);