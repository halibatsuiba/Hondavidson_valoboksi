#pragma once

#include <stdint.h>

// ============================================================
// ESP32-C3-Zero
// ULN2003 outputs
// ============================================================

#define ULN_IN1  2
#define ULN_IN2  3
#define ULN_IN3  4
#define ULN_IN4  5

// Number of ULN2003 channels used
#define ULN_CHANNELS 4

// ULN2003 inputs are active HIGH:
// HIGH = output transistor ON
// LOW  = output transistor OFF
inline constexpr bool ULN_ACTIVE_HIGH = true;

inline constexpr uint8_t LEFT_FRONT_BLINKER = ULN_IN1;
inline constexpr uint8_t RIGHT_FRONT_BLINKER = ULN_IN2;
inline constexpr uint8_t LEFT_REAR_BLINKER = ULN_IN3;
inline constexpr uint8_t RIGHT_REAR_BLINKER = ULN_IN4;

inline constexpr uint8_t LEFT_BLINKER_SWITCH_PIN = 6;
inline constexpr uint8_t RIGHT_BLINKER_SWITCH_PIN = 8;
inline constexpr uint8_t BRAKE_SWITCH_PIN = 7;

// ============================================================
// 12 V supply
// ============================================================

inline constexpr float LOAD_VOLTAGE = 12.0f;

// ============================================================
// PlatformIO / ESP32
// ============================================================

inline constexpr uint32_t SERIAL_BAUD = 115200;
