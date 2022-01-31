#ifndef RPC_CALLS_H
#define RPC_CALLS_H

#include <WString.h>

class Bitkub {
public:
    static int getBtcPrice(String &json);
    static int getBtcPrice(double &price, double &high, double &low, double &open);
};


class BitStamp {
public:
    static int getBtcPrice(String &json);
    static int getBtcPrice(double &price, double &high, double &low, double &open);
};


struct BitcoinNodeInfo {
	String hash;
	uint32_t height;
	uint32_t time;
	uint32_t mempool_size;
	uint32_t unconfirmed_txs;
	double feerate_1blk;
	double feerate_6blk;
	double feerate_144blk;
	BitcoinNodeInfo() : hash(""), height(0), time(0),
		mempool_size(0), unconfirmed_txs(0),
		feerate_1blk(0.0), feerate_6blk(0.0), feerate_144blk(0.0) {}
};


class BitcoinCore {
public:
    static String getBitcoinFullNodeUrl();
    static int getbestblockhash_json(String &json);
    static int getblockstats_json(const String &hash, String &json);
    static int getmempoolinfo_json(String &json);
    static int estimatesmartfee_json(int nblocks, String &json);

    static int getblockchaininfo_json(String &blockchaininfo);
    static int getnewaddress(String &newaddress);
    static int getnewaddress_json(String &newaddress);
    static int getbalances(double &avaliable, double &pending, double &total);
    static int getbalances_json(String &balances);


    static int getnodeinfo(BitcoinNodeInfo &node_info);

    static int listreceivebyaddress(String &json);
    static int listtransactions(String &json);
};

#endif
