
#include <cstdio>
#include "miosix.h"
#include <interfaces-impl/transceiver.h>
#include <interfaces-impl/transceiver_timer.h>
#include <interfaces-impl/power_manager.h>

using namespace std;
using namespace miosix;

int main()
{
    //auto& power_manager = PowerManager::instance();
    auto& transceiver = Transceiver::instance();
    transceiver.configure(TransceiverConfiguration());
    transceiver.turnOn();

    unsigned char ID = 1;
    long long int wait_time = 1000000;

    unsigned char recvpkt[125];

    long long int start_ts = 0;

    RecvResult result;

    printf("I am an anchor, my ID is %d\n", ID);
    printf("ready to receive\n");

    while (true) {
        try {
            result = transceiver.recv(recvpkt, sizeof(recvpkt), infiniteTimeout);
            ledOn();
        } catch (exception& e) {
            printf("Exception: %s\n", e.what());
        }
        if(recvpkt[0] == ID) {
            auto send_time = result.timestamp + wait_time;
            transceiver.sendAt(recvpkt, result.size, send_time);
            ledOff();
            printf("Received packet @ %lld\n", result.timestamp);
            printf("Replying after %lld\n", wait_time);
        }
    }
}
