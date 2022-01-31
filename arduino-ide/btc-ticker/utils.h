#ifndef UTILS_H
#define UTILS_H

#include <time.h>
#include <WString.h>

#include "qrencoder.h"

String localtimeString();
String timeToString(const time_t &t);

uint32_t getChipId();
String getDeviceId();
String getWifiStatus();
struct tm myGetLocaltime();
struct tm configLocaltime();

typedef QREncoder<6, ECC_MEDIUM> qr_version_t;
void drawQRCode(const qr_version_t &qrn, const String &s = "");
void drawQRCodeText(const qr_version_t &qrn);

void displayBtcAddressQr();
void displayBalances();

String thousandSeparator(int n);

void wifiConnect();

#endif
