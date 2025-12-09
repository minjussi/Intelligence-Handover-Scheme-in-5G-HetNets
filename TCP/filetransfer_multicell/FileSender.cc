#include <cmath>
#include <inet/common/TimeTag_m.h>
#include "FileSender.h"

Define_Module(FileSender);
using namespace inet;
using namespace omnetpp;

FileSender::FileSender()
{
    _self_msg = nullptr;
}

FileSender::~FileSender()
{
    cancelAndDelete(_self_msg);
}

void FileSender::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) // Initialize
    {
        _self_msg = new cMessage("sendTimer");
        _local_port = par("localPort");
        _dest_port = par("destPort");

        _payload_size = par("payloadSize");
        _total_frames = par("nFrames");
        _current_frame = 0;

        EV_INFO << "FileSender initialized. Total packets: " << _total_frames << ", Payload: " << _payload_size << "B" << endl;
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) // socket setup & connect
    {
        const char *destAddrs = par("destAddress");
        _dest_address = L3AddressResolver().resolve(destAddrs);
        _socket.setOutputGate(gate("socketOut"));
        _socket.setCallback(this); // callback function
        if (_local_port != -1)
        {
            _socket.bind(_local_port);
        }

        EV_INFO << ">>> [SENDER] Connecting to: " << _dest_address << " : " << _dest_port << endl;
        // Try connection first
        _socket.connect(_dest_address, _dest_port);
    }
}

void FileSender::handleMessage(cMessage *msg)
{
    // 1. Data Transmission
    if (msg->isSelfMessage())
    {
        if (_current_frame < _total_frames)
        {
            sendPacket();
            simtime_t interval = par("samplingTime");
            scheduleAt(simTime() + interval, _self_msg);
        }
        else
        {
            // if the transmission end, socket close
            if (_socket.isOpen())
            {
                _socket.close();
                cancelAndDelete(_self_msg);
                _self_msg = nullptr;
            }
        }
    }

    // 2. Connection Established or ACK
    else
    {
        _socket.processMessage(msg);
    }
}

void FileSender::socketEstablished(inet::TcpSocket *socket)
{
    EV_INFO << ">>> [SENDER] TCP Connection SUCCESS! Starting Timer..." << endl;

    SimTime start_time = par("startTime");
    scheduleAt(simTime() + start_time, _self_msg);
}

void FileSender::sendPacket()
{
    Packet* packet = new Packet("File");
    auto fileData = makeShared<FilePacket>();

    fileData->setNframes(_total_frames);
    fileData->setIDframe(_current_frame++);
    fileData->setPayloadTimestamp(simTime());
    fileData->setPayloadSize(_payload_size);
    fileData->setChunkLength(B(_payload_size));
    
    packet->insertAtBack(fileData);

    _socket.send(packet);
}

void FileSender::socketDataArrived(inet::TcpSocket *socket, inet::Packet *packet, bool urgent)
{
    delete packet;
}

void FileSender::socketPeerClosed(inet::TcpSocket *socket)
{
    if (_socket.isOpen())
    {
        _socket.close();
    }
}

void FileSender::finish()
{
    recordScalar("Total Frames Sent", _current_frame);
}
