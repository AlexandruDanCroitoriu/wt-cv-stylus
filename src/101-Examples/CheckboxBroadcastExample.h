#pragma once

#include <Wt/WContainerWidget.h>
#include <Wt/WCheckBox.h>
#include <thread>
#include <mutex>
#include <functional>
#include <vector>
#include <array>

/*
 * Simple interface to uniquely identify a client for checkbox management
 */
class CheckboxClient {
};

/*
 * A (singleton) server class which manages shared checkbox states
 */
class CheckboxBroadcastServer {
public:
    static CheckboxBroadcastServer& getInstance() {
        static CheckboxBroadcastServer instance;
        return instance;
    }

    void connect(CheckboxClient *client, const std::function<void(int, bool)>& function);
    void disconnect(CheckboxClient *client);
    void updateCheckbox(int index, bool checked);
    bool getCheckboxState(int index) const;
    std::array<bool, 10> getAllStates() const;

private:
    CheckboxBroadcastServer();
    ~CheckboxBroadcastServer() = default;

    struct Connection {
        Connection(const std::string& id, CheckboxClient *c, const std::function<void(int, bool)>& f)
            : sessionId(id), client(c), function(f) { }

        std::string sessionId;
        CheckboxClient *client;
        std::function<void(int, bool)> function;
    };

    mutable std::mutex mutex_;
    std::array<bool, 10> checkboxStates_;
    std::vector<Connection> connections_;
};

/*
 * A widget that manages 10 checkboxes with synchronized state across sessions
 */
class CheckboxBroadcastWidget : public Wt::WContainerWidget, public CheckboxClient
{
public:
    CheckboxBroadcastWidget();
    virtual ~CheckboxBroadcastWidget();

private:
    std::array<Wt::WCheckBox*, 10> checkboxes_;
    bool updating_;

    void onCheckboxChanged(int index);
    void updateFromServer(int index, bool checked);
    void initializeStates();
};

/*
 * Container widget that demonstrates the checkbox broadcast functionality
 */
class CheckboxBroadcastExample : public Wt::WContainerWidget
{
public:
    CheckboxBroadcastExample();
};
