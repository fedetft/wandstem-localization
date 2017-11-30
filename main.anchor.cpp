
#include <cstdio>
#include "miosix.h"
#include <interfaces-impl/vht.h>
#include <interfaces-impl/transceiver.h>
#include <interfaces-impl/transceiver_timer.h>
#include <interfaces-impl/power_manager.h>
#define DELAY 1000000 //in ns = 1ms

using namespace std;
using namespace miosix;

int main()
{
    //auto& power_manager = PowerManager::instance();
    TimeConversion tc(EFM32_HFXO_FREQ);
    auto& vht = VHT::instance();
    auto& transceiver = Transceiver::instance();
    transceiver.configure(TransceiverConfiguration(2450,5)); //5dBm
    transceiver.turnOn();

    unsigned char ID = 1;
    long long int wait_time = tc.ns2tick(DELAY); //1ms

    unsigned char recvpkt[125];

    RecvResult result;

    printf("I am an anchor, my ID is %d\n", ID);
    printf("ready to receive\n");

    vht.stopResyncSoft();
    while (true) {
        try {
            result = transceiver.recv(recvpkt, sizeof(recvpkt), infiniteTimeout, Transceiver::Unit::TICK, HardwareTimer::Correct::CORR);
            ledOn();
        } catch (exception& e) {
            printf("Exception: %s\n", e.what());
        }
        if(recvpkt[0] == ID) {
            long long int send_time = result.timestamp + wait_time;
            transceiver.sendAt(recvpkt, result.size, send_time, Transceiver::Unit::TICK);
            ledOff();
            printf("Received packet @ %lld\n", result.timestamp);
            printf("Replying after %lld\n ticks", wait_time);
        }
    }
}
