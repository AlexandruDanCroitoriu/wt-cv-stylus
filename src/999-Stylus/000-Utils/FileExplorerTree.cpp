#include "999-Stylus/000-Utils/FileExplorerTree.h"

#include <filesystem>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>
#include <fstream>
#include <Wt/WMessageBox.h>
#include <Wt/WLineEdit.h>
#include <Wt/WLabel.h>
#include <Wt/WDialog.h>
#include <regex>
#include <Wt/WApplication.h>
#include <Wt/WText.h>
#include <Wt/WIconPair.h>
#include <Wt/WIcon.h>

namespace Stylus
{

    FileExplorerTree::FileExplorerTree(std::shared_ptr<StylusState> state, StylusEditorManagementData data)
        : Wt::WContainerWidget(),
          state_(state),
          data_(data)
    {
        setStyleClass("flex flex-col max-h-screen select-none");
        setLayoutSizeAware(true);
        // tree header
        // setMinimumSize(Wt::WLength(240, Wt::LengthUnit::Pixel), Wt::WLength(100, Wt::LengthUnit::ViewportHeight));
        setMinimumSize(Wt::WLength(240, Wt::LengthUnit::Pixel), Wt::WLength::Auto);
        setMaximumSize(Wt::WLength(1000, Wt::LengthUnit::Pixel), Wt::WLength(100, Wt::LengthUnit::ViewportHeight));

        contents_ = addWidget(std::make_unique<Wt::WContainerWidget>());
        contents_->setStyleClass("w-full flex-[1] overflow-y-auto overflow-x-hidden flex flex-col stylus-scrollbar");
        tree_ = contents_->addWidget(std::make_unique<Wt::WTree>());

        // footer_ = addWidget(std::make_unique<Wt::WContainerWidget>());
        // footer_->setStyleClass("flex items-center justify-between p-[3px] border-t border-solid");
        // footer_->hide();
        setTreeFolderWidgets();
        folders_changed_.connect(this, [=]()
        {
            setTreeFolderWidgets();
        });

    }

    void FileExplorerTree::layoutSizeChanged(int width, int height)
    {
        if (width >= 240 && width <= 1000)
        {
            width_changed_.emit(std::to_string(width));
        }
    }

    TreeNode* FileExplorerTree::selectedNode()
    {
        if (tree_) {
            auto selected_nodes = tree_->selectedNodes();
            if (!selected_nodes.empty()) {
                // Return the first selected node as TreeNode*, or nullptr if cast fails
                return dynamic_cast<TreeNode*>(*selected_nodes.begin());
            }
        }
        return nullptr;
    }

