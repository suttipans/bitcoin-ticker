/**
 * @file qrencoder.h
 * @author Suttipan Sittirak (suttipan.sittirak@gmail.com)
 * @brief Wrapper around qrcode
 * @version 0.1
 * @date 2022-01-18
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef QRENCODER_H
#define QRENCODER_H

#include "utility/qrcode.h"

template <uint8_t qr_version = 3, uint8_t err_corrention = ECC_LOW>
class QREncoder {
public:
    QREncoder() {}
    QREncoder(const String &s) {
        encode(s);
    }

    void encode(const String &s) {
        qrcode_initText(&qrcode, qrcodeData, qr_version, err_corrention, s.c_str());
    }

    bool getmodule(uint8_t x, uint8_t y) const {
        return qrcode_getModule(const_cast<QRCode *>(&qrcode), x, y);
    }

    uint8_t size() const {
        return qrcode.size;
    }

private:
    QRCode qrcode;
    uint8_t qrcodeData[((qr_version * 4 + 17)*(qr_version * 4 + 17) + 7) / 8];
};

#endif
