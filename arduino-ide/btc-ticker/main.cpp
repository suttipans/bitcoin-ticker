/**
 * @file main.cpp
 * @author suttipan.sittirak@gmail.com
 * @brief Bitcoin ticker and Clock with some wallet functions
 * @version 0.1
 * @date 2021-12-31
 * @copyright Copyright (c) 2022
 * @note
 *    Prototype using:
 *      - M5Stack Grey
 *      - M5Stack Core2
 */

#include "M5.h"
#include <atomic>
#include "SD.h"

#include "my_config.h"
#include "get_btcinfo.h"
#include "my_webserver.h"
#include "render.h"
#include "utils.h"

#ifdef SHT3X_SENSOR
#include "SHT31.h"

SHT31 sht30;
float tmperature = 0.0;
float humidity = 0.0;
SemaphoreHandle_t sht30Mutex = xSemaphoreCreateMutex();
#endif


void setup() {
    M5.begin();
    #ifdef ARDUINO_M5Stack_Core_ESP32
    M5.Power.begin();
    #endif

    String s = getDeviceId();
    Serial.println(s);
    M5.Lcd.println(s);

    if (!SD.begin()) {
        Serial.println("SD card mount Failed");
    } else {
        uint8_t cardType = SD.cardType();
        if (cardType == CARD_NONE) {
            Serial.println("No SD card attached");
        } else {
            Serial.print("SD card Type: ");
            if (cardType == CARD_MMC) {
                Serial.println("MMC");
            } else if (cardType == CARD_SD) {
                Serial.println("SDSC");
            } else if (cardType == CARD_SDHC) {
                Serial.println("SDHC");
            } else {
                Serial.println("UNKNOWN");
            }

            Serial.printf("SD card found, size: %lluMB\n", SD.cardSize() / (1024 * 1024));
            Serial.println("Reading from SD card: /btcticker.conf");
            MyConfig::readConfigFromSD(SD, "/btcticker.conf");
        }
    }

    wifiConnect();
    configLocaltime();

    String lt = localtimeString();
    Serial.printf("Local time: %s\n", lt.c_str());
    M5.Lcd.printf("Local time: %s\n", lt.c_str());

    xTaskCreatePinnedToCore(task_get_btcinfo, "task_get_btcinfo", 8*1024, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(task_my_webserver, "task_my_webserver", 16*1024, NULL, 1, NULL, 1);

    delay(2000);
    M5.Lcd.clear();

    #ifdef SHT3X_SENSOR
    sht30.begin(G21, G22);
    #endif

    xTaskCreatePinnedToCore(task_rendering, "task_rendering", 4 * 1024, NULL, 1, NULL, 0);
}

std::atomic<uint8_t> user_mode{0};

void loop() {
    static uint32_t tp_1hr  = 0;   // for next 1 hours timeout
    static uint32_t tp_1sec = 0;
    static uint32_t tp_60sec = 0;
    uint32_t tp;

    M5.update();
    if (M5.BtnB.wasPressed()) {
        user_mode++;
        delay(20);
        if (user_mode == 1) {       // receiving address
            displayBtcAddressQr();
        } else if (user_mode == 2) {
            displayBalances();     // display balances
        }
        if (user_mode > 2) {
            user_mode = 0;
        }
    }

    // Press A, Escape back
    if (M5.BtnA.wasPressed()) {
        user_mode = 0;
    }

    tp = millis();
    if (tp > tp_1hr) {
        // next update, refresh the time from NTP
        tp_1hr = tp + 60 * 60 * 1000;
        Serial.printf("%06ld: ", millis());
        Serial.print(F("getting time from NTP server\n"));
        struct tm tm = configLocaltime();
        Serial.print(&tm, "%A, %B %d %Y %H:%M:%S\n");
    } else if (tp > tp_60sec) {
        tp_60sec = tp + 60*1000;
        Serial.println(localtimeString());

        #ifdef SHT3X_SENSOR
        if (sht30.isConnected()) {
            sht30.read();
            xSemaphoreTake(sht30Mutex, portMAX_DELAY);
            tmperature = sht30.getTemperature() - 2.0;
            humidity = sht30.getHumidity();
            xSemaphoreGive(sht30Mutex);
        }
        #endif
    } if (tp > tp_1sec) {
        tp_1sec = tp + 1000;
        myGetLocaltime();
    }

    yield();
    delay(50);
}
