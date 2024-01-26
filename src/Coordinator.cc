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

#include "Coordinator.h"

Define_Module(Coordinator);


void Coordinator::initialize()
{
    // TODO - Generated method body

    // Read one line from the input file.
    std:: string file_path_coord  = "C:\\omnetpp-6.0.1\\samples\\groupProject\\coordinator.txt";
    std::ifstream input_file;
    input_file.open(file_path_coord);
    if (!input_file.is_open())
    {
        EV << "Error opening the input file in the coordinator.";
    }

    std:: string lines;
    getline (input_file, lines);
    int nodeIdToSend= lines[1]-'0';
    int startTime=lines[3]-'0';

    cMessage *msg = new cMessage("start");

    if(nodeIdToSend == 0)
    {
        sendDelayed(msg, startTime, "gout0"); //of id=3
    }
    else
    {
        sendDelayed(msg, startTime, "gout1");//of id=4
    }

}

void Coordinator::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}
