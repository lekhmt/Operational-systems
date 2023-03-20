#ifndef _SOCKET_H
#define _SOCKET_H

#include <string>
#include "message.h"

using namespace std;

class Socket {
public:
    Socket(void *context, SocketType socketType, const string& address) :
            socketType(socketType), address(address) {
        socket = createSocket(context, socketType);
        switch (socketType) {
            case SocketType::PUBLISHER:
                bindSocket(socket, address);
                break;
            case SocketType::SUBSCRIBER:
                connectSocket(socket, address);
                break;
            default:
                throw logic_error("undefined connection type");
        }
    }

    ~Socket() {
        try {
            switch (socketType) {
                case SocketType::PUBLISHER:
                    unbindSocket(socket, address);
                    break;
                case SocketType::SUBSCRIBER:
                    disconnectSocket(socket, address);
                    break;
            }
            closeSocket(socket);
        } catch (exception& ex){
            cout << "Socket wasn't closed: " << ex.what() << endl;
        }
    }

    void send(Message message) {
        if (socketType == SocketType::PUBLISHER){
            sendMessage(socket, message);
        } else {
            throw logic_error("SUBSCRIBER can't send messages");
        }
    }

    Message receive() {
        if (socketType == SocketType::SUBSCRIBER){
            return getMessage(socket);
        } else {
            throw logic_error("PUBLISHER can't receive messages");
        }
    }

    string getAddress() const {
        return address;
    }

    void *&getSocket() {
        return socket;
    }

private:
    void *socket;
    SocketType socketType;
    string address;
};



#endif