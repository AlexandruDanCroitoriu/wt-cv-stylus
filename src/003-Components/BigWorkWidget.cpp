#include "003-Components/BigWorkWidget.h"
#include <Wt/WApplication.h>
#include <Wt/WText.h>
#include <iostream>
#include <thread>
#include <chrono>

BigWorkWidget::BigWorkWidget()
    : Wt::WContainerWidget()
{
    addStyleClass("bg-surface border border-outline rounded-radius p-4 mb-4");
    
    // Add title
    auto title = addWidget(std::make_unique<Wt::WText>("Server Push Demo"));
    title->addStyleClass("text-lg font-semibold mb-4 text-on-surface-strong");
    
    // Add description
    auto description = addWidget(std::make_unique<Wt::WText>(
        "This demonstrates server push functionality with a background worker thread. "
        "The progress bar updates in real-time while work is being performed in another thread."));
    description->addStyleClass("text-sm text-on-surface mb-4");

    startButton_ = addWidget(std::make_unique<Wt::WPushButton>("Start"));
    startButton_->clicked().connect(startButton_, &Wt::WPushButton::disable);
    startButton_->clicked().connect(this, &BigWorkWidget::startBigWork);
    startButton_->addStyleClass("bg-primary text-on-primary px-4 py-2 rounded-radius mr-2 mb-2");

    progress_ = addWidget(std::make_unique<Wt::WProgressBar>());
    progress_->setInline(false);
    progress_->setMinimum(0);
    progress_->setMaximum(20);
    progress_->addStyleClass("w-full h-2 bg-surface-alt rounded-radius mb-2");
}

BigWorkWidget::~BigWorkWidget() {
    if (workThread_.get_id() != std::this_thread::get_id() &&
        workThread_.joinable())
        workThread_.join();
}

void BigWorkWidget::startBigWork() {
    Wt::WApplication *app = Wt::WApplication::instance();

    // Enable server push
    app->enableUpdates(true);

    if (workThread_.joinable())
        workThread_.join();
    workThread_
        = std::thread(std::bind(&BigWorkWidget::doBigWork, this, app));

    progress_->setValue(0);
    startButton_->setText("Working...");
}

/*
 * This function runs from another thread.
 *
 * From within this thread, we cannot use WApplication::instance(),
 * since that uses thread-local storage. We can only access
 * WApplication::instance() after we have grabbed its update-lock.
 */
void BigWorkWidget::doBigWork(Wt::WApplication *app)
{
    for (unsigned i = 0; i < 20; ++i) {
        // Do 50 ms of simulated work.
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Get the application update lock to update the user-interface
        // with a progress indication.
        Wt::WApplication::UpdateLock uiLock(app);
        if (uiLock) {
            progress_->setValue(i + 1);
            app->triggerUpdate();
        } else
            return;
    }

    Wt::WApplication::UpdateLock uiLock(app);

    if (uiLock) {
        startButton_->enable();
        startButton_->setText("Start Again!");

        app->triggerUpdate();

        // Disable server push
        app->enableUpdates(false);
    } else
        return;
}
