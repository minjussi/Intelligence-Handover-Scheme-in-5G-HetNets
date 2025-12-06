#include <inet/common/INETDefs.h>
#include <inet/transportlayer/contract/udp/UdpSocket.h> // UDP
#include <inet/networklayer/common/L3AddressResolver.h>

#include "FilePacket_m.h" // data format

#define WINDOW_SIZE 0.1 // throughput calculates every 0.1sec

class FileReceiver : public omnetpp::cSimpleModule
{
    inet::UdpSocket socket;

    unsigned long _rx_count = 0;
    unsigned long _rx_byte_sum_window = 0;
    unsigned long _total_rx_bytes = 0;

    unsigned long _tx_seq_id = 0;
    unsigned long _expected_seq_id = 0;

    unsigned long _rx_seq_id = 0;
    unsigned long _loss_seq_id_cnt = 0;

    omnetpp::SimTime _time_delay_sum = 0;
    omnetpp::SimTime _time_packet_last = 0;
    omnetpp::SimTime _time_last_inst_tp_cal = 0;
    omnetpp::SimTime _time_window_size = WINDOW_SIZE;
    omnetpp::SimTime _before_rx_time;

    // Result file
    unsigned int _temp_str_counter = 0;
    std::ostringstream _temp_str;
    std::ofstream _result_csv;
    
protected:
    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    void initialize(int stage) override;
    // when packet is arrived, handleMessage is executed
    void handleMessage(omnetpp::cMessage *msg) override;
    void finish() override;
};

#endif
