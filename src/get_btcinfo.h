#ifndef GET_BTCINFO_H
#define GET_BTCINFO_H

#include <WString.h>

class BtcInfo {
   public:
    String local_stime;
    String utc_stime;
    String btcthb_price;
    String btcusd_price;
    String block_height;
    String block_age;
    String mempool_blk;
    String unconfirmed_txs;
    String smart_feerates;
    unsigned long tp = 0;
};


BtcInfo getBitcoinNodeInfo();
void task_get_btcinfo(void* param);

#endif
