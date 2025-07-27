#pragma once
#include <tinyxml2.h>
#include "999-Stylus/000-Utils/StylusState.h"

#include <Wt/WString.h>
#include <Wt/WSignal.h>


namespace Stylus
{

    class XMLFileBrain
    {
        public:
            XMLFileBrain(std::shared_ptr<StylusState> state, std::string file_path);
            
            std::shared_ptr<tinyxml2::XMLDocument> doc_;
            std::string file_path_;
            
            void setFile(std::string file_path);

            
            bool isValidTemplateFile();
            std::map<std::string, tinyxml2::XMLElement*> id_and_message_nodes_;
            
            Wt::Signal<tinyxml2::XMLElement*, bool> xml_node_selected_;
            Wt::Signal<> file_saved_;
            
            tinyxml2::XMLElement* selected_node_;
            std::shared_ptr<StylusState> state_;
            private:
            std::map<std::string, tinyxml2::XMLElement*> getIdsAndMessageNodes();
            
            
    };
}