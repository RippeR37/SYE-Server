#ifndef UDPWINSOCKET_HPP_INCLUDED
#define UDPWINSOCKET_HPP_INCLUDED

#include "../UDPSocket.hpp"
#include <winsock2.h>
#include <iostream>


class UDPWinSocket : public UDPSocket {
    public:
        UDPWinSocket() { }
       ~UDPWinSocket() { }

        bool InitSocket() {
            if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
                std::cerr << "UDPWinSocket | Error while initializing WinSock!\n";
                return false;
            }

            RecvAddr.sin_family         = AF_INET;
            SendAddr.sin_family         = AF_INET;

            return true;
        }

        void SetRecvAddr(int incoming_port) {
            RecvAddr.sin_port           = htons(incoming_port);
            RecvAddr.sin_addr.s_addr    = htonl(INADDR_ANY);
        }

        void SetSendAddr(char* ip, int outgoing_port) {
            SendAddr.sin_port           = htons(outgoing_port);
            SendAddr.sin_addr.s_addr    = inet_addr(ip);
        }


        void SetSendAddr(u_long ip, u_short outgoing_port) {
            SendAddr.sin_port           = outgoing_port;
            SendAddr.sin_addr.s_addr    = ip;
        }

        void SetSendAddrAsReply() {
            SendAddr.sin_port           = SenderAddr.sin_port;
            SendAddr.sin_addr.s_addr    = SenderAddr.sin_addr.s_addr;
        }

        bool SetSocket() {
            sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if(sock == INVALID_SOCKET) {
                std::cerr << "UDPWinSocket | Error while creating a socket!\n";
                Close();
                return false;
            }
            return true;
        }

        bool Bind() {
            if(bind(sock, (sockaddr*)&RecvAddr, sizeof(RecvAddr)) == SOCKET_ERROR) {
                std::cerr << "UDPWinSocket | Error while binding socket!\n";
                Close();
                return false;
            }
            return true;
        }

        int  ReadData(char* buffer, int buffer_size, int timeout_s, int timeout_ms) {
            static int recvlen = 0;
            static int activity;
            static int senderAddrSize;
            static fd_set read_fds;
            static timeval timeout;

            senderAddrSize = sizeof(SenderAddr);
            FD_ZERO(&read_fds);
            FD_SET(sock, &read_fds);

            timeout.tv_sec  = timeout_s;
            timeout.tv_usec = timeout_ms * 1000;

            activity = select(0, &read_fds, NULL, NULL, &timeout);
            if(activity == SOCKET_ERROR || activity == 0) {
                return 0;
            }

            if(activity > 0) {
                recvlen = recvfrom(sock, buffer, buffer_size, 0, (SOCKADDR*)&SenderAddr, &senderAddrSize);
            }

            return recvlen;
        }

        int  SendData(char* buffer, int buffer_size) {
            int result;
            result = sendto(sock, buffer, buffer_size, 0, (SOCKADDR*)&SendAddr, sizeof(SendAddr));

            return result;
        }

        bool Close() {
            closesocket(sock);
            if(WSACleanup() == SOCKET_ERROR) {
                std::cerr << "UDPWinSocket | Error while shutting down WinSock!\n";
                return false;
            }
            return true;
        }

        sockaddr_in& GetSender() {
            return SenderAddr;
        }

    private:
        WSADATA         wsaData;
        SOCKET          sock;

        sockaddr_in     RecvAddr;
        sockaddr_in     SendAddr;
        sockaddr_in     SenderAddr;
};

#endif // UDPWINSOCKET_HPP_INCLUDED
