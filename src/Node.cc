#include "Node.h"

Define_Module(Node);

Packet_Base *packet_to_retransmit_TO;
std:: string file_path_0 = "C:\\omnetpp-6.0.1\\samples\\groupProject\\test.txt";
std:: string file_path_1 = "C:\\omnetpp-6.0.1\\samples\\groupProject\\test1.txt";
std::ifstream input_file;
std::ofstream outputFile;



std::bitset<8> Node:: calculate_check_sum(std::string payload)
{
    // Find the number of characters in the payload.
    int payload_length = payload.size();

    // Initialize XOR with zero.
    std::bitset<8> XOR(0);

    // Update XOR according to the characters in the payload.
    for(int i=0; i<payload_length; i++)
    {
        std::bitset<8> new_char (payload[i]);
        XOR = XOR ^ new_char;
    }

    return XOR;
}

std::string Node:: frame(std::string payload)
{
    // Find the number of characters in the payload.
    int payload_length = payload.size();

    // Start Flag.
    std::string framed_payload = "#";

    // Frame the payload.
    for(int i=0; i<payload_length; i++)
    {
        if(payload[i]=='#' || payload[i]=='/')
        {
            framed_payload += '/';
        }

        framed_payload += payload[i];
    }

    // End Flag.
    framed_payload += '#';

    return framed_payload;
}

std::string Node:: modify(std::string payload, int &modifiedBit)
{
    // STEP 1: Characters string to binary string.

    // Declare the binary payload.
    std::string payload_binary = "";

    // Find the number of characters in the payload.
    int payload_length = payload.size();

    // Update the binary payload according to the characters in the payload.
    for(int i=0; i<payload_length; i++)
    {
        std::bitset<8> new_char (payload[i]);
        payload_binary += new_char.to_string();
    }

    // STEP 2: Apply the error to the binary payload.

    // Choose the index of the bit you want to modify.
    int errored_bit_index = int(uniform (0, payload_binary.size()));
    modifiedBit=errored_bit_index;

    // Modify the chosen bit.
    if(payload_binary[errored_bit_index] == '1')
    {
        payload_binary[errored_bit_index] = '0';
    }
    else
    {
        payload_binary[errored_bit_index] = '1';
    }

    // STEP 3: Binary string to characters string.

    std::string payload_modified = "";

    for(int i=0; i<payload_binary.size(); i+=8)
    {
        std::bitset<8> new_char(payload_binary.substr(i,8) );

        char character = char(new_char.to_ulong());;

        payload_modified += character;
    }

    return payload_modified;
}

void Node:: circular_increment(int& next_frame_to_send)
{
    next_frame_to_send = (next_frame_to_send + 1) % (2*WS);
}



std:: string Node:: read_one_line()
{
    if (!input_file.is_open())
    {
        EV << "Error opening the input file in the sender.";
    }

    std:: string new_line;
    getline (input_file, new_line);

    return new_line;
}


