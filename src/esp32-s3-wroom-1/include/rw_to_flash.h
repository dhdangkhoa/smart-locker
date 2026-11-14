#pragma once

#include <Preferences.h>

#define MAX_PACKAGES 2


struct QRCode_Data {
    char MaDH[10];
    char NVGH[10];
    char KH[10];
};


QRCode_Data processReadData(const uint8_t payload[], int payload_len);

void writePackageData(const QRCode_Data &data);

int readAllPackageData(QRCode_Data list[], int maxItems);

void printAllPackageData();
void deletePackageData(const char* iMaDH);

void clearAllPackageData();