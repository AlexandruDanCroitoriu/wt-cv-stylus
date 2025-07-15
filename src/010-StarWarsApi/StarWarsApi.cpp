#include "010-StarWarsApi/StarWarsApi.h"
#include "003-Components/Button.h"

#include <Wt/WText.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

StarWarsApi::StarWarsApi()
{
    // addNew<Wt::WText>("This is the Portofolio Page. It will contain information about my projects, skills, and experiences. Stay tuned for updates!");
    
    auto navigation = addNew<Wt::WContainerWidget>();
    navigation->setStyleClass("flex items-center");
    stack_ = addNew<Wt::WStackedWidget>();
    // stack_->setStyleClass("border p-4");

    cpr::Response r = cpr::Get(cpr::Url{base_url_});
    r.status_code;                  // 200
    r.header["content-type"];       // application/json; charset=utf-8
    r.text;   
    
    // star_wars_wrapper->addNew<Wt::WText>("Star Wars API response status code: " + std::to_string(r.status_code));
    // star_wars_wrapper->addNew<Wt::WText>("Star Wars API response content type: " + r.header["content-type"]);
    // star_wars_wrapper->addNew<Wt::WText>("Star Wars API response text: " + r.text);

    nlohmann::json json_response = nlohmann::json::parse(r.text);

    for (auto it = json_response.begin(); it != json_response.end(); ++it) {
        std::string key = it.key();
        std::string url = it.value();
        // star_wars_wrapper->addNew<Wt::WText>(key + ": " + value);
        auto nav_btn = navigation->addNew<Button>(key, "text-sm", PenguinUiWidgetTheme::BtnPrimaryOutline);
        
        nav_btn->clicked().connect([=](){
            setView(url);
        });
        if(key == "films") 
            nav_btn->clicked().emit(Wt::WMouseEvent());

    }
    // addNew<Wt::WText>("GitHub API response cookies: " + r.;
}

void StarWarsApi::setView(std::string url)
{
    if(std::find(data_already_added_to_stack_.begin(), data_already_added_to_stack_.end(), url) == data_already_added_to_stack_.end()) {

        auto wrapper = stack_->addNew<Wt::WContainerWidget>();
        wrapper->setStyleClass("space-y-4");
        data_already_added_to_stack_.push_back(url);
        stack_->setCurrentWidget(wrapper);
        // First, fetch the schema to get required fields
        cpr::Response schema_response = cpr::Get(cpr::Url{url + "/schema"});
        std::vector<std::string> required_fields;
        
        if(schema_response.status_code == 200) {
            try {
                nlohmann::json schema = nlohmann::json::parse(schema_response.text);
                if(schema.contains("required") && schema["required"].is_array()) {
                    for(const auto& field : schema["required"]) {
                        required_fields.push_back(field.get<std::string>());
                    }
                }
            } catch(const std::exception& e) {
                std::cout << "Schema parsing error: " << e.what() << std::endl;
            }
        }
        
        // Now fetch the actual films data
        cpr::Response films_response = cpr::Get(cpr::Url{url});

        
        if(films_response.status_code == 200) {
            try {
                nlohmann::json data = nlohmann::json::parse(films_response.text);
                
                // Display schema info first
                auto schema_info = wrapper->addNew<Wt::WContainerWidget>();
                schema_info->setStyleClass("bg-blue-50 p-4 rounded-lg mb-4");
                auto schema_title = schema_info->addNew<Wt::WText>("Required Fields from Schema:");
                schema_title->setStyleClass("font-bold text-blue-800 mb-2");
                
                auto fields_list = schema_info->addNew<Wt::WContainerWidget>();
                fields_list->setStyleClass("flex flex-wrap gap-2");
                for(const auto& field : required_fields) {
                    auto field_tag = fields_list->addNew<Wt::WText>(field);
                    field_tag->setStyleClass("bg-blue-200 text-blue-800 px-2 py-1 rounded text-xs");
                }
                
                // Display data showing only required fields
                if(data.is_array()) {
                    for(const auto& data_item : data) {
                        auto card = wrapper->addNew<Wt::WContainerWidget>();
                        card->setStyleClass("bg-gray-100 p-6 rounded-lg shadow-md");
                        
                        // Only display required fields
                        for(const auto& field : required_fields) {
                            if(data_item.contains(field)) {
                                auto field_container = card->addNew<Wt::WContainerWidget>();
                                field_container->setStyleClass("mb-2");
                                
                                auto field_label = field_container->addNew<Wt::WText>(field + ": ");
                                field_label->setStyleClass("font-semibold text-gray-700");
                                
                                // Handle different field types
                                if(field == "opening_crawl") {
                                    auto crawl = field_container->addNew<Wt::WText>(data_item[field].get<std::string>());
                                    crawl->setStyleClass("text-sm italic bg-black text-yellow-300 p-2 rounded font-mono block mt-1");
                                } else if(field == "episode_id") {
                                    auto episode = field_container->addNew<Wt::WText>(std::to_string(data_item[field].get<int>()));
                                    episode->setStyleClass("text-blue-600 font-bold");
                                } else if(data_item[field].is_array()) {
                                    auto array_size = field_container->addNew<Wt::WText>("(" + std::to_string(data_item[field].size()) + " items)");
                                    array_size->setStyleClass("text-gray-500 text-sm");
                                } else if(data_item[field].is_string()) {
                                    auto value = field_container->addNew<Wt::WText>(data_item[field].get<std::string>());
                                    value->setStyleClass("text-gray-800");
                                }
                            }
                        }
                    }
                }
            } catch(const std::exception& e) {
                auto error_msg = wrapper->addNew<Wt::WText>("Error parsing films data: " + std::string(e.what()));
                error_msg->setStyleClass("text-red-600 ");
            }
        } else {
            auto error_msg = wrapper->addNew<Wt::WText>("Failed to fetch films data. Status: " + std::to_string(films_response.status_code));
            error_msg->setStyleClass("text-red-600 ");
        }
        
    }else {
        stack_->setCurrentIndex(std::distance(data_already_added_to_stack_.begin(), std::find(data_already_added_to_stack_.begin(), data_already_added_to_stack_.end(), url)));
    }
}
