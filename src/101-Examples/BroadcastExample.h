#pragma once

#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <functional>
#include <vector>

/*
 * Simple interface to uniquely identify a client
 */
class Client {
};

/*
 * A (singleton) server class which would protect and manage a shared
 * resource. In our example we take a simple counter as data.
 */
class BroadcastServer {
public:
    static BroadcastServer& getInstance() {
        static BroadcastServer instance;
        return instance;
    }

    void connect(Client *client, const std::function<void()>& function);
    void disconnect(Client *client);
    int getCount() const;

private:
    BroadcastServer();
    ~BroadcastServer();

    struct Connection {
        Connection(const std::string& id, Client *c, const std::function<void()>& f)
            : sessionId(id), client(c), function(f) { }

        std::string sessionId;
        Client *client;
        std::function<void()> function;
    };

    mutable std::mutex mutex_;
    std::thread thread_;
    int counter_;
    bool stop_;
    std::vector<Connection> connections_;

    void run();
};

/*
 * A widget which displays the server data, keeping itself up-to-date
 * using server push.
 */
class BroadcastWidget : public Wt::WText, public Client
{
public:
    BroadcastWidget();
    virtual ~BroadcastWidget();

private:
    void updateData();
};

/*
 * Container widget that demonstrates the broadcast functionality
 */
class BroadcastExample : public Wt::WContainerWidget
{
public:
    BroadcastExample();
};
