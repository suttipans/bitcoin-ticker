/**
 * @file my_webserver.cpp
 * @author Suttipan Sittirak (suttipan.sittirak@gmail.com)
 * @brief Serving Web requests
 * @version 0.1
 * @date 2022-01-12
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "M5.h"
#include <WebServer.h>
#include <ArduinoJson.h>

#include "my_webserver.h"
#include "rpc_calls.h"
#include "qrencoder.h"
#include "utils.h"

WebServer server(80);

static void handle_root() {
    String html =
        "<html>"
        "<meta http-equiv='refresh' content='10'/>"
        "<title>BtcTicker</title>"
        "<body>";
    html += "<a href='/'><b>Bitcoin Ticker</b></a><br>";
    html += localtimeString() + "<br><br>";

    html += "<a href='/getnewaddress'>Create new receiving address</a><br>";
    html += "<a href='/about'>About</a><br>";
    html +=
        "</body>"
        "</html>";

    server.send(200, "text/html", html);
}


static void handle_about() {
    String html =
        "<html>"
        "<title>BtcTicker - About</title>"
        "<body>";
    html += "<a href='/'><b>Bitcoin Ticker</b></a><br>";
    html += localtimeString() + "<br>";
    html += "<pre>" + getDeviceId() + "</pre>";
    html += "<pre>" + getWifiStatus() + "</pre>";

    html +=
        "</body>"
        "</html>";

    server.send(200, "text/html", html);
}


static void handle_getnewaddress() {
    String newaddress;
    DynamicJsonDocument jsondoc(256);
    qr_version_t qrn;

    if (BitcoinCore::getnewaddress(newaddress) == 0) {

        qrn.encode(newaddress);
        String qr;
        for (uint8_t y = 0; y < qrn.size(); ++y) {
            for (uint8_t x = 0; x < qrn.size(); ++x) {
                qr += (qrn.getmodule(x, y) ? "&#x2588;&#x2588;" : "&nbsp;&nbsp;");
            }
            qr += "<br>";
        }

        String html =
            "<html>"
            "<head><style>div.qrcode { line-height: 86%; text-align: center; margin: auto; width: 90%; padding: 10px; }</style></head>"
            "<title>BtcTicker - New receiving address</title>"
            "<body>"
            "<div class='qrcode'>";
        html += "<a href='/'><b>Bitcoin Ticker</b></a><br>";
        html += localtimeString() + "<br><br>";
        html += "<code>" + qr + "</code><br>";
        html += "<code>" + newaddress + "</code><br>";
        html += "</div></body></html>";

        server.send(200, "text/html", html);
    } else {
        server.send(400, "text/html", "Bad Request");
    }
}


static void my_setup() {
    server.on("/", handle_root);
    server.on("/about", handle_about);
    server.on("/getnewaddress", handle_getnewaddress);
    server.on("/setup", []() { server.send(400, "text/plain", "coming soon.."); });
    server.onNotFound([]() { server.send(404, "text/html", "Not Found"); });
    server.begin();

    M5.Lcd.print("HTTP server started\n");
}


void task_my_webserver(void* param) {
    my_setup();

    while (1) {
        server.handleClient();
        yield();
        delay(10);
    }
}