
#include <cstdio>
#include "miosix.h"
#include <interfaces-impl/vht.h>
#include <interfaces-impl/transceiver.h>
#include <interfaces-impl/transceiver_timer.h>
#include <interfaces-impl/power_manager.h>
#define ANCHORS 1
#define SLEEP 50
#define ANCH_DELAY 1000000 //in ns = 1ms
#define RECV_TIMEOUT 10000000 //in ns = 10ms

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

    unsigned char sendpkt[10];
    memset(sendpkt,0,sizeof(sendpkt));
    unsigned char recvpkt[125];

    long long int start_ns = 0, start_tick = 0;
    long long int anchor_delay = tc.ns2tick(ANCH_DELAY);

    RecvResult result;

    float tof = 0.f;
    const float alpha = 0.9f;
    vht.stopResyncSoft();

    while (true) {
        for(int i=1; i<=ANCHORS; i++)
        {
            // Send packet
            printf("Sending to anchor %d. ", i);
            sendpkt[0] = i;  //select anchor saving the number in the payload
            start_ns = getTime();
            start_tick = tc.ns2tick(start_ns);
            start_tick += 30000000;
            //vht.stopResyncSoft();
            transceiver.sendAt(sendpkt, sizeof(sendpkt), start_tick, Transceiver::Unit::TICK);
            ledOn();
            try {
                result = transceiver.recv(recvpkt, sizeof(recvpkt), start_tick+tc.ns2tick(RECV_TIMEOUT), Transceiver::Unit::TICK);
                //vht.startResyncSoft();
                ledOff();
            } catch (exception& e) {
                printf("Exception: %s\n", e.what());
            }
            if(recvpkt[0] == i && result.timestampValid && result.error==RecvResult::OK)
            {
                printf("received response, rtt=%lld, ", result.timestamp - start_tick - anchor_delay);
                tof = alpha * tof + (1.0f - alpha) * max(static_cast<float>(result.timestamp - start_tick - anchor_delay)/2.0f, 0.f);
                printf("tof (tick)=%5.1f (m)=%5.1f, ", tof, tof / static_cast<float>(EFM32_HFXO_FREQ) * 299702561);
                printf("rssi=%d\n", result.rssi);
                //299702561 m/s
            }
            else if(result.error!=RecvResult::OK)
            {
                printf("receive error: %d\n", result.error);
            }
            else if(!result.timestampValid)
            {
                printf("timestamp not valid\n");
            }
            else
            {
                printf("wrong response\n");
            }
            //memDump(recvpkt,result.size);
        }
    Thread::sleep(SLEEP);
    }
}
