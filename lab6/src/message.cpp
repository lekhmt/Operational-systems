#include <tuple>
#include <cstring>
#include "headers/message.h"
#include <unistd.h>
#include <iostream>

using namespace std;

atomic<int> Message::counter;

Message::Message() {
    command = CommandType::ERROR;
    uniqueIndex = counter++;
    withoutProcessing = false;
}

Message::Message(CommandType command, int toIndex, int size, const double *value, int createIndex)
        : command(command), toIndex(toIndex), size(size), uniqueIndex(counter++), withoutProcessing(false),
          createIndex(createIndex) {
    for (int i = 0; i < size; ++i) {
        this->value[i] = value[i];
    }
}

Message::Message(CommandType command, int toIndex, int createIndex)
        : command(command), toIndex(toIndex), uniqueIndex(counter++), withoutProcessing(false),
          createIndex(createIndex) {}

bool operator==(const Message &lhs, const Message &rhs) {
    return tie(lhs.command, lhs.toIndex, lhs.createIndex, lhs.uniqueIndex) ==
           tie(rhs.command, rhs.toIndex, rhs.createIndex, rhs.uniqueIndex);
}

int &Message::getCreateIndex() {
    return createIndex;
}

int &Message::getToIndex() {
    return toIndex;
}

void *createContext() {
    void *context = zmq_ctx_new();
    if (!context) {
        throw runtime_error("unable to create new context");
    }
    return context;
}

void destroyContext(void *context) {
    sleep(1);
    if (zmq_ctx_destroy(context)) {
        throw runtime_error("unable to destroy context");
    }
}

int getSocketType(SocketType type) {
    switch (type) {
        case SocketType::PUBLISHER:
            return ZMQ_PUB;
        case SocketType::SUBSCRIBER:
            return ZMQ_SUB;
        default:
            throw runtime_error("undefined socket type");
    }
}

void *createSocket(void *context, SocketType type) {
    int zmq_type = getSocketType(type);
    void *socket = zmq_socket(context, zmq_type);
    if (!socket) {
        throw runtime_error("unable to create socket");
    }
    return socket;
}

void closeSocket(void *socket) {
    sleep(1);
    if (zmq_close(socket)) {
        throw runtime_error("unable to close socket");
    }
}

string createAddress(AddressType type, pid_t id) {
    switch (type) {
        case AddressType::PARENT_PUB:
            return "ipc://parent_publisher_" + to_string(id);
        case AddressType::CHILD_PUB_LEFT:
            return "ipc://child_publisher_left_" + to_string(id);
        case AddressType::CHILD_PUB_RIGHT:
            return "ipc://child_publisher_right" + to_string(id);
        default:
            throw runtime_error("wrong address type");
    }
}

void bindSocket(void *socket, const string& address) {
    if (zmq_bind(socket, address.data())) {
        throw runtime_error("unable to bind socket");
    }
}

void unbindSocket(void *socket, const string& address) {
    sleep(1);
    if (zmq_unbind(socket, address.data())) {
        throw runtime_error("unable to unbind socket");
    }
}

void connectSocket(void *socket, const string& address) {
    if (zmq_connect(socket, address.data())) {
        throw runtime_error("unable to connect socket");
    }
    zmq_setsockopt(socket, ZMQ_SUBSCRIBE, nullptr, 0);
}

void disconnectSocket(void *socket, const string& address) {
    if (zmq_disconnect(socket, address.data())) {
        throw runtime_error("unable to disconnect socket.");
    }
}

void createMessage(zmq_msg_t *zmq_msg, Message &msg) {
    zmq_msg_init_size(zmq_msg, sizeof(msg));
    memcpy(zmq_msg_data(zmq_msg), &msg, sizeof(msg));
}

void sendMessage(void *socket, Message &msg) {
    zmq_msg_t zmq_msg;
    createMessage(&zmq_msg, msg);
    if (!zmq_msg_send(&zmq_msg, socket, 0)) {
        throw runtime_error("unable to send message");
    }
    zmq_msg_close(&zmq_msg);
}

Message getMessage(void *socket) {
    zmq_msg_t zmq_msg;
    zmq_msg_init(&zmq_msg);
    if (zmq_msg_recv(&zmq_msg, socket, 0) == -1) {
        return {"error"};
    }
    Message msg;
    memcpy(&msg, zmq_msg_data(&zmq_msg), sizeof(msg));
    zmq_msg_close(&zmq_msg);
    return msg;
}