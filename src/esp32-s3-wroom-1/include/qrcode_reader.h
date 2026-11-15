#pragma once

#include <quirc.h>
#include <Arduino.h>
#include "esp_camera.h"
#include "rw_to_flash.h"
#include "lcd.h"

extern volatile bool isAwake;
extern volatile unsigned long wakeUpTimer;

void qrScannerTask(void *pvParameters);
