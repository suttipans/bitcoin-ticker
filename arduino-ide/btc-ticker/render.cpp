/**
 * @file render.cpp
 * @author Suttipan Sittirak (suttipan.sittirak@gmail.com)
 * @brief  Drawing stuff on the screen
 * @version 0.1
 * @date 2022-01-12
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "M5.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <atomic>
#include "Free_Fonts.h"

#include "get_btcinfo.h"
#include "qrencoder.h"
#include "utils.h"

// header files for icons
#include "Bitcoin_jpg.h"
#include "Thai_jpg.h"
#include "Us_jpg.h"

extern SemaphoreHandle_t xMutex;
extern BtcInfo btcinfo;
extern std::atomic<uint8_t> user_mode;

#ifdef SHT3X_SENSOR
extern float tmperature;
extern float humidity;
extern SemaphoreHandle_t sht30Mutex;
#endif

std::atomic<bool> need_redraw{true};

static void drawShortTime() {
    struct tm tm = myGetLocaltime();
    char tbuf[16];
    M5.Lcd.setTextSize(1);
    M5.Lcd.setFreeFont(FF1);
    M5.Lcd.setTextDatum(TL_DATUM);
    M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    snprintf(tbuf, sizeof(tbuf), "%02d:%02d", tm.tm_hour, tm.tm_min);
    M5.Lcd.drawString(tbuf, 260, 1, GFXFF);
}


static void drawDigitalClock() {
    static uint8_t oss = UINT8_MAX;
    static uint32_t tp_1sec = 0;  // for next 1 second timeout

    if (tp_1sec > millis() ) {
        return;
    }
    tp_1sec = millis() + 1000;  // set next update for 1 second later

    static uint8_t xcolon = 0, xsecs = 0;
    struct tm tm = myGetLocaltime();
    uint8_t hh, mm, ss;
    ss = tm.tm_sec;
    mm = tm.tm_min;
    hh = tm.tm_hour;
    char tbuf[64];
    strftime(tbuf, sizeof(tbuf), "%A, %B %d %Y", &tm);

    #ifdef SHT3X_SENSOR
    xSemaphoreTake(sht30Mutex, portMAX_DELAY);
    float Temp = tmperature;
    float RH   = humidity;
    xSemaphoreGive(sht30Mutex);
    #endif

    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextFont(8);
    M5.Lcd.setTextDatum(TL_DATUM);

    int xpos = 0;
    int ypos = (M5.Lcd.height() - M5.Lcd.fontHeight(8)) / 2 - 40;
    int ysecs = ypos + 24;

    M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    // Draw hours and minutes
    if (hh < 10) xpos += M5.Lcd.drawChar('0', xpos, ypos, 8);  // Add hours leading zero for 24 hr clock
    xpos += M5.Lcd.drawNumber(hh, xpos, ypos, 8);              // Draw hours
    xcolon = xpos;                                             // Save colon coord for later to flash on/off later
    xpos += M5.Lcd.drawChar(':', xpos, ypos - 8, 8);
    if (mm < 10) xpos += M5.Lcd.drawChar('0', xpos, ypos, 8);  // Add minutes leading zero
    xpos += M5.Lcd.drawNumber(mm, xpos, ypos, 8);              // Draw minutes
    xsecs = xpos;

    if (oss != ss) {                                           // Redraw seconds time every second
        oss = ss;
        xpos = xsecs;

        if (ss % 2) {                                       // Flash the colons on/off
            M5.Lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);  // Set colour to grey to dim colon
            M5.Lcd.drawChar(':', xcolon, ypos - 8, 8);      // Hour:minute colon
            xpos += M5.Lcd.drawChar(':', xsecs, ysecs, 6);  // Seconds colon
        } else {
            M5.Lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
            M5.Lcd.drawChar(':', xcolon, ypos - 8, 8);      // Hour:minute colon
            xpos += M5.Lcd.drawChar(':', xsecs, ysecs, 6);  // Seconds colon
        }

        // Draw seconds
        M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
        if (ss < 10) xpos += M5.Lcd.drawChar('0', xpos, ysecs, 6);  // Add leading zero
        M5.Lcd.drawNumber(ss, xpos, ysecs, 6);                      // Draw seconds
    }

    M5.Lcd.setTextSize(1);
    M5.Lcd.setFreeFont(FF17);
    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.drawString(tbuf, M5.Lcd.width() / 2, M5.Lcd.height() / 2 + 20, GFXFF);
    M5.Lcd.drawString("Bangkok, UTC +7", M5.Lcd.width() / 2, M5.Lcd.height() / 2 + 60, GFXFF);
    #ifdef SHT3X_SENSOR
    snprintf(tbuf, sizeof(tbuf), "TEMP: %.1f C", Temp);
    M5.Lcd.drawString(tbuf, 10, 220, GFXFF);
    snprintf(tbuf, sizeof(tbuf), "RH: %.0f%%", RH);
    M5.Lcd.drawString(tbuf, 275, 220, GFXFF);
    #endif
}


static void drawBtcInfo_THB() {
    static uint32_t tp_1sec = 0;  // for next 1 second timeout
    static String old_price;

    if (need_redraw || millis() > tp_1sec) {
        tp_1sec = millis() + 1000;  // set next update for 1 second later
    } else {
        return;
    }
    drawShortTime();

    BtcInfo info = getBitcoinNodeInfo();
    info.btcthb_price.trim();
    if (need_redraw) {
        ;
    } else if (info.btcthb_price == old_price) {
        return;
    } else {
        old_price = info.btcthb_price;
    }

    M5.Lcd.setTextSize(2);
    M5.Lcd.setFreeFont(FF19);
    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);

    M5.Lcd.drawString(info.btcthb_price, M5.Lcd.width() / 2, M5.Lcd.height() / 2 - 20, 4);
    //M5.Lcd.drawString("199,999,999", M5.Lcd.width() / 2, M5.Lcd.height() / 2 - 20, 4);

    M5.Lcd.setTextSize(1);
    M5.Lcd.setFreeFont(FF17);
    M5.Lcd.drawString("BTC/THB", M5.Lcd.width() / 2, M5.Lcd.height() / 2 + 40, GFXFF);
    M5.Lcd.drawString("@Bitkub", M5.Lcd.width() / 2, M5.Lcd.height() / 2 + 60, GFXFF);
    M5.Lcd.drawJpg(Bitcoin_jpg, sizeof(Bitcoin_jpg), 15, 150);
    M5.Lcd.drawJpg(Thai_jpg, sizeof(Thai_jpg), 320 - 64 - 15, 150);
}


static void drawBtcInfo_USD() {
    static uint32_t tp_1sec = 0;  // for next 1 second timeout
    static String old_price;

    if (need_redraw || millis() > tp_1sec) {
        tp_1sec = millis() + 1000;  // set next update for 1 second later
    } else {
        return;
    }

    drawShortTime();

    BtcInfo info;
    xSemaphoreTake(xMutex, portMAX_DELAY);
    info = btcinfo;
    xSemaphoreGive(xMutex);

    info.btcusd_price.trim();
    if (need_redraw) {
        ;
    } else if (info.btcusd_price == old_price) {
        return;
    } else {
        old_price = info.btcusd_price;
    }

    M5.Lcd.setTextSize(2);
    M5.Lcd.setFreeFont(FF19);
    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    info.btcusd_price.trim();
    String s = "$" + info.btcusd_price;
    M5.Lcd.drawString(s, M5.Lcd.width() / 2, M5.Lcd.height() / 2 - 20, GFXFF);

    M5.Lcd.setTextSize(1);
    M5.Lcd.setFreeFont(FF17);
    M5.Lcd.drawString("BTC/USD", M5.Lcd.width() / 2, M5.Lcd.height() / 2 + 40, GFXFF);
    M5.Lcd.drawString("@Bitstamp", M5.Lcd.width() / 2, M5.Lcd.height() / 2 + 60, GFXFF);

    M5.Lcd.drawJpg(Bitcoin_jpg, sizeof(Bitcoin_jpg), 15, 150);
    M5.Lcd.drawJpg(Us_jpg, sizeof(Us_jpg), 320 - 64 - 15, 150);
}


static void drawMemPool_info() {
    static uint32_t tp_1sec = 0;  // for next 1 second timeout

    if (need_redraw || millis() > tp_1sec) {
        tp_1sec = millis() + 1000;  // set next update for 1 second later
    } else {
        return;
    }

    drawShortTime();

    BtcInfo info;
    xSemaphoreTake(xMutex, portMAX_DELAY);
    info = btcinfo;
    xSemaphoreGive(xMutex);  // release mutex

    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextFont(4);
    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);

    char buf[64];
    snprintf(buf, sizeof(buf), "Height: %s", info.block_height.c_str());
    M5.Lcd.drawString(buf, M5.Lcd.width()/2, 20);

    snprintf(buf, sizeof(buf), "Age: %s(min.)", info.block_age.c_str());
    M5.Lcd.drawString(buf,  M5.Lcd.width()/2, 60);
    snprintf(buf, sizeof(buf), "Mempool Blocks: %s", info.mempool_blk.c_str());
    M5.Lcd.drawString(buf,  M5.Lcd.width()/2, 100);

    snprintf(buf, sizeof(buf), "Unconfirmed Txs: %s", info.unconfirmed_txs.c_str());
    M5.Lcd.drawString(buf,  M5.Lcd.width()/2, 140);

    M5.Lcd.drawString("Smart fees(1,6,144):",  M5.Lcd.width()/2, 180);
    snprintf(buf, sizeof(buf), "%s", info.smart_feerates.c_str());
    M5.Lcd.drawString(buf,  M5.Lcd.width()/2, 210);
}

unsigned long tp_last_page = 0;
int page_idx = 0;
int counter = 1;

class Page {
   public:
    void (*render)(void);    // page renderer
    unsigned long duration;  // time to display in ms.
};

Page pages[] = {{drawDigitalClock, 5000},
                {drawBtcInfo_THB,  5000},
                {drawBtcInfo_USD,  5000},
                {drawMemPool_info, 5000}
};

int n_page = (sizeof(pages) / sizeof(Page));

void task_rendering(void* param) {
    tp_last_page = millis();
    while (1) {
        if (user_mode == 0) {
            if (need_redraw) {
                M5.Lcd.clear();
            }
            pages[page_idx].render();
            need_redraw = false;

            if (millis() - tp_last_page > pages[page_idx].duration) {
                page_idx = (page_idx + 1) % n_page;
                tp_last_page = millis();
                need_redraw = true;
            }
        } else {
            need_redraw = true;
        }

        yield();
        delay(10);
    }
}
