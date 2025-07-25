#include "999-Stylus/Stylus.h"
#include <Wt/WStackedWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>
#include <Wt/WApplication.h>
#include <Wt/WStackedWidget.h>
#include <fstream>
#include <Wt/WRandom.h>

#include "001-App/App.h"
#include <Wt/Auth/Identity.h>
#include <Wt/WMenu.h>

namespace Stylus
{

    Stylus::Stylus(Session& session)
        : session_(session)
    {
        setOffsets(0, Wt::Side::Top | Wt::Side::Bottom | Wt::Side::Left | Wt::Side::Right);
        titleBar()->children()[0]->removeFromParent();
        setStyleClass("!border-0 overflow-auto stylus-background");
        titleBar()->hide();
        titleBar()->setStyleClass("p-0 flex items-center overflow-x-visible h-[40px]");
        contents()->setStyleClass("h-[100vh] overflow-y-auto overflow-x-visible flex");
        setModal(false);
        setResizable(false);
        setMovable(false);

        setMinimumSize(Wt::WLength(100, Wt::LengthUnit::ViewportWidth), Wt::WLength(100, Wt::LengthUnit::ViewportHeight));
        setLayoutSizeAware(true);
     
        wApp->doJavaScript(WT_CLASS R"(
            .$(')" + id() + R"(').oncontextmenu = function() {
                event.cancelBubble = true;
                event.returnValue = false;
                return false;
            };
            document.addEventListener('keydown', function(event) {
                if (event.altKey && (event.key === 'ArrowLeft' || event.key === 'ArrowRight')) {
                    event.preventDefault();
                    // Your custom logic here if needed
                } else if ((event.ctrlKey || event.metaKey) && event.key === 's') {
                    event.preventDefault();
                }
            });
        )");
        // wApp->useStyleSheet(wApp->docRoot() + "/static/stylus/stylus.css?v=" + Wt::WRandom::generateId());
        // wApp->require(wApp->docRoot() + "/static/stylus/monaco-edditor.js");
        
        wApp->require(wApp->docRoot() + "/static/stylus/monaco-edditor.js");
        wApp->messageResourceBundle().use("../../static/stylus-resources/xml/101-stylus/svg");
        // Wt::WString stylus_css_file_path_ = "../../static/tailwind.css?v=" + Wt::WRandom::generateId();
        // wApp->useStyleSheet(stylus_css_file_path_.toUTF8());

        state_ = std::make_shared<StylusState>();
      
        navbar_wrapper_ = contents()->addNew<Wt::WContainerWidget>();
        content_stack_ = contents()->addNew<Wt::WStackedWidget>();
        menu_ = navbar_wrapper_->addNew<Wt::WMenu>(content_stack_);
        menu_->setStyleClass("flex flex-col items-center h-full");
        navbar_wrapper_->setStyleClass("flex flex-col items-center h-full border-r border-solid stylus-scrollbar");
        content_stack_->setStyleClass("w-screen h-screen");

        // std::unique_ptr<XmlFilesManager> files_manager_ptr = std::make_unique<XmlFilesManager>(state_);
        // std::unique_ptr<CssFilesManager> css_files_manager_ptr = std::make_unique<CssFilesManager>(state_);
        // std::unique_ptr<JsFilesManager> js_files_manager_ptr = std::make_unique<JsFilesManager>(state_);
        // std::unique_ptr<TailwindConfigManager> tailwind_config_ptr = std::make_unique<TailwindConfigManager>(state_);
        std::unique_ptr<ImagesManager> images_manager_ptr = std::make_unique<ImagesManager>(state_);
        // std::unique_ptr<Settings> settings_ptr = std::make_unique<Settings>(state_);

        // xml_files_manager_ = files_manager_ptr.get();
        // css_files_manager_ = css_files_manager_ptr.get();
        // js_files_manager_ = js_files_manager_ptr.get();
        // tailwind_config_ = tailwind_config_ptr.get();
        images_manager_ = images_manager_ptr.get();
        // settings_ = settings_ptr.get();

        // xml_file_manager_menu_item_ = menu_->addItem(std::make_unique<Wt::WMenuItem>("", std::move(files_manager_ptr), Wt::ContentLoading::Lazy));
        // css_menu_item_ = menu_->addItem(std::make_unique<Wt::WMenuItem>("", std::move(css_files_manager_ptr), Wt::ContentLoading::Lazy));
        // javascript_menu_item_ = menu_->addItem(std::make_unique<Wt::WMenuItem>("", std::move(js_files_manager_ptr), Wt::ContentLoading::Lazy));
        // tailwind_menu_item_ = menu_->addItem(std::make_unique<Wt::WMenuItem>("", std::move(tailwind_config_ptr), Wt::ContentLoading::Lazy));
        // images_menu_item_ = menu_->addItem(std::make_unique<Wt::WMenuItem>("", std::move(images_manager_ptr), Wt::ContentLoading::Lazy));
        // settings_menu_item_ = menu_->addItem(std::make_unique<Wt::WMenuItem>("", std::move(settings_ptr), Wt::ContentLoading::Lazy));

        // xml_file_manager_menu_item_ = menu_->addItem(std::make_unique<Wt::WMenuItem>("", std::move(files_manager_ptr)));
        // css_menu_item_ = menu_->addItem(std::make_unique<Wt::WMenuItem>("", std::move(css_files_manager_ptr)));
        // javascript_menu_item_ = menu_->addItem(std::make_unique<Wt::WMenuItem>("", std::move(js_files_manager_ptr)));
        // tailwind_menu_item_ = menu_->addItem(std::make_unique<Wt::WMenuItem>("", std::move(tailwind_config_ptr)));
        images_menu_item_ = menu_->addItem(std::make_unique<Wt::WMenuItem>("", std::move(images_manager_ptr)));
        // settings_menu_item_ = menu_->addItem(std::make_unique<Wt::WMenuItem>("", std::move(settings_ptr)));

        // auto xml_svg_temp = xml_file_manager_menu_item_->anchor()->insertNew<Wt::WTemplate>(0, Wt::WString::tr("stylus-svg-xml-logo"));
        // auto css_svg_temp = css_menu_item_->anchor()->insertNew<Wt::WTemplate>(0, Wt::WString::tr("stylus-svg-css-logo"));
        // auto javascript_svg_temp = javascript_menu_item_->anchor()->insertNew<Wt::WTemplate>(0, Wt::WString::tr("stylus-svg-javascript-logo"));
        // auto tailwind_svg_temp = tailwind_menu_item_->anchor()->insertNew<Wt::WTemplate>(0, Wt::WString::tr("stylus-svg-tailwind-logo"));
        auto images_svg_temp = images_menu_item_->anchor()->insertNew<Wt::WTemplate>(0, Wt::WString::tr("stylus-svg-images-logo"));
        // auto settings_svg_temp = settings_menu_item_->anchor()->insertNew<Wt::WTemplate>(0, Wt::WString::tr("stylus-svg-settings-logo"));

        // dark_mode_toggle_ = navbar_wrapper_->addNew<DarkModeToggle>(session_);
        // theme_switcher_ = navbar_wrapper_->addNew<ThemeSwitcher>(session_);
        
        std::string nav_btns_styles = "w-[35px] m-[3px] p-1 cursor-pointer rounded-md flex items-center justify-center";

        // xml_svg_temp->setStyleClass(nav_btns_styles);
        // css_svg_temp->setStyleClass(nav_btns_styles);
        // javascript_svg_temp->setStyleClass(nav_btns_styles + " p-1");
        // tailwind_svg_temp->setStyleClass(nav_btns_styles);
        images_svg_temp->setStyleClass(nav_btns_styles);
        // // settings_svg_temp->setStyleClass(nav_btns_styles);
        
        // xml_file_manager_menu_item_->addStyleClass("m-1");
        // css_menu_item_->addStyleClass("m-1");
        // javascript_menu_item_->addStyleClass("m-1");
        // tailwind_menu_item_->addStyleClass("m-1");
        images_menu_item_->addStyleClass("m-1");
        // settings_menu_item_->addStyleClass("m-1");
        // dark_mode_toggle_->addStyleClass("m-1 text-xs");
        // theme_switcher_->addStyleClass("m-1 text-xs");
        

        // xml_files_manager_ = content_stack_->addNew<XmlFilesManager>(state_);
        // css_files_manager_ = content_stack_->addNew<CssFilesManager>(state_);
        // js_files_manager_ = content_stack_->addNew<JsFilesManager>(state_);
        // tailwind_config_ = content_stack_->addNew<TailwindConfigManager>(state_);
        images_manager_ = content_stack_->addNew<ImagesManager>(state_);
        // settings_ = content_stack_->addNew<Settings>(state_);

        auto selected_menu = state_->stylus_node_->Attribute("selected-menu");
        std::cout << "Selected menu: " << (selected_menu ? selected_menu : "null") << std::endl;
        // if (std::strcmp(selected_menu, "templates") == 0) {
        //     menu_->select(xml_file_manager_menu_item_);
        // }else if (std::strcmp(selected_menu, "tailwind") == 0) {
        //     menu_->select(tailwind_menu_item_);
        // }else if (std::strcmp(selected_menu, "css") == 0) {
        //     menu_->select(css_menu_item_);
        // }else if (std::strcmp(selected_menu, "javascript") == 0) {
        //     menu_->select(javascript_menu_item_);
        // }else if (std::strcmp(selected_menu, "settings") == 0) {
        //     menu_->select(settings_menu_item_);
        // }else if (std::strcmp(selected_menu, "images") == 0) {
        //     menu_->select(images_menu_item_);
        // }else {
        //     menu_->select(xml_file_manager_menu_item_);
        // }
        
        // menu_->itemSelected().connect(this, [=](Wt::WMenuItem* item){
        //     if (item == xml_file_manager_menu_item_) {
        //         state_->stylus_node_->SetAttribute("selected-menu", "templates");
        //         state_->doc_->SaveFile(state_->state_file_path_.c_str());
        //     }else if (item == tailwind_menu_item_) {
        //         state_->stylus_node_->SetAttribute("selected-menu", "tailwind");
        //         state_->doc_->SaveFile(state_->state_file_path_.c_str());
        //     }else if (item == css_menu_item_) {
        //         state_->stylus_node_->SetAttribute("selected-menu", "css");
        //         state_->doc_->SaveFile(state_->state_file_path_.c_str());
        //     }else if (item == javascript_menu_item_) {
        //         state_->stylus_node_->SetAttribute("selected-menu", "javascript");
        //         state_->doc_->SaveFile(state_->state_file_path_.c_str());
        //     }else if (item == settings_menu_item_) {
        //         state_->stylus_node_->SetAttribute("selected-menu", "settings");
        //         state_->doc_->SaveFile(state_->state_file_path_.c_str());
        //     }else if (item == images_menu_item_) {
        //         state_->stylus_node_->SetAttribute("selected-menu", "images");
        //         state_->doc_->SaveFile(state_->state_file_path_.c_str());
        //     }
        // });

        if (state_->stylus_node_->BoolAttribute("open"))
            show();
        else
            hide();

        // dark_mode_toggle_->changed().connect(this, [=]()
        //                                              {
        //     bool dark = dark_mode_toggle_->isChecked();
        //     xml_files_manager_->editor_->setDarkTheme(dark);
        //     state_->stylus_node_->SetAttribute("dark-mode", dark ? "true" : "false");
        //     state_->doc_->SaveFile(state_->state_file_path_.c_str());
        // });

        // if (state_->stylus_node_->BoolAttribute("dark-mode"))
        // {
        //     xml_files_manager_->editor_->setDarkTheme(true);
        // }

        // css_files_manager_->file_saved().connect(this, [=](){ 
        //     if(!state_->settings_node_->BoolAttribute("use-tailwind-cdn"))
        //         tailwind_config_->generateCssFile(); 
        // });
        // xml_files_manager_->file_saved().connect(this, [=](){ 
        //     if(!state_->settings_node_->BoolAttribute("use-tailwind-cdn"))
        //         tailwind_config_->generateCssFile(); 
        // });

        // if(!state_->settings_node_->BoolAttribute("use-tailwind-cdn")){
        //     tailwind_config_->generateCssFile();
        // }
        setupKeyboardShortcuts();
    }
    
