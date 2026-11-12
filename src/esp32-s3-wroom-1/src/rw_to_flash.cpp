#include <sstream>
#include <cstring>
#include "rw_to_flash.h"


Preferences preferences;
/*
	uint8_t			payload[QUIRC_MAX_PAYLOAD];
	int			payload_len;
*/

QRCode_Data processReadData(const uint8_t payload[], int payload_len) {
    char savedData[50];
    int j = 0;
    for (int i = 0; i < payload_len; i++) {
        if (payload[i] == ':') {
            i++;
            while (i < payload_len && payload[i] != ' ') {
                savedData[j++] = static_cast<char>(payload[i++]);      
            }
        }
        savedData[j++] = ' ';
    }
    savedData[j++] = '\0';
    QRCode_Data qr;
    std::stringstream ss(savedData);
    ss >> qr.MaDH >> qr.NVGH >> qr.KH;
    return qr;
}

void writePackageData(const QRCode_Data &data) {
    preferences.begin("package-data", false);
    
    int count = preferences.getInt("count", 0);
    
    char key[10];
    char existingCombined[30];
    char existingMaDH[10];

    for (int i = 0; i < count; i++) {
        snprintf(key, sizeof(key), "item_%d", i);
        
        if (preferences.getString(key, existingCombined, sizeof(existingCombined)) > 0) {
            if (sscanf(existingCombined, "%14[^,]", existingMaDH) == 1) {
                if (strcmp(existingMaDH, data.MaDH) == 0) {
                    Serial.printf("Bo qua: Ma don '%s' da ton tai!\n", data.MaDH);
                    preferences.end(); 
                    return;
                }
            }
        }
    }
    
    if (count >= MAX_PACKAGES) {
        Serial.println("Bo nho day! Khong the luu them.");
        preferences.end();
        return;
    }


    snprintf(key, sizeof(key), "item_%d", count);


    char combined[30];
    snprintf(combined, sizeof(combined), "%s, %s, %s", data.MaDH, data.NVGH, data.KH);


    preferences.putString(key, combined);
    preferences.putInt("count", count + 1);
    
    preferences.end();
    Serial.printf("Da luu don thu %d: %s\n", count + 1, combined);
}

void readAllPackageData() {
    preferences.begin("package-data", true);

    int count = preferences.getInt("count", 0);
    Serial.printf("\n=== DANH SACH DON HANG TRONG FLASH (%d don) ===\n", count);

    if (count == 0) {
        Serial.println("Chua co du lieu nao trong Flash.");
        preferences.end();
        return;
    }

    char key[16];
    char combined[64];

    for (int i = 0; i < count; i++) {
        snprintf(key, sizeof(key), "item_%d", i);
        
        if (preferences.getString(key, combined, sizeof(combined)) > 0) {
            Serial.printf("%d. %s\n", i + 1, combined);
        }
    }

    preferences.end();
}

void deletePackageData(const char* iMaDH) {
preferences.begin("package-data", false);

    char key[10];
    char combined[30];
    char existingMaDH[10];

    for (int i = 0; i < MAX_PACKAGES; i++) {
        snprintf(key, sizeof(key), "item_%d", i);
        

        if (preferences.isKey(key)) {
            if (preferences.getString(key, combined, sizeof(combined)) > 0) {
                if (sscanf(combined, "%14[^,]", existingMaDH) == 1) {
                    if (strcmp(existingMaDH, iMaDH) == 0) {
                        preferences.remove(key); 
                        preferences.end();
                        Serial.printf("Da xoa don '%s' tai o so %d (de trong).\n", iMaDH, i);
                        return;
                    }
                }
            }
        }
    }

    preferences.end();
    Serial.printf("Khong tim thay ma don '%s'!\n", iMaDH);
}


void clearAllPackageData() {
    preferences.begin("package-data", false);
    preferences.clear(); 
    preferences.end();
    Serial.println("Da xoa sach du lieu trong Flash!");
}