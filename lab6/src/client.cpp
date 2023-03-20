#include <cstring>
#include <iostream>
#include <unistd.h>
#include <utility>
#include <vector>
#include <algorithm>
#include <csignal>
#include "headers/message.h"
#include "headers/socket.h"

using namespace std;

class Client {
private:
    int id;
    void *context;
    bool terminated;

public:
    Socket *childPublisherLeft;
    Socket *childPublisherRight;
    Socket *parentPublisher;
    Socket *parentSubscriber;
    Socket *leftSubscriber;
    Socket *rightSubscriber;

    Client(int id, const string& parentAddress) : id(id) {
        context = createContext();
        string address = createAddress(AddressType::CHILD_PUB_LEFT, getpid());
        childPublisherLeft = new Socket(context, SocketType::PUBLISHER, address);
        address = createAddress(AddressType::CHILD_PUB_RIGHT, getpid());
        childPublisherRight = new Socket(context, SocketType::PUBLISHER, address);
        address = createAddress(AddressType::PARENT_PUB, getpid());
        parentPublisher = new Socket(context, SocketType::PUBLISHER, address);
        parentSubscriber = new Socket(context, SocketType::SUBSCRIBER, parentAddress);
        leftSubscriber = nullptr;
        rightSubscriber = nullptr;
        terminated = false;
    }

    ~Client() {
        if (terminated) return;
        terminated = true;
        try {
            delete childPublisherLeft;
            delete childPublisherRight;
            delete parentPublisher;
            delete parentSubscriber;
            delete leftSubscriber;
            delete rightSubscriber;
            destroyContext(context);
        } catch (runtime_error &err) {
            cout << "Server wasn't stopped " << err.what() << endl;
        }
    }

    void messageProcessing(Message msg) {
        switch (msg.command) {
            case CommandType::ERROR:
                throw runtime_error("error message received");
            case CommandType::RETURN: {
                msg.getToIndex() = SERVER_ID;
                sendUp(msg);
                break;
            }
            case CommandType::CREATE_CHILD: {
                msg.getCreateIndex() = addChild(msg.getCreateIndex());
                msg.getToIndex() = SERVER_ID;
                sendUp(msg);
                break;
            }
            case CommandType::REMOVE_CHILD: {
                if (msg.withoutProcessing) {
                    sendUp(msg);
                    break;
                }
                if (msg.toIndex != getId() && msg.toIndex != UNIVERSAL_MESSAGE) {
                    sendDown(msg);
                    break;
                }
                msg.getToIndex() = UNIVERSAL_MESSAGE;
                sendDown(msg);
                this->~Client();
                throw invalid_argument("Exiting child...");
            }
            case CommandType::EXEC_CHILD: {
                double res = 0.0;
                for (int i = 0; i < msg.size; ++i) {
                    res += msg.value[i];
                }
                msg.getToIndex() = SERVER_ID;
                msg.getCreateIndex() = getId();
                msg.value[0] = res;
                sendUp(msg);
                break;
            }
            default:
                throw runtime_error("undefined command");
        }
    }

    void sendUp(Message msg) const {
        msg.withoutProcessing = true;
        parentPublisher->send(msg);
    }

    void sendDown(Message msg) const {
        msg.withoutProcessing = false;
        childPublisherLeft->send(msg);
        childPublisherRight->send(msg);
    }

    int getId() const {
        return id;
    }

    int addChild(int childId) {
        pid_t pid = fork();
        if (pid == -1) throw runtime_error("fork error");
        if (!pid) {
            string address;
            if (childId < id) {
                address = childPublisherLeft->getAddress();
            } else {
                address = childPublisherRight->getAddress();
            }
            execl("client", "client", to_string(childId).data(), address.data(), nullptr);
            throw runtime_error("execl error");
        }
        string address = createAddress(AddressType::PARENT_PUB, pid);
        size_t timeout = 10000;
        if (id > childId) {
            leftSubscriber = new Socket(context, SocketType::SUBSCRIBER, address);
            //zmq_setsockopt(leftSubscriber->getSocket(), ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
        } else {
            rightSubscriber = new Socket(context, SocketType::SUBSCRIBER, address);
            //zmq_setsockopt(rightSubscriber->getSocket(), ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
        }
        return pid;
    }

};

Client *clientPointer = nullptr;

void terminate(int) {
    if (clientPointer) {
        clientPointer->~Client();
    }
    cout << to_string(getpid()) + " successfully terminated" << endl;
    exit(0);
}

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        cout << "-1" << endl;
        return -1;
    }
    try {

        // Ctrl + C
        if (signal(SIGINT, terminate) == SIG_ERR) {
            throw runtime_error("Can not set SIGINT signal");
        }

        // Segmentation fault
        if (signal(SIGSEGV, terminate) == SIG_ERR) {
            throw runtime_error("Can not set SIGSEGV signal");
        }

        // kill
        if (signal(SIGTERM, terminate) == SIG_ERR) {
            throw runtime_error("Can not set SIGTERM signal");
        }

        Client client(stoi(argv[1]), string(argv[2]));
        clientPointer = &client;
        cout << getpid() << ": client " << client.getId() << " successfully started" << endl;
        while(true) {
            Message msg = client.parentSubscriber->receive();
            if (msg.toIndex != client.getId() && msg.toIndex != UNIVERSAL_MESSAGE) {
                if (msg.withoutProcessing) {
                    client.sendUp(msg);
                } else {
                    try {
                        if (client.getId() < msg.toIndex) {
                            msg.withoutProcessing = false;
                            client.childPublisherRight->send(msg);
                            msg = client.rightSubscriber->receive();
                        } else {
                            msg.withoutProcessing = false;
                            client.childPublisherLeft->send(msg);
                            msg = client.leftSubscriber->receive();
                        }
                        if (msg.command == CommandType::REMOVE_CHILD && msg.toIndex == PARENT_SIGNAL) {
                            msg.toIndex = SERVER_ID;
                            if (client.getId() < msg.getCreateIndex()) {
                                delete client.rightSubscriber;
                                client.rightSubscriber = nullptr;
                            } else {
                                delete client.leftSubscriber;
                                client.leftSubscriber = nullptr;
                            }
                        }
                        client.sendUp(msg);
                    } catch (...) {
                        client.sendUp(Message());
                    }
                }
            } else {
                clientPointer->messageProcessing(msg);
            }
        }
    } catch (runtime_error &err) {
        cout << getpid() << ": " << err.what() << '\n';
    } catch (invalid_argument &inv) {
        cout << getpid() << ": " << inv.what() << '\n';
    }
    return 0;
}