void Node:: send_message(int& next_frame_to_send)
{
    ACKs[next_frame_to_send] = 0; //initializing the ack position in ack array

    // Read one line from the input file.
    std:: string line = read_one_line();


    // If reached EOF.
    if(line.size() == 0)
    {
        return;
    }



    // Extract the payload from the read line.
    std:: string payload = line.substr(5);

    // Extract the error parameters from the read line.
    int modification = line[0] - '0';
    int loss = line[1] - '0';
    int duplication = line[2] - '0';
    int delay = line[3] - '0';


    int modifiedBit;
    std:: string payload_modified = modify(payload, modifiedBit);
    std::bitset<8> check_sum (calculate_check_sum(payload));

    std:: string payload_framed = frame(payload);
    std:: string payload_modified_framed = frame(payload_modified);

    // Create a packed object for the framed payload.
    Packet_Base *packet = new Packet_Base("Sender");

    packet->setSeq_num(next_frame_to_send);
    packet->setFrame_type(0);
    packet->setAck_nack_num(-1);
    packet->setPayload(payload_framed.c_str());
    packet->setChecksum(check_sum);

    // Create a packed object for the modified framed payload.
    Packet_Base *packet_modified = new Packet_Base("Sender");

    packet_modified->setSeq_num(next_frame_to_send);
    packet_modified->setFrame_type(0);
    packet_modified->setAck_nack_num(-1);
    packet_modified->setPayload(payload_modified_framed.c_str());
    packet_modified->setChecksum(check_sum);

    Packet_Base *packet_to_send = new Packet_Base("Sender");
    Packet_Base *packet_to_send_duplicate = new Packet_Base("Sender");

    double cummulative_propagation_delay = count_of_sent_messages * PD;

    outputFile<< "At time ["<<simTime()+cummulative_propagation_delay-PD<<"] , Node["<<nodeId<<"] , Introducing channel error with code ["<<modification<<loss<<duplication<<delay<<"]. "<<"and msg = "<<payload<<endl;

    std::string checksumString;
    for(int i=0;i<8;i++)
    {
        checksumString += std::bitset<1>(check_sum[i]).to_string();
    }
    int printmodified= -1;
    std::string payloadInLoss=packet->getPayload();
    if(modification == 1)
    {
        payloadInLoss=packet_modified->getPayload();
        printmodified=modifiedBit;
    }
    if (loss)
    {

        int printDelayed = ED;
        if(delay==0)
        {
            printDelayed=0;
        }
        outputFile<<"At time ["<<simTime()+cummulative_propagation_delay<<"], Node["<<nodeId<<"] [sent] frame with seq_num=["<< next_frame_to_send <<"] and payload= "<<payloadInLoss<<" and trailer=["<<checksumString<< "] and Modified ["<<printmodified<<"] Lost[Yes], Duplicate["<<duplication<<"], Delay["<<printDelayed<<"]"<<endl;

        if(duplication)
        {
            outputFile<<"At time ["<< simTime() + cummulative_propagation_delay + DD  <<"], Node["<<nodeId<<"] [sent] frame with seq_num=["<< next_frame_to_send <<"] and payload= "<<payloadInLoss<<" and trailer=["<<checksumString<< "] and Modified ["<<printmodified<<"] Lost[Yes], Duplicate["<<2<<"], Delay["<<printDelayed<<"]"<<endl;
        }
        count_of_sent_messages ++;
    }
    else
    {
        if (modification)
        {
            packet_to_send = packet_modified;

            packet_to_send_duplicate->setSeq_num(packet_modified->getSeq_num());
            packet_to_send_duplicate->setFrame_type(packet_modified->getFrame_type());
            packet_to_send_duplicate->setAck_nack_num(packet_modified->getAck_nack_num());
            packet_to_send_duplicate->setPayload(packet_modified->getPayload());
            packet_to_send_duplicate->setChecksum(packet_modified->getChecksum());



        }
        else
        {
            packet_to_send = packet;

            packet_to_send_duplicate->setSeq_num(packet->getSeq_num());
            packet_to_send_duplicate->setFrame_type(packet->getFrame_type());
            packet_to_send_duplicate->setAck_nack_num(packet->getAck_nack_num());
            packet_to_send_duplicate->setPayload(packet->getPayload());
            packet_to_send_duplicate->setChecksum(packet->getChecksum());
        }

        if (delay)
        {
            simtime_t send_after = cummulative_propagation_delay + TD + ED;
            sendDelayed(packet_to_send, send_after, "gout");
            count_of_sent_messages ++;
            outputFile<<"At time ["<<simTime() + cummulative_propagation_delay<<"], Node["<<nodeId<<"] [sent] frame with seq_num=["<< next_frame_to_send <<"] and payload= "<<packet_to_send->getPayload()<<" and trailer=["<<checksumString<< "] and Modified ["<<printmodified<<"] Lost[No], Duplicate["<<duplication<<"], Delay["<<ED<<"]"<<endl;
        }
        else
        {
            simtime_t send_after = cummulative_propagation_delay + TD;
            sendDelayed(packet_to_send, send_after, "gout");
            count_of_sent_messages ++;
            outputFile<<"At time ["<<simTime() + cummulative_propagation_delay<<"], Node["<<nodeId<<"] [sent] frame with seq_num=["<< next_frame_to_send <<"] and payload= "<<packet_to_send->getPayload()<<" and trailer=["<<checksumString<< "] and Modified ["<<printmodified<<"] Lost[No], Duplicate["<<duplication<<"], Delay["<<0<<"]"<<endl;
        }

        if (duplication && delay)
        {
            simtime_t send_after = cummulative_propagation_delay + TD + ED + DD;
            sendDelayed(packet_to_send_duplicate, send_after, "gout");
            outputFile<<"At time ["<<simTime() + cummulative_propagation_delay + DD<<"], Node["<<nodeId<<"] [sent] frame with seq_num=["<< next_frame_to_send <<"] and payload= "<<packet_to_send->getPayload()<<" and trailer=["<<checksumString<< "] and Modified ["<<printmodified<<"] Lost[No], Duplicate["<<2<<"], Delay["<<ED<<"]"<<endl;

        }
        else if (duplication && !delay)
        {
            simtime_t send_after = cummulative_propagation_delay + TD + DD;
            sendDelayed(packet_to_send_duplicate, send_after, "gout");
            outputFile<<"At time ["<<simTime() + cummulative_propagation_delay + DD<<"], Node["<<nodeId<<"] [sent] frame with seq_num=["<< next_frame_to_send <<"] and payload= "<<packet_to_send->getPayload()<<" and trailer=["<<checksumString<< "] and Modified ["<<printmodified<<"] Lost[No], Duplicate["<<2<<"], Delay["<<0<<"]"<<endl;
        }
    }

    // Schedule a message for after the timeout, if an ack was received before then,
    // this scheduled message would be cancelled.

    // In omnet, you are not allowed to schedule the same object/message twice.
    // Hence we will create a duplicate window called "retransmission".
    // So that we could "schedule at" the "retransmission" and "sendDelayed" at "window.

    // This is the packet to retransmit in the case of a timeout.
    packet_to_retransmit_TO = new Packet_Base("Sender");

    packet_to_retransmit_TO->setSeq_num(packet->getSeq_num());
    packet_to_retransmit_TO->setFrame_type(packet->getFrame_type());
    packet_to_retransmit_TO->setAck_nack_num(packet->getAck_nack_num());
    packet_to_retransmit_TO->setPayload(packet->getPayload());
    packet_to_retransmit_TO->setChecksum(packet->getChecksum());

    // The "sender" receives the packet in a self message after the timeout.
    // It then can "sendDelayed" the corrected version of the packet.
    simtime_t send_after = cummulative_propagation_delay + TO;
    scheduleAt(simTime() + send_after, packet_to_retransmit_TO);

    retransmissions[next_frame_to_send]= packet_to_retransmit_TO;

    //retransmissions

    Packet_Base *packet_to_retransmit_NACK = new Packet_Base("Sender");

    packet_to_retransmit_NACK->setSeq_num(packet->getSeq_num());
    packet_to_retransmit_NACK->setFrame_type(packet->getFrame_type());
    packet_to_retransmit_NACK->setAck_nack_num(packet->getAck_nack_num());
    packet_to_retransmit_NACK->setPayload(packet->getPayload());
    packet_to_retransmit_NACK->setChecksum(packet->getChecksum());


    // Save the unmodified packet to the window.
    buffer[next_frame_to_send] = packet_to_retransmit_NACK;


    // Increment the next_frame_to_send.
    circular_increment(next_frame_to_send);
}


