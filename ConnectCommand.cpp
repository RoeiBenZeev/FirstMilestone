//
// Created by roei on 18/12/2019.
//

#include "ConnectCommand.h"
#include "Expression.h"
#include "globals.h"
#include "Var.h"
#include "VariablesSymbolTable.h"

/*
 * The execute function make the connection with the server.
 * param : vector v with the information we need to make the connection.
 * */
void ConnectCommand::execute(vector<string> v) {
    Expression *portExpression = nullptr;
    int port;
    string ipAsString = v[0];
    string portAsString = v[1];
    try {
        portExpression = globalInterpreter->interpret(portAsString);
        port = portExpression->calculate();
        delete portExpression;
        //delete globalInterpreter;
    } catch (const char *e) {
        if (portExpression != nullptr) {
            delete portExpression;
        }
        if (globalInterpreter != nullptr) {
            delete globalInterpreter;
        }
        std::cout << e << std::endl;
    }
    //create socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        //error
        cerr << "Could not create a socket" << endl;
        throw "-1";
        //return -1;
    }
    //convert string to char*
    int length = ipAsString.length();
    char *ip = new char[length + 1];
    strcpy(ip, ipAsString.c_str());

    //we create sockaddr obj to hold sim of server
    sockaddr_in address; //means IP4
    address.sin_family = AF_INET;// IP4
    address.sin_addr.s_addr = inet_addr(ip); // the IP sim we get as argument
    address.sin_port = htons(port);


    //requesting a connection with the server on local
    int is_connect = connect(clientSocket, (struct sockaddr *) &address, sizeof(address));
    if (is_connect == -1) {
        cerr << "Could not connect to host server" << endl;
        throw "-2";
        //return  -2;
    } else {
        cout << "Client is now connected to server" << endl;
    }
    //opening thread to keep the connect to the server , and send data while program is running.
    thread clientServer(sendData, clientSocket);
    clientServer.detach();
}

/*
 * sendData function is responsible to run a background thread and send data to the server.
 */
void ConnectCommand::sendData(int clientSocket) {
    //char buffer[1024] = {0};
    string sim;
    //loops run while keepRun is true
    while (keepRun) {
        for (auto pair : VariablesSymbolTable::getInstance().getVariablesMap()) {
            //check if this var already is not initial
            if (pair.second->getType() == Var::VarType::OutputVar &&
                pair.second->getUpdateCondition() == Var::UpdateFlag::NotUpdated) {
                //preparing the string for sending
                sim = "set ";
                sim.append(pair.second->getSim());
                sim.append(" ");
                sim.append(to_string(pair.second->getValue()));
                sim.append("\r\n");
                char *h = new char[sim.length() + 1];
                //convert string to char array.
                strcpy(h, sim.c_str());
                //sending the command to the server.
                int is_sent = send(clientSocket, h, strlen(h), 0);
                if (is_sent == -1) {
                    cout << "Error sending msg" << endl;
                } else {
                }
                pair.second->setUpdateCondition(Var::UpdateFlag::IsUpdated);
            }
        }
        //make sleep a bit to let other threads run
        sleep(0.01);
    }
    close(clientSocket);
}
