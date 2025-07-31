#include "999-Stylus/000-Utils/StylusState.h"
#include <Wt/WApplication.h>
#include <Wt/WServer.h>
#include <Wt/WIOService.h>
#include <Wt/WRandom.h>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <regex>
#include <boost/regex.hpp>
#include "999-Stylus/000-Utils/XMLFileBrain.h"
// #include "101-Stylus/001-XmlFilesManager/Preview/XMLTreeNode.h"

namespace Stylus
{

    
    std::vector<std::pair<std::string, std::vector<std::string>>> StylusEditorManagementData::getFolders()
    {
        std::vector<std::pair<std::string, std::vector<std::string>>> return_folders;
        std::vector<std::string> folders;

        for (const auto &entry : std::filesystem::directory_iterator(root_folder_path_))
        {
            if (entry.is_directory())
            {
                folders.push_back(entry.path().filename().string());
            }
        }
        for (const auto &folder : folders)
        {
            std::vector<std::string> files;
            for (const auto &entry : std::filesystem::directory_iterator(root_folder_path_ + folder))
            {
                if (entry.is_regular_file())
                {
                    files.push_back(entry.path().filename().string());
                }
            }
            return_folders.push_back(std::make_pair(folder, files));
        }
        // sort
        std::sort(return_folders.begin(), return_folders.end(), [](const auto &a, const auto &b)
                    { return a.first < b.first; });
        for (auto &folder : return_folders)
        {
            std::sort(folder.second.begin(), folder.second.end());
        }
        // folders_changed_.emit("");
        return return_folders;
    }

    
    std::string trimWitespace(std::string str)
    {
        str.erase(0, str.find_first_not_of(" \t\n\r\f\v")); // trim from start
        str.erase(str.find_last_not_of(" \t\n\r\f\v") + 1); // trim from end
        return str;
    }
    std::string trimAllWitespace(std::string str)
    {
        str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());
        return str;
    }

    StylusState::StylusState()
        : doc_(std::make_shared<tinyxml2::XMLDocument>())
    {
        state_file_path_ = "../../static/stylus/stylus-state.xml";

        js_editor_data_.extension_ = "js";
        js_editor_data_.root_folder_path_ = "../../static/stylus-resources/js/";
        js_editor_data_.root_resource_url_ = "static/stylus-resources/js/";
        js_editor_data_.getFolders();

        xml_editor_data_.extension_ = "xml";
        xml_editor_data_.root_folder_path_ = "../../static/stylus-resources/xml/";
        xml_editor_data_.root_resource_url_ = "static/stylus-resources/xml/";
        xml_editor_data_.getFolders();

        css_editor_data_.extension_ = "css";
        css_editor_data_.root_folder_path_ = "../../static/stylus-resources/tailwind4/css/";
        css_editor_data_.root_resource_url_ = "static/stylus-resources/tailwind4/css/";
        css_editor_data_.getFolders();

        tailwind_config_editor_data_.extension_ = "css";
        tailwind_config_editor_data_.root_folder_path_ = "../../static/stylus-resources/tailwind-config/";
        tailwind_config_editor_data_.root_resource_url_ = "static/stylus-resources/tailwind-config/";

        tailwind_config_file_path_ = "../../static/stylus-resources/tailwind4/input.css";

        doc_->LoadFile(state_file_path_.c_str());
        if (doc_->ErrorID() != tinyxml2::XML_SUCCESS)
        {
            std::cerr << "Error loading stylus state XML file: " << doc_->ErrorID() << std::endl;
            // crete file
            std::ofstream file(state_file_path_);
            if (!file.is_open())
            {
                std::cerr << "Error creating file: " << state_file_path_ << std::endl;
                return;
            }
            doc_->LoadFile(state_file_path_.c_str());
        }
        stylus_node_ = doc_->FirstChildElement("stylus");
        if (stylus_node_ == nullptr)
        {
            std::cerr << "Error finding <stylus> node in XML file." << std::endl;
            stylus_node_ = doc_->NewElement("stylus");
            stylus_node_->SetAttribute("selected-menu", "templates");
            stylus_node_->SetAttribute("open", "true");
            doc_->InsertFirstChild(stylus_node_);
        }
        xml_node_ = stylus_node_->FirstChildElement("xml-manager");
        if (xml_node_ == nullptr)
        {
            std::cerr << "Error finding <xml-manager> node in XML file." << std::endl;
            xml_node_ = doc_->NewElement("xml-manager");
            xml_node_->SetAttribute("editor-width", 500);
            xml_node_->SetAttribute("editor-hidden", false);
            xml_node_->SetAttribute("preview-tree-hidden", false);
            xml_node_->SetAttribute("preview-elem-hidden", false);
            xml_node_->SetAttribute("control-center-hidden", false);
            xml_node_->SetAttribute("sidebar-width", 300);
            xml_node_->SetAttribute("selected-file-path", "");
            xml_node_->SetAttribute("preview-widget-sidebar-width", "300");
            xml_node_->SetAttribute("preview-widget-width", "300");
            xml_node_->SetAttribute("template-var-control-center-hidden", "false");

            stylus_node_->InsertEndChild(xml_node_);
        }
        css_node_ = stylus_node_->FirstChildElement("css-manager");
        if (css_node_ == nullptr)
        {
            std::cerr << "Error finding <css-manager> node in XML file." << std::endl;
            css_node_ = doc_->NewElement("css-manager");
            css_node_->SetAttribute("sidebar-width", 300);
            css_node_->SetAttribute("selected-file-path", "");
            stylus_node_->InsertEndChild(css_node_);
        }
        js_node_ = stylus_node_->FirstChildElement("js-manager");
        if (js_node_ == nullptr)
        {
            std::cerr << "Error finding <js-manager> node in XML file." << std::endl;
            js_node_ = doc_->NewElement("js-manager");
            js_node_->SetAttribute("sidebar-width", 300);
            js_node_->SetAttribute("selected-file-path", "");
            stylus_node_->InsertEndChild(js_node_);
        }
        tailwind_config_node_ = stylus_node_->FirstChildElement("tailwind-config");
        if (tailwind_config_node_ == nullptr)
        {
            std::cerr << "Error finding <tailwind-config> node in XML file." << std::endl;
            tailwind_config_node_ = doc_->NewElement("tailwind-config");
            tailwind_config_node_->SetAttribute("editor-width", 500);
            tailwind_config_node_->SetAttribute("selected-file-name", "");
            stylus_node_->InsertEndChild(tailwind_config_node_);
        }
        settings_node_ = stylus_node_->FirstChildElement("settings");
        if (settings_node_ == nullptr)
        {
            std::cerr << "Error finding <settings> node in XML file." << std::endl;
            settings_node_ = doc_->NewElement("settings");
            settings_node_->SetAttribute("use-tailwind-cdn", "true");
            stylus_node_->InsertEndChild(settings_node_);
        }
        images_manager_node_ = stylus_node_->FirstChildElement("images-manager");
        if (images_manager_node_ == nullptr)
        {
            std::cerr << "Error finding <images-manager> node in XML file." << std::endl;
            images_manager_node_ = doc_->NewElement("images-manager");
        }
        copy_node_ = doc_->FirstChildElement("copy");
        if (copy_node_ == nullptr)
        {
            std::cerr << "Error finding <copy> node in XML file." << std::endl;
            copy_node_ = doc_->NewElement("copy");
            doc_->InsertEndChild(copy_node_);
        }
        organizeXmlNode(copy_node_, state_file_path_);
        doc_->SaveFile(state_file_path_.c_str());
        if (doc_->ErrorID() != tinyxml2::XML_SUCCESS)
        {
            std::cerr << "Error saving XML file: " << doc_->ErrorID() << std::endl;
        }
        std::cout << "\n\nStylusState initialized successfully.\n\n";
    }

    std::string StylusState::getFileText(std::string file_path)
    {
        std::ifstream file(file_path);
        if (!file.is_open())
        {
            std::cout << "\n\n Failed to read file Stylus state : " << file_path << "\n\n";
            return "!Failed to read file!";
        }

        std::string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        Wt::WString file_content_wt = Wt::WString::fromUTF8(file_content);
        return file_content;
    }

    void StylusState::organizeXmlNode(tinyxml2::XMLElement *node, std::string file_path)
    {
        // std::cout << "\n\nOrganizing XML node: " << node->Name() << "\n\n";
        if (node != node->GetDocument()->RootElement())
        {
            if (isCondNode(node))
            {
                // std::cout << "\n\nCondition node\n";
                if (node->PreviousSibling() && node->PreviousSibling()->ToText() && Stylus::trimAllWitespace(node->PreviousSibling()->ToText()->Value()).compare("${") != 0)
                {
                    std::cout << "\n\n selected previous sibling check \n\n";
                    std::string text = node->PreviousSibling()->ToText()->Value();
                    std::string before_start_cond = text.substr(0, text.length() - 2);
                    std::string after_start_cond = text.substr(text.length() - 2, 2);
                    // std::cout << "Splitting previous sibling text: '" << Stylus::trimAllWitespace(text) << "' before_start_cond '" << Stylus::trimAllWitespace(before_start_cond) << "' after_start_cond '" << Stylus::trimAllWitespace(after_start_cond) << "'\n";
                    node->PreviousSibling()->ToText()->SetValue(before_start_cond.c_str());
                    auto parent_node = node->Parent();
                    parent_node->InsertAfterChild(node->PreviousSibling(), node->GetDocument()->NewText(after_start_cond.c_str()));
                }
                if (node->NextSibling() && node->NextSibling()->ToText() && Stylus::trimAllWitespace(node->NextSibling()->ToText()->Value()).compare("}") != 0)
                {
                    std::cout << "\n\n selected next sibling check \n\n";
                    std::string text = node->NextSibling()->ToText()->Value();
                    std::string after_cond_text = text.substr(0, 1);
                    std::string after_after_cont_text = text.substr(1, text.length() - 1);
                    // std::cout << "Splitting next sibling text: '" << Stylus::trimAllWitespace(text) << "' after_cond_text '" << Stylus::trimAllWitespace(after_cond_text) << "' after_after_cont_text '" << Stylus::trimAllWitespace(after_after_cont_text) << "'\n";
                    node->NextSibling()->ToText()->SetValue(after_cond_text.c_str());
                    auto parent_node = node->Parent();
                    parent_node->InsertAfterChild(node->NextSibling(), node->GetDocument()->NewText(after_after_cont_text.c_str()));
                }
                if (node->FirstChild() && node->FirstChild()->ToText() && Stylus::trimAllWitespace(node->FirstChild()->ToText()->Value()).compare("}") != 0)
                {
                    std::cout << "\n\n selected first child check \n\n";
                    std::string text = node->FirstChild()->ToText()->Value();
                    std::string first_child_text = text.substr(0, 1);
                    std::string after_first_child_text = text.substr(1, text.length() - 1);
                    // std::cout << "Splitting first child text:'" << Stylus::trimAllWitespace(text) << "' \nfirst_child_text '" << Stylus::trimAllWitespace(first_child_text) << "' \nafter_first_child_text '" << Stylus::trimAllWitespace(after_first_child_text) << "'\n";
                    node->FirstChild()->ToText()->SetValue(first_child_text.c_str());
                    node->InsertAfterChild(node->FirstChild(), node->GetDocument()->NewText(after_first_child_text.c_str()));
                }
                if (node->LastChild() && node->LastChild()->ToText() && Stylus::trimAllWitespace(node->LastChild()->ToText()->Value()).compare("${") != 0)
                {
                    std::cout << "\n\n selected last child check \n\n";
                    std::string text = node->LastChild()->ToText()->Value();
                    std::cout << "text: '" << Stylus::trimAllWitespace(text) << "'\n";
                    std::string last_child_text = text.substr(text.length() - 2, 2);
                    std::string before_end_cond = text.substr(0, text.length() - 2);
                    // std::cout << "Splitting last child text: '" << Stylus::trimAllWitespace(text) << "' before_end_cond '" << Stylus::trimAllWitespace(before_end_cond) << "' last_child_text '" << Stylus::trimAllWitespace(last_child_text) << "'\n";
                    node->LastChild()->ToText()->SetValue(last_child_text.c_str());
                    node->InsertAfterChild(node->LastChild()->PreviousSibling(), node->GetDocument()->NewText(before_end_cond.c_str()));
                }
                // if(node->LastChild() && node->LastChild()->ToText() && Stylus::trimAllWitespace(node->LastChild()->ToText()->Value()).compare("${") != 0)
                // {
                //     std::cout << "\n\n selected last child check \n\n";
                //     std::string text = node->LastChild()->ToText()->Value();
                //     std::string last_child_text = text.substr(text.length()-2, 2);
                //     std::string before_end_cond = text.substr(0, text.length()-2);
                //     std::cout << "Splitting last child text: '" << Stylus::trimAllWitespace(text) << "' before_end_cond '" << Stylus::trimAllWitespace(before_end_cond) << "' last_child_text '" << Stylus::trimAllWitespace(last_child_text) << "'\n";
                //     node->LastChild()->ToText()->SetValue(last_child_text.c_str());
                //     node->InsertAfterChild(node->LastChild()->PreviousSibling(), doc_->NewText(before_end_cond.c_str()));
                // }
            }
        }
        else
        {
            if (std::string(node->Name()).compare("messages") != 0 &&
                std::string(node->Name()).compare("message") != 0 &&
                std::string(node->Name()).compare("div") != 0 &&
                std::string(node->Name()).compare("stylus") != 0)
            {
                node->SetName("div");
                node->GetDocument()->SaveFile(file_path.c_str());
            }
        }
        auto child_node = node->FirstChild();
        while (child_node)
        {
            if (child_node->ToElement())
            {
                organizeXmlNode(child_node->ToElement(), file_path);
            }
            else if (child_node->ToText())
            {
            }
            child_node = child_node->NextSibling();
        }
    }

    bool StylusState::isCondNode(tinyxml2::XMLElement *node)
    {
        auto prev_node = node->PreviousSibling();
        auto next_node = node->NextSibling();
        auto first_child = node->FirstChild();
        auto last_child = node->LastChild();
        if (prev_node && prev_node->ToText() &&
            next_node && next_node->ToText() &&
            first_child && first_child->ToText() &&
            last_child && last_child->ToText())
        {

            std::string prev_node_text = Stylus::trimAllWitespace(prev_node->ToText()->Value());
            std::string next_node_text = Stylus::trimAllWitespace(next_node->ToText()->Value());
            std::string first_child_text = Stylus::trimAllWitespace(first_child->ToText()->Value());
            std::string last_child_text = Stylus::trimAllWitespace(last_child->ToText()->Value());

            if (prev_node_text.length() < 2 || next_node_text.length() < 1 ||
                first_child_text.length() < 1 || last_child_text.length() < 2)
            {
                return false;
            }
            prev_node_text = prev_node_text.substr(prev_node_text.length() - 2, 2);
            next_node_text = next_node_text.substr(0, 1);
            first_child_text = first_child_text.substr(0, 1);
            last_child_text = last_child_text.substr(last_child_text.length() - 2, 2);

            if (prev_node_text.compare("${") == 0 && next_node_text.compare("}") == 0 &&
                first_child_text.compare("}") == 0 && last_child_text.compare("${") == 0)
            {
                return true;
            }
        }
        return false;
    }

    void StylusState::logMessage(const std::string& message, const LogMessageType& type)
    {
        std::string type_str = "Info"; // Default type
        switch(type)
        {
            case LogMessageType::Debug:
                type_str = "debug";
                break;
            case LogMessageType::Info:
                type_str = "info";
                break;
            case LogMessageType::Warning:
                type_str = "warning";
                break;
            case LogMessageType::Secure:
                type_str = "secure";
                break;
            case LogMessageType::Error:
                type_str = "error";
                break;
            case LogMessageType::Fatal:
                type_str = "fatal";
                break;
        }
        
        // Setup the logger
        Wt::WLogger logger;
        logger.setFile("../../stylus_logs.txt");
        
        // Add an entry
        Wt::WLogEntry entry = logger.entry(type_str);
        entry << Wt::WLogger::timestamp << Wt::WLogger::sep
            << '[' << wApp->sessionId() << ']' << Wt::WLogger::sep
            << '[' << type_str << ']' << Wt::WLogger::sep
            << message;
    }


    tinyxml2::XMLElement* StylusState::getMessageNode(std::string folder_name, std::string file_name, std::string message_id)
    {
        if (folder_name.empty() || file_name.empty() || message_id.empty())
        {
            std::cerr << "Error: Invalid parameters for getMessageNode." << std::endl;
            return nullptr; // Return null if parameters are invalid
        }
        std::string path = folder_name + "/" + file_name;
        auto searched_file_brain = xml_file_brains_.find(path);
        if (searched_file_brain == xml_file_brains_.end())
        {
            std::cerr << "Error: File brain not found for path: " << path << std::endl;
            return nullptr; // Return null if file brain is not found
        }
        auto xml_file_brain = searched_file_brain->second;
        if (xml_file_brain == nullptr)
        {
            std::cerr << "Error: XML file brain is null for path: " << path << std::endl;
            return nullptr; // Return null if file brain is null
        }
        auto message_node = xml_file_brain->id_and_message_nodes_[message_id];
        if (message_node == nullptr)
        {
            std::cerr << "Error: Message node is null for message ID: " << message_id << std::endl;
            return nullptr; // Return null if message node is null
        }
        // std::cout << "Found message node for message ID: " << message_id << std::endl;
        // std::cout << "Message node name: " << message_node->Name() << std::endl;
        // std::cout << "Message node text: " << (message_node->GetText() ? message_node->GetText() : "null") << std::endl;
        return message_node; // Return the found message node
    }

    TempNodeVarData StylusState::getTempNodeVarData(tinyxml2::XMLElement *node)
    {
        TempNodeVarData data;
        // ^\$\{[ ]?[a-z:]*[ ]?\}
        // ^\$\{[ ]?[a-z:]*[a-zA-Z0-9\(\)\:\-\_\[\]=\"\'\/\.\~ ]*?\}
        std::string text = "";
        if (node == nullptr || isCondNode(node) || node->ChildElementCount() != 1 || !node->FirstChild() || !node->FirstChild()->ToText())
        {
            // std::cout << "\n   Error: Node is null or does not have the expected structure for TempNodeVarData.\n";
            return data; // Return empty data if node is null
        }
        std::cout << "\n   Parsing node: " << node->Name() << "\n";
        if(node->FirstChild() && boost::regex_match(node->FirstChild()->ToText()->Value(), boost::regex(R"(^\$\{[ ]?[a-z:]*[a-zA-Z0-9\(\)\:\-\_\[\]=\"\'\/\.\~ ]*?\})")))
        {
            std::cout << "\n   Parsing text from first child of node: " << node->Name() << "\n";
            text = node->FirstChild()->ToText()->Value();
        }else if (isCondNode(node) && node->FirstChild()->NextSibling() && boost::regex_match(node->FirstChild()->NextSibling()->ToText()->Value(), boost::regex(R"(^\$\{[ ]?[a-z:]*[a-zA-Z0-9\(\)\:\-\_\[\]=\"\'\/\.\~ ]*?\})")))
        {
            std::cout << "\n   Parsing text from next sibling of first child of node: " << node->Name() << "\n";
            text = node->FirstChild()->NextSibling()->ToText()->Value();
        }else {
            std::cout << "\n   Error: Node does not match expected format for TempNodeVarData.\n";
            return data; // Return empty data if node does not match the expected format
        }
        std::cout << "Parsing text: '" << text << "'\n";
        // std::cout << "\n\nParsing node: " << node->FirstChild()->ToText()->Value() << "\n\n";
        // ${tr:some-text message="000-examples/asada.xml:some-text"}
        // ${function_:var_name_ attr1="value1" attr2="value2"}
        size_t end_pos = 0;
        if (text[0] == '$' && text[1] == '{')
        {
            end_pos = text.find('}');
            if (end_pos != std::string::npos && end_pos > 2)
            {
                text = text.substr(2, end_pos - 2); // Extract content between ${ and }
            }
            else return data;
        }
        else return data;

        text = Stylus::trimWitespace(text);
        size_t colon_pos = text.find(':');
        if (colon_pos != std::string::npos)
        {
            data.function_ = text.substr(0, colon_pos);
            if (text.find_first_of(" ") != std::string::npos)
            {
                data.var_name_ = text.substr(colon_pos + 1, text.find_first_of(" ") - colon_pos - 1);
            }
            else
            {
                data.var_name_ = text.substr(colon_pos + 1);
            }
            // std::cout << "Found colon. function_: '" << data.function_ << "', var_name_: '" << data.var_name_ << "'" << std::endl;
        }
        else
        {
            if (text.find_first_of(" ") != std::string::npos)
            {
                data.var_name_ = text.substr(0, text.find_first_of(" "));
            }
            else
            {
                data.var_name_ = text; // No function, just variable name
            }
        }

        // parse attributes
        size_t attr_start = text.find_first_of(" ");
        if (attr_start == std::string::npos)
        {
            return data; // No attributes found
        }

        std::string attributes_str = text.substr(attr_start + 1);
        // std::cout << "Parsing attributes: '" << attributes_str << "'" << std::endl;
        size_t pos = 0;
        while ((pos = attributes_str.find('=')) != std::string::npos)
        {
            std::string attr_name = attributes_str.substr(0, pos);
            attr_name = Stylus::trimWitespace(attr_name);
            std::cout << "Found attribute: '" << attr_name << "'" << std::endl;
            attributes_str.erase(0, pos + 1); // Remove the attribute name and '='
            size_t end_quote_pos = attributes_str.find_first_of("\"'");
            if (end_quote_pos == std::string::npos)
            {
                std::cerr << "Error: Invalid attribute format in: " << text << std::endl;
                return data; // Return data as is if atribute format is invalid
            }
            // Remove starting and ending quotes from attribute value
            if (!attributes_str.empty() && (attributes_str[0] == '"' || attributes_str[0] == '\'')) {
                attributes_str.erase(0, 1);
            }
            end_quote_pos = attributes_str.find_first_of("\"'");
            std::string attr_value = attributes_str.substr(0, end_quote_pos);
            data.attributes_[attr_name] = attr_value;
            attributes_str.erase(0, end_quote_pos + 1); // Remove the value and the closing quote
        }
        // std::cout << "\n\n";
        return data;
    }

    MessageAttributeData TempNodeVarData::getMessageAttributeData(std::string message_attribute_value)
    {
        MessageAttributeData data;
        
        // std::cout << "Parsing message_attribute_value: '" << message_attribute_value << "'" << std::endl;
        if (message_attribute_value.empty() || message_attribute_value.length() < 3)
        {
            std::cerr << "Error: Invalid message attribute value: " << message_attribute_value << std::endl;
            return MessageAttributeData(); // Return empty data if value is invalid
        }
        size_t slash_pos = message_attribute_value.find('/');
        if (slash_pos != std::string::npos)
        {
            data.folder_name_ = message_attribute_value.substr(0, slash_pos);
            data.file_name_ = message_attribute_value.substr(slash_pos + 1);
        }
        
        // std::cout << "  Trimmed values: folder_name_='" << data.folder_name_ << "', file_name_='" << data.file_name_ << "', message_id_='" << data.message_id_ << "'" << std::endl;
        return data;
    }




}