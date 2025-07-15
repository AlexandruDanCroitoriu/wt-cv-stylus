#pragma once

#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WProgressBar.h>
#include <thread>

class BigWorkWidget : public Wt::WContainerWidget
{
public:
    BigWorkWidget();
    virtual ~BigWorkWidget();

private:
    Wt::WPushButton *startButton_;
    Wt::WProgressBar *progress_;
    std::thread workThread_;

    void startBigWork();
    void doBigWork(Wt::WApplication *app);
};
