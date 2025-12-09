#ifndef _FILERECEIVER_H_
#define _FILERECEIVER_H_

#include <string.h>
#include <fstream>
#include <cmath>
#include <iostream>
#include <vector>

#include <omnetpp.h>
#include <inet/common/INETDefs.h>
#include <inet/transportlayer/contract/tcp/TcpSocket.h> // TCP
#include <inet/transportlayer/contract/tcp/TcpCommand_m.h>
#include <inet/networklayer/common/L3AddressResolver.h>
#include <inet/common/packet/Packet.h>

#include "FilePacket_m.h" // data format

#define WINDOW_SIZE 0.1 // throughput calculates every 0.1sec

class FileReceiver : public omnetpp::cSimpleModule, public inet::TcpSocket::ICallback
{
    inet::TcpSocket socket;
    inet::TcpSocket _data_socket;     // 실제 데이터 주고받는 소켓

    bool _data_socket_flag = false;     // 데이터 소켓이 준비됐는지 여부

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

    // TCP callback function
    virtual void socketDataArrived(inet::TcpSocket *socket, inet::Packet *packet, bool urgent) override;
    virtual void socketAvailable(inet::TcpSocket *socket, inet::TcpAvailableInfo *availableInfo) override;
    virtual void socketEstablished(inet::TcpSocket *socket) override {}
    virtual void socketPeerClosed(inet::TcpSocket *socket) override {}
    virtual void socketClosed(inet::TcpSocket *socket) override {}
    virtual void socketFailure(inet::TcpSocket *socket, int code) override {}
    virtual void socketStatusArrived(inet::TcpSocket *socket, inet::TcpStatusInfo *status) override {}
    virtual void socketDeleted(inet::TcpSocket *socket) override {}
};

#endif