    void FileExplorerTree::setTreeFolderWidgets()
    {
        auto node = std::make_unique<TreeNode>(data_.root_folder_path_, TreeNodeType::Folder, data_.root_folder_path_, data_);
        auto root_folder = node.get();
        tree_->setTreeRoot(std::move(node));
        tree_->setSelectionMode(Wt::SelectionMode::Single);
        tree_->treeRoot()->label()->setTextFormat(Wt::TextFormat::Plain);
        tree_->treeRoot()->expand();
        // tree_->treeRoot()->setLoadPolicy(Wt::ContentLoading::NextLevel);
        std::vector<std::pair<std::string, std::vector<std::string>>> folders = data_.getFolders();
        

        for (auto folder : folders)
        {
            TreeNode *folder_tree_node = dynamic_cast<TreeNode *>(tree_->treeRoot()->addChildNode(std::make_unique<TreeNode>(folder.first, TreeNodeType::Folder, data_.root_folder_path_, data_)));

            for (const auto &file : folder.second)
            {
                auto file_tree_node = dynamic_cast<TreeNode *>(folder_tree_node->addChildNode(std::make_unique<TreeNode>(file, TreeNodeType::File, data_.root_folder_path_ + folder.first + "/", data_)));

                if (selected_file_path_.compare(folder.first + "/" + file) == 0)
                {
                    // if (editor_->unsavedChanges())
                    // {
                    //     file_tree_node->addStyleClass("unsaved-changes");
                    // }
                    tree_->select(file_tree_node);
                }

                file_tree_node->selected().connect(this, [=](bool selected)
                                                   {
                if(selected){
                    selected_file_path_ = file_tree_node->parentNode()->label()->text().toUTF8() + "/" + file_tree_node->label()->text().toUTF8();
                    file_selected_.emit(selected_file_path_);
                } });

                file_tree_node->folders_changed_.connect(this, [=]()
                                                         {
                // if(selected_file_path.compare("") != 0) {
                //     selected_file_path_ = selected_file_path;
                // }
                // getFolders();
                // file_selected_.emit(selected_file_path_);
                folders_changed_.emit(); });
            }

            if (folder.second.size() > 0)
            {
                folder_tree_node->expand();
            }

            folder_tree_node->folders_changed_.connect(this, [=]()
                                                       {
            // if(selected_file_path.compare("") != 0) {
            //     selected_file_path_ = selected_file_path;
            // }
            // getFolders();
            // file_selected_.emit(selected_file_path_);
            folders_changed_.emit(); });
        }

        root_folder->folders_changed_.connect(this, [=]()
                                              {
                                                  folders_changed_.emit();
                                                  // if(selected_file_path.compare("") != 0) {
                                                  //     selected_file_path_ = selected_file_path;
                                                  // }
                                                  // getFolders();
                                                  // file_selected_.emit(selected_file_path_);
                                                  // setTreeFolderWidgets();
                                              });
    }

    TreeNode::TreeNode(std::string name, TreeNodeType type, std::string path, StylusEditorManagementData data)
        : Wt::WTreeNode(name),
          type_(type),
          path_(path),
          data_(data)
    {
        // setStyleClass("relative");
        label_wrapper_ = labelArea();
        label_wrapper_->addStyleClass("flex items-center cursor-pointer mr-[3px] text-on-surface");

        if (type_ == TreeNodeType::Folder)
        {
            setSelectable(false);
            // if(name.compare(path_) != 0)
            // {
            // std::cout << "\n\nFolder name: " << name << " path: " << path_ << std::endl;
            acceptDrops("file", "Wt-item");
            // }
        }
        else if (type_ == TreeNodeType::File)
        {
            setSelectable(true);
        }
        setParentWidget(parentNode());

        selected().connect(this, [=](bool selected)
                           {
        if(selected)
        {
            auto drag_handle = label_wrapper_->addWidget(std::make_unique<Wt::WTemplate>(tr("stylus-svg-drag-handle")));
            drag_handle->setStyleClass("w-4 h-4 flex items-center justify-center ml-auto"); 
            drag_handle->clicked().preventPropagation();
            drag_handle->setDraggable("file", this, false, this);
        } });

        label_wrapper_->mouseWentUp().connect(this, [=](const Wt::WMouseEvent &event)
                                              {
        if (event.button() == Wt::MouseButton::Right) {
            showPopup(event);
        } });

        std::unique_ptr<Wt::WIconPair> icon;
        if (type_ == TreeNodeType::Folder)
        {
            setLabelIcon(std::make_unique<Wt::WIconPair>("./static/stylus/yellow-folder-closed.png", "./static/stylus/yellow-folder-open.png", false));
        }
        else if (type_ == TreeNodeType::File)
        {
            setLabelIcon(std::make_unique<Wt::WIconPair>("static/stylus/document.png", "static/stylus/document.png", false));
        }
    }

