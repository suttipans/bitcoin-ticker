/**
 * @file get_btcinfo.cpp
 * @author Suttipan Sittirak (suttipan.sittirak@gmail.com)
 * @brief Get Bitcoin info from server
 * @version 0.1
 * @date 2022-01-12
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <Arduino.h>

#include "get_btcinfo.h"
#include "rpc_calls.h"
#include "utils.h"

BtcInfo btcinfo;
SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();


BtcInfo getBitcoinNodeInfo() {
    xSemaphoreTake(xMutex, portMAX_DELAY);
    BtcInfo info = btcinfo;
    xSemaphoreGive(xMutex);

    return info;
}


static void getPrices(BtcInfo& btcinfo) {
    double btcthb_price{0.0}, btcthb_high{0.0}, btcthb_low{0.0}, btcthb_open{0.0};
    double btcusd_price{0.0}, btcusd_high{0.0}, btcusd_low{0.0}, btcusd_open{0.0};

    if (Bitkub::getBtcPrice(btcthb_price, btcthb_high, btcthb_low, btcthb_open) != 0) {
        Serial.println("Error: get_btc_price THB");
        return;
    }

    if (BitStamp::getBtcPrice(btcusd_price, btcusd_high, btcusd_low, btcusd_open) != 0) {
        Serial.println("Error get_btc_price USD");
        return;
    }

    String price_thb = thousandSeparator(std::floor(btcthb_price));
    String price_usd = thousandSeparator(std::floor(btcusd_price));

    xSemaphoreTake(xMutex, portMAX_DELAY);
    btcinfo.btcthb_price = price_thb;
    btcinfo.btcusd_price = price_usd;
    xSemaphoreGive(xMutex);
}


static void getBtcInfo(BtcInfo& btcinfo) {
    BitcoinNodeInfo node_info;
    int rc = BitcoinCore::getnodeinfo(node_info);
    if (rc != 0) {
        Serial.printf("Error get_bitcoin_node_info: %d\n", rc);
        return;
    }

    BtcInfo info;
    info.tp = millis();
    //info.btcthb_price = String(std::floor(btcthb_price), 0);
    //info.btcusd_price = String(std::floor(btcusd_price), 0);
    info.block_height   = thousandSeparator(node_info.height);
    info.block_age      = String((time(nullptr) - node_info.time)/60.0, 1);
    info.mempool_blk    = String(node_info.mempool_size/1048576.0, 1);
    info.unconfirmed_txs = thousandSeparator(node_info.unconfirmed_txs);
    info.smart_feerates = String(node_info.feerate_1blk, 0) + String(",") +
                          String(node_info.feerate_6blk, 0) + String(",") +
                          String(node_info.feerate_144blk, 0) + "(sat/vB)";

    xSemaphoreTake(xMutex, portMAX_DELAY);
    info.btcthb_price = btcinfo.btcthb_price;
    info.btcusd_price = btcinfo.btcusd_price;
    btcinfo = info;
    xSemaphoreGive(xMutex);
}

// Thread worker, periodically get prices and bitcoin mempool info
void task_get_btcinfo(void* param) {
    static unsigned long tp_1min = 0;
    static unsigned long tp_10sec = 0;

    while (1) {
        if (millis() > tp_10sec || tp_10sec == 0) {
            tp_10sec = millis() + 10000L;
            getBtcInfo(btcinfo);
        }

        if (millis() > tp_1min || tp_1min == 0) {
            tp_1min = millis() + 60000L;
            getPrices(btcinfo);
        }

        yield();
        delay(50);
    }
}