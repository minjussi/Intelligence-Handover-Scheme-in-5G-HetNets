#ifndef _FILESENDER_H_
#define _FILESENDER_H_

#include <string.h>
#include <omnetpp.h>

#include <inet/common/INETDefs.h>
#include <inet/transportlayer/contract/tcp/TcpSocket.h> // TCP
#include <inet/transportlayer/contract/tcp/TcpCommand_m.h>
#include <inet/networklayer/common/L3AddressResolver.h>
#include <inet/common/packet/Packet.h>

#include "FilePacket_m.h"

using namespace inet;

class FileSender : public omnetpp::cSimpleModule, public inet::TcpSocket::ICallback
{
    inet::TcpSocket _socket;
    
    // Self messages
    cMessage* _self_msg;
    
    // Configuration parameters
    int _local_port;
    int _dest_port;
    inet::L3Address _dest_address;

    unsigned int _total_frames;
    unsigned int _current_frame;
    int _payload_size; // one frame byte size
    
  public:
    FileSender();
    virtual ~FileSender();

  protected:
    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    void initialize(int stage) override;
    void handleMessage(omnetpp::cMessage *msg) override;
    void finish() override;
    void sendPacket();
    
    // Callback function
    virtual void socketDataArrived(inet::TcpSocket *socket, inet::Packet *packet, bool urgent) override;
    virtual void socketAvailable(inet::TcpSocket *socket, inet::TcpAvailableInfo *availableInfo) override {}

    // Connection Established
    virtual void socketEstablished(inet::TcpSocket *socket) override;
    virtual void socketPeerClosed(inet::TcpSocket *socket) override;
    virtual void socketClosed(inet::TcpSocket *socket) override {}
    virtual void socketFailure(inet::TcpSocket *socket, int code) override {}
    virtual void socketStatusArrived(inet::TcpSocket *socket, inet::TcpStatusInfo *status) override {}
    virtual void socketDeleted(inet::TcpSocket *socket) override {}
};

#endif