///////////////////////////////////

std::string Node:: deframe(std::string payload_framed)
{
    std::string payload;

    int payload_framed_length = payload_framed.size();

    for(int i=1; i<payload_framed_length-1; i++)
    {
        if(payload_framed[i]=='/')
        {
            i++;
        }

        payload += payload_framed[i];
    }

    return payload;
}


int Node:: in_between(int a, int b, int c)
{
    if((a<=b) && (b<c) || ((c<a) && (a<=b)) || ((b<c) && (c<a)))
    {
        return 1;
    }

    return 0;
}

void Node::initialize()
{
    // TODO - Generated method body
    WS=getParentModule()->par("WS");
    WR=getParentModule()->par("WR");
    DD=getParentModule()->par("DD");
    TD=getParentModule()->par("TD");
    ED=getParentModule()->par("ED");
    TO=getParentModule()->par("TO");
    PD=getParentModule()->par("PD");

    //sender.cc initialize
    upper_bound=WS;
    retransmissions.resize(2*WS);
    buffer.resize(2*WS);
    ACKs.resize(2*WS, 0);
    NACKs.resize(2*WS, 0);
    packet_already_received.resize(2*WS, 0);


    if(getId()==3)
    {
        nodeId=0;
        file_path=file_path_0;
    }
    else
    {
        nodeId=1;
        file_path=file_path_1;
    }
}

