#ifndef _WRAP_ZMQ_H
#define _WRAP_ZMQ_H

#include <tuple>
#include <vector>
#include <atomic>
#include "zmq.h"

using namespace std;

#define UNIVERSAL_MESSAGE (-1)
#define SERVER_ID (-2)
#define PARENT_SIGNAL (-3)

enum struct SocketType {
    PUBLISHER,
    SUBSCRIBER,
};

enum struct CommandType {
    ERROR,
    RETURN,
    CREATE_CHILD,
    REMOVE_CHILD,
    EXEC_CHILD,
};

enum struct AddressType {
    CHILD_PUB_LEFT,
    CHILD_PUB_RIGHT,
    PARENT_PUB,
};

#define MAX_CAP 1000

class Message {
protected:
    static std::atomic<int> counter;
public:
    CommandType command = CommandType::ERROR;
    int toIndex;
    int createIndex;
    int uniqueIndex;
    bool withoutProcessing;
    int size = 0;
    double value[MAX_CAP] = {0};

    Message();

    Message(CommandType command, int toIndex, int size, const double *value, int createIndex);

    Message(CommandType new_command, int new_to_id, int new_id);

    Message(string str){
        if (str == "error"){
            createIndex = -10;
        }
    }

    friend bool operator==(const Message &lhs, const Message &rhs);

    int &getCreateIndex();

    int &getToIndex();

};

void *createContext();

void destroyContext(void *context);

int getSocketType(SocketType type);

void *createSocket(void *context, SocketType type);

void closeSocket(void *socket);

string createAddress(AddressType type, pid_t id);

void bindSocket(void *socket, const string& address);

void unbindSocket(void *socket, const string& address);

void connectSocket(void *socket, const string& address);

void disconnectSocket(void *socket, const string& address);

void createMessage(zmq_msg_t *zmq_msg, Message &msg);

void sendMessage(void *socket, Message &msg);

Message getMessage(void *socket);

#endif