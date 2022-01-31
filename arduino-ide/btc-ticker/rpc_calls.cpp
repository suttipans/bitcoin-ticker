/**
 * @file rpc_calls.cpp
 * @author Suttipan Sittirak (suttipan.sittirak@gmail.com)
 * @brief Rpc calls to get prices from hosts, rpc calls to Bitcoin fullnode.
 * @version 0.1
 * @date 2022-01-18
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <base64.h>

#include "rpc_calls.h"
#include "my_config.h"

extern String DEVICE_ID;

// Bitkup API host
const String BITKUB_API_HOST = "https://api.bitkub.com";
const String BITKUB_API_KEY = "";
const String BITKUB_API_SECRET = "";

// Bitstap API host
const String BITSTAMP_API_HOST = "https://www.bitstamp.net";

/**
 * @brief A class wrapper around HTTPClient
 *
 */
class HttpRequest : public HTTPClient {
public:
    HttpRequest() { }

    HttpRequest(const String &url) {
        begin(url);
    }

    virtual ~HttpRequest() {
        httpc.end();
    }

    bool begin(const String &url) {
        return httpc.begin(url);
    }

    int get(String &t) {
        int rc = httpc.GET();
        if (rc > 0) {
            if (rc == HTTP_CODE_OK) {
                t = httpc.getString();
                return rc;
            } else {
                String err = httpc.getString();
                Serial.printf("HTTP status: %d -- %s\n", rc, err.c_str());
            }
        } else {
            String err = httpc.errorToString(rc);
            Serial.printf("Error HTTP Post: %d -- %s\n", rc, err.c_str());
        }

        return 1;
    }

    int post(const String& data, String &t) {
        int rc = httpc.POST(data);
        if (rc > 0) {
            if (rc == HTTP_CODE_OK) {
                t = httpc.getString();
                return rc;
            } else {
                String err = httpc.getString();
                Serial.printf("HTTP status: %d -- %s\n", rc, err.c_str());
            }
        } else {
            String err = httpc.errorToString(rc);
            Serial.printf("Error HTTP Post: %d -- %s\n", rc, err.c_str());
        }

        return 1;
    }

private:
    HTTPClient httpc;
};


int Bitkub::getBtcPrice(double &price, double &high, double &low, double &open) {
    StaticJsonDocument<1024> jsondoc;
    String bitkub_json;
    if (Bitkub::getBtcPrice(bitkub_json) != 200) {
        Serial.printf("Error: get_btc_price THB\n");
        return 1;
    }
    if (deserializeJson(jsondoc, bitkub_json) != DeserializationError::Ok) {
        return 1;
    }

    price     = jsondoc["THB_BTC"]["last"];
    low       = jsondoc["THB_BTC"]["low24hr"];
    high      = jsondoc["THB_BTC"]["high24hr"];
    open      = jsondoc["THB_BTC"]["prevClose"];
    return 0;
}

int Bitkub::getBtcPrice(String &json) {
    HttpRequest req(BITKUB_API_HOST + "/api/market/ticker?sym=THB_BTC");
    req.addHeader("Content-Type", "text/plain");
    int rc = req.get(json);
    return rc;
}


int BitStamp::getBtcPrice(double &price, double &high, double &low, double &open) {
    StaticJsonDocument<1024> jsondoc;
    String bitstamp_json;
    if (BitStamp::getBtcPrice(bitstamp_json) != 200) {
        Serial.printf("Error: get_btc_price USD\n");
        return 1;
    }
    if (deserializeJson(jsondoc, bitstamp_json) != DeserializationError::Ok) {
        return 1;
    }

    price = jsondoc["last"];
    low  = jsondoc["low"];
    high = jsondoc["high"];
    open = jsondoc["open"];

    return 0;
}


int BitStamp::getBtcPrice(String &json) {
    HttpRequest req(BITSTAMP_API_HOST + "/api/v2/ticker/btcusd");
    req.addHeader("Content-Type", "text/plain");
    int rc = req.get(json);
    return rc;
}


