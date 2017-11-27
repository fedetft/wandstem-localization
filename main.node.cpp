
#include <cstdio>
#include "miosix.h"
#include <interfaces-impl/transceiver.h>
#include <interfaces-impl/transceiver_timer.h>
#include <interfaces-impl/power_manager.h>
#define ANCHORS 1
#define SLEEP 500

using namespace std;
using namespace miosix;

int main()
{
    //auto& power_manager = PowerManager::instance();
    auto& transceiver = Transceiver::instance();
    transceiver.configure(TransceiverConfiguration());
    transceiver.turnOn();

    unsigned char sendpkt[10];
    memset(sendpkt,0,sizeof(sendpkt));
    unsigned char recvpkt[125];

    long long int start_ts = 0;

    RecvResult result;

    while (true) {
        for(int i=1; i<=ANCHORS; i++)
        {
            // Send packet
            sendpkt[0] = i;  //select anchor saving the number in the payload
            start_ts = getTime();
            start_ts += 30000000;
            transceiver.sendAt(sendpkt, sizeof(sendpkt), start_ts, Transceiver::Unit::NS);
            //printf("packet sent to anchor %c @ %lld\n", i, start_ts);
            ledOn();
            try {
                result = transceiver.recv(recvpkt, sizeof(recvpkt), infiniteTimeout, Transceiver::Unit::NS);
                ledOff();
            } catch (exception& e) {
                printf("Exception: %s\n", e.what());
            }
            if(recvpkt[0] == i && result.timestampValid)
            {
                printf("received response @ %lld\n", result.timestamp);
                printf("delta = %lld\n", result.timestamp - start_ts);
            }
            else if(!result.timestampValid)
            {
                printf("timestamp not valid\n");
            }
            else
            {
                printf("wrong response\n");
            }
            memDump(recvpkt,result.size);
        }
    Thread::sleep(SLEEP);
    }
}