    void Stylus::setupKeyboardShortcuts()
    {
      
        wApp->globalKeyWentDown().connect([=](Wt::WKeyEvent e)
                                                                  { 
        if (e.modifiers().test(Wt::KeyboardModifier::Alt)){
            if(e.modifiers().test(Wt::KeyboardModifier::Shift)){
                if(e.key() == Wt::Key::Q){

                    if(navbar_wrapper_->isHidden())
                    {
                        state_->stylus_node_->SetAttribute("navigation-bar-hidden", "false");
                        navbar_wrapper_->animateShow(Wt::WAnimation(Wt::AnimationEffect::SlideInFromLeft, Wt::TimingFunction::EaseInOut, 500));
                    }else
                    {
                        state_->stylus_node_->SetAttribute("navigation-bar-hidden", "true");
                        navbar_wrapper_->animateHide(Wt::WAnimation(Wt::AnimationEffect::SlideInFromLeft, Wt::TimingFunction::EaseInOut, 500));
                    }
                    state_->doc_->SaveFile(state_->state_file_path_.c_str());
                }
            //     else if(e.key() == Wt::Key::Up){
            //         if(!xml_files_manager_->selected_file_brain_) return;
            //         auto selected_node = xml_files_manager_->selected_file_brain_->selected_node_;
            //         if(!selected_node) return;
            //         auto parent_node = selected_node->Parent();
            //         if(!selected_node->PreviousSiblingElement()) return;
            //         if(selected_node->PreviousSiblingElement()->PreviousSiblingElement())
            //         {
            //             auto prev_prev_elem_node = selected_node->PreviousSiblingElement()->PreviousSiblingElement();
            //             auto prev_node = selected_node->PreviousSibling();
            //             auto next_node = selected_node->NextSibling();

            //             if(state_->isCondNode(selected_node) && 
            //                 state_->isCondNode(prev_prev_elem_node)
            //             ){
            //                 // selected node, prev prev sibling elements are condition nodes
            //                 StylusState::logMessage("selected node, prev prev sibling elements are condition nodes");
            //                 auto end_condition_node = prev_prev_elem_node->NextSibling();
            //                 parent_node->InsertAfterChild(end_condition_node, prev_node);
            //                 parent_node->InsertAfterChild(prev_node, selected_node);
            //                 parent_node->InsertAfterChild(selected_node, next_node);
            //             }else if(state_->isCondNode(selected_node)){
            //                 // selected node is condition node
            //                 StylusState::logMessage("selected node is condition node");
            //                 parent_node->InsertAfterChild(prev_prev_elem_node, prev_node);
            //                 parent_node->InsertAfterChild(prev_node, selected_node);
            //                 parent_node->InsertAfterChild(selected_node, next_node);
            //             }else if(state_->isCondNode(prev_prev_elem_node)){
            //                 // prev prev sibling element is condition node
            //                 StylusState::logMessage("prev prev sibling element is condition node");
            //                 auto end_condition_node = prev_prev_elem_node->NextSibling();
            //                 parent_node->InsertAfterChild(end_condition_node, selected_node);
            //             }else {
            //                 // selected node and prev prev sibling element are not condition nodes
            //                 StylusState::logMessage("selected node and prev prev sibling element are not condition nodes");
            //                 parent_node->InsertAfterChild(prev_prev_elem_node, selected_node);
            //             }
            //         }else {
            //             if(state_->isCondNode(selected_node)){
            //                 // selected node is condition node
            //                 auto prev_node = selected_node->PreviousSibling();
            //                 auto next_node = selected_node->NextSibling();
            //                 // if(state_->isCondNode(selected_node->PreviousSiblingElement()) && 
            //                 //     XMLTreeNode::trimAllWitespace(selected_node->PreviousSiblingElement()->NextSibling()->ToText()->Value()).compare("}${") == 0    
            //                 // ){
            //                 //     selected_node->PreviousSiblingElement()->NextSibling()->ToText()->SetValue("}");
            //                 //     prev_node = xml_files_manager_->selected_file_brain_->doc_->NewText("${");
            //                 // }
            //                 if(state_->isCondNode(parent_node->ToElement())){
            //                     // selected node and parent node are condition nodes
            //                     StylusState::logMessage("selected node and parent node are condition nodes");
            //                     auto end_condition_node = parent_node->FirstChild();
            //                     parent_node->InsertAfterChild(end_condition_node, prev_node);
            //                     parent_node->InsertAfterChild(prev_node, selected_node);
            //                     parent_node->InsertAfterChild(selected_node, next_node);
            //                 }else {
            //                     parent_node->InsertFirstChild(prev_node);
            //                     parent_node->InsertAfterChild(prev_node, selected_node);
            //                     parent_node->InsertAfterChild(selected_node, next_node);
            //                 }
            //             }else {
            //                 StylusState::logMessage("selected node is not condition node");
            //                 StylusState::logMessage("selected node: " + std::string(selected_node->ToElement()->Name()));
            //                 if(state_->isCondNode(parent_node->ToElement())){
            //                     // selected node and parent node are condition nodes
            //                     StylusState::logMessage("selected node and parent node are condition nodes");
            //                     auto end_condition_node = parent_node->FirstChild();
            //                     parent_node->InsertAfterChild(end_condition_node, selected_node);
            //                 }else {
            //                     // selected node and parent node are not condition nodes
            //                     StylusState::logMessage("selected node and parent node are not condition nodes");
            //                     parent_node->InsertFirstChild(selected_node);
            //                 }
            //             }
            //         }
            //         xml_files_manager_->selected_file_brain_->doc_->SaveFile(xml_files_manager_->selected_file_brain_->file_path_.c_str());
            //         xml_files_manager_->selected_file_brain_->file_saved_.emit();
            //     }else if(e.key() == Wt::Key::Down){
            //         if(!xml_files_manager_->selected_file_brain_) return;
            //         auto selected_node = xml_files_manager_->selected_file_brain_->selected_node_;
            //         if(!selected_node) return;
            //         auto parent_node = selected_node->Parent();
            //         auto prev_node = selected_node->PreviousSibling();
            //         auto next_node = selected_node->NextSibling();
            //         if(!selected_node->NextSiblingElement()) return;
            //         if(state_->isCondNode(selected_node) && 
            //             state_->isCondNode(selected_node->NextSiblingElement())
            //         ){
            //             // selected node and next sibling element are condition nodes
            //             StylusState::logMessage("selected node and next sibling element are condition nodes");
            //             parent_node->InsertAfterChild(selected_node->NextSiblingElement()->NextSibling(), prev_node);
            //             parent_node->InsertAfterChild(prev_node, selected_node);
            //             parent_node->InsertAfterChild(selected_node, next_node);
            //         }else if(state_->isCondNode(selected_node)){
            //             // selected node is condition node
            //             StylusState::logMessage("selected node is condition node");
            //             parent_node->InsertAfterChild(selected_node->NextSiblingElement(), prev_node);
            //             parent_node->InsertAfterChild(prev_node, selected_node);
            //             parent_node->InsertAfterChild(selected_node, next_node);
            //         }else if(state_->isCondNode(selected_node->NextSiblingElement())){
            //             // next sibling element is condition node
            //             StylusState::logMessage("next sibling element is condition node");
            //             auto end_condition_node = selected_node->NextSiblingElement()->NextSibling();
            //             parent_node->InsertAfterChild(end_condition_node, selected_node);
            //         }else {
            //             // selected node and next sibling element are not condition nodes
            //             StylusState::logMessage("selected node and next sibling element are not condition nodes");
            //             parent_node->InsertAfterChild(selected_node->NextSiblingElement(), selected_node);
            //         }
            //         xml_files_manager_->selected_file_brain_->doc_->SaveFile(xml_files_manager_->selected_file_brain_->file_path_.c_str());
            //         xml_files_manager_->selected_file_brain_->file_saved_.emit();
            //     }else if(e.key() == Wt::Key::Left){
            //         if(!xml_files_manager_->selected_file_brain_) return;
            //         auto selected_node = xml_files_manager_->selected_file_brain_->selected_node_;
            //         if(selected_node && 
            //             selected_node != xml_files_manager_->selected_file_brain_->doc_->RootElement() &&
            //             selected_node->Parent() != xml_files_manager_->selected_file_brain_->doc_->RootElement())
            //         {
            //             auto parent_node = selected_node->Parent();
            //             auto grand_parent_node = parent_node->Parent();
            //             auto parent_prev_node_elem = parent_node->PreviousSiblingElement();
            //             auto prev_node = selected_node->PreviousSibling();
            //             auto next_node = selected_node->NextSibling();
            //             if(parent_prev_node_elem){
            //                 if(state_->isCondNode(parent_prev_node_elem) && 
            //                     state_->isCondNode(selected_node)
            //                 ){
            //                     // parent prev node and selected node are condition nodes
            //                     StylusState::logMessage("parent prev node and selected node are condition nodes");
            //                     auto parent_prev_node = parent_prev_node_elem->NextSibling();
            //                     grand_parent_node->InsertAfterChild(parent_prev_node, prev_node);
            //                     grand_parent_node->InsertAfterChild(prev_node, selected_node);
            //                     grand_parent_node->InsertAfterChild(selected_node, next_node);
            //                 }else if(state_->isCondNode(parent_prev_node_elem)){
            //                     // parent prev node is condition node
            //                     StylusState::logMessage("parent prev node is condition node");
            //                     grand_parent_node->InsertAfterChild(parent_prev_node_elem->NextSibling(), selected_node);
            //                 }else if(state_->isCondNode(selected_node)){
            //                     // selected node is condition node
            //                     StylusState::logMessage("selected node is condition node");
            //                     grand_parent_node->InsertAfterChild(parent_prev_node_elem, prev_node);
            //                     grand_parent_node->InsertAfterChild(prev_node, selected_node); 
            //                     grand_parent_node->InsertAfterChild(selected_node, next_node);
            //                 }else {
            //                     StylusState::logMessage("selected node and parent prev node are not condition nodes");
            //                     grand_parent_node->InsertAfterChild(parent_prev_node_elem, selected_node);
            //                 }
            //             }else {
            //                 if(state_->isCondNode(selected_node) && 
            //                     state_->isCondNode(grand_parent_node->ToElement())
            //                 ){
            //                     // selected node and grand parent node are condition nodes
            //                     StylusState::logMessage("selected node and grand parent node are condition nodes"+std::string(grand_parent_node->FirstChild()->ToText()->Value()) + " | " +  grand_parent_node->LastChild()->ToText()->Value());
            //                     auto end_condition_node = grand_parent_node->FirstChild();
            //                     grand_parent_node->InsertAfterChild(end_condition_node, prev_node);
            //                     grand_parent_node->InsertAfterChild(prev_node, selected_node);
            //                     grand_parent_node->InsertAfterChild(selected_node, next_node);                                

            //                 }else if (state_->isCondNode(selected_node)){
            //                     // selected node is condition node
            //                     StylusState::logMessage("selected node is condition node");
            //                     grand_parent_node->InsertFirstChild(prev_node);
            //                     grand_parent_node->InsertAfterChild(prev_node, selected_node);
            //                     grand_parent_node->InsertAfterChild(selected_node, next_node);
            //                 }else if(state_->isCondNode(grand_parent_node->ToElement())){
            //                     // grand parent node is condition node
            //                     StylusState::logMessage("grand parent node is condition node");
            //                     auto end_condition_node = grand_parent_node->FirstChild();
            //                     grand_parent_node->InsertAfterChild(end_condition_node, selected_node);
            //                 }else {
            //                     // selected node and grand parent node are not condition nodes
            //                     StylusState::logMessage("selected node and grand parent node are not condition nodes");
            //                     grand_parent_node->InsertFirstChild(selected_node);
            //                 }
            //             }
            //             xml_files_manager_->selected_file_brain_->doc_->SaveFile(xml_files_manager_->selected_file_brain_->file_path_.c_str());
            //             xml_files_manager_->selected_file_brain_->file_saved_.emit();
            //         }
            //     }else if(e.key() == Wt::Key::Right){
            //         if(!xml_files_manager_->selected_file_brain_) return;
            //         auto selected_node = xml_files_manager_->selected_file_brain_->selected_node_;
            //         if(selected_node && selected_node != xml_files_manager_->selected_file_brain_->doc_->RootElement() &&
            //             selected_node->NextSiblingElement()){
            //             auto next_node_elem = selected_node->NextSiblingElement();
            //             auto next_node = selected_node->NextSibling();
            //             auto prev_node = selected_node->PreviousSibling();

            //             if(state_->isCondNode(next_node_elem) &&
            //             state_->isCondNode(selected_node)
            //             ){
            //                 // selected node and next node are condition nodes
            //                 StylusState::logMessage("selected node and next node are condition nodes");
            //                 auto end_condition_node = next_node_elem->FirstChild();
            //                 next_node_elem->InsertAfterChild(end_condition_node, prev_node);
            //                 next_node_elem->InsertAfterChild(prev_node, selected_node);
            //                 next_node_elem->InsertAfterChild(selected_node, next_node);
            //             }else if(state_->isCondNode(selected_node) && !(next_node_elem->FirstChild() && next_node_elem->FirstChild()->ToText())){
            //                 // selected node is condition node
            //                 StylusState::logMessage("selected node is condition node");
            //                 next_node_elem->InsertFirstChild(prev_node);
            //                 next_node_elem->InsertAfterChild(prev_node, selected_node);
            //                 next_node_elem->InsertAfterChild(selected_node, next_node);
            //             }else if(state_->isCondNode(next_node_elem)){
            //                 // next node is condition node
            //                 StylusState::logMessage("next node is condition node");
            //                 auto end_condition_node = next_node_elem->FirstChild();
            //                 next_node_elem->InsertAfterChild(end_condition_node, selected_node);
            //             }else if(!(next_node_elem->FirstChild() && next_node_elem->FirstChild()->ToText())){
            //                 // selected node and next node are not condition nodes
            //                 StylusState::logMessage("selected node and next node are not condition nodes");
            //                 next_node_elem->InsertFirstChild(selected_node);
            //             }        
            //         }
            //         xml_files_manager_->selected_file_brain_->doc_->SaveFile(xml_files_manager_->selected_file_brain_->file_path_.c_str());
            //         xml_files_manager_->selected_file_brain_->file_saved_.emit();
            //     }else if(e.key() == Wt::Key::Key_1){
            //         if(content_stack_->currentWidget() == xml_files_manager_)
            //         {
            //             if(state_->xml_node_->BoolAttribute("navigation-bar-hidden"))
            //             {
            //                 xml_files_manager_->grid_layout_->itemAt(0)->widget()->animateShow(Wt::WAnimation(Wt::AnimationEffect::SlideInFromLeft, Wt::TimingFunction::EaseInOut, 500));
            //                 state_->xml_node_->SetAttribute("navigation-bar-hidden", "false");
            //             }else
            //             {
            //                 xml_files_manager_->grid_layout_->itemAt(0)->widget()->animateHide(Wt::WAnimation(Wt::AnimationEffect::SlideInFromLeft, Wt::TimingFunction::EaseInOut, 500));
            //                 state_->xml_node_->SetAttribute("navigation-bar-hidden", "true");
            //             }
            //             state_->doc_->SaveFile(state_->state_file_path_.c_str());
            //         }else if(content_stack_->currentWidget() == css_files_manager_)
            //         {
            //             if(state_->css_node_->BoolAttribute("navigation-bar-hidden"))
            //             {
            //                 css_files_manager_->grid_layout_->itemAt(0)->widget()->animateShow(Wt::WAnimation(Wt::AnimationEffect::SlideInFromLeft, Wt::TimingFunction::EaseInOut, 500));
            //                 state_->css_node_->SetAttribute("navigation-bar-hidden", "false");
            //             }else
            //             {
            //                 css_files_manager_->grid_layout_->itemAt(0)->widget()->animateHide(Wt::WAnimation(Wt::AnimationEffect::SlideInFromLeft, Wt::TimingFunction::EaseInOut, 500));
            //                 state_->css_node_->SetAttribute("navigation-bar-hidden", "true");
            //             }
            //             state_->doc_->SaveFile(state_->state_file_path_.c_str());
            //         }else if(content_stack_->currentWidget() == js_files_manager_)
            //         {
            //             if(state_->js_node_->BoolAttribute("navigation-bar-hidden"))
            //             {
            //                 js_files_manager_->grid_layout_->itemAt(0)->widget()->animateShow(Wt::WAnimation(Wt::AnimationEffect::SlideInFromLeft, Wt::TimingFunction::EaseInOut, 500));
            //                 state_->js_node_->SetAttribute("navigation-bar-hidden", "false");
            //             }else
            //             {
            //                 js_files_manager_->grid_layout_->itemAt(0)->widget()->animateHide(Wt::WAnimation(Wt::AnimationEffect::SlideInFromLeft, Wt::TimingFunction::EaseInOut, 500));
            //                 state_->js_node_->SetAttribute("navigation-bar-hidden", "true");
            //             }
            //             state_->doc_->SaveFile(state_->state_file_path_.c_str());
            //         }
            //     }else if(e.key() == Wt::Key::Key_2){
            //         if(content_stack_->currentWidget() == xml_files_manager_)
            //         {
            //             if(state_->xml_node_->BoolAttribute("editor-hidden"))
            //             {
            //                 xml_files_manager_->editor_->animateShow(Wt::WAnimation(Wt::AnimationEffect::SlideInFromLeft, Wt::TimingFunction::EaseInOut, 500));
            //                 state_->xml_node_->SetAttribute("editor-hidden", "false");
            //             }else
            //             {
            //                 xml_files_manager_->editor_->animateHide(Wt::WAnimation(Wt::AnimationEffect::SlideInFromLeft, Wt::TimingFunction::EaseInOut, 500));
            //                 state_->xml_node_->SetAttribute("editor-hidden", "true");
            //             }
            //             state_->doc_->SaveFile(state_->state_file_path_.c_str());
            //         }
            //     }else if(e.key() == Wt::Key::Key_3){
            //         if(content_stack_->currentWidget() == xml_files_manager_)
            //         {
            //             if(state_->xml_node_->BoolAttribute("preview-tree-hidden"))
            //             {
            //                 xml_files_manager_->tree_wrapper_->animateShow(Wt::WAnimation(Wt::AnimationEffect::SlideInFromLeft, Wt::TimingFunction::EaseInOut, 500));
            //                 state_->xml_node_->SetAttribute("preview-tree-hidden", "false");
            //             }else
            //             {
            //                 xml_files_manager_->tree_wrapper_->animateHide(Wt::WAnimation(Wt::AnimationEffect::SlideInFromLeft, Wt::TimingFunction::EaseInOut, 500));
            //                 state_->xml_node_->SetAttribute("preview-tree-hidden", "true");
            //             }
            //             state_->doc_->SaveFile(state_->state_file_path_.c_str());
            //         }
            //     }else if(e.key() == Wt::Key::Key_4){
            //         if(content_stack_->currentWidget() == xml_files_manager_)
            //         {
            //             if(state_->xml_node_->BoolAttribute("preview-elem-hidden"))
            //             {
            //                 xml_files_manager_->elem_wrapper_->animateShow(Wt::WAnimation(Wt::AnimationEffect::SlideInFromRight, Wt::TimingFunction::EaseInOut, 500));
            //                 state_->xml_node_->SetAttribute("preview-elem-hidden", "false");
            //             }else
            //             {
            //                 xml_files_manager_->elem_wrapper_->animateHide(Wt::WAnimation(Wt::AnimationEffect::SlideInFromRight, Wt::TimingFunction::EaseInOut, 500));
            //                 state_->xml_node_->SetAttribute("preview-elem-hidden", "true");
            //             }
            //             state_->doc_->SaveFile(state_->state_file_path_.c_str());
            //         }
            //     }else if(e.key() == Wt::Key::Key_5){
            //         if(content_stack_->currentWidget() == xml_files_manager_)
            //         {
            //             if(state_->xml_node_->BoolAttribute("control-center-hidden"))
            //             {
            //                 xml_files_manager_->control_center_->animateShow(Wt::WAnimation(Wt::AnimationEffect::SlideInFromRight, Wt::TimingFunction::EaseInOut, 500));
            //                 state_->xml_node_->SetAttribute("control-center-hidden", "false");
            //             }else
            //             {
            //                 xml_files_manager_->control_center_->animateHide(Wt::WAnimation(Wt::AnimationEffect::SlideInFromRight, Wt::TimingFunction::EaseInOut, 500));
            //                 state_->xml_node_->SetAttribute("control-center-hidden", "true");
            //             }
            //             state_->doc_->SaveFile(state_->state_file_path_.c_str());
            //         }
            //     }else if(e.key() == Wt::Key::Key_6){
            //         if(content_stack_->currentWidget() == xml_files_manager_)
            //         {
            //             if(state_->xml_node_->BoolAttribute("template-var-control-center-hidden"))
            //             {
            //                 xml_files_manager_->template_var_control_center_->animateShow(Wt::WAnimation(Wt::AnimationEffect::SlideInFromRight, Wt::TimingFunction::EaseInOut, 500));
            //                 state_->xml_node_->SetAttribute("template-var-control-center-hidden", "false");
            //             }else
            //             {
            //                 xml_files_manager_->template_var_control_center_->animateHide(Wt::WAnimation(Wt::AnimationEffect::SlideInFromRight, Wt::TimingFunction::EaseInOut, 500));
            //                 state_->xml_node_->SetAttribute("template-var-control-center-hidden", "true");
            //             }
            //             state_->doc_->SaveFile(state_->state_file_path_.c_str());
            //         }
            //     }

            // }else if (e.key() == Wt::Key::Q){
            //     if(isHidden()){
            //         animateShow(Wt::WAnimation(Wt::AnimationEffect::Pop, Wt::TimingFunction::EaseInOut, 500));
            //         content_stack_->currentWidget()->refresh();
            //         state_->stylus_node_->SetAttribute("open", "true");
            //     }else{
            //         animateHide(Wt::WAnimation(Wt::AnimationEffect::Pop, Wt::TimingFunction::EaseInOut, 500));
            //         state_->stylus_node_->SetAttribute("open", "false");
            //         Wt::WMessageResourceBundle& resource_boundle = wApp->messageResourceBundle();
            //         wApp->refresh();
                    
            //     }
            //     state_->doc_->SaveFile(state_->state_file_path_.c_str());
            // }
            // else if (e.key() == Wt::Key::Key_1){
            //     menu_->select(xml_file_manager_menu_item_);
            // }else if (e.key() == Wt::Key::Key_2){
            //     menu_->select(css_menu_item_);
            // }else if (e.key() == Wt::Key::Key_3){
            //     menu_->select(javascript_menu_item_);
            // }else if (e.key() == Wt::Key::Key_4){
            //     menu_->select(tailwind_menu_item_);
            // }else if(e.key() == Wt::Key::Key_5){
            //     menu_->select(images_menu_item_);
            // }else if (e.key() == Wt::Key::Key_6){
            //     menu_->select(settings_menu_item_);
            // }else if (e.key() == Wt::Key::Key_7){
            //     // dynamic_cast<App*>(wApp)->dark_mode_changed_.emit(!state_->stylus_node_->BoolAttribute("dark-mode"));
            //     dark_mode_toggle_->setChecked(!dark_mode_toggle_->isChecked());
            //     dark_mode_toggle_->changed().emit();
            // }else if (e.key() == Wt::Key::Up){
            //     if(!xml_files_manager_->selected_file_brain_) return;
            //     auto selected_node = xml_files_manager_->selected_file_brain_->selected_node_;
            //     if(!selected_node) return;
            //     auto prev_node = selected_node->PreviousSiblingElement();
            //     if(prev_node != nullptr)
            //     {
            //         xml_files_manager_->selected_file_brain_->xml_node_selected_.emit(prev_node, true);
            //         return;
            //     }
            //     auto parent_node = selected_node->Parent();
            //     if(parent_node != nullptr)
            //     {
            //         auto last_child = parent_node->LastChildElement();
            //         if(last_child != nullptr)
            //         {
            //             xml_files_manager_->selected_file_brain_->xml_node_selected_.emit(last_child, true);
            //             return;
            //         }
            //     }
            // }else if (e.key() == Wt::Key::Down){
            //     if(!xml_files_manager_->selected_file_brain_) return;
            //     auto selected_node = xml_files_manager_->selected_file_brain_->selected_node_;
            //     if(!selected_node) return;
            //     auto next_node = selected_node->NextSiblingElement();
            //     if(next_node != nullptr)
            //     {
            //         xml_files_manager_->selected_file_brain_->xml_node_selected_.emit(next_node, true);
            //         return;
            //     }
            //     auto parent_node = selected_node->Parent();
            //     if(parent_node != nullptr)
            //     {
            //         auto first_child = parent_node->FirstChildElement();
            //         if(first_child != nullptr)
            //         {
            //             xml_files_manager_->selected_file_brain_->xml_node_selected_.emit(first_child, true);
            //             return;
            //         }
            //     }
            // }else if (e.key() == Wt::Key::Left){
            //     if(!xml_files_manager_->selected_file_brain_) return;
            //     auto selected_node = xml_files_manager_->selected_file_brain_->selected_node_;
            //     if(selected_node && selected_node->ToElement() != xml_files_manager_->selected_file_brain_->doc_->RootElement())
            //     {
            //         auto parent_node = selected_node->Parent();
            //         if(parent_node != nullptr)
            //         {
            //             xml_files_manager_->selected_file_brain_->xml_node_selected_.emit(parent_node->ToElement(), true);
            //             return;
            //         }
            //     }
            // }else if (e.key() == Wt::Key::Right){
            //     if(!xml_files_manager_->selected_file_brain_) return;
            //     auto selected_node = xml_files_manager_->selected_file_brain_->selected_node_;
            //     if(!selected_node) return;
            //     {
            //         auto first_child = selected_node->FirstChildElement();
            //         if(first_child != nullptr)
            //         {
            //             xml_files_manager_->selected_file_brain_->xml_node_selected_.emit(first_child, true);
            //             return;
            //         }
            //     }
            // }else if (e.key() == Wt::Key::Enter){
            //     if(!xml_files_manager_->selected_file_brain_) return;
            //     auto selected_node = xml_files_manager_->selected_file_brain_->selected_node_;
            //     StylusState::logMessage(" enter key pressed");
            //     if(!selected_node) return;
            //     StylusState::logMessage(" enter key pressed");

            //     auto new_node = xml_files_manager_->selected_file_brain_->doc_->NewElement("div");
            //     StylusState::logMessage(" enter key pressed");

            //     if(xml_files_manager_->selected_file_brain_->selected_node_ == xml_files_manager_->selected_file_brain_->doc_->RootElement()) {
            //         StylusState::logMessage(" selected node is root element");
            //         if(selected_node->FirstChildElement() && state_->isCondNode(selected_node->FirstChildElement())){
            //             StylusState::logMessage("selected node first child is condition node");
            //             selected_node->InsertFirstChild(new_node);
            //         }else if(selected_node->FirstChild() && selected_node->FirstChild()->ToText()){
            //             StylusState::logMessage("selected node first child is text node");
            //             new_node = xml_files_manager_->selected_file_brain_->selected_node_;
            //         }else {
            //             StylusState::logMessage("selected node root and first child are not condition or text nodes");
            //             new_node = xml_files_manager_->selected_file_brain_->doc_->NewElement("div");
            //             selected_node->InsertFirstChild(new_node);
            //         }
            //     }else {
            //         if(state_->isCondNode(selected_node))
            //         {
            //             StylusState::logMessage("selected node is condition node");
            //             auto end_condition_node = selected_node->FirstChild();
            //             selected_node->InsertAfterChild(end_condition_node, new_node);
            //         }else if (selected_node->FirstChildElement() && state_->isCondNode(selected_node->FirstChildElement()))
            //         {
            //             StylusState::logMessage("selected node first child is condition node");
            //             selected_node->InsertFirstChild(new_node);
            //         }else {
            //             StylusState::logMessage("selected node and first child are not condition nodes");
            //             auto first_child = selected_node->FirstChild();
            //             if(first_child && first_child->ToText()){
            //                 auto parent_node = selected_node->Parent();
            //                 parent_node->InsertAfterChild(selected_node, new_node);
            //             }else if ((selected_node->FirstChild() && selected_node->FirstChild()->ToElement()) || !selected_node->FirstChild()){
            //                 selected_node->InsertFirstChild(new_node);
            //             }
            //         }
            //     }
            //     xml_files_manager_->selected_file_brain_->doc_->SaveFile(xml_files_manager_->selected_file_brain_->file_path_.c_str());
            //     xml_files_manager_->selected_file_brain_->selected_node_ = new_node;
            //     xml_files_manager_->selected_file_brain_->file_saved_.emit();
            // }else if( e.key() == Wt::Key::Delete){
            //     if(!xml_files_manager_->selected_file_brain_) return;
            //     auto selected_node = xml_files_manager_->selected_file_brain_->selected_node_;
            //     if(!selected_node || selected_node == xml_files_manager_->selected_file_brain_->doc_->RootElement()) return;
            //     auto parent_node = selected_node->Parent();
            //     auto prev_node = selected_node->PreviousSiblingElement();
            //     auto next_node = selected_node->NextSiblingElement();
            //     if(parent_node)
            //     {
            //         if(state_->isCondNode(selected_node))
            //         {
            //             parent_node->DeleteChild(selected_node->NextSibling());
            //             parent_node->DeleteChild(selected_node->PreviousSibling());
            //         }
            //         parent_node->DeleteChild(selected_node);
            //         if(prev_node)
            //         {
            //             xml_files_manager_->selected_file_brain_->selected_node_ = prev_node;
            //         }else if(next_node)
            //         {
            //             xml_files_manager_->selected_file_brain_->selected_node_ = next_node;
            //         }else{
            //             xml_files_manager_->selected_file_brain_->selected_node_ = parent_node->ToElement();
            //         }
            //         xml_files_manager_->selected_file_brain_->doc_->SaveFile(xml_files_manager_->selected_file_brain_->file_path_.c_str());
            //         xml_files_manager_->selected_file_brain_->file_saved_.emit();
            //     }
            // }else if(e.key() == Wt::Key::C){
            //     if(!xml_files_manager_->selected_file_brain_) return;
            //     if(!state_->copy_node_) return;
            //     state_->copy_node_->DeleteChildren();
            //     auto selected_node = xml_files_manager_->selected_file_brain_->selected_node_;
            //     if(!selected_node) return;
            //     if(state_->isCondNode(selected_node))
            //     {
            //         StylusState::logMessage("selected node is condition node");
            //         auto prev_node = selected_node->PreviousSibling()->DeepClone(state_->doc_.get());
            //         auto next_node = selected_node->NextSibling()->DeepClone(state_->doc_.get());
            //         state_->copy_node_->InsertFirstChild(prev_node);
            //         auto copy_selected_node = state_->copy_node_->InsertAfterChild(prev_node, selected_node->DeepClone(state_->doc_.get()));
            //         state_->copy_node_->InsertAfterChild(copy_selected_node, next_node);
            //     }else {
            //         StylusState::logMessage("selected node and first child are not condition nodes");
            //         state_->copy_node_->InsertFirstChild(selected_node->DeepClone(state_->doc_.get()));
            //     }
            //     state_->organizeXmlNode(state_->copy_node_->ToElement(), state_->state_file_path_);
            //     state_->doc_->SaveFile(state_->state_file_path_.c_str());
            // }else if(e.key() == Wt::Key::V){
            //     if(!xml_files_manager_->selected_file_brain_) return;
            //     auto selected_node = xml_files_manager_->selected_file_brain_->selected_node_;
            //     if(!selected_node) return;
            //     auto copy_node = state_->copy_node_;
            //     if(!copy_node) return;
            //     StylusState::logMessage(" Paste node ");
            //     bool is_selected_cond = state_->isCondNode(selected_node);
            //     bool is_copy_cond = state_->isCondNode(copy_node->FirstChildElement());
            //     if(is_selected_cond && is_copy_cond)
            //     {
            //         // copy and selected are conditions
            //         StylusState::logMessage("copy and selected are conditions");
            //         auto prev_node = copy_node->FirstChild()->DeepClone(xml_files_manager_->selected_file_brain_->doc_.get());
            //         auto cond_node = selected_node->DeepClone(xml_files_manager_->selected_file_brain_->doc_.get());
            //         auto next_node = copy_node->LastChild()->DeepClone(xml_files_manager_->selected_file_brain_->doc_.get());
            //         selected_node->InsertAfterChild(selected_node->FirstChild(), prev_node);
            //         selected_node->InsertAfterChild(prev_node, cond_node);
            //         selected_node->InsertAfterChild(cond_node, next_node);
            //         xml_files_manager_->selected_file_brain_->selected_node_ = cond_node->ToElement();
            //         state_->organizeXmlNode(cond_node->ToElement(), state_->state_file_path_);
            //     }else if(is_selected_cond && !is_copy_cond)
            //     {
            //         // copy is not condition
            //         StylusState::logMessage("copy is not condition");
            //         auto copy_node_child = copy_node->FirstChild()->DeepClone(xml_files_manager_->selected_file_brain_->doc_.get());
            //         selected_node->InsertAfterChild(selected_node->FirstChild(), copy_node_child);
            //         xml_files_manager_->selected_file_brain_->selected_node_ = copy_node_child->ToElement();
            //     }else if(!is_selected_cond && is_copy_cond)
            //     {
            //         // selected is not condition
            //         StylusState::logMessage("selected is not condition copy is condition");

            //         if(selected_node != xml_files_manager_->selected_file_brain_->doc_->RootElement())
            //         {
            //             StylusState::logMessage("selected node is not root element");
            //             if(selected_node->FirstChild() && selected_node->FirstChild()->ToText()){
            //                 StylusState::logMessage("selected node first child is not text 1");
            //                 auto parent_node = selected_node->Parent();
            //                 auto prev = parent_node->InsertAfterChild(selected_node, copy_node->FirstChild()->DeepClone(xml_files_manager_->selected_file_brain_->doc_.get()));
            //                 auto new_selected = parent_node->InsertAfterChild(prev, copy_node->FirstChildElement()->DeepClone(xml_files_manager_->selected_file_brain_->doc_.get()));
            //                 parent_node->InsertAfterChild(new_selected, copy_node->LastChild()->DeepClone(xml_files_manager_->selected_file_brain_->doc_.get()));
            //                 xml_files_manager_->selected_file_brain_->selected_node_ = new_selected->ToElement();
            //                 state_->organizeXmlNode(new_selected->ToElement(), state_->state_file_path_);
            //             }else {
            //                 StylusState::logMessage("selected node has child element and first child is not text");
            //                 auto prev = selected_node->InsertFirstChild(copy_node->FirstChild()->DeepClone(xml_files_manager_->selected_file_brain_->doc_.get()));
            //                 auto new_selected = selected_node->InsertAfterChild(prev, copy_node->FirstChildElement()->DeepClone(xml_files_manager_->selected_file_brain_->doc_.get()));
            //                 selected_node->InsertAfterChild(new_selected, copy_node->LastChild()->DeepClone(xml_files_manager_->selected_file_brain_->doc_.get()));
            //                 xml_files_manager_->selected_file_brain_->selected_node_ = new_selected->ToElement();
            //                 state_->organizeXmlNode(new_selected->ToElement(), state_->state_file_path_);
            //             }
            //         }else {
            //             StylusState::logMessage("selected node is root element");

            //             if(selected_node->FirstChild() && selected_node->FirstChild()->ToText() && selected_node->FirstChildElement() && 
            //             state_->isCondNode(selected_node->FirstChildElement())
            //             ){
            //                 StylusState::logMessage("selected node first child is not text 2");
            //                 auto prev = selected_node->InsertFirstChild(copy_node->FirstChild()->DeepClone(xml_files_manager_->selected_file_brain_->doc_.get()));
            //                 auto new_selected = selected_node->InsertAfterChild(prev, copy_node->FirstChildElement()->DeepClone(xml_files_manager_->selected_file_brain_->doc_.get()));
            //                 selected_node->InsertAfterChild(new_selected, copy_node->LastChild()->DeepClone(xml_files_manager_->selected_file_brain_->doc_.get()));
            //                 xml_files_manager_->selected_file_brain_->selected_node_ = new_selected->ToElement();
            //                 state_->organizeXmlNode(new_selected->ToElement(), state_->state_file_path_);
            //             }else if((selected_node->FirstChild() && !selected_node->FirstChild()->ToText() || !selected_node->FirstChild())){
            //                 StylusState::logMessage("selected node first child is not text 3");
            //                 auto prev = selected_node->InsertFirstChild(copy_node->FirstChild()->DeepClone(xml_files_manager_->selected_file_brain_->doc_.get()));
            //                 auto new_selected = selected_node->InsertAfterChild(prev, copy_node->FirstChildElement()->DeepClone(xml_files_manager_->selected_file_brain_->doc_.get()));
            //                 selected_node->InsertAfterChild(new_selected, copy_node->LastChild()->DeepClone(xml_files_manager_->selected_file_brain_->doc_.get()));
            //                 xml_files_manager_->selected_file_brain_->selected_node_ = new_selected->ToElement();
            //                 state_->organizeXmlNode(new_selected->ToElement(), state_->state_file_path_);
            //             }
            //         }
            //     }else {
            //         // selected and copy are not conditions
            //         StylusState::logMessage("selected and copy are not conditions");
            //         auto copy_node_child = copy_node->FirstChild()->DeepClone(xml_files_manager_->selected_file_brain_->doc_.get());
            //         if(selected_node->FirstChild() && selected_node->FirstChild()->ToText()){
            //             if(selected_node != xml_files_manager_->selected_file_brain_->doc_->RootElement())
            //             {
            //                 StylusState::logMessage("selected node is not root element");
            //                 auto parent_node = selected_node->Parent();
            //                 auto new_selected = parent_node->InsertAfterChild(selected_node, copy_node_child);
            //                 xml_files_manager_->selected_file_brain_->selected_node_ = new_selected->ToElement();
            //             }
            //         }else {
            //             xml_files_manager_->selected_file_brain_->selected_node_ = selected_node->InsertFirstChild(copy_node_child)->ToElement();
            //         }
                                        
            //     }
                
            //     xml_files_manager_->selected_file_brain_->doc_->SaveFile(xml_files_manager_->selected_file_brain_->file_path_.c_str());
            //     xml_files_manager_->selected_file_brain_->file_saved_.emit();
            }
        } });


    }

}
