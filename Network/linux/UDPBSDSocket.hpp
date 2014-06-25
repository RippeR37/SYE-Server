#ifndef UDPBSDSOCKET_HPP_INCLUDED
#define UDPBSDSOCKET_HPP_INCLUDED

#include "../UDPSocket.hpp"
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


class UDPBSDSocket : public UDPSocket {
    public:
        UDPBSDSocket() { }
       ~UDPBSDSocket() { }

        bool InitSocket() {
            bzero(&RecvAddr, sizeof(RecvAddr));
            bzero(&SendAddr, sizeof(SendAddr));

            RecvAddr.sin_family     = AF_INET;
            SendAddr.sin_family     = AF_INET;

            return true;
        }

        void SetRecvAddr(int incoming_port) {
            RecvAddr.sin_port           = htons(incoming_port);
            RecvAddr.sin_addr.s_addr    = htonl(INADDR_ANY);
        }

        void SetSendAddr(char* ip, int outgoing_port) {
            SendAddr.sin_port           = htons(outgoing_port);
            inet_pton(AF_INET, ip, &SendAddr.sin_addr);
        }

        void SetSendAddr(ulong ip, u_short outgoing_port) {
            SendAddr.sin_port           = outgoing_port;
            SendAddr.sin_addr.s_addr    = ip;
        }

        void SetSendAddrAsReply() {
            SendAddr.sin_port           = SenderAddr.sin_port;
            SendAddr.sin_addr.s_addr    = SenderAddr.sin_addr.s_addr;
        }

        bool SetSocket() {
            sock = socket(AF_INET, SOCK_DGRAM, 0);
            if(sock == -1) {
                std::cerr << "UDPBSDSocket | Error while creating a socket!\n";
                return false;
            }
            return true;
        }

        bool Bind() {
            if(bind(sock, (struct sockaddr*)&RecvAddr, sizeof(RecvAddr)) == -1) {
                std::cerr << "UDPBSDSocket | Error while binding a socket!\n";
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
            timeout.tv_usec = timeout_ms * 100;

            activity = select(sock+1, &read_fds, NULL, NULL, &timeout);
            if(activity < 1) {
                return 0;
            } else {
                recvlen = recvfrom(sock, buffer, buffer_size, 0, (struct sockaddr*)&SenderAddr, &senderAddrSize);
            }

            return recvlen;
        }

        int  SendData(char* buffer, int buffer_size) {
            int result;
            result = sendto(sock, buffer, buffer_size, 0, (struct sockaddr*)&SendAddr, sizeof(SendAddr));

            return result;
        }

        bool Close() {
            if(close(sock) == -1)
                return false;
            return true;
        }

        sockaddr_in& GetSender() {
            return SenderAddr;
        }

    private:
        int                     sock;

        struct sockaddr_in      RecvAddr;
        struct sockaddr_in      SendAddr;
        struct sockaddr_in      SenderAddr;
};

#endif // UDPBSDSOCKET_HPP_INCLUDED
