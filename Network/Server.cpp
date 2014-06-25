#include <iostream>
#include <cstdlib>

#include "Server.h"
#include "../Game.h"


namespace Network {

    CServer::CServer() {
        udpsocket = new UDPWinSocket();

        ThreadListenerActive = false;
        ThreadSenderActive = false;

        AppType  = APPLICATION_TYPE::TYPE_SERVER;              // default type
        AppState = CONNECTION_STATE::STATE_UNCONNECTED;        // default state

        ClientID   = 0;
        MaxClients = 0;
        Shutdown   = false;
    }

    CServer::~CServer() {
        if(ThreadListenerActive) {
            std::cout << "Net::Server | Waiting for TL to join... ";
            ThreadListener.join();
            std::cout << "done." << std::endl;
        }

        if(ThreadSenderActive) {
            std::cout << "Net::Server | Waiting for TS to join... ";
            ThreadSender.join();
            std::cout << "done." << std::endl;
        }

        for(ClientInfo* Client : Clients) {
            if(Client != NULL) {
                delete Client;
            }
        }

        udpsocket->Close();
        delete udpsocket;
    }

    void CServer::StartServer(int port) {
        MaxClients = 2;
        ServerPort = port;
        Clients.resize(MaxClients, NULL);

        udpsocket->InitSocket();
        udpsocket->SetRecvAddr(port);
        udpsocket->SetSocket();
        udpsocket->Bind();

        ThreadListener = std::thread(&CServer::StartListening, this);
        ThreadSender   = std::thread(&CServer::StartSending,   this);
    }

    void CServer::ConnectTo(std::string ip, int port) {
        ServerIP   = ip;
        ServerPort = port;
        ClientID   = rand() % 1337;
        AppType    = TYPE_CLIENT;
        AppState   = STATE_TRYING_C;

        MaxClients = 1;
        Clients.resize(MaxClients, NULL);

        udpsocket->InitSocket();
        udpsocket->SetRecvAddr(0);  // sets any port to use
        udpsocket->SetSendAddr((char*)ServerIP.c_str(), ServerPort);
        udpsocket->SetSocket();
        udpsocket->Bind();

        /*** Trying to connect to server with provided address ***/
        int retries_left = 3;
        int recvlen;
        char con_req[MSG_SPECIAL_SIZE];
        char con_res[MSG_SPECIAL_SIZE];
        con_req[0] = con_req[1] = (int)MSG_REQ_CONNECT;
        con_req[2] = (int)ClientID;

        while(AppState != STATE_CONNECTED && retries_left > 0) {
            std::cout << "Net::Client | Trying to connect to server..." << std::endl;

            udpsocket->SendData((char*)con_req, MSG_SPECIAL_SIZE);
            recvlen = udpsocket->ReadData((char*)con_res, MSG_SPECIAL_SIZE, 10, 0);

            if(recvlen > 0) {
                /*** Checking response data ***/
                if(con_res[0] == con_res[1]) {
                    if(con_res[0] == (char)MSG_ACK) {
                        ClientID = (int)con_res[2];
                        std::cout << "Net::Client | Connection ACK from Server (id: " << ClientID << ")" << std::endl;
                        AppState = STATE_CONNECTED;

                        Clients[0] = new ClientInfo(0, udpsocket->GetSender());
                    } else {
                        std::cout << "Net::Client | Connection request rejected" << std::endl;
                        retries_left = 0;
                    }
                }
            }
            --retries_left;
        }

        if(AppState == STATE_CONNECTED) {
            ThreadListener = std::thread(&CServer::StartListening, this);
            ThreadSender   = std::thread(&CServer::StartSending,   this);
        } else {
            // TODO (Damian#1#): Odkomentowac jesli potrzebne beda stany serwera!
            //AppState == STATE_UNCONNECTED;
            std::cout << "Net::Client | Could not connect to server!" << std::endl;
        }
    }

    void CServer::StartListening() {
        int   recvlen;
        char* buffer;

        if(IsServer()) {
            std::cout << "Net::Server | Server is listening on port " << ServerPort << std::endl;
        } else {
            std::cout << "Net::Client | Client is listening" << std::endl;
        }

        buffer = new char[MSG_DATA_BUFFER_SIZE];
        ThreadListenerActive = true;

        while(true) {
            if(CGame::Get().IsEnd() || IsShuttingDown())
                break;

            recvlen = udpsocket->ReadData(buffer, MSG_DATA_BUFFER_SIZE, 5, 0);
            if(recvlen > MSG_MIN_HEADER_SIZE) {
                /*** Verify if its response from connected client, not someone else ***/
                if(VerifyResponse(buffer, recvlen)) {
                    if(IsServer()) {
                        udpsocket->SetSendAddrAsReply();
                        CServer::ProcessServerInput(buffer, recvlen);
                    } else {
                        CServer::ProcessClientInput(buffer, recvlen);
                    }
                }
            }
        }

        delete buffer;
    }

