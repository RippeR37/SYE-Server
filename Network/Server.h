#ifndef CSERVER_H_INCLUDED
#define CSERVER_H_INCLUDED

#include <list>
#include <thread>
#include <chrono>
#include <vector>

#include "Types.h"
#include "UDPSocket.hpp"

namespace Network {

    #define MSG_SPECIAL_SIZE 16
    #define MSG_MIN_HEADER_SIZE 2
    #define MSG_DATA_BUFFER_SIZE 1024
    #define THREAD_SLEEP_TIME_MS 15

    /***
        CMP - Custom Message Protocol
        1. Connecting (one ack)
            Server (listening)          Client (connecting)
                                <---    MSG_REQ_CONNECTION
            MSG_ACK             --->

        2. Disconnecting (one ack)
            Side #1                     Side #2
            MSG_REQ_DISCONNECT  --->
                                <---    MSG_ACK

        3. Data (no confirmation)
            Side #1                     Side #2
            MSG_DATA (w/ data)  --->
    ***/

    /***
        DATA Packet
        [   1B   ][   1B   ][  *B  ]
         MSG_DATA  ClientID  _DATA_
    ***/

    class CServer {
        public:
            CServer();
           ~CServer();

            void StartServer(int port);
            void ConnectTo(std::string ip, int port);

            void StartListening();
            void StartSending();
            void Disconnect();

            void ProcessClientInput(char* buffer, int buffer_size);
            void ProcessServerInput(char* buffer, int buffer_size);
            bool VerifyResponse(char* buffer, int buffer_size);
            void AddMessege(std::string message);

            bool IsShuttingDown()       { return Shutdown; }
            bool IsServer()             { return AppType  == TYPE_SERVER; }
            bool IsConnected()          { return AppState == STATE_CONNECTED; }

        private:
            int                         ClientID;
            int                         ServerPort;
            std::string                 ServerIP;
            std::list<std::string>      MessageQueue;

            int                         MaxClients;
            std::vector<ClientInfo*>    Clients;

            UDPSocket*                  udpsocket;
            APPLICATION_TYPE            AppType;
            CONNECTION_STATE            AppState;

            bool                        Shutdown;
            bool                        ThreadListenerActive;
            bool                        ThreadSenderActive;

            std::thread                 ThreadListener;
            std::thread                 ThreadSender;
    };

}

#endif // CSERVER_H_INCLUDED
