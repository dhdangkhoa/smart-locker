#include "rw_to_flash.h"
#include "sensors.h"
#include <sstream>
#include <cstring>

static Preferences preferences;

QRCode_Data processReadData(const uint8_t payload[], int payload_len) {
    char savedData[64];
    int j = 0;
    for (int i = 0; i < payload_len && j < (int)sizeof(savedData) - 1; i++) {
        if (payload[i] == ':') {
            i++;
            while (i < payload_len && payload[i] != ' ' && j < (int)sizeof(savedData) - 1) {
                savedData[j++] = static_cast<char>(payload[i++]);      
            }
            savedData[j++] = ' ';
        }
    }
    savedData[j] = '\0';

    QRCode_Data qr;
    memset(&qr, 0, sizeof(qr));
    std::stringstream ss(savedData);
    ss >> qr.MaDH >> qr.NVGH >> qr.KH;
    qr.slot_id = 0;
    return qr;
}

// Lưu đơn hàng vào ngăn tủ chỉ định (slot 1 hoặc slot 2)
bool savePackageToSlot(uint8_t slot_id, const QRCode_Data &data) {
    if (slot_id < 1 || slot_id > MAX_PACKAGES) return false;

    preferences.begin("package-data", false);
    char key[16];
    snprintf(key, sizeof(key), "slot_%d", slot_id);

    char combined[64];
    snprintf(combined, sizeof(combined), "%s,%s,%s,%d", data.MaDH, data.NVGH, data.KH, slot_id);
    preferences.putString(key, combined);
    preferences.end();

    Serial.printf("[Flash] Da luu don '%s' vao Ngan %d: %s\n", data.MaDH, slot_id, combined);
    return true;
}

// Đọc thông tin đơn hàng từ ngăn tủ chỉ định
bool readPackageFromSlot(uint8_t slot_id, QRCode_Data &data) {
    if (slot_id < 1 || slot_id > MAX_PACKAGES) return false;

    preferences.begin("package-data", true);
    char key[16];
    snprintf(key, sizeof(key), "slot_%d", slot_id);

    char combined[64];
    memset(&data, 0, sizeof(data));

    if (preferences.isKey(key)) {
        if (preferences.getString(key, combined, sizeof(combined)) > 0) {
            int s_id = 0;
            sscanf(combined, "%15[^,],%15[^,],%15[^,],%d", data.MaDH, data.NVGH, data.KH, &s_id);
            data.slot_id = (uint8_t)s_id;
            preferences.end();
            return true;
        }
    }

    preferences.end();
    return false;
}

// Xóa đơn hàng khỏi ngăn tủ (khi khách đã nhận hàng)
void removePackageFromSlot(uint8_t slot_id) {
    if (slot_id < 1 || slot_id > MAX_PACKAGES) return;

    preferences.begin("package-data", false);
    char key[16];
    snprintf(key, sizeof(key), "slot_%d", slot_id);

    if (preferences.isKey(key)) {
        preferences.remove(key);
        Serial.printf("[Flash] Da xoa don tai Ngan %d.\n", slot_id);
    }
    preferences.end();
}

// Kiểm tra xem mã đơn hàng đã tồn tại trong tủ hay chưa
bool isOrderCodeExists(const char* maDH) {
    if (!maDH || strlen(maDH) == 0) return false;

    for (uint8_t s = 1; s <= MAX_PACKAGES; s++) {
        QRCode_Data data;
        if (readPackageFromSlot(s, data)) {
            if (strcmp(data.MaDH, maDH) == 0) {
                return true;
            }
        }
    }
    return false;
}

// Tìm kiếm đơn hàng theo mã nhập từ bàn phím (khớp với MaDH hoặc KH)
bool findPackageByCode(const char* code, QRCode_Data &outData) {
    if (!code || strlen(code) == 0) return false;

    for (uint8_t s = 1; s <= MAX_PACKAGES; s++) {
        QRCode_Data data;
        if (readPackageFromSlot(s, data)) {
            if (strcmp(data.MaDH, code) == 0 || strcmp(data.KH, code) == 0) {
                outData = data;
                return true;
            }
        }
    }
    return false;
}

// Đồng bộ trạng thái ngăn tủ (g_slots) và đèn LED theo dữ liệu Flash tại S0
void syncSlotsFromFlash(void) {
    Serial.println("[Flash] Dang dong bo trang thai cac ngan tu tu Flash (S0)...");
    for (uint8_t s = 1; s <= MAX_PACKAGES; s++) {
        QRCode_Data data;
        bool hasData = readPackageFromSlot(s, data);
        uint8_t idx = s - 1;

        if (hasData) {
            g_slots[idx].state = SLOT_OCCUPIED;
            g_slots[idx].has_parcel = true;
            Sensor_SetSlotLED(s, true);
            Serial.printf("  -> Ngan %d: CO HANG (MaDH: %s, KH: %s)\n", s, data.MaDH, data.KH);
        } else {
            g_slots[idx].state = SLOT_EMPTY;
            g_slots[idx].has_parcel = false;
            Sensor_SetSlotLED(s, false);
            Serial.printf("  -> Ngan %d: TRONG\n", s);
        }
    }
}

// ==========================================
// CÁC HÀM CŨ ĐỂ TƯƠNG THÍCH NGƯỢC
// ==========================================
void writePackageData(const QRCode_Data &data) {
    uint8_t slot = Sensor_FindLowestEmptySlot();
    if (slot != 0) {
        savePackageToSlot(slot, data);
    } else {
        Serial.println("[Flash] Tu da day, khong the luu!");
    }
}

int readAllPackageData(QRCode_Data list[], int maxItems) {
    int count = 0;
    for (uint8_t s = 1; s <= MAX_PACKAGES && count < maxItems; s++) {
        QRCode_Data d;
        if (readPackageFromSlot(s, d)) {
            list[count++] = d;
        }
    }
    return count;
}

void printAllPackageData(void) {
    QRCode_Data list[MAX_PACKAGES];
    int total = readAllPackageData(list, MAX_PACKAGES);
    Serial.printf("\n--- DANH SACH TRONG FLASH (%d don) ---\n", total);
    for (int i = 0; i < total; i++) {
        Serial.printf("%d. Ngan %d: MaDH=%s, NVGH=%s, KH=%s\n",
                      i + 1, list[i].slot_id, list[i].MaDH, list[i].NVGH, list[i].KH);
    }
}

void deletePackageData(const char* iMaDH) {
    for (uint8_t s = 1; s <= MAX_PACKAGES; s++) {
        QRCode_Data d;
        if (readPackageFromSlot(s, d)) {
            if (strcmp(d.MaDH, iMaDH) == 0) {
                removePackageFromSlot(s);
                return;
            }
        }
    }
}

void clearAllPackageData(void) {
    preferences.begin("package-data", false);
    preferences.clear();
    preferences.end();
    Serial.println("[Flash] Da xoa toan bo du lieu package-data trong Flash!");
}
