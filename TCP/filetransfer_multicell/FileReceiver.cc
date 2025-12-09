#include "FileReceiver.h"
#include <limits>
#include <inet/common/packet/Packet.h>

Define_Module(FileReceiver);

using namespace inet;
using namespace omnetpp;
using namespace std;

void FileReceiver::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) // Initialize
    {
        // variables initialize
        _rx_count = 0;                  // Number of successfully received packets
        _rx_byte_sum_window = 0;        // Received bytes in the current window
        _total_rx_bytes = 0;            // total bytes received

        _tx_seq_id = 0;                 // the received packet id
        _expected_seq_id = 0;           // the expected packet id to receive

        _rx_seq_id = 0;                 // the last packet id that the receiver received well
        _loss_seq_id_cnt = 0;           // lost packets count

        _time_delay_sum = SIMTIME_ZERO;             // Total delay time (for averaging)
        _time_packet_last = SIMTIME_ZERO;           // Timestamp of the last received packet
        _time_last_inst_tp_cal = SIMTIME_ZERO;      // Last throughput calculation time
        _time_window_size = WINDOW_SIZE;            // Throughput calculation window size (100ms)
        _before_rx_time = SIMTIME_ZERO;             // Timestamp of the last received packet

        _temp_str.str("");
        _temp_str.clear();
        _temp_str_counter = 0;


        // Create CSV file
        stringstream fileName;

        int ueIndex = getParentModule()->getIndex();
        std::ostringstream ueStr;
        ueStr << std::setw(2) << std::setfill('0') << ueIndex;
        int runNumber = getEnvir()->getConfigEx()->getActiveRunNumber();
        std::ostringstream runStr;
        runStr << std::setw(2) << std::setfill('0') << runNumber;

        fileName << "results/run" << runStr.str() << "_ue" << ueStr.str() << "_stats.csv";
        
        _result_csv.open(fileName.str());
        if (_result_csv.is_open())
        {
            _result_csv << "Time" << "," << "Rx_ID" << "," << "Rx_Delay" << ","
                    << "Rx_Gap" << "," << "Inst_Tp" << ","
                    << "Avg_Tp" << "," << "LossRate" << "\n";
        }
    }
    // if 로 수정
    if (stage == INITSTAGE_APPLICATION_LAYER) // Socket Initialize
    {
        int port = par("localPort");

        if (port != -1)
        {
            socket.setOutputGate(gate("socketOut"));
            socket.setCallback(this);
            socket.bind(port);
            socket.listen();

            EV_INFO << ">>> [RECEIVER] I am listening on PORT: " << port << endl;
        }
    }
}

void FileReceiver::socketAvailable(inet::TcpSocket *socket, inet::TcpAvailableInfo *info)
{
    EV_INFO << "[RECEIVER] socketAvailable(): new incoming connection" << endl;

    _data_socket = inet::TcpSocket(info);
    _data_socket.setOutputGate(gate("socketOut"));
    _data_socket.setCallback(this);
    _data_socket_flag = true;

    socket->accept(info->getNewSocketId());
}

void FileReceiver::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        delete msg;
        return;
    }

    int kind = msg->getKind();
    EV_INFO << ">>> [RECEIVER] Message Arrived! Kind: " << kind << endl;

    if (_data_socket_flag && _data_socket.belongsToSocket(msg))
    {
        _data_socket.processMessage(msg);
    }
    else
    {
        socket.processMessage(msg);
    }

    // socket automatically calls socketDataArrived function
    //socket.processMessage(msg);
}