    void TreeNode::dropEvent(Wt::WDropEvent event)
    {
        TreeNode *source_node = static_cast<TreeNode *>(event.source());

        if (source_node->parentNode() != this && label()->text().toUTF8().compare(path_) != 0)
        {
            Wt::WTreeNode *new_node;
            std::unique_ptr<Wt::WTreeNode> removed_node = source_node->parentNode()->removeChildNode(source_node);
            new_node = addChildNode(std::move(removed_node));

            // check if there is a file with the same name
            std::string new_file_name = source_node->label()->text().toUTF8();
            std::string new_file_path = path_ + new_file_name;
            if (std::filesystem::exists(new_file_path))
            {
                auto message_box = addChild(std::make_unique<Wt::WMessageBox>(
                    "File already exists",
                    R"(
                    <div class='flex-1 text-center font-bold text-2xl'>)" +
                        new_file_name + R"(</div>
                )",
                    Wt::Icon::Warning, Wt::StandardButton::None));
                message_box->setOffsets(100, Wt::Side::Top);
                message_box->setModal(true);

                message_box->setStyleClass("");
                message_box->titleBar()->setStyleClass("flex items-center justify-center p-[8px] cursor-pointer border-b border-solid text-xl font-bold");
                message_box->contents()->setStyleClass("flex flex-col");
                auto content = message_box->contents()->addWidget(std::make_unique<Wt::WContainerWidget>());
                auto footer = message_box->contents()->addWidget(std::make_unique<Wt::WContainerWidget>());
                content->setStyleClass("p-[8px]");
                footer->setStyleClass("flex items-center justify-between p-[8px]");
                auto error_label = content->addWidget(std::make_unique<Wt::WText>(""));
                error_label->setStyleClass("w-full text-[#B22222] text-md font-semibold");
                auto label = content->addWidget(std::make_unique<Wt::WLabel>("File with the same name already exists. Do you want to replace it?"));
                auto confirm_btn = footer->addWidget(std::make_unique<Wt::WPushButton>("Confirm"));
                confirm_btn->setStyleClass("btn-default");
                auto cancel_btn = footer->addWidget(std::make_unique<Wt::WPushButton>("Cancel"));
                cancel_btn->setStyleClass("btn-red");
                cancel_btn->clicked().connect(this, [=]()
                                              { message_box->reject(); });
                confirm_btn->clicked().connect(this, [=]()
                                               { message_box->accept(); });
                message_box->finished().connect(this, [=]()
                                                {
                if (message_box->result() == Wt::DialogCode::Accepted) {
                    std::filesystem::path old_path(source_node->path_ + source_node->label()->text().toUTF8());
                    std::filesystem::path new_path(path_ + this->label()->text().toUTF8() + "/" + source_node->label()->text().toUTF8());
                    std::filesystem::rename(old_path, new_path);
                    // folders_changed_.emit(this->label()->text().toUTF8() + "/" + source_node->label()->text().toUTF8());
                    folders_changed_.emit();
                }else {
                    source_node->selected().emit(true);
                }
                removeChild(message_box); });
                message_box->show();
                return;
            }

