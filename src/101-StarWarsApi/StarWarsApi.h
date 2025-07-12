#pragma once
#include <Wt/WContainerWidget.h>
#include "004-Dbo/Session.h"
#include <Wt/WStackedWidget.h>

class StarWarsApi : public Wt::WContainerWidget
{
public:
    StarWarsApi();
private:
    Wt::WStackedWidget* stack_;
    std::vector<std::string> data_already_added_to_stack_;
    std::string base_url_ = "https://swapi.info/api/";

    void setView(std::string url);
};