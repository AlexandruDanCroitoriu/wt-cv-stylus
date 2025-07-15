#include "101-Examples/CheckboxBroadcastExample.h"
#include <Wt/WApplication.h>
#include <Wt/WServer.h>
#include <Wt/WText.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WGridLayout.h>

CheckboxBroadcastServer::CheckboxBroadcastServer()
{
    // Initialize all checkboxes as unchecked
    checkboxStates_.fill(false);
}

void CheckboxBroadcastServer::connect(CheckboxClient *client, const std::function<void(int, bool)>& function)
{
    std::unique_lock<std::mutex> lock(mutex_);
    connections_.push_back(
        Connection(Wt::WApplication::instance()->sessionId(), client, function));
}

void CheckboxBroadcastServer::disconnect(CheckboxClient *client)
{
    std::unique_lock<std::mutex> lock(mutex_);
    for (unsigned i = 0; i < connections_.size(); ++i) {
        if (connections_[i].client == client) {
            connections_.erase(connections_.begin() + i);
            return;
        }
    }
}

void CheckboxBroadcastServer::updateCheckbox(int index, bool checked)
{
    if (index < 0 || index >= 10) return;
    
    {
        std::unique_lock<std::mutex> lock(mutex_);
        checkboxStates_[index] = checked;

        /* Notify all connected clients about the change */
        for (unsigned i = 0; i < connections_.size(); ++i) {
            Connection& c = connections_[i];
            Wt::WServer::instance()->post(c.sessionId, [=]() {
                c.function(index, checked);
            });
        }
    }
}

bool CheckboxBroadcastServer::getCheckboxState(int index) const
{
    if (index < 0 || index >= 10) return false;
    
    std::unique_lock<std::mutex> lock(mutex_);
    return checkboxStates_[index];
}

std::array<bool, 10> CheckboxBroadcastServer::getAllStates() const
{
    std::unique_lock<std::mutex> lock(mutex_);
    return checkboxStates_;
}

CheckboxBroadcastWidget::CheckboxBroadcastWidget()
    : updating_(false)
{
    Wt::WApplication *app = Wt::WApplication::instance();

    // Connect to the server
    CheckboxBroadcastServer::getInstance().connect(this, 
        std::bind(&CheckboxBroadcastWidget::updateFromServer, this, std::placeholders::_1, std::placeholders::_2));

    app->enableUpdates(true);

    // Create layout
    auto layout = setLayout(std::make_unique<Wt::WGridLayout>());
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 1);

    // Create 10 checkboxes in a 5x2 grid
    for (int i = 0; i < 10; ++i) {
        int row = i / 2;
        int col = i % 2;
        
        auto checkboxContainer = std::make_unique<Wt::WContainerWidget>();
        checkboxContainer->addStyleClass("flex items-center space-x-2 p-2");
        
        auto checkbox = checkboxContainer->addNew<Wt::WCheckBox>();
        checkbox->addStyleClass("form-checkbox h-4 w-4 text-primary");
        
        auto label = checkboxContainer->addNew<Wt::WText>(Wt::WString("Checkbox {1}").arg(i + 1));
        label->addStyleClass("text-sm text-on-surface");
        
        checkboxes_[i] = checkbox;
        
        // Connect the checkbox change event
        checkbox->changed().connect([=]() {
            if (!updating_) {
                onCheckboxChanged(i);
            }
        });
        
        layout->addWidget(std::move(checkboxContainer), row, col);
    }

    // Initialize with current server states
    initializeStates();
}

CheckboxBroadcastWidget::~CheckboxBroadcastWidget()
{
    CheckboxBroadcastServer::getInstance().disconnect(this);
    Wt::WApplication::instance()->enableUpdates(false);
}

void CheckboxBroadcastWidget::onCheckboxChanged(int index)
{
    if (index < 0 || index >= 10 || !checkboxes_[index]) return;
    
    bool checked = checkboxes_[index]->isChecked();
    CheckboxBroadcastServer::getInstance().updateCheckbox(index, checked);
}

void CheckboxBroadcastWidget::updateFromServer(int index, bool checked)
{
    if (index < 0 || index >= 10 || !checkboxes_[index]) return;
    
    updating_ = true;
    checkboxes_[index]->setChecked(checked);
    updating_ = false;
    
    Wt::WApplication::instance()->triggerUpdate();
}

void CheckboxBroadcastWidget::initializeStates()
{
    updating_ = true;
    auto states = CheckboxBroadcastServer::getInstance().getAllStates();
    for (int i = 0; i < 10; ++i) {
        if (checkboxes_[i]) {
            checkboxes_[i]->setChecked(states[i]);
        }
    }
    updating_ = false;
}

CheckboxBroadcastExample::CheckboxBroadcastExample()
{
    setStyleClass("border border-outline rounded-radius p-4 mb-4 bg-surface");
    
    auto layout = setLayout(std::make_unique<Wt::WVBoxLayout>());
    
    auto title = layout->addWidget(std::make_unique<Wt::WText>("Checkbox Broadcast Example"));
    title->addStyleClass("text-xl font-bold mb-2 text-on-surface-strong");
    
    auto description = layout->addWidget(std::make_unique<Wt::WText>(
        "This example demonstrates synchronized checkbox states across multiple sessions. "
        "When you check or uncheck a checkbox, the change is immediately broadcast to all other "
        "connected clients in real-time."));
    description->addStyleClass("text-sm text-on-surface mb-4");
    
    auto checkbox_widget = layout->addWidget(std::make_unique<CheckboxBroadcastWidget>());
    checkbox_widget->addStyleClass("bg-surface-alt p-4 rounded border border-outline");
    
    auto info = layout->addWidget(std::make_unique<Wt::WText>(
        "Open multiple browser tabs/windows and check/uncheck boxes to see the "
        "synchronization in action across all sessions."));
    info->addStyleClass("text-xs text-on-surface-weak mt-4 italic");
}
