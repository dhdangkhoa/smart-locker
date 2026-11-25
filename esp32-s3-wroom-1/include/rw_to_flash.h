#ifndef RW_TO_FLASH_H
#define RW_TO_FLASH_H

#include <Arduino.h>
#include <Preferences.h>

#define MAX_PACKAGES 2

struct QRCode_Data {
    char MaDH[16];
    char NVGH[16];
    char KH[16];
    uint8_t slot_id; // 1 hoặc 2
};

// Hàm trích xuất dữ liệu từ chuỗi mã QR
QRCode_Data processReadData(const uint8_t payload[], int payload_len);

// ==========================================
// CÁC HÀM QUẢN LÝ DỮ LIỆU ĐƠN HÀNG TRÊN FLASH
// ==========================================
bool savePackageToSlot(uint8_t slot_id, const QRCode_Data &data);
bool readPackageFromSlot(uint8_t slot_id, QRCode_Data &data);
void removePackageFromSlot(uint8_t slot_id);
bool isOrderCodeExists(const char* maDH);
bool findPackageByCode(const char* code, QRCode_Data &outData);
void syncSlotsFromFlash(void);

// ==========================================
// CÁC HÀM CŨ ĐỂ TƯƠNG THÍCH NGƯỢC
// ==========================================
void writePackageData(const QRCode_Data &data);
int readAllPackageData(QRCode_Data list[], int maxItems);
void printAllPackageData(void);
void deletePackageData(const char* iMaDH);
void clearAllPackageData(void);

#endif // RW_TO_FLASH_H