void FileReceiver::socketDataArrived(inet::TcpSocket *socket, inet::Packet *packet, bool urgent)
{
    // already received packet
    auto fileHeader = packet->popAtFront<FilePacket>();

    // tx sequence id get (current sequence ID)
    _tx_seq_id = fileHeader->getIDframe();

    // 1. Loss Packet Check
    // if it is not the first packet && the id is bigger than expected
    if (_expected_seq_id > 0 && _tx_seq_id > _expected_seq_id)
    {
        long lost_count = _tx_seq_id - _expected_seq_id;
        _loss_seq_id_cnt += lost_count;
     }
     // update the expected id
     _expected_seq_id = _tx_seq_id + 1;

     // increas rx frame count
     _rx_count++;

    // Warmup: Initializing Time
    // do not need to check initialize time data
    if (simTime() > getSimulation()->getWarmupPeriod())
    {
        // packet receive time set - handleMessage run == packet rx
        SimTime rx_time = simTime(); //packet rx time

        // rx payload size set
        int rx_data_size = fileHeader->getPayloadSize();
        _total_rx_bytes += rx_data_size;

        // 2. Calculate packet delay
        // Rx Delay: payload time to rx time  - real transmission time
        SimTime rx_delay = rx_time - packet->getCreationTime();
        _time_delay_sum += rx_delay;

        // 3. Calculate time since last packet
        // Rx Gap: before_rx to rx
        SimTime time_since_last_packet = SIMTIME_ZERO;

        if (_before_rx_time > SIMTIME_ZERO)
        {
            time_since_last_packet = rx_time - _before_rx_time;
        }
        // update before_rx
        _before_rx_time = rx_time;

        // 4. Instant Throughput
        // Inst_Tp: sum_byte_size_during_window_time / (now_finish_window_time - before_finish_window_time)
        double inst_tp = 0.0;
        _rx_byte_sum_window += rx_data_size;
        SimTime time_diff = rx_time - _time_last_inst_tp_cal;

        if (time_diff >= _time_window_size)
        {
            inst_tp = (double)(_rx_byte_sum_window/ time_diff.dbl());
            _rx_byte_sum_window = 0;
            _time_last_inst_tp_cal = rx_time;
        }

        // 5. Average Throughput
        // Avg_Tp: total_received_bytes / passed_simulation_time
        double avg_tp = 0.0;
        SimTime passed_time = simTime() - getSimulation()->getWarmupPeriod();

        if (passed_time.dbl() > 0)
        {
            avg_tp = (double)(_total_rx_bytes / passed_time.dbl());
        }

        // 6. Loss Rate
        // Loss Rate: lost packets / total sent packets
        double loss_rate = 0.0;
        if (_tx_seq_id >= 0)
        {
            loss_rate = (double)_loss_seq_id_cnt / (double)(_tx_seq_id + 1);
        }


        // temp str wirte
        _temp_str << rx_time.dbl() << ","
                << _rx_count << ","
                << rx_delay.dbl() << ","
                << time_since_last_packet.dbl() << ","
                << inst_tp << ","
                << avg_tp << ","
                << loss_rate << "\n";
        _temp_str_counter++;

        // csv file write
        //if (_temp_str_counter >= 200)
        //{
            if (_result_csv.is_open())
            {
                _result_csv << _temp_str.str();
                _result_csv.flush();
            }
            _temp_str.str("");
            _temp_str.clear();
            _temp_str_counter = 0;
        //}
    }
    delete packet;
}

void FileReceiver::finish()
{
    double avg_tp = 0.0;
    double avg_delay = 0.0;
    double final_avg_tp = 0.0;

    // download time
    SimTime cum_set_time = simTime() - getSimulation()->getWarmupPeriod();

    if (cum_set_time > 0)
    {
        avg_delay = (double)(_time_delay_sum.dbl() / (double)_rx_count);
        final_avg_tp = (double)(_total_rx_bytes / cum_set_time.dbl());
    }

    double last_loss_rate = 0;
    if (_tx_seq_id > 0)
    {
        last_loss_rate = (double)((double)(_loss_seq_id_cnt) / (double)(_tx_seq_id + 1));
    }

    if (_result_csv.is_open())
    {
        // csv file write
        if (_temp_str_counter > 0)
        {
            if (_result_csv.is_open())
            {
                _result_csv << _temp_str.str();
            }
            _temp_str.str("");
            _temp_str.clear();
            _temp_str_counter = 0;
        }

        _result_csv << endl << "Final Statistics" << endl
                   << "Frames Received(Current/Total): " << _rx_count << "/" << (_tx_seq_id + 1) <<"\n"
                   << "Total Bytes Received: " << _total_rx_bytes << " bytes\n"
                   << "Average Delay: " << avg_delay << " seconds\n"
                   << "Average Throughput: " << final_avg_tp << " bytes/sec\n"
                   << "Frame Loss Rate: " << (last_loss_rate * 100) << "%\n";
        _result_csv.close();
    }
}
