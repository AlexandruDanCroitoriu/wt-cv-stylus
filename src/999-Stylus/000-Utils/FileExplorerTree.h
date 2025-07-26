#pragma once

#include "999-Stylus/000-Utils/StylusState.h"

#include <Wt/WContainerWidget.h>
#include <Wt/WTree.h>
#include <Wt/WTreeNode.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WSignal.h>

namespace Stylus {

    class FileExplorerTree : public Wt::WContainerWidget
    {
    public:
        FileExplorerTree(std::shared_ptr<StylusState> state, StylusEditorManagementData data);
        Wt::WContainerWidget* contents_;
        // Wt::WContainerWidget* footer_;
        Wt::Signal<Wt::WString>& width_changed() { return width_changed_; }
        Wt::Signal<std::string>& folders_changed() { return folders_changed_; }
        Wt::Signal<std::string>& file_selected() { return file_selected_; }
        
        void setTreeFolderWidgets();
        protected:
        // Custom implementation
        void layoutSizeChanged(int width, int height) override;
        
        private:
        Wt::Signal<Wt::WString> width_changed_;
        Wt::Signal<std::string> folders_changed_; // used mostly by xml files manager to recreate xml file brains
        Wt::Signal<std::string> file_selected_; // returns path of the selected file 

        std::shared_ptr<StylusState> state_;
        StylusEditorManagementData data_;
        Wt::WTree* tree_;
        std::string selected_file_path_;
    };

    enum TreeNodeType
    {
        Folder,
        File
    };

    class TreeNode : public Wt::WTreeNode
    {
    public:
        TreeNode(std::string name, TreeNodeType type, std::string path, StylusEditorManagementData data);

        Wt::Signal<std::string> folders_changed_; // emits selected file path

        void showPopup(const Wt::WMouseEvent& event);
        Wt::WContainerWidget* label_wrapper_;
        std::string path_;

        void dropEvent(Wt::WDropEvent event);
        
        TreeNodeType type_;
        StylusEditorManagementData data_;


    private:
        std::unique_ptr<Wt::WPopupMenu> popup_;

        void createNewFolderDialog();
        void createRenameFolderDialog();
        void createRemoveFolderMessageBox();

        void createNewFileDialog();
        void createRenameFileDialog();
        void deleteFileMessageBox();

        void copyFilePathToClipboard();
        std::string getNodeImportString();

    };



}