            // move file to this folder
            std::filesystem::path old_path(source_node->path_ + source_node->label()->text().toUTF8());
            std::filesystem::path new_path(path_ + label()->text().toUTF8() + "/" + source_node->label()->text().toUTF8());
            std::filesystem::rename(old_path, new_path);
            // folders_changed_.emit(label()->text().toUTF8() + "/" + source_node->label()->text().toUTF8());
            folders_changed_.emit();
        }
        else
        {
            source_node->selected().emit(true);
        }
    }

    void TreeNode::showPopup(const Wt::WMouseEvent &event)
    {
        if (!popup_)
        {
            popup_ = std::make_unique<Wt::WPopupMenu>();
            popup_->setStyleClass("bg-surface !text-on-surface");
            
            if (type_ == TreeNodeType::Folder)
            {
                if (label()->text().toUTF8().compare(path_) == 0)
                {
                    popup_->addItem("Create New Folder")->clicked().connect(this, [=]()
                                                                            { createNewFolderDialog(); });
                }
                else
                {
                    popup_->addItem("copy import to clipboard")->clicked().connect(this, [=]()
                                                                                   { copyFilePathToClipboard(); });
                    popup_->addSeparator();
                    popup_->addItem("Create New File")->clicked().connect(this, [=]()
                                                                          { createNewFileDialog(); });
                    popup_->addItem("Rename Folder")->clicked().connect(this, [=]()
                                                                        { createRenameFolderDialog(); });
                    popup_->addSeparator();
                    popup_->addItem("Delete Folder")->clicked().connect(this, [=]()
                                                                        { createRemoveFolderMessageBox(); });
                }
            }
            else if (type_ == TreeNodeType::File)
            {
                popup_->addItem("copy import to clipboard")->clicked().connect(this, [=]()
                                                                               { copyFilePathToClipboard(); });
                popup_->addSeparator();
                popup_->addItem("Rename File")->clicked().connect(this, [=]()
                                                                  { createRenameFileDialog(); });
                popup_->addSeparator();
                popup_->addItem("Delete File")->clicked().connect(this, [=]()
                                                                  { deleteFileMessageBox(); });
            }
        }

        if (popup_->isHidden())
            popup_->popup(event);
        else
            popup_->hide();
    }

    void TreeNode::createNewFolderDialog()
    {
        auto dialog = Wt::WApplication::instance()->root()->addChild(std::make_unique<Wt::WDialog>("Create new folder"));

        dialog->setModal(true);
        dialog->rejectWhenEscapePressed();
        dialog->setOffsets(100, Wt::Side::Top);

        dialog->setStyleClass("");
        dialog->titleBar()->setStyleClass("flex items-center justify-center p-[8px] cursor-pointer border-b border-solid text-xl font-bold");
        dialog->contents()->setStyleClass("flex flex-col");

        auto content = dialog->contents()->addWidget(std::make_unique<Wt::WContainerWidget>());
        auto footer = dialog->contents()->addWidget(std::make_unique<Wt::WContainerWidget>());

        content->setStyleClass("p-[8px]");
        footer->setStyleClass("flex items-center justify-between p-[8px]");

        auto input_wrapper = content->addWidget(std::make_unique<Wt::WContainerWidget>());
        input_wrapper->setStyleClass("flex flex-col items-center justify-center");
        auto error_label = content->addWidget(std::make_unique<Wt::WText>(""));
        error_label->setStyleClass("w-full text-[#B22222] text-md font-semibold");

        auto label = input_wrapper->addWidget(std::make_unique<Wt::WLabel>("Name"));
        auto new_folder_name_input = input_wrapper->addWidget(std::make_unique<Wt::WLineEdit>());
        new_folder_name_input->setStyleClass("w-full min-w-[200px] placeholder:text-slate-400 text-sm border rounded-md px-3 py-2 transition duration-300 ease focus:outline-none shadow-sm");
        label->setBuddy(new_folder_name_input);

        auto confirm_btn = footer->addWidget(std::make_unique<Wt::WPushButton>("Confirm"));
        confirm_btn->setStyleClass("btn-default");
        auto cancel_btn = footer->addWidget(std::make_unique<Wt::WPushButton>("Cancel"));
        cancel_btn->setStyleClass("btn-red");

        cancel_btn->clicked().connect(this, [=]()
                                      { dialog->reject(); });
        new_folder_name_input->enterPressed().connect(this, [=]()
                                                      { confirm_btn->clicked().emit(Wt::WMouseEvent()); });
        confirm_btn->clicked().connect(this, [=]()
                                       { 
        // check if the folder already exists
        std::string new_folder_name = new_folder_name_input->text().toUTF8();
        std::string pattern = R"(^[a-zA-Z0-9-_]+$)";
        if(std::regex_match(new_folder_name, std::regex(pattern)) == false) {
            error_label->setText("Match reges:" + pattern);
            return;
        }
        std::filesystem::path new_path(path_ + new_folder_name);
        if (std::filesystem::exists(new_path)) {
            error_label->setText("Folder with the same name already exists.");
        }else {
            dialog->accept();                
        } });
        dialog->finished().connect(this, [=]()
                                   {
        if (dialog->result() == Wt::DialogCode::Accepted) {
            std::string new_folder_name = new_folder_name_input->text().toUTF8();
            std::filesystem::path new_path(path_ + new_folder_name);
            std::filesystem::create_directory(new_path);
            folders_changed_.emit();
        }
        removeChild(dialog); });
        dialog->show();
    }
    void TreeNode::createRenameFolderDialog()
    {
        auto dialog = Wt::WApplication::instance()->root()->addChild(std::make_unique<Wt::WDialog>("Create new folder"));

        dialog->setModal(true);
        dialog->rejectWhenEscapePressed();
        dialog->setOffsets(100, Wt::Side::Top);

        dialog->setStyleClass("");
        dialog->titleBar()->setStyleClass("flex items-center justify-center p-[8px] cursor-pointer border-b border-solid text-xl font-bold");
        dialog->contents()->setStyleClass("flex flex-col");

        auto content = dialog->contents()->addWidget(std::make_unique<Wt::WContainerWidget>());
        auto footer = dialog->contents()->addWidget(std::make_unique<Wt::WContainerWidget>());

        content->setStyleClass("p-[8px]");
        footer->setStyleClass("flex items-center justify-between p-[8px]");

        auto input_wrapper = content->addWidget(std::make_unique<Wt::WContainerWidget>());
        input_wrapper->setStyleClass("flex flex-col items-center justify-center");
        auto error_label = content->addWidget(std::make_unique<Wt::WText>(""));
        error_label->setStyleClass("w-full text-[#B22222] text-md font-semibold");

        auto label = input_wrapper->addWidget(std::make_unique<Wt::WLabel>("current name: " + this->label()->text()));
        auto new_folder_name_input = input_wrapper->addWidget(std::make_unique<Wt::WLineEdit>(this->label()->text()));
        new_folder_name_input->setStyleClass("w-full min-w-[200px] placeholder:text-slate-400 text-sm border rounded-md px-3 py-2 transition duration-300 ease focus:outline-none shadow-sm");
        label->setBuddy(new_folder_name_input);

        auto confirm_btn = footer->addWidget(std::make_unique<Wt::WPushButton>("Confirm"));
        confirm_btn->setStyleClass("btn-default");
        auto cancel_btn = footer->addWidget(std::make_unique<Wt::WPushButton>("Cancel"));
        cancel_btn->setStyleClass("btn-red");

        cancel_btn->clicked().connect(this, [=]()
                                      { dialog->reject(); });
        new_folder_name_input->enterPressed().connect(this, [=]()
                                                      { confirm_btn->clicked().emit(Wt::WMouseEvent()); });
        confirm_btn->clicked().connect(this, [=]()
                                       { 
        // check if the folder already exists
        std::string new_folder_name = new_folder_name_input->text().toUTF8();
        std::string pattern = R"(^[a-z0-9-_]+$)";
        if(std::regex_match(new_folder_name, std::regex(pattern)) == false) {
            error_label->setText("Match reges:" + pattern);
            return;
        }
        std::filesystem::path new_path(path_ + new_folder_name);
        if (std::filesystem::exists(new_path)) {
            error_label->setText("Folder with the same name already exists.");
        }else {
            dialog->accept();                
        } });
        dialog->finished().connect(this, [=]()
                                   {
        if (dialog->result() == Wt::DialogCode::Accepted) {
            std::string new_folder_name = new_folder_name_input->text().toUTF8();
            std::filesystem::path old_path(path_ + this->label()->text().toUTF8());
            std::filesystem::path new_path(path_ + new_folder_name);
            std::filesystem::rename(old_path, new_path);
            this->label()->setText(new_folder_name);
            folders_changed_.emit();
        }
        removeChild(dialog); });
        dialog->show();
    }
    void TreeNode::createRemoveFolderMessageBox()
    {
        auto message_box = addChild(std::make_unique<Wt::WMessageBox>(
            "Rename folder ?",
            R"(
            <div class='flex-1 text-center font-bold text-2xl'>)" +
                label()->text() + R"(</div>
        )",
            Wt::Icon::Warning, Wt::StandardButton::None));
        message_box->setOffsets(100, Wt::Side::Top);
        message_box->setModal(true);

        message_box->setStyleClass("");
        message_box->titleBar()->setStyleClass("flex items-center justify-center p-[8px] cursor-pointer border-b border-solid text-xl font-bold");
        message_box->contents()->addStyleClass("flex items-center");
        message_box->footer()->setStyleClass("flex items-center justify-between p-[8px]");

        auto delete_btn = message_box->addButton("Delete", Wt::StandardButton::Yes);
        auto cancel_btn = message_box->addButton("Cancel", Wt::StandardButton::No);
        delete_btn->setStyleClass("btn-red");
        cancel_btn->setStyleClass("btn-default");

        message_box->buttonClicked().connect([=]
                                             {
        if (message_box->buttonResult() == Wt::StandardButton::Yes)
            {
            std::filesystem::path file_path = path_ + label()->text().toUTF8();

            // delete folder and all its contents
            std::filesystem::remove_all(file_path);
            folders_changed_.emit();
            
        }
        removeChild(message_box); });

        message_box->show();
    }

    void TreeNode::createNewFileDialog()
    {
        auto dialog = Wt::WApplication::instance()->root()->addChild(std::make_unique<Wt::WDialog>("Create new file in folder " + label()->text()));
        dialog->setOffsets(100, Wt::Side::Top);
        dialog->setModal(true);
        dialog->rejectWhenEscapePressed();
        dialog->setStyleClass("");
        dialog->titleBar()->setStyleClass("flex items-center justify-center p-[8px] cursor-pointer border-b border-solid text-xl font-bold");
        dialog->contents()->setStyleClass("flex flex-col");

        auto content = dialog->contents()->addWidget(std::make_unique<Wt::WContainerWidget>());
        auto footer = dialog->contents()->addWidget(std::make_unique<Wt::WContainerWidget>());

        content->setStyleClass("p-[8px]");
        footer->setStyleClass("flex items-center justify-between p-[8px]");

        auto input_wrapper = content->addWidget(std::make_unique<Wt::WContainerWidget>());
        input_wrapper->setStyleClass("flex flex-col items-center justify-center");
        auto error_label = content->addWidget(std::make_unique<Wt::WText>(""));
        error_label->setStyleClass("w-full text-md font-semibold");

        auto label = input_wrapper->addWidget(std::make_unique<Wt::WLabel>("Name"));
        auto new_file_name_input = input_wrapper->addWidget(std::make_unique<Wt::WLineEdit>());
        new_file_name_input->setStyleClass("w-full min-w-[200px] text-sm border rounded-md px-3 py-2 transition duration-300 ease focus:outline-none shadow-sm focus:shadow");
        label->setBuddy(new_file_name_input);

        auto confirm_btn = footer->addWidget(std::make_unique<Wt::WPushButton>("Confirm"));
        confirm_btn->setStyleClass("btn-default");
        auto cancel_btn = footer->addWidget(std::make_unique<Wt::WPushButton>("Cancel"));
        cancel_btn->setStyleClass("btn-red");

        cancel_btn->clicked().connect(this, [=]()
                                      { dialog->reject(); });
        new_file_name_input->enterPressed().connect(this, [=]()
                                                    { confirm_btn->clicked().emit(Wt::WMouseEvent()); });

        confirm_btn->clicked().connect(this, [=]()
                                       { 
        // check if the file name already exists
        std::string new_file_name = new_file_name_input->text().toUTF8();
        std::string pattern = R"(^[a-z-.]+$)";
        if(std::regex_match(new_file_name, std::regex(pattern)) == false) {
            error_label->setText("Match reges:" + pattern);
            return;
        }
        std::filesystem::path new_path(path_ + this->label()->text().toUTF8() + "/" +  new_file_name);
        if (std::filesystem::exists(new_path)) {
            error_label->setText("file with the same name already exists.");
        }else {
            dialog->accept();                
        } });
        dialog->finished().connect(this, [=]()
                                   {
        if (dialog->result() == Wt::DialogCode::Accepted) {
            std::string new_file_name = new_file_name_input->text().toUTF8();
            std::filesystem::path new_path(path_ + this->label()->text().toUTF8() + "/" + new_file_name);
            std::ofstream new_file(new_path);
            if(data_.extension_.compare("xml") == 0) {
                new_file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
                new_file << "<!-- Created with Stylus Editor -->\n";
                new_file << "<messages>\n";
                new_file << "\t<message id=\"example\">\n";
                new_file << "\t\t<div>Example div text</div>\n";
                new_file << "\t\t<p>Example paragraph text</p>\n";
                new_file << "\t</message>\n";
                new_file << "</messages>\n";
                new_file.close();
            }
            // folders_changed_.emit(this->label()->text().toUTF8() + "/" + new_file_name);
            folders_changed_.emit();
        }
        removeChild(dialog); });

        dialog->show();
    }
    void TreeNode::createRenameFileDialog()
    {
        auto dialog = Wt::WApplication::instance()->root()->addChild(std::make_unique<Wt::WDialog>("Rename File: " + label()->text()));

        dialog->setModal(true);
        dialog->rejectWhenEscapePressed();
        dialog->setOffsets(100, Wt::Side::Top);

        dialog->setStyleClass("");
        dialog->titleBar()->setStyleClass("flex items-center justify-center p-[8px] cursor-pointer border-b border-solid text-xl font-bold");
        dialog->contents()->setStyleClass("flex flex-col");

        auto content = dialog->contents()->addWidget(std::make_unique<Wt::WContainerWidget>());
        auto footer = dialog->contents()->addWidget(std::make_unique<Wt::WContainerWidget>());

        content->setStyleClass("p-[8px]");
        footer->setStyleClass("flex items-center justify-between p-[8px]");

        auto input_wrapper = content->addWidget(std::make_unique<Wt::WContainerWidget>());
        input_wrapper->setStyleClass("flex flex-col items-center justify-center");
        auto error_label = content->addWidget(std::make_unique<Wt::WText>(""));
        error_label->setStyleClass("w-full text-[#B22222] text-md font-semibold");

        auto new_file_name_input = input_wrapper->addWidget(std::make_unique<Wt::WLineEdit>(this->label()->text()));
        new_file_name_input->setStyleClass("w-full min-w-[200px] placeholder:text-slate-400 text-sm border rounded-md px-3 py-2 transition duration-300 ease focus:outline-none shadow-sm");

        auto confirm_btn = footer->addWidget(std::make_unique<Wt::WPushButton>("Confirm"));
        confirm_btn->setStyleClass("btn-default");
        auto cancel_btn = footer->addWidget(std::make_unique<Wt::WPushButton>("Cancel"));
        cancel_btn->setStyleClass("btn-red");

        cancel_btn->clicked().connect(this, [=]()
                                      { dialog->reject(); });
        new_file_name_input->enterPressed().connect(this, [=]()
                                                    { confirm_btn->clicked().emit(Wt::WMouseEvent()); });
        confirm_btn->clicked().connect(this, [=]()
                                       { 
        // check if the file name already exists
        std::string new_file_name = new_file_name_input->text().toUTF8();
        std::string pattern = R"(^[a-z-.]+$)";
        if(std::regex_match(new_file_name, std::regex(pattern)) == false) {
            error_label->setText("Match reges:" + pattern);
            return;
        }
        std::filesystem::path new_path(path_ + new_file_name);
        if (std::filesystem::exists(new_path)) {
            error_label->setText("file with the same name already exists.");
        }else {
            dialog->accept();                
        } });
        dialog->finished().connect(this, [=]()
                                   {
        if (dialog->result() == Wt::DialogCode::Accepted) {
            std::string new_file_name = new_file_name_input->text().toUTF8();
            std::filesystem::path old_path(path_ + this->label()->text().toUTF8());
            std::filesystem::path new_path(path_ + new_file_name);
            std::filesystem::rename(old_path, new_path);
            // folders_changed_.emit(parentNode()->label()->text().toUTF8() + "/" + new_file_name);
            folders_changed_.emit();
        }
        removeChild(dialog); });
        dialog->show();
    }
    void TreeNode::deleteFileMessageBox()
    {
        auto message_box = addChild(std::make_unique<Wt::WMessageBox>(
            "Delete file: " + label()->text() + " ?", "",
            Wt::Icon::None, Wt::StandardButton::None));
        message_box->setOffsets(100, Wt::Side::Top);
        message_box->setModal(true);

        message_box->setStyleClass("");
        message_box->titleBar()->setStyleClass("flex items-center justify-center p-[8px] cursor-pointer border-b border-solid text-xl font-bold");
        message_box->contents()->addStyleClass("flex items-center");
        message_box->footer()->setStyleClass("flex items-center justify-between p-[8px]");

        auto delete_btn = message_box->addButton("Delete", Wt::StandardButton::Yes);
        auto cancel_btn = message_box->addButton("Cancel", Wt::StandardButton::No);
        delete_btn->setStyleClass("btn-red");
        cancel_btn->setStyleClass("btn-default");

        message_box->buttonClicked().connect([=]
                                             {
        if (message_box->buttonResult() == Wt::StandardButton::Yes)
            {
            std::filesystem::path file_path = path_ + label()->text().toUTF8();
            // delete file
            if (std::filesystem::remove(file_path)) {
                folders_changed_.emit();
            } else {
                Wt::WApplication::instance()->log("ERROR") << "\n\nError deleting file.\n\n";                    
            }
        }
        removeChild(message_box); });

        message_box->show();
    }

    void TreeNode::copyFilePathToClipboard()
    {
        Wt::WApplication::instance()->doJavaScript(
            "navigator.clipboard.writeText('" + getNodeImportString() + "');",
            "copyFilePathToClipboard");
    }
    std::string TreeNode::getNodeImportString()
    {
        std::string path = path_ + label()->text().toUTF8();
        std::string return_imports = "";

        if (type_ == TreeNodeType::Folder)
        {
            // get all the child file node imports into return_imports
            for (auto *child : childNodes())
            {
                TreeNode *child_node = dynamic_cast<TreeNode *>(child);
                if (child_node && child_node->type_ == TreeNodeType::File)
                {
                    return_imports += child_node->getNodeImportString();
                }
            }
        }
        else if (type_ == TreeNodeType::File)
        {
            if (data_.extension_.compare("xml") == 0)
            {
                if (path.size() >= 4 && path.compare(path.size() - 4, 4, ".xml") == 0)
                {
                    path = path.substr(0, path.size() - 4);
                }
                return_imports += "messageResourceBundle().use(\"" + path + "\");\\n";
            }
            else if (data_.extension_.compare("css") == 0)
            {
                return_imports += "useStyleSheet(\"" + path + "\");\\n";
            }
            else if (data_.extension_.compare("js") == 0)
            {
                return_imports += "require(\"" + path + "\");\\n";
            }
        }
        std::cout << "\n\n getNodeImportString: \n\n"
                  << return_imports << "\n\n";
        return return_imports;
    }
}
