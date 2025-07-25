#include "999-Stylus/000-Utils/StylusPanelWrapper.h"


namespace Stylus {

    StylusPanelWrapper::StylusPanelWrapper(std::shared_ptr<StylusState> state)
        : state_(state)
    {
        setStyleClass("h-[100vh]");
    }
}