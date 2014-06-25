#include <iostream>
#include "Game.h"

CGame::CGame() {
    server = new Network::CServer();

    m_end = false;
}

CGame::~CGame() {
    delete server;
}

void CGame::Init() {
    int         ServerPort;
    std::string ServerMode;
    std::string ServerIP;

    std::cout << "Welcome! Please select mode: " << std::endl;
    std::cout << "- host [port]"          << std::endl;
    std::cout << "- connect [ip] [port]"  << std::endl;

    /**
     * Connecting to server or starting own
    **/
    while(ServerMode != "host" && ServerMode != "connect") {
        std::cout << std::endl << "> ";
        std::cin >> ServerMode;


        if(ServerMode == "host") {
            std::cin >> ServerPort;

            Server().StartServer(ServerPort);
        } else if(ServerMode == "connect") {
            std::cin >> ServerIP >> ServerPort;

            Server().ConnectTo(ServerIP, ServerPort);
            if(Server().IsConnected() == false)
                ServerMode = "";
        }
    }
}

void CGame::Start() {
    std::string msg;
    std::cout << "Welcome! Write 'disconnect' to quit." << std::endl;

    while(Server().IsShuttingDown() == false) {
        std::getline(std::cin, msg);

        if(msg == "disconnect") {
            Server().Disconnect();
            break;
        }

        Server().AddMessege(msg);
    }
}

void CGame::End() {
    std::cout << "Appliaction is closing..." << std::endl;
    m_end = true;
}