void Node::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    if(firsttime && !(strcmp("start",msg->getName())))
    {
        EV<<" my id= "<<getId();
        firsttime=0;

        isSender=1;

        //intiliaze

        //writing to file
        outputFile.open("output.txt");
        if (!outputFile.is_open())
        {
            EV << "Error opening the output file in the sender.";
        }

        //opening input file
        input_file.open(file_path);

        for(int i=0; i<WS ;i++)
        {
            send_message(next_frame_to_send);

        }
        count_of_sent_messages = 1;

    }
    else
    {
        if(isSender==1) //sender
        {
            Packet_Base *mmsg = check_and_cast<Packet_Base *>(msg);
            std::string print_payload = mmsg->getPayload();
            int seq_num = mmsg->getSeq_num();
            // A timeout occured.
            if(msg->isSelfMessage())
            {
                outputFile<<"Time out event at time ["<<simTime()<<"], at Node["<<nodeId<<"] for frame with seq_num=["<<mmsg->getSeq_num()<<"]"<<endl;

                simtime_t send_after = PD + TD;
                sendDelayed(mmsg, send_after, "gout");

                std::string checksumString;
                for(int i=0;i<8;i++)
                {
                    checksumString += std::bitset<1>(mmsg->getChecksum()[i]).to_string();
                }

                outputFile<<"At time ["<<simTime() + PD <<"], Node["<<nodeId<<"] [sent] frame with seq_num=["<< mmsg->getSeq_num() <<"] and payload= "<<mmsg->getPayload()<<" and trailer=["<<checksumString<< "] and Modified ["<<-1<<"] Lost[No], Duplicate["<<0<<"], Delay["<<0<<"]"<<endl;
                EV<< print_payload<<" "<<seq_num<<"   Timeout detected."<<endl;
            }
            else
            {
                // Received an ACK/NACK from the receiver.
                int frame_type = mmsg->getFrame_type();
                int ack_nack_num = mmsg->getAck_nack_num();

                if (frame_type == 1)
                {
                    ACKs[ack_nack_num] = 1;

                    if (ack_nack_num == ack_expected)
                    {
                        while(ACKs[ack_expected])
                        {
                            circular_increment(ack_expected);
                            send_message(next_frame_to_send);
                        }
                    }
                    cancelEvent(retransmissions[ack_nack_num]);
                    EV<< print_payload<<" "<<seq_num<<"   ACK Received."<<endl;

                }
                else
                {
                    simtime_t send_after = PD + TD;
                    sendDelayed(buffer[ack_nack_num], send_after, "gout");

                    std::string checksumString;
                    for(int i=0;i<8;i++)
                    {
                        checksumString += std::bitset<1>( buffer[ack_nack_num]->getChecksum()[i]).to_string();
                    }

                    outputFile<<"At time ["<<simTime() + PD <<"], Node["<<nodeId<<"] [sent] frame with seq_num=["<< buffer[ack_nack_num]->getSeq_num() <<"] and payload= "<<buffer[ack_nack_num]->getPayload()<<" and trailer=["<<checksumString<< "] and Modified ["<<-1<<"] Lost[No], Duplicate["<<0<<"], Delay["<<0<<"]"<<endl;
                    cancelEvent(retransmissions[ack_nack_num]);
                    EV<< print_payload<<" "<<seq_num<<"   NACK Received."<<endl;
                }

                count_of_sent_messages = 1;
            }

        }
        ///////////////////////////////////////////////////////////////////////////////////////////

        else   //receiver
        {

            if(lastTimeStamp==simTime())
            {
                totalAtSameTime++;
            }
            else
            {
                totalAtSameTime=1;
            }
            lastTimeStamp=simTime();

          //  EV<<"total= "<<totalAtSameTime<<endl;
          //  EV<<"SIMTime= "<<simTime()<<endl;

            // The received message.
            Packet_Base *packet_received = check_and_cast<Packet_Base *>(msg);

            int seq_num = packet_received->getSeq_num();
            int farme_type = packet_received->getFrame_type();
            int ack_nack_num = packet_received->getAck_nack_num();
            std::string payload_received = packet_received->getPayload();
            std::bitset<8> check_sum_received = packet_received->getChecksum();

            // Deframing then calculating the check sum.
            std::string payload_deframed = deframe(payload_received);
            std::bitset<8> check_sum_calculated(calculate_check_sum(payload_deframed));

            // The ACK/NACK.
            Packet_Base *ack_nack_packet = new Packet_Base("Receiver");

            ack_nack_packet->setSeq_num(packet_received->getSeq_num());
            ack_nack_packet->setAck_nack_num(packet_received->getSeq_num()); // Important field.
            ack_nack_packet->setPayload(packet_received->getPayload());
            ack_nack_packet->setChecksum(packet_received->getChecksum());

            std::string checksumString;
            for(int i=0;i<8;i++)
            {
                checksumString += std::bitset<1>( packet_received->getChecksum()[i]).to_string();
            }


            if (check_sum_calculated == check_sum_received)
            {

                if (in_between(frame_expected, seq_num, upper_bound))
                {
                    if (packet_already_received[seq_num] == 0)
                    {
                        ack_nack_packet->setFrame_type(1);
                        EV << payload_received<< " "<<seq_num<<"  Correct Message Received." << endl;

                        outputFile<<"At time ["<<simTime() <<"], Node["<<nodeId<<"] [received] frame with seq_num=["<< seq_num <<"] and payload= "<<payload_received<<" and trailer=["<<checksumString<< "] and Modified ["<<-1<<"] Lost[No], Duplicate["<<0<<"], Delay["<<0<<"]"<<endl;

                        packet_received->setPayload(payload_deframed.c_str());
                        buffer[seq_num] = packet_received;
                        packet_already_received[seq_num] = 1;

                        simtime_t send_after = (PD*totalAtSameTime) + TD;
                        sendDelayed(ack_nack_packet, send_after, "gout");
                        outputFile<< "At time["<< simTime() + (PD*totalAtSameTime) <<"], Node["<<nodeId<<"] Sending [ACK] with number["<<ack_nack_packet->getAck_nack_num()<<"],  loss [No]"<<endl;


                        if (seq_num == frame_expected)
                        {
                            // Sliding
                            while (packet_already_received[frame_expected])
                            {
                                EV << buffer[frame_expected]->getPayload()<<" "<<frame_expected<<"   To network layer"<< endl;
                                packet_already_received[frame_expected] = 0;
                                NACKs[frame_expected]=0;
                                circular_increment(frame_expected);
                                circular_increment(upper_bound);
                            }
                        }
                    }
                    else
                    {
                        outputFile<<"At time ["<<simTime() <<"], Node["<<nodeId<<"] [received] frame with seq_num=["<< seq_num <<"] and payload= "<<payload_received<<" and trailer=["<<checksumString<< "] and Modified ["<< 0<<"] Lost[No], Duplicate["<<1<<"], Delay["<<0<<"]"<<endl;
                        EV << payload_received<<" "<<seq_num<<"   Duplicate Delivery." << endl;
                    }
                }
                else
                {
                    EV << payload_received<<" "<<seq_num<<"   Out of window delivery." << endl;
                }
            }
            else
            {

                ack_nack_packet->setFrame_type(2);
                EV << payload_received<<" "<<seq_num<<"   Incorrect Message Received." << endl;

                if(  NACKs[seq_num] == 0) //not sent a nack on this packet before
                {
                    outputFile<<"At time ["<<simTime() <<"], Node["<<nodeId<<"] [received] frame with seq_num=["<< seq_num <<"] and payload= "<<payload_received<<" and trailer=["<<checksumString<< "] and Modified ["<< 1<<"] Lost[No], Duplicate["<<0<<"], Delay["<<0<<"]"<<endl;

                    simtime_t send_after = (PD*totalAtSameTime)+TD;
                    sendDelayed(ack_nack_packet, send_after, "gout");
                    outputFile<< "At time["<< simTime() + (PD*totalAtSameTime) <<"], Node["<<nodeId<<"] Sending [NACK] with number["<<ack_nack_packet->getAck_nack_num()<<"],  loss [No]"<<endl;
                    NACKs[seq_num]=1;
                }
                else
                {
                    outputFile<<"At time ["<<simTime() <<"], Node["<<nodeId<<"] [received] frame with seq_num=["<< seq_num <<"] and payload= "<<payload_received<<" and trailer=["<<checksumString<< "] and Modified ["<< 1<<"] Lost[No], Duplicate["<<1<<"], Delay["<<0<<"]"<<endl;
                }
            }

        }

    }

}
