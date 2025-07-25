#include "999-Stylus/000-Utils/XMLFileBrain.h"

namespace Stylus
{
    XMLFileBrain::XMLFileBrain(std::shared_ptr<StylusState> state, std::string file_path)
    : state_(state),
    // doc_(std::make_shared<tinyxml2::XMLDocument>(true, tinyxml2::PEDANTIC_WHITESPACE))
    doc_(std::make_shared<tinyxml2::XMLDocument>())
    {
        setFile(file_path);
    }
    
    void XMLFileBrain::setFile(std::string file_path)
    {
        file_path_ = file_path;
        // Load the XML file
        tinyxml2::XMLError eResult = doc_->LoadFile(file_path.c_str());
        if (eResult != tinyxml2::XML_SUCCESS) {
            std::cout << "\n\nError loading XML file: " << doc_->ErrorIDToName(eResult) << "\n\n";
            std::cout << "File not found: " << file_path << "\n\n";
            selected_node_ = nullptr;
        }else {
            selected_node_ = doc_->RootElement();
            xml_node_selected_.emit(selected_node_, false);
            if(isValidTemplateFile()) {
               id_and_message_nodes_ = getIdsAndMessageNodes();
            } 
        }
    }

    bool XMLFileBrain::isValidTemplateFile()
    {
        if(!doc_ || !doc_->RootElement() || std::string(doc_->RootElement()->Name()).compare("messages") != 0) {
            std::cout<< "\n\nInvalid XML file: 1" << file_path_ << "\n\n";
            return false;
        }
        auto messages_node = doc_->RootElement();
        auto message_node = messages_node->FirstChildElement("message");
        if(!message_node || message_node->Attribute("id") == nullptr) {
            std::cout << "\n\nInvalid XML file: 2" << file_path_ << "\n\n";
            return false;
        }
        return true;
    }

    std::map<std::string, tinyxml2::XMLElement*> XMLFileBrain::getIdsAndMessageNodes()
    {
        std::map<std::string, tinyxml2::XMLElement*> ids_and_message_nodes;
        if(!isValidTemplateFile()){
            return ids_and_message_nodes; // return empty map if the file is not valid
        }
        auto messages_node = doc_->RootElement();
        for (auto message_node = messages_node->FirstChildElement("message"); message_node != nullptr; message_node = message_node->NextSiblingElement("message"))
        {
            if (message_node->Attribute("id") != nullptr)
            {
                std::string id = message_node->Attribute("id");
                ids_and_message_nodes[id] = message_node;
                // std::cout << "Found message node with id: " << id << std::endl;
            }
        }
        return ids_and_message_nodes;
    }


}