String BitcoinCore::getBitcoinFullNodeUrl() {
    return String("http://") + MyConfig::getBitcoinRpcAuth() +"@" +
           MyConfig::getBitcoinRpcHost() + ":" + String(MyConfig::getBitcoinRpcPort());
}


int BitcoinCore::getbestblockhash_json(String &json) {
    String data = R"({"jsonrpc": "1.0", "id": ")" + DEVICE_ID +
                  R"(", "method": "getbestblockhash"})";

    String url = getBitcoinFullNodeUrl();
    HttpRequest req(url);
    req.addHeader("Content-Type", "text/plain");
    int rc = req.post(data, json);
    return rc;
}


int BitcoinCore::getblockstats_json(const String& hash, String &json) {
    String data = R"({"jsonrpc": "1.0", "id": ")" + DEVICE_ID +
                    R"(", "method": "getblockstats", "params": [")" + hash + R"("] })";
    String url = getBitcoinFullNodeUrl();
    HttpRequest req(url);
    req.addHeader("Content-Type", "text/plain");
    int rc = req.post(data, json);
    return rc;
}


int BitcoinCore::getmempoolinfo_json(String &json) {
    String data = R"({"jsonrpc": "1.0", "id": ")" + DEVICE_ID +
                    R"(", "method": "getmempoolinfo"})";
    String url = getBitcoinFullNodeUrl();
    HttpRequest req(url);
    req.addHeader("Content-Type", "text/plain");
    int rc = req.post(data, json);
    return rc;
}

int BitcoinCore::estimatesmartfee_json(int nblocks, String &json) {
    String data = R"({"jsonrpc": "1.0", "id": ")" +  DEVICE_ID +
                    R"(", "method": "estimatesmartfee", "params": [)"
                        + String(nblocks)
                        + R"(]})";
    String url = getBitcoinFullNodeUrl();
    HttpRequest req(url);
    req.addHeader("Content-Type", "text/plain");
    int rc = req.post(data, json);
    return rc;
}


int BitcoinCore::getblockchaininfo_json(String &json) {
    String data = R"({"jsonrpc": "1.0", "id": ")" +  DEVICE_ID +
                    R"(", "method": "getblockchaininfo", "params": []})";
    String url = getBitcoinFullNodeUrl();
    HttpRequest req(url);
    req.addHeader("Content-Type", "text/plain");
    int rc = req.post(data, json);
    return rc;
}


int BitcoinCore::getnewaddress(String& newaddress) {
    String json;
    StaticJsonDocument<512> jsondoc;

    if (getnewaddress_json(json) == 200) {
        DeserializationError error = deserializeJson(jsondoc, json);
        if (!error) {
            String serror = jsondoc["error"];
            if (serror == "null") {
                newaddress = String((const char *)jsondoc["result"]);
                return 0;
            }
        }
    }

    return 1;
}


int BitcoinCore::getnewaddress_json(String &json) {
    String data = R"({"jsonrpc": "1.0", "id": ")" +  DEVICE_ID +
                    R"(", "method": "getnewaddress", "params": ["", "bech32"]})";
    String url = getBitcoinFullNodeUrl();
    HttpRequest req(url);
    req.addHeader("Content-Type", "text/plain");
    int rc = req.post(data, json);
    return rc;
}


int BitcoinCore::getbalances(double &avaliable, double &pending, double &total) {
    String json;
    StaticJsonDocument<512> jsondoc;

    if (BitcoinCore::getbalances_json(json) == 200) {
        DeserializationError error = deserializeJson(jsondoc, json);
        if (!error) {
            avaliable = jsondoc["result"]["mine"]["trusted"];
            pending   = jsondoc["result"]["mine"]["untrusted_pending"];
            total     = jsondoc["result"]["mine"]["immature"];

            return 0;
        }
    }

    return 1;
}


