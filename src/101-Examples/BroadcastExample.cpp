#include "101-Examples/BroadcastExample.h"
#include <Wt/WApplication.h>
#include <Wt/WServer.h>
#include <Wt/WText.h>
#include <Wt/WVBoxLayout.h>

BroadcastServer::BroadcastServer()
    : counter_(0), stop_(false)
{
    thread_ = std::thread(std::bind(&BroadcastServer::run, this));
}

BroadcastServer::~BroadcastServer()
{
    stop_ = true;
    if (thread_.joinable()) {
        thread_.join();
    }
}

void BroadcastServer::connect(Client *client, const std::function<void()>& function)
{
    std::unique_lock<std::mutex> lock(mutex_);
    connections_.push_back(
        Connection(Wt::WApplication::instance()->sessionId(), client, function));
}

void BroadcastServer::disconnect(Client *client)
{
    std::unique_lock<std::mutex> lock(mutex_);
    for (unsigned i = 0; i < connections_.size(); ++i) {
        if (connections_[i].client == client) {
            connections_.erase(connections_.begin() + i);
            return;
        }
    }
}

int BroadcastServer::getCount() const
{
    std::unique_lock<std::mutex> lock(mutex_);
    return counter_;
}

void BroadcastServer::run()
{
    /*
     * This method simulates changes to the data that happen in a background
     * thread.
     */
    for (;;) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        if (stop_)
            return;

        {
            std::unique_lock<std::mutex> lock(mutex_);
            ++counter_;

            /* This is where we notify all connected clients. */
            for (unsigned i = 0; i < connections_.size(); ++i) {
                Connection& c = connections_[i];
                Wt::WServer::instance()->post(c.sessionId, c.function);
            }
        }
    }
}

BroadcastWidget::BroadcastWidget()
    : Wt::WText()
{
    Wt::WApplication *app = Wt::WApplication::instance();

    BroadcastServer::getInstance().connect(this, std::bind(&BroadcastWidget::updateData, this));

    app->enableUpdates(true);

    updateData();
}

BroadcastWidget::~BroadcastWidget()
{
    BroadcastServer::getInstance().disconnect(this);
    Wt::WApplication::instance()->enableUpdates(false);
}

void BroadcastWidget::updateData()
{
    setText(Wt::WString("Count: {1}").arg(BroadcastServer::getInstance().getCount()));
    Wt::WApplication::instance()->triggerUpdate();
}

BroadcastExample::BroadcastExample()
{
    setStyleClass("border border-outline rounded-radius p-4 mb-4 bg-surface");
    
    auto layout = setLayout(std::make_unique<Wt::WVBoxLayout>());
    
    auto title = layout->addWidget(std::make_unique<Wt::WText>("Broadcast Example"));
    title->addStyleClass("text-xl font-bold mb-2 text-on-surface-strong");
    
    auto description = layout->addWidget(std::make_unique<Wt::WText>(
        "This example demonstrates server push functionality. "
        "A background thread increments a counter every second, "
        "and all connected clients are automatically updated in real-time."));
    description->addStyleClass("text-sm text-on-surface mb-4");
    
    auto counter_label = layout->addWidget(std::make_unique<Wt::WText>("Live Counter:"));
    counter_label->addStyleClass("text-sm font-semibold text-on-surface-strong mb-2");
    
    auto broadcast_widget = layout->addWidget(std::make_unique<BroadcastWidget>());
    broadcast_widget->addStyleClass("text-lg font-mono bg-surface-alt p-2 rounded border border-outline text-primary");
    
    auto info = layout->addWidget(std::make_unique<Wt::WText>(
        "Open multiple browser tabs/windows to see the same counter "
        "updating simultaneously across all sessions."));
    info->addStyleClass("text-xs text-on-surface-weak mt-4 italic");
}