    void CServer::StartSending() {
        ThreadSenderActive = true;

        while(true) {
            if(CGame::Get().IsEnd() || IsShuttingDown())
                break;

            /*** Send to all each message from queue ***/
            while(MessageQueue.empty() == false) {
                for(ClientInfo* Client : Clients) {
                    if(Client != NULL) {
                        udpsocket->SetSendAddr(Client->clientAddr.sin_addr.s_addr, Client->clientAddr.sin_port);
                        udpsocket->SendData((char*)MessageQueue.front().c_str(), MessageQueue.front().size());
                    }
                }
                MessageQueue.pop_front();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_SLEEP_TIME_MS));
        }
    }

    void CServer::Disconnect() {
        char message[MSG_SPECIAL_SIZE];

        AppState   = STATE_TRYING_D;
        message[0] = message[1] = (char)MSG_REQ_DISCONNECT;
        message[2] = ClientID;

        for(auto Client : Clients) {
            if(Client != NULL) {
                udpsocket->SetSendAddr(Client->clientAddr.sin_addr.s_addr, Client->clientAddr.sin_port);
                udpsocket->SendData((char*)message, MSG_SPECIAL_SIZE);
            }
        }

        Shutdown = true;
        AppState = STATE_UNCONNECTED;;
    }

    void CServer::ProcessClientInput(char* buff, int buff_size) {
        if(buff[0] == (char)MSG_DATA && buff[1] == 0) {
            /*** Process incoming data ***/
            buff[buff_size] = '\0';
            buff += 2;

            std::cout << "S: " << buff << std::endl;
        } else if(buff[0] == buff[1]) {
            if(buff[0] == (char)MSG_REQ_DISCONNECT) {
                int  recvID = buff[2];
                char reply[MSG_SPECIAL_SIZE];

                // Inform Application's main engine that it should shut down
                Shutdown = true;

                // Sending ACK reply for disconnecting client
                reply[0] = reply[1] = char(MSG_ACK);
                udpsocket->SendData((char*)reply, MSG_SPECIAL_SIZE);

                // Cleaning workspace
                delete Clients[recvID];
                Clients[recvID] = NULL;
                std::cout << "Net::Client | Server disconnected!" << std::endl;
            }
        }
    }

    void CServer::ProcessServerInput(char* buff, int buff_size) {
        if(buff[0] == (char)MSG_DATA) {
            /*** Process incoming data ***/
            int recvID = buff[1];
            buff[buff_size] = '\0';
            buff += 2;
            std::cout << recvID << ": " << buff << std::endl;

        } else if(buff[0] == buff[1]) {
            int new_ClientID = -1;
            char reply[MSG_SPECIAL_SIZE];

            if(buff[0] == (char)MSG_REQ_CONNECT) {
                for(int i = 0; i < MaxClients; ++i) {
                    if(Clients[i] == NULL) {
                        new_ClientID = i;

                        Clients[i] = new ClientInfo(i, udpsocket->GetSender());
                        break;
                    }
                }

                if(new_ClientID != -1) {
                    reply[0] = reply[1] = (char)MSG_ACK;
                    reply[2] = new_ClientID;

                    udpsocket->SendData((char*)reply, MSG_SPECIAL_SIZE);
                    std::cout << "Net::Server | New client connected (ID: " << new_ClientID << ")" << std::endl;
                } else {
                    reply[0] = reply[1] = (char)MSG_REQ_DENIED;
                    udpsocket->SendData((char*)reply, MSG_SPECIAL_SIZE);
                }
            } else if(buff[0] == (char)MSG_REQ_DISCONNECT) {
                // Update client slots with new empty one
                int recvID = buff[2];

                // Sending ACK reply for disconnecting client
                reply[0] = reply[1] = char(MSG_ACK);
                udpsocket->SendData((char*)reply, MSG_SPECIAL_SIZE);

                // Cleaning workspace for new clients to come
                delete Clients[recvID];
                Clients[recvID] = NULL;
                std::cout << "Net::Server | Client (ID: " << recvID << ") disconnected" << std::endl;
            }
        }
    }

    bool CServer::VerifyResponse(char* buff, int buff_len) {
        int recvID;

        if(buff[0] == (char)MSG_DATA) {
            /*** Verify data packet ***/
            recvID = buff[1];

            if(MaxClients > recvID) {
                if(Clients[recvID] != NULL) {
                    if(Clients[recvID]->clientAddr.sin_port == udpsocket->GetSender().sin_port &&
                       Clients[recvID]->clientAddr.sin_addr.s_addr == udpsocket->GetSender().sin_addr.s_addr)
                        return true;
                }
            }
        } else if(buff[0] == buff[1]) {
            if(buff[0] == (char)MSG_REQ_CONNECT)
                return true;

            /*** Verify MSG_REQ_DISCONNECT ***/
            if(buff[0] == (char)MSG_REQ_DISCONNECT) {
                recvID = buff[2];
                if(MaxClients > recvID) {
                    if(Clients[recvID] != NULL) {
                        if(Clients[recvID]->clientAddr.sin_port == udpsocket->GetSender().sin_port &&
                           Clients[recvID]->clientAddr.sin_addr.s_addr == udpsocket->GetSender().sin_addr.s_addr)
                            return true;
                    }
                }
            }
        }

        return false;
    }

    void CServer::AddMessege(std::string message) {
        message.insert(message.begin(), ClientID);
        message.insert(message.begin(), (char)MSG_DATA);
        MessageQueue.push_back(message);
    }

}
