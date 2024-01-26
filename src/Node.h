//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef _PROJECTNETWORKSNEW_NODE_H
#define _PROJECTNETWORKSNEW_NODE_H

#include <omnetpp.h>
#include <bitset>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <iostream>
#include "packet_m.h"



using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
public:


    int WS;
    int WR;
    double TO;
    double PD;
    double TD;
    double ED;
    double DD;

    int nodeId;
    std:: string file_path;
    int ack_expected = 0; // Lower bound of the window
    int next_frame_to_send = 0; // Upper bound of the window
    simtime_t lastTimeStamp= -1;
    int totalAtSameTime=1;

    //std::vector<Packet_Base*> window;

    std::vector<Packet_Base*> retransmissions;

    std::vector<Packet_Base*> buffer; // Window is between ack_expected and next_frame_to_send
    std::vector<int> ACKs;

    int count_of_sent_messages = 1; // First message should be scheduled after 1 PD.

    int firsttime=1;
    int isSender=0;
///////////////receiver//////////////////////
    int frame_expected = 0; // Lower bound of the window
       int upper_bound; // Upper bound of the window

       std::vector<int> NACKs;
       std::vector<int> packet_already_received; // True if this packet has been received.



protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    std::bitset<8> calculate_check_sum(std::string payload);
    std::string frame(std::string payload);
    std::string modify(std::string payload, int &modifiedBit);
    void circular_increment(int& next_frame_to_send);
    std:: string read_one_line();
    void sleep();
    void send_message(int& next_frame_to_send);
    int in_between(int a, int b, int c);
    std::string deframe(std::string payload_framed);

};

#endif
