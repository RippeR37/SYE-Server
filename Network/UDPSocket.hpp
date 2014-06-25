#ifndef UDPSOCKET_HPP_INCLUDED
#define UDPSOCKET_HPP_INCLUDED

/**
 * Abstract class provides unified access to native Socket API on different platforms.
 * Implemented platforms are:
 * - Windows (WinSock2)     in win32/UDPWinSocket.hpp
 * - Linux (BSD Sockets)    in linux/UDPBSDSocket.hpp
 *
 * InitSocket()     Initialization of sockets (on some platforms necessary)
 * Set____Addr()    Setting up addresses and ports for structures
 * SetSocket()      Creates socket for future use.
 * Bind()           Binds socket to previously set up adresses and ports
 * Close()          Closes socket
 * ReadData()       Read data from socket to given buffer with timeout
 * SendData()       Sends data to SendAddr address from given buffer
**/

class UDPSocket {
    public:
        UDPSocket() { }
        virtual ~UDPSocket() { }

        virtual bool InitSocket() = 0;
        virtual void SetRecvAddr(int incoming_port) = 0;
        virtual void SetSendAddr(char* ip, int outgoing_port) = 0;
        virtual void SetSendAddr(u_long ip, u_short outgoing_port) = 0;
        virtual void SetSendAddrAsReply() = 0;
        virtual bool SetSocket() = 0;
        virtual bool Bind() = 0;
        virtual bool Close() = 0;

        virtual sockaddr_in& GetSender() = 0;

        virtual int  ReadData(char* buffer, int buffer_size, int timeout_s, int timeout_ms) = 0;
        virtual int  SendData(char* buffer, int buffer_size) = 0;

    protected:

};

/**
 * Including implementation of UDPSocket() in given platform
**/
#ifdef __WIN32
#include "win32/UDPWinSocket.hpp"
#endif
#ifdef __LINUX
#include "linux/UDPBSDSocket.hpp"
#endif

#endif // UDPSOCKET_HPP_INCLUDED
