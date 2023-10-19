#ifndef STOPWAIT_GBNRDTRECIEVER_H
#define STOPWAIT_GBNRDTRECIEVER_H

#include "RdtReceiver.h"
#include "Global.h"

class GBNRdtReceiver: public RdtReceiver{
private:
    int expected_seqnum;
    Packet last_ack_pkt;

public:
    GBNRdtReceiver();
    virtual ~GBNRdtReceiver();

public:

    void receive(const Packet &packet);
};


#endif //STOPWAIT_GBNRDTRECIEVER_H
