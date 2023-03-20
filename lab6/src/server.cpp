#include <iostream>
#include <vector>
#include <unistd.h>
#include <csignal>
#include "headers/message.h"
#include "headers/socket.h"
#include "headers/tree.h"
#include "zmq.h"

#define SECOND 1'000'000

void *receiveFunction(void *server);

void *heartbeatFunction(void *server);

class Server {
public:

    void commandProcessing(const string &cmd) {
        if (cmd == "create") {
            int id;
            cin >> id;
            createChild(id);
        } else if (cmd == "exec") {
            int id;
            cin >> id;
            int n;
            cin >> n;
            execChild(id, n);
        } else if (cmd == "exit") {
            throw invalid_argument("Exiting...");
        } else if (cmd == "heartbeat") {
            heartbeat();
        } else if (cmd == "status") {
            int id;
            cin >> id;
            if (!getTree().find(id)) {
                throw runtime_error("Error: node " + to_string(id) + "  doesn't exist");
            }
            if (check(id)) {
                cout << "OK" << endl;
            } else {
                cout << "Node " + to_string(id) + " is unavailable" << endl;
            }
        } else {
            cout << "invalid command\n";
        }
    }

    Server() {
        context = createContext();
        pid = getpid();
        string address = createAddress(AddressType::CHILD_PUB_LEFT, pid);
        publisher = new Socket(context, SocketType::PUBLISHER, address);
        if (pthread_create(&receiveMessage, nullptr, receiveFunction, this) != 0) {
            throw runtime_error("thread create error");
        }
        working = true;
    }

    ~Server() {
        if (!working) return;
        working = false;
        send(Message(CommandType::REMOVE_CHILD, 0, 0));
        try {
            delete publisher;
            delete subscriber;
            publisher = nullptr;
            subscriber = nullptr;
            destroyContext(context);
            usleep(7.5 * SECOND);
        } catch (runtime_error &err) {
            cout << "Server wasn't stopped " << err.what() << endl;
        }
    }

    void send(Message msg) {
        msg.withoutProcessing = false;
        publisher->send(msg);
    }

    void createChild(int id) {
        if (t.find(id)) {
            throw runtime_error("Error: node " + to_string(id) + " already exists");
        }
        if (t.getPlace(id) && !check(t.getPlace(id))) {
            throw runtime_error("Error: parent node " + to_string(t.getPlace(id)) + " is unavailable");
        }
        send(Message(CommandType::CREATE_CHILD, t.getPlace(id), id));
        t.insert(id);
    }

    void execChild(int id, int n) {
        double nums[n];
        for (int i = 0; i < n; ++i) {
            int cur;
            cin >> cur;
            nums[i] = cur;
        }
        if (!t.find(id)) {
            throw runtime_error("Error: node " + to_string(id) + " doesn't exist");
        }
        if (!check(id)) {
            throw runtime_error("Error: node " + to_string(id) + " is unavailable");
        }
        send(Message(CommandType::EXEC_CHILD, id, n, nums, 0));
    }

    bool check(int id) {
        Message msg(CommandType::RETURN, id, 0);
        send(msg);
        usleep(SECOND);
        msg.getToIndex() = SERVER_ID;
        return lastMessage == msg;
    }

