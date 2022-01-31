#include "M5.h"
#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <string>
#include <atomic>

#include "rpc_calls.h"
#include "utils.h"
#include "my_config.h"

String localtimeString() {
    struct tm tm = myGetLocaltime();
    char time_buf[64];
    snprintf(time_buf, sizeof(time_buf), "%04d-%02d-%02d %02d:%02d:%02d",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec);

    return String(time_buf);
}

String timeToString(const time_t &t) {
    String ts;
    int hh, mm, ss;
    long tt = t; //+7*60*60;  // adj. for Bankok timezone
    hh = (tt % 86400L) / 3600;
    mm = (tt % 3600) / 60;
    ss = tt % 60;

    ts = "";
    if (hh < 10) ts += '0';
    ts += String(hh) + ':';
    if (mm < 10) ts += '0';
    ts += String(mm) + ':';
    if (ss < 10) ts += '0';
    ts += String(ss);

    return ts;
}

uint32_t getChipId() {
    uint32_t chipId = 0;

    for (int i = 0; i < 17; i = i + 8) {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }

    return chipId;
}


String getDeviceId() {
    String text;
    char buf[64];
    uint32_t chipId = getChipId();

    text.reserve(256);
    snprintf(buf, sizeof(buf), "ESP32 Chip model: %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
    text += buf;
    snprintf(buf, sizeof(buf), "Number of cores: %d\n", ESP.getChipCores());
    text += buf;
    snprintf(buf, sizeof(buf), "Chip ID: %ud\n", chipId);
    text += buf;

    snprintf(buf, sizeof(buf), "RAM: %u\n", ESP.getHeapSize());
    text += buf;
    snprintf(buf, sizeof(buf), "PSRAM: %u\n", ESP.getPsramSize());
    text += buf;
    snprintf(buf, sizeof(buf), "Flash: %u\n", ESP.getFlashChipSize());
    text += buf;

    snprintf(buf, sizeof(buf), "SDK Version: %s\n", ESP.getSdkVersion());
    text += buf;
    snprintf(buf, sizeof(buf), "Compiler GCC version: %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
    text += buf;
    snprintf(buf, sizeof(buf), "Build time: %s\n", __TIMESTAMP__);
    text += buf;

    snprintf(buf, sizeof(buf), "Device ID: %s\n", DEVICE_ID.c_str());
    text += buf;

    return text;
}

String getWifiStatus() {
    String text;
    if (WiFi.isConnected()) {
        char buf[64];
        text.reserve(128);
        snprintf(buf, sizeof(buf), "WiFi connected -- SSID: %s\n", WiFi.SSID().c_str());
        text += buf;
        snprintf(buf, sizeof(buf), "IP address: %s\n", WiFi.localIP().toString().c_str());
        text += buf;
        snprintf(buf, sizeof(buf), "Signal strength (RSSI): %d dBm\n", WiFi.RSSI());
        text += buf;
    }
    return text;
}


struct tm configLocaltime() {
    static char nptServer[64];

    strncpy(nptServer, MyConfig::getNtpServer().c_str(), sizeof(nptServer));
    configTime(MyConfig::getGmtOffset(),
               MyConfig::getDayLightOffset(),
               nptServer);
    return myGetLocaltime();
}


struct tm tm_localtime;
SemaphoreHandle_t tMutex = xSemaphoreCreateMutex();

struct tm myGetLocaltime() {
    struct tm tm;
    xSemaphoreTake(tMutex, portMAX_DELAY);
    getLocalTime(&tm_localtime);
    tm = tm_localtime;
    xSemaphoreGive(tMutex);
    return tm;
}

void drawQRCode(const qr_version_t &qrn, const String &s) {
    M5.Lcd.clear();
    M5.Lcd.fillRoundRect(0, 0, M5.Lcd.width(), M5.Lcd.height(), 15, TFT_LIGHTGREY);

    const uint8_t rect_size = 4;
    uint16_t x_offset = (M5.Lcd.width() - qrn.size()*rect_size)/2;
    uint16_t y_offset = (M5.Lcd.height() - qrn.size()*rect_size)/2;
    for (uint8_t y = 0; y < qrn.size(); y++) {
        for (uint8_t x = 0; x < qrn.size(); x++) {
            if (qrn.getmodule(x, y)) {
                M5.Lcd.fillRect(x * rect_size + x_offset,
                               y * rect_size + y_offset,
                               rect_size, rect_size, TFT_BLACK);
            }
        }
    }
    if (!s.isEmpty()) {
        M5.Lcd.setTextFont(2);
        M5.Lcd.setTextDatum(CC_DATUM);
        M5.Lcd.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
        M5.Lcd.drawString("Receiving Address:", M5.Lcd.width() / 2, 10, 2);
        M5.Lcd.drawString(s, M5.Lcd.width() / 2, M5.Lcd.height() - M5.Lcd.fontHeight(2) * 2 + 10, 2);
    }
    M5.Lcd.display();
}


void drawQRCodeText(const qr_version_t &qrn) {
    for (uint8_t y = 0; y < qrn.size(); ++y) {
        for (uint8_t x = 0; x < qrn.size(); ++x) {
            Serial.print(qrn.getmodule(x, y) ? "\u2588\u2588" : "\u0020\u0020");
        }
        Serial.print("\n");
    }
}

void displayBtcAddressQr() {
    String newaddress;
    qr_version_t qrn;

    if (BitcoinCore::getnewaddress(newaddress) == 0) {
        qrn.encode(newaddress);
        //drawQRCodeText(qrn);
        drawQRCode(qrn, newaddress);
    }
}

void displayBalances() {
    double trusted, untrusted_pending, immature;

    if (BitcoinCore::getbalances(trusted, untrusted_pending, immature) == 0) {
        M5.Lcd.clear();
        M5.Lcd.fillRoundRect(0, 0, M5.Lcd.width(), M5.Lcd.height(), 15, TFT_LIGHTGREY);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextFont(4);
        M5.Lcd.setTextDatum(TL_DATUM);
        M5.Lcd.setTextColor(TFT_BLUE, TFT_LIGHTGREY);

        char buf[32];
        M5.Lcd.drawString("Balances: (BTC)", 10, 30);
        M5.Lcd.drawString("Avaliable:", 10, 60);
        snprintf(buf, sizeof(buf), "%.8f", trusted);
        M5.Lcd.drawString(buf, 130, 60);

        M5.Lcd.drawString("Pending:", 10, 90);
        snprintf(buf, sizeof(buf), "%.8f", untrusted_pending);
        M5.Lcd.drawString(buf, 130, 90);

        M5.Lcd.drawString("Total:", 10, 120);
        snprintf(buf, sizeof(buf), "%.8f", immature);
        M5.Lcd.drawString(buf, 130, 120);
    }
}

// Function to put thousands
// separators in the given integer
String thousandSeparator(int n) {
	std::string ans;

	// Convert the given integer
	// to equivalent string
	std::string num = String(n).c_str();

	// Initialise count
	int count = 0;

	// Traverse the string in reverse
	for (int i = num.size() - 1; i >= 0; i--) {
		count++;
		ans.push_back(num[i]);

		// If three characters
		// are traversed
		if (count == 3) {
			ans.push_back(',');
			count = 0;
		}
	}

	// Reverse the string to get
	// the desired output
	std::reverse(ans.begin(), ans.end());

	// If the given string is
	// less than 1000
	if (ans.size() % 4 == 0) {
		// Remove ','
		ans.erase(ans.begin());
	}

	return String(ans.c_str());
}


void wifiConnect() {
    Preferences preferences;
    bool no_password = true;
    String ssid = MyConfig::getWifiSSID();
    String password = MyConfig::getWifiPassword();

    if (ssid.isEmpty() && password.isEmpty()) {
        if (preferences.begin("btc-ticker", false)) {
            ssid = preferences.getString("WiFi-SSID:", "");
            password = preferences.getString("WiFi-Password:", "");
            M5.Lcd.println("Read SSID and password from EEPROM..");

            if (!ssid.isEmpty() && !password.isEmpty()) {
                //M5.Lcd.printf("SSID: %s  Password: %s\n", ssid, password);
                no_password = false;
            } else {
                M5.Lcd.println("SSID or Password not found!...");
                no_password = true;
            }
        }
    } else {
        no_password = false;
    }

    if (no_password && (ssid.isEmpty() || password.isEmpty())) {
        // Init WiFi as Station, start SmartConfig
        WiFi.mode(WIFI_AP_STA);
        WiFi.beginSmartConfig();

        // Wait for SmartConfig packet from mobile
        M5.Lcd.println(F("Waiting for SmartConfig."));
        while (!WiFi.smartConfigDone()) {
            M5.Lcd.print(".");
            delay(500);
        }
        M5.Lcd.println("");
        M5.Lcd.println(F("SmartConfig received."));
    } else {
        M5.Lcd.printf("Connecting to WiFi - SSID: %s .", ssid.c_str());
        WiFi.begin(ssid.c_str(), password.c_str());
    }

    while (WiFi.status() != WL_CONNECTED) {
        M5.Lcd.print(".");
        delay(500);
    }
    M5.Lcd.print("\n");
    M5.Lcd.print(getWifiStatus());

    if (no_password) {
        size_t len1 = preferences.putString("WiFi-SSID:", WiFi.SSID());
        size_t len2 = preferences.putString("WiFi-Password:", WiFi.psk());
        if (len1 == WiFi.SSID().length() && len2 == WiFi.psk().length()) {
            M5.Lcd.println(F("SSID and password saved into EERPOM."));
        }
    }
}
