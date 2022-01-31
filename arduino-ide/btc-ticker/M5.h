/**
 * @file M5.h
 * @author Suttipan Sittirak (suttipan.sittirak@gmail.com)
 * @brief  Convenient M5Stack header files
 * @version 0.1
 * @date 2022-01-27
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef M5_H
#define M5_H

#ifdef ARDUINO_M5STACK_Core2
    #include <M5Core2.h>
#elif ARDUINO_M5Stack_Core_ESP32
    #include <M5Stack.h>
#elif ARDUINO_M5Stick_C
    #include <M5StickCPlus.h>
#endif

#include <Arduino.h>

#endif