    bool check(int id, int time) {
        int timeout = 4 * time;
        zmq_setsockopt(getSubscriber()->getSocket(), ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
        Message msg(CommandType::RETURN, id, 0);
        send(msg);
        usleep(timeout * 1000);
        //msg.getToIndex() = SERVER_ID;

        std::cout << "msg Node " << id << " " << msg.toIndex << " " << msg.createIndex << " " << msg.uniqueIndex << endl;
        std::cout << "lst Node " << id << " " << lastMessage.toIndex << " " << lastMessage.createIndex << " " << lastMessage.uniqueIndex << endl;
        return true;
    }

    Socket *&getPublisher() {
        return publisher;
    }

    Socket *&getSubscriber() {
        return subscriber;
    }

    void *getContext() {
        return context;
    }

    Tree &getTree() {
        return t;
    }

    Message lastMessage;

    pthread_t heartbeatThread;
    int heartbeatTime;
    bool isHeartbeat;

    void heartbeat() {
        if (!isHeartbeat) {
            int time;
            cin >> time;
            heartbeatTime = time;
            isHeartbeat = true;
            if (pthread_create(&heartbeatThread, nullptr, heartbeatFunction, this) != 0) {
                throw runtime_error("thread create error");
            }
        } else {
            isHeartbeat = false;
            if (pthread_join(heartbeatThread, nullptr) != 0) {
                throw runtime_error("thread join error");
            }
        }
    }

private:
    pid_t pid;
    Tree t;
    void *context;
    Socket *publisher;
    Socket *subscriber;
    bool working;
    pthread_t receiveMessage;
};


void *receiveFunction(void *server) {
    auto *serverPointer = (Server *) server;
    try {
        pid_t child_pid = fork();
        if (child_pid == -1) throw runtime_error("Can not fork.");
        if (child_pid == 0) {
            execl("client", "client", "0", serverPointer->getPublisher()->getAddress().data(), nullptr);
            throw runtime_error("Can not execl");
        }
        string address = createAddress(AddressType::PARENT_PUB, child_pid);
        serverPointer->getSubscriber() = new Socket(serverPointer->getContext(), SocketType::SUBSCRIBER, address);
        serverPointer->getTree().insert(0);
        while(true) {

            Message msg = serverPointer->getSubscriber()->receive();
            if (msg.command == CommandType::ERROR) {
                continue;
            }
            serverPointer->lastMessage = msg;
            switch (msg.command) {
                case CommandType::CREATE_CHILD:
                    cout << "OK: " << msg.getCreateIndex() << endl;
                    break;
                case CommandType::RETURN:
                    break;
                case CommandType::EXEC_CHILD:
                    cout << "OK: response from node " << msg.getCreateIndex() << " is " << msg.value[0] << endl;
                    break;
                default:
                    break;
            }
        }
    } catch (runtime_error &err) {
        cout << "Server wasn't started " << err.what() << endl;
    }
    return nullptr;
}

void *heartbeatFunction(void *server) {
    auto *serverPointer = (Server *) server;
    while (serverPointer->isHeartbeat) {
        vector<int> tmp = serverPointer->getTree().getElements();
        bool answer = true;
        for (int &e: tmp) {
            if (!(serverPointer->check(e, serverPointer->heartbeatTime))) {
                answer = false;
                cout << "Heartbeat: node " << e << " is unavailable now\n";
            }
        }
        if (answer) {
            cout << "OK\n";
        }
    }
}

Server *serverPointer = nullptr;

void terminate(int) {
    if (serverPointer) {
        serverPointer->~Server();
    }
    cout << to_string(getpid()) + " successfully terminated" << endl;
    exit(0);
}

int main() {
    try {

        // ctrl + C
        if (signal(SIGINT, terminate) == SIG_ERR) {
            throw runtime_error("Can not set SIGINT signal");
        }

        // segmentation fault
        if (signal(SIGSEGV, terminate) == SIG_ERR) {
            throw runtime_error("Can not set SIGSEGV signal");
        }

        // kill
        if (signal(SIGTERM, terminate) == SIG_ERR) {
            throw runtime_error("Can not set SIGTERM signal");
        }

        Server server = Server();
        serverPointer = &server;
        cout << getpid() << " server started correctly!\n";
        while (true) {
            try {
                string cmd;
                while (cin >> cmd) {
                    server.commandProcessing(cmd);
                }
            } catch (const runtime_error &arg) {
                cout << arg.what() << endl;
            }
        }
    } catch (const runtime_error &arg) {
        cout << arg.what() << endl;
    } catch (...) {}
    return 0;
}