int BitcoinCore::getbalances_json(String &json) {
    String data = R"({"jsonrpc": "1.0", "id": ")" +  DEVICE_ID +
                    R"(", "method": "getbalances", "params": []})";
    String url = getBitcoinFullNodeUrl();
    HttpRequest req(url);
    req.addHeader("Content-Type", "text/plain");
    int rc = req.post(data, json);
    return rc;
}


int BitcoinCore::getnodeinfo(BitcoinNodeInfo &node_info) {
    String blockhash;
    uint32_t block_height;
    uint32_t block_time;
	uint32_t mempool_size;
	uint32_t unconfirmed_txs;
    double feerate_1blk, feerate_6blk, feerate_144blk;

    String response;
    DynamicJsonDocument jsondoc(2048);

    if (BitcoinCore::getbestblockhash_json(response) != 200) {
        return 1;
    }
    if (deserializeJson(jsondoc, response) != DeserializationError::Ok) {
        return 1;
    }
    if (jsondoc["error"] != nullptr) {
        return 1;
    }
    //Serial.printf("getbestblockhash response: [%s]\n", response.c_str());
    blockhash = String((const char *)jsondoc["result"]);

    if (BitcoinCore::getblockstats_json(blockhash, response) != 200) {
        return 1;
    }
    if (deserializeJson(jsondoc, response) != DeserializationError::Ok) {
        return 1;
    }
    if (jsondoc["error"] != nullptr) {
        return 1;
    }
    //Serial.printf("getblockstats response: [%s]\n", response.c_str());
    block_height = uint32_t(jsondoc["result"]["height"]);
    block_time   = uint32_t(jsondoc["result"]["time"]);

    if (BitcoinCore::getmempoolinfo_json(response) != 200) {
        return 1;
    }
    if (deserializeJson(jsondoc, response) != DeserializationError::Ok) {
        return 1;
    }
    if (jsondoc["error"] != nullptr) {
        return 1;
    }
    //Serial.printf("getmempoolinfo response: [%s]\n", response.c_str());
    mempool_size   = jsondoc["result"]["bytes"];
	unconfirmed_txs = jsondoc["result"]["size"];

    if (BitcoinCore::estimatesmartfee_json(1, response) != 200) {
        return 1;
    }
    if (deserializeJson(jsondoc, response) != DeserializationError::Ok) {
        return 1;
    }
    if (jsondoc["error"] != nullptr) {
        return 1;
    }
    //Serial.printf("estimatesmartfee 1 response: [%s]\n", response.c_str());
    feerate_1blk = double(jsondoc["result"]["feerate"]) * 100000.0;

    if (BitcoinCore::estimatesmartfee_json(6, response) != 200) {
        return 1;
    }
    if (deserializeJson(jsondoc, response) != DeserializationError::Ok) {
        return 1;
    }
    if (jsondoc["error"] != nullptr) {
        return 1;
    }
    //Serial.printf("estimatesmartfee 6 response: [%s]\n", response.c_str());
    feerate_6blk = double(jsondoc["result"]["feerate"]) * 100000.0;


    if (BitcoinCore::estimatesmartfee_json(144, response) != 200) {
        return 1;
    }
    if (deserializeJson(jsondoc, response) != DeserializationError::Ok) {
        return 1;
    }
    if (jsondoc["error"] != nullptr) {
        return 1;
    }
    //Serial.printf("estimatesmartfee 144 response: [%s]\n", response.c_str());
    feerate_144blk = double(jsondoc["result"]["feerate"]) * 100000.0;

    node_info.hash   = blockhash;
    node_info.height = block_height;
    node_info.time   = block_time;
    node_info.mempool_size   = mempool_size;
    node_info.unconfirmed_txs = unconfirmed_txs;
    node_info.feerate_1blk   = feerate_1blk;
    node_info.feerate_6blk   = feerate_6blk;
    node_info.feerate_144blk = feerate_144blk;

    return 0;
}


int BitcoinCore::listreceivebyaddress(String &json) {
    return 0;
}


int BitcoinCore::listtransactions(String& json) {
    return 0;
}
