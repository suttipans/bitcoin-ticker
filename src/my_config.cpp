#include "M5.h"
#include <map>

#include "SD.h"
#include "SPI.h"

#include "my_config.h"
#include "utils.h"

String DEVICE_ID = "M5-" + String(getChipId());

/**
 * @brief Three ways to set these parameter.
 *      1) Your environment variables during build process
 *          for Unix like systems, e.g.
 *          export WIFI_SSID=\"WIFI-SSID\"
 *          export WIFI_PASS=\"WIFI-PASS\" ...
 *      2) Hardcode them in the #defines below
 *      3) Read them from a SD memory card
 *
 *  BITCOIN_RPC_HOST: Bitcoin full node address
 *  BITCOIN_RPC_PORT: usually 8332 for mainnet, 18332 for testnet
 *  BITCOIN_RPC_AUTH: is username and password in "user:password" format
 *
 *  *** NOTES!! on Bitcoin Core machine: ***
 *      Add or modify these parameters in bitcoin.conf file.
 *          - rpcuser and rpcpassword or rpcauth
 *          - rpcallowip=your_ip_addresss
 *          - server = 1
 */


/**
 * @brief  If no SD card found, leaving WIFI_SSIID and WIFI_PASS blank,
 *      this will active smartconfig.
 *      If a SD card found, all parameters will be read the SD card.
 *      If you want to hardcode ssid, password and other parameters,
 *      do not insert the SD card.
 */
#ifndef WIFI_SSID
#define WIFI_SSID           ""
#endif
#ifndef WIFI_PASS
#define WIFI_PASS           ""
#endif
#ifndef BITCOIN_RPC_HOST                               // e.g. 192.168.1.21
#define BITCOIN_RPC_HOST    "BITCOIN-CORE-IP-ADDESS"
#endif
#ifndef BITCOIN_RPC_PORT                               // rpc port number, usually 8332 for mainnet, 18332 for testnet
#define BITCOIN_RPC_PORT    "8332"
#endif
#ifndef BITCOIN_RPC_AUTH                               // username and password in "user:password" format
#define BITCOIN_RPC_AUTH    "user:password"
#endif
#ifndef NTP_SERVER                                     // use "pool.ntp.org" or your local time server
#define NTP_SERVER          "pool.ntp.org"
#endif
#ifndef GMT_OFFSET
#define GMT_OFFSET          "7"
#endif
#ifndef DAYLIGHT_OFFSET
#define DAYLIGHT_OFFSET     "0"
#endif

std::map<String, String> MyConfig::configMap {
    {"WIFI_SSID", WIFI_SSID},
    {"WIFI_PASS", WIFI_PASS},

    {"BITCOIN_RPC_HOST", BITCOIN_RPC_HOST},
    {"BITCOIN_RPC_PORT", BITCOIN_RPC_PORT},
    {"BITCOIN_RPC_AUTH", BITCOIN_RPC_AUTH},

    {"NTP_SERVER",      NTP_SERVER},
    {"GMT_OFFSET",      GMT_OFFSET},
    {"DAYLIGHT_OFFSET", DAYLIGHT_OFFSET}
};


void MyConfig::readConfigFromSD(fs::FS &fs, const char *path) {
    File file = fs.open(path);
    if (!file) {
        Serial.printf("Failed to open file %s for reading\n", path);
        return;
    }

    String key, value;
    String line;
    const int MAX_LINES = 32;
    int count = 0;
    while (file.available()) {
        if (count > MAX_LINES)
            break;

        count++;

        line = file.readStringUntil('\n');
        line.trim();
        if (line[0] == '#' || line.length() == 0)
            continue;

        //Serial.println(line);
        int p2 = line.indexOf('=', 0);
        if (p2 >= 1) {
            key = line.substring(0, p2);
            value = line.substring(p2+1, line.length());

            key.trim();
            value.trim();
            if (MyConfig::configMap.find(key) != MyConfig::configMap.end()) {
                MyConfig::configMap[key] = value;
                //Serial.printf("[%d] %s=%s\n", count, key.c_str(), value.c_str());
            } else {
                Serial.printf("Config file %s error line: %d -- %s unknown option!\n", path, count, key.c_str());
            }
         } else {
             Serial.printf("Config file %s error line: %d\n", path, count);
         }
    }
    file.close();
}
