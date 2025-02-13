#pragma once
#ifndef I_ARDUINO_IO_H
#define I_ARDUINO_IO_H

#ifdef DEV_SSPS3_RUN_ON_PLC

#include <Arduino.h>

#else

#include <cstdint>
#include <cstdio>

#define millis()                (0) // Для упрощения – возвращает 0 (в реальном приложении можно подключить chrono)
#define delay(ms)                   /* dummy delay */

#define INPUT               0x01
#define OUTPUT              0x03 
#define PULLUP              0x04
#define INPUT_PULLUP        0x05
#define PULLDOWN            0x08
#define INPUT_PULLDOWN      0x09
#define OPEN_DRAIN          0x10
#define OUTPUT_OPEN_DRAIN   0x13
#define ANALOG              0xC0

#define LOW                 0x0
#define HIGH                0x1


bool digitalRead(uint8_t pin)               { return false; }
void digitalWrite(uint8_t pin, uint8_t val) {}
void pinMode(uint8_t pin, uint8_t mode)     {}

#endif // !DEV_SSPS3_RUN_ON_PLC

#endif // !I_ARDUINO_IO_H
