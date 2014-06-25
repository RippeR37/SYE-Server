#ifndef NETWORK_TYPES_H_INCLUDED
#define NETWORK_TYPES_H_INCLUDED

#include <winsock2.h>

namespace Network {

    enum APPLICATION_TYPE {
        TYPE_SERVER         = 0,
        TYPE_CLIENT         = 1,
    };

    enum CONNECTION_STATE {
        STATE_UNCONNECTED   = 0,
        STATE_CONNECTED     = 1,
        STATE_TRYING_C      = 2,
        STATE_TRYING_D      = 3,
    };

    enum MSG_TYPE {
        MSG_ACK             = 100,
        MSG_REQ_CONNECT     = 101,
        MSG_REQ_DISCONNECT  = 102,
        MSG_REQ_DENIED      = 103,
        MSG_DATA            = 104,
    };

    struct ClientInfo {
        ClientInfo() { }
        ClientInfo(int id, sockaddr_in& addr) : clientID(id), clientAddr(addr) { }

        int         clientID;
        sockaddr_in clientAddr;
    };

}

#endif // NETWORK_TYPES_H_INCLUDED
