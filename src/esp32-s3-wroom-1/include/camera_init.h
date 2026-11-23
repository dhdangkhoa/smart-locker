#ifndef CAMERA_INIT_H
#define CAMERA_INIT_H

#include "esp_camera.h"
// Select camera model in board_config.h
#include "board_config.h"

bool initCamera();
void configureSensor();

#endif // CAMERA_INIT_H