// This class represents the packet to be sent by both the sender and the receiver throughout the algorithm.
cplusplus {{
        #include <bitset>
        typedef  std::bitset<8> bits;
}}
class bits { @existingClass; };

packet Packet {
        @customize(true);
        
        int seq_num;
        int frame_type;
        int ack_nack_num;
        string payload;
        bits checksum;
}
