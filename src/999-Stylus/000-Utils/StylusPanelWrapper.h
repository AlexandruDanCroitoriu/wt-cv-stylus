#pragma once
#include <Wt/WContainerWidget.h>
#include "999-Stylus/000-Utils/StylusState.h"

namespace Stylus {

    class StylusPanelWrapper : public Wt::WContainerWidget
    {
    public:
        StylusPanelWrapper(std::shared_ptr<StylusState> state);

        std::shared_ptr<StylusState> state_;
    private:
        
    };
}
