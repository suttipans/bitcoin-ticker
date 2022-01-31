#ifndef MY_CONFIG_H
#define MY_CONFIG_H

#include <WString.h>
#include <map>
#include "FS.h"

extern String DEVICE_ID;

class MyConfig {
public:
    static String getWifiSSID()     { return configMap["WIFI_SSID"]; }
    static String getWifiPassword() { return configMap["WIFI_PASS"]; }

    static String getBitcoinRpcHost() { return configMap["BITCOIN_RPC_HOST"]; }
    static long getBitcoinRpcPort()   { return configMap["BITCOIN_RPC_PORT"].toInt(); }
    static String getBitcoinRpcAuth() { return configMap["BITCOIN_RPC_AUTH"]; }

    static String getNtpServer()    { return configMap["NTP_SERVER"]; }
    static long getGmtOffset()      { return configMap["GMT_OFFSET"].toInt()*60*60;  }
    static long getDayLightOffset() { return configMap["DAYLIGHT_OFFSET"].toInt()*60*60; }

    static void readConfigFromSD(fs::FS &fs, const char *path);
    static std::map<String, String> configMap;
};

#endif
