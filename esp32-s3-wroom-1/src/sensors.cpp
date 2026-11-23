#include "sensors.h"

// Mảng trạng thái của các ngăn tủ
SlotData_t g_slots[2] = {
    { 1, SLOT_EMPTY, DOOR_CLOSED, false, false },
    { 2, SLOT_EMPTY, DOOR_CLOSED, false, false }
};

void Sensor_Init(void) {
    pinMode(PIN_SW_DOOR_1, INPUT);
    pinMode(PIN_SW_DOOR_2, INPUT);
    pinMode(PIN_SW_PARCEL_1, INPUT);
    pinMode(PIN_SW_PARCEL_2, INPUT);

    pinMode(PIN_BTN_PWR, INPUT_PULLUP);

    pinMode(PIN_LED_SLOT_1, OUTPUT);
    pinMode(PIN_LED_SLOT_2, OUTPUT);
    digitalWrite(PIN_LED_SLOT_1, LOW);
    digitalWrite(PIN_LED_SLOT_2, LOW);

    Sensor_UpdateAllSlots();
}

static int Sensor_ReadPinDebounced(uint8_t pin) {
    int first_read = digitalRead(pin);
    delay(DEBOUNCE_TIME_MS);
    int second_read = digitalRead(pin);
    return (first_read == second_read) ? second_read : digitalRead(pin);
}

DoorState_t Sensor_GetDoorState(uint8_t slot_id) {
    uint8_t pin = (slot_id == 1) ? PIN_SW_DOOR_1 : PIN_SW_DOOR_2;
    return (Sensor_ReadPinDebounced(pin) == LOW) ? DOOR_OPEN : DOOR_CLOSED;
}

bool Sensor_HasParcel(uint8_t slot_id) {
    uint8_t pin = (slot_id == 1) ? PIN_SW_PARCEL_1 : PIN_SW_PARCEL_2;
    return (Sensor_ReadPinDebounced(pin) == HIGH);
}

void Sensor_SetSlotLED(uint8_t slot_id, bool turn_on) {
    uint8_t led_pin = (slot_id == 1) ? PIN_LED_SLOT_1 : PIN_LED_SLOT_2;
    digitalWrite(led_pin, turn_on ? HIGH : LOW);
    g_slots[slot_id - 1].led_on = turn_on;
}

void Sensor_UpdateAllSlots(void) {
    for (uint8_t i = 0; i < 2; i++) {
        uint8_t slot_id = i + 1;
        g_slots[i].door       = Sensor_GetDoorState(slot_id);
        g_slots[i].has_parcel = Sensor_HasParcel(slot_id);
        g_slots[i].state      = g_slots[i].has_parcel ? SLOT_OCCUPIED : SLOT_EMPTY;
        Sensor_SetSlotLED(slot_id, g_slots[i].has_parcel);
    }
}

uint8_t Sensor_FindLowestEmptySlot(void) {
    Sensor_UpdateAllSlots();
    for (uint8_t i = 0; i < 2; i++) {
        if (!g_slots[i].has_parcel) {
            return g_slots[i].slot_id; // Trả về 1 hoặc 2
        }
    }
    return 0; // Cả 2 ngăn đều đã có hàng
}

bool Sensor_WaitForDoorState(uint8_t slot_id, DoorState_t target_state, uint32_t timeout_ms) {
    unsigned long start_time = millis();
    while (true) {
        Sensor_UpdateAllSlots();

        if (Sensor_GetDoorState(slot_id) == target_state) {
            g_slots[slot_id - 1].door = target_state;
            return true;
        }
        if (timeout_ms > 0 && (millis() - start_time >= timeout_ms)) {
            g_slots[slot_id - 1].state = SLOT_FAULT;
            return false;
        }
        delay(50);
    }
}

bool Sensor_IsButtonPressed(uint8_t btn_pin) {
    if (btn_pin == 0xFF) return false;
    if (digitalRead(btn_pin) == LOW) {
        delay(DEBOUNCE_TIME_MS);
        if (digitalRead(btn_pin) == LOW) {
            while (digitalRead(btn_pin) == LOW) {
                delay(10);
            }
            return true;
        }
    }
    return false;
}
