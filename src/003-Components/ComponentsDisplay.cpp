#include "003-Components/ComponentsDisplay.h"

#include "002-Theme/Theme.h"
#include "003-Components/Button.h"
#include "003-Components/MonacoEditor.h"
#include "003-Components/VoiceRecorder.h"
#include "003-Components/BigWorkWidget.h"
#include "101-Examples/BroadcastExample.h"
#include "101-Examples/CheckboxBroadcastExample.h"

#include <Wt/WApplication.h>
#include <Wt/WTable.h>
#include <Wt/WTableCell.h>
#include <Wt/WComboBox.h>
#include <Wt/WBoxLayout.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WGridLayout.h>
#include <Wt/WPopupWidget.h>
#include <Wt/WTemplate.h>

ComponentsDisplay::ComponentsDisplay()
{
    setStyleClass("container h-[100%]");
    createBigWorkWidget();
    // createBroadcastExample();
    createCheckboxBroadcastExample();
    createVoiceRecorder();
    createMonacoEditor();
    createButtons();
}

void ComponentsDisplay::createVoiceRecorder()
{
    auto wrapper = addWidget(std::make_unique<Wt::WContainerWidget>());
    wrapper->addNew<VoiceRecorder>();
    // wrapper->addNew<VoiceRecorder>();
}

void ComponentsDisplay::createMonacoEditor()
{
    auto wrapper = addNew<Wt::WContainerWidget>();
    wrapper->setStyleClass("min-h-fit overflow-y-auto flex flex-col border border-outline bg-surface rounded-radius");

    auto header_wrapper = wrapper->addWidget(std::make_unique<Wt::WContainerWidget>());
    auto content_wrapper = wrapper->addWidget(std::make_unique<Wt::WContainerWidget>());

    header_wrapper->setStyleClass("flex items-center border-b border-outline");
    content_wrapper->setStyleClass("h-[70vh] lg:h-[30vh] flex flex-col lg:flex-row space-x-0.5 space-y-0.5 bg-surface-alt");

    auto css_editor = content_wrapper->addNew<MonacoEditor>("css");
    css_editor->setEditorText("../../static/stylus-resources/empty-file.css");
    css_editor->addStyleClass("w-full h-1/3 lg:w-1/3 lg:h-full");

    auto js_editor = content_wrapper->addNew<MonacoEditor>("javascript");
    js_editor->setEditorText("../../static/stylus-resources/empty-file.js");
    js_editor->addStyleClass("w-full h-1/3 lg:w-1/3 lg:h-full");

    auto html_editor = content_wrapper->addNew<MonacoEditor>("html");
    html_editor->setEditorText("../../static/stylus-resources/empty-file.html");
    html_editor->addStyleClass("w-full h-1/3 lg:w-1/3 lg:h-full");

    auto editor_dark_mode_btn = header_wrapper->addNew<Button>("light", "m-1.5 text-xs ", PenguinUiWidgetTheme::BtnAlternate);

    editor_dark_mode_btn->clicked().connect([=]() {
        bool is_dark = editor_dark_mode_btn->text().toUTF8().compare("dark") == 0;
        editor_dark_mode_btn->setText(is_dark ? "light" : "dark");
        MonacoEditor::setDarkTheme(is_dark);
    });

    auto line_wrap_btn = header_wrapper->addNew<Button>("toggle line wrap", "m-1.5 text-xs ", PenguinUiWidgetTheme::BtnAlternate);
    line_wrap_btn->clicked().connect([=]() {
        css_editor->toggleLineWrap();
        js_editor->toggleLineWrap();
        html_editor->toggleLineWrap();
    });

    auto mini_map_btn = header_wrapper->addNew<Button>("toggle mini map", "m-1.5 text-xs ", PenguinUiWidgetTheme::BtnAlternate);
    mini_map_btn->clicked().connect([=]() {
        css_editor->toggleMinimap();
        js_editor->toggleMinimap();
        html_editor->toggleMinimap();
    });

    auto info_tooltip = header_wrapper->addNew<Wt::WText>("i");
    dynamic_cast<Theme*>(wApp->theme().get())->applyTheme(info_tooltip, PenguinUiWidgetTheme::BtnDefault);
    info_tooltip->addStyleClass("ml-auto mr-2 text-sm !rounded-full");


    std::string tooltip_template = 
R"(<div class='bg-surface text-on-surface-strong p-2 rounded-radius border border-outline'>
    <div class='text-lg font-semibold mb-2 border-b'>Monaco Editor Instructions</div>
    <div class='text-sm mb-2'>Keyboard Shortcuts:</div>
    <ul class='list-disc pl-4'>
        <li>Alt + x - Toggle mini map</li>
        <li>Alt + z - Toggle line wrapping</li>
    </ul>
    <div class='text-sm mb-2'>How it works:</div>
    <ul class='list-disc pl-4 mb-2'>
        <li>The editor will grow to the parent size.</li>
        <li>Content is loaded by providing a file path.</li>
        <li>The path root is from the executable directory.</li>
        <li>The file content is loaded from the client by requesting the resource file.</li>
    </ul>
    <div class='text-sm mb-2'>Example usage:</div>
<pre class='bg-surface-alt p-2 rounded text-xs overflow-x-auto' id='monaco-example-code'><code class='language-cpp'>// Create the MonacoEditor widget for html, css, xml, js, ...
auto editor = parent->addNew&lt;MonacoEditor&gt;("html");
editor->setEditorText("../../static/stylus-resources/empty-file.html");
editor->toggleLineWrap(); // Toggle line wrapping
editor->toggleMinimap(); // Toggle mini map
MonacoEditor::setDarkTheme(true); // Dark theme is set for all editors so its a static member

// Automatically save the file when changes are available
editor->avalable_save().connect([=]() {
    if (editor->unsavedChanges())
        editor->SaveFile();
});
editor->setEditorReadOnly(false); // Set the editor to read only mode</code></pre>
    <button type='button' class='mt-2 px-3 py-1 rounded bg-primary text-white text-xs' onclick='
        const code = document.getElementById(&quot;monaco-example-code&quot;).innerText;
        navigator.clipboard.writeText(code);'>
            Copy to Clipboard
        </button>
</div>")";
    // info_tooltip->setToolTip(tooltip_template, Wt::TextFormat::UnsafeXHTML);
    info_tooltip->clicked().connect([=]() {
        Wt::WPopupWidget *popup = new Wt::WPopupWidget(std::make_unique<Wt::WTemplate>(tooltip_template));
        popup->setStyleClass("border divide-y divide-outline border-outline bg-surface rounded-radius shadow-2xl");
        popup->setMaximumSize(Wt::WLength(100, Wt::LengthUnit::ViewportWidth), Wt::WLength::Auto);
        popup->setAnchorWidget(info_tooltip, Wt::Orientation::Vertical);
        popup->setTransient(true);
        popup->show();
    });

}


void ComponentsDisplay::createButtons()
{
    auto table_wrapper = addNew<Wt::WContainerWidget>();
    table_wrapper->setStyleClass("relative overflow-x-auto my-4 max-w-full w-fit border border-outline rounded-radius");

    auto table_header_wrapper = table_wrapper->addNew<Wt::WContainerWidget>();
    table_header_wrapper->addStyleClass("flex items-center p-2 space-x-2 sticky left-0");
    
    auto size_combobox = table_header_wrapper->addNew<Wt::WComboBox>();
    size_combobox->addStyleClass("w-24");
    size_combobox->insertItem(ButtonSize::XS, "xs");
    size_combobox->insertItem(ButtonSize::SM, "sm");
    size_combobox->insertItem(ButtonSize::MD, "md");
    size_combobox->insertItem(ButtonSize::LG, "lg");
    size_combobox->insertItem(ButtonSize::XL, "xl");

    table_header_wrapper->addNew<Wt::WText>("Button examples")->setStyleClass("ml-2 text-lg font-semibold text-on-surface-strong font-title");

    auto table = table_wrapper->addNew<Wt::WTable>();
    
    table->setStyleClass("text-center text-on-surface border-outline ");
    table->rowAt(0)->setStyleClass("border-t border-outline bg-surface-alt text-on-surface-strong font-semibold");
    table->columnAt(0)->setStyleClass("bg-surface-alt");
    
    // set the header border and text styles
    table->elementAt(0, 0)->addNew<Wt::WText>("buttons")->setStyleClass("text-sm p-1 capitalize");

    table->elementAt(1, 0)->addStyleClass("border-r border-outline text-sm p-1 capitalize text-on-surface-strong font-semibold");
    table->elementAt(2, 0)->addStyleClass("border-r border-outline text-sm p-1 capitalize text-on-surface-strong font-semibold");
    table->elementAt(3, 0)->addStyleClass("border-r border-outline text-sm p-1 capitalize text-on-surface-strong font-semibold");
    table->elementAt(4, 0)->addStyleClass("border-r border-outline text-sm p-1 capitalize text-on-surface-strong font-semibold");
    table->elementAt(5, 0)->addStyleClass("border-r border-outline text-sm p-1 capitalize text-on-surface-strong font-semibold");
    table->elementAt(6, 0)->addStyleClass("border-r border-outline text-sm p-1 capitalize text-on-surface-strong font-semibold");
    table->elementAt(7, 0)->addStyleClass("border-r border-outline text-sm p-1 capitalize text-on-surface-strong font-semibold");
    table->elementAt(8, 0)->addStyleClass("border-r border-outline text-sm p-1 capitalize text-on-surface-strong font-semibold");

    table->elementAt(0, 1)->addStyleClass("border-b border-outline text-sm p-1 capitalize");
    table->elementAt(0, 2)->addStyleClass("border-b border-outline text-sm p-1 capitalize");
    table->elementAt(0, 3)->addStyleClass("border-b border-outline text-sm p-1 capitalize");
    table->elementAt(0, 4)->addStyleClass("border-b border-outline text-sm p-1 capitalize");
    table->elementAt(0, 5)->addStyleClass("border-b border-outline text-sm p-1 capitalize");
    table->elementAt(0, 6)->addStyleClass("border-b border-outline text-sm p-1 capitalize");

    table->elementAt(1, 0)->addNew<Wt::WText>("primary");
    table->elementAt(2, 0)->addNew<Wt::WText>("seccondary");
    table->elementAt(3, 0)->addNew<Wt::WText>("success");
    table->elementAt(4, 0)->addNew<Wt::WText>("danger");
    table->elementAt(5, 0)->addNew<Wt::WText>("warning");
    table->elementAt(6, 0)->addNew<Wt::WText>("info");
    table->elementAt(7, 0)->addNew<Wt::WText>("alternate");
    table->elementAt(8, 0)->addNew<Wt::WText>("inverse");

    table->elementAt(0, 1)->addNew<Wt::WText>("default");
    table->elementAt(0, 2)->addNew<Wt::WText>("outline");
    table->elementAt(0, 3)->addNew<Wt::WText>("ghost");
    table->elementAt(0, 4)->addNew<Wt::WText>("with icon");
    table->elementAt(0, 5)->addNew<Wt::WText>("action");
    table->elementAt(0, 6)->addNew<Wt::WText>("loader");



    // auto btn = 
    setCopyToClipboardAction(table->elementAt(1, 1)->addNew<Button>("default", "m-1.5", PenguinUiWidgetTheme::BtnPrimary), size_combobox, "->addNew<Button>(\"default\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnPrimary);");
    setCopyToClipboardAction(table->elementAt(2, 1)->addNew<Button>("default", "m-1.5", PenguinUiWidgetTheme::BtnSecondary), size_combobox, "->addNew<Button>(\"default\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnSecondary);");
    setCopyToClipboardAction(table->elementAt(3, 1)->addNew<Button>("default", "m-1.5", PenguinUiWidgetTheme::BtnSuccess), size_combobox, "->addNew<Button>(\"default\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnSuccess);");
    setCopyToClipboardAction(table->elementAt(4, 1)->addNew<Button>("default", "m-1.5", PenguinUiWidgetTheme::BtnDanger), size_combobox, "->addNew<Button>(\"default\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnDanger);");
    setCopyToClipboardAction(table->elementAt(5, 1)->addNew<Button>("default", "m-1.5", PenguinUiWidgetTheme::BtnWarning), size_combobox, "->addNew<Button>(\"default\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnWarning);");
    setCopyToClipboardAction(table->elementAt(6, 1)->addNew<Button>("default", "m-1.5", PenguinUiWidgetTheme::BtnInfo), size_combobox, "->addNew<Button>(\"default\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnInfo);");
    setCopyToClipboardAction(table->elementAt(7, 1)->addNew<Button>("default", "m-1.5", PenguinUiWidgetTheme::BtnAlternate), size_combobox, "->addNew<Button>(\"default\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnAlternate);");
    setCopyToClipboardAction(table->elementAt(8, 1)->addNew<Button>("default", "m-1.5", PenguinUiWidgetTheme::BtnInverse), size_combobox, "->addNew<Button>(\"default\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnInverse);");

    setCopyToClipboardAction(table->elementAt(1, 2)->addNew<Button>("outline", "m-1.5", PenguinUiWidgetTheme::BtnPrimaryOutline), size_combobox, "->addNew<Button>(\"outline\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnPrimaryOutline);");
    setCopyToClipboardAction(table->elementAt(2, 2)->addNew<Button>("outline", "m-1.5", PenguinUiWidgetTheme::BtnSecondaryOutline), size_combobox, "->addNew<Button>(\"outline\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnSecondaryOutline);");
    setCopyToClipboardAction(table->elementAt(3, 2)->addNew<Button>("outline", "m-1.5", PenguinUiWidgetTheme::BtnSuccessOutline), size_combobox, "->addNew<Button>(\"outline\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnSuccessOutline);");
    setCopyToClipboardAction(table->elementAt(4, 2)->addNew<Button>("outline", "m-1.5", PenguinUiWidgetTheme::BtnDangerOutline), size_combobox, "->addNew<Button>(\"outline\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnDangerOutline);");
    setCopyToClipboardAction(table->elementAt(5, 2)->addNew<Button>("outline", "m-1.5", PenguinUiWidgetTheme::BtnWarningOutline), size_combobox, "->addNew<Button>(\"outline\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnWarningOutline);");
    setCopyToClipboardAction(table->elementAt(6, 2)->addNew<Button>("outline", "m-1.5", PenguinUiWidgetTheme::BtnInfoOutline), size_combobox, "->addNew<Button>(\"outline\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnInfoOutline);");
    setCopyToClipboardAction(table->elementAt(7, 2)->addNew<Button>("outline", "m-1.5", PenguinUiWidgetTheme::BtnAlternateOutline), size_combobox, "->addNew<Button>(\"outline\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnAlternateOutline);");
    setCopyToClipboardAction(table->elementAt(8, 2)->addNew<Button>("outline", "m-1.5", PenguinUiWidgetTheme::BtnInverseOutline), size_combobox, "->addNew<Button>(\"outline\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnInverseOutline);");

    setCopyToClipboardAction(table->elementAt(1, 3)->addNew<Button>("ghost", "m-1.5", PenguinUiWidgetTheme::BtnPrimaryGhost), size_combobox, "->addNew<Button>(\"ghost\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnPrimaryGhost);");
    setCopyToClipboardAction(table->elementAt(2, 3)->addNew<Button>("ghost", "m-1.5", PenguinUiWidgetTheme::BtnSecondaryGhost), size_combobox, "->addNew<Button>(\"ghost\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnSecondaryGhost);");
    setCopyToClipboardAction(table->elementAt(3, 3)->addNew<Button>("ghost", "m-1.5", PenguinUiWidgetTheme::BtnSuccessGhost), size_combobox, "->addNew<Button>(\"ghost\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnSuccessGhost);");
    setCopyToClipboardAction(table->elementAt(4, 3)->addNew<Button>("ghost", "m-1.5", PenguinUiWidgetTheme::BtnDangerGhost), size_combobox, "->addNew<Button>(\"ghost\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnDangerGhost);");
    setCopyToClipboardAction(table->elementAt(5, 3)->addNew<Button>("ghost", "m-1.5", PenguinUiWidgetTheme::BtnWarningGhost), size_combobox, "->addNew<Button>(\"ghost\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnWarningGhost);");
    setCopyToClipboardAction(table->elementAt(6, 3)->addNew<Button>("ghost", "m-1.5", PenguinUiWidgetTheme::BtnInfoGhost), size_combobox, "->addNew<Button>(\"ghost\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnInfoGhost);");
    setCopyToClipboardAction(table->elementAt(7, 3)->addNew<Button>("ghost", "m-1.5", PenguinUiWidgetTheme::BtnAlternateGhost), size_combobox, "->addNew<Button>(\"ghost\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnAlternateGhost);");
    setCopyToClipboardAction(table->elementAt(8, 3)->addNew<Button>("ghost", "m-1.5", PenguinUiWidgetTheme::BtnInverseGhost), size_combobox, "->addNew<Button>(\"ghost\", \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnInverseGhost);");

    setCopyToClipboardAction(table->elementAt(1, 4)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:plus-primary").toUTF8() + "icon"), "m-1.5", PenguinUiWidgetTheme::BtnPrimaryWithIcon), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:plus-primary\").toUTF8() + \"icon\"), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnPrimaryWithIcon);");
    setCopyToClipboardAction(table->elementAt(2, 4)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:plus-seccondary").toUTF8() + "icon"), "m-1.5", PenguinUiWidgetTheme::BtnSecondaryWithIcon), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:plus-seccondary\").toUTF8() + \"icon\"), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnSecondaryWithIcon);");
    setCopyToClipboardAction(table->elementAt(3, 4)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:plus-success").toUTF8() + "icon"), "m-1.5", PenguinUiWidgetTheme::BtnSuccessWithIcon), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:plus-success\").toUTF8() + \"icon\"), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnSuccessWithIcon);");
    setCopyToClipboardAction(table->elementAt(4, 4)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:plus-danger").toUTF8() + "icon"), "m-1.5", PenguinUiWidgetTheme::BtnDangerWithIcon), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:plus-danger\").toUTF8() + \"icon\"), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnDangerWithIcon);");
    setCopyToClipboardAction(table->elementAt(5, 4)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:plus-warning").toUTF8() + "icon"), "m-1.5", PenguinUiWidgetTheme::BtnWarningWithIcon), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:plus-warning\").toUTF8() + \"icon\"), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnWarningWithIcon);");
    setCopyToClipboardAction(table->elementAt(6, 4)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:plus-info").toUTF8() + "icon"), "m-1.5", PenguinUiWidgetTheme::BtnInfoWithIcon), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:plus-info\").toUTF8() + \"icon\"), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnInfoWithIcon);");
    setCopyToClipboardAction(table->elementAt(7, 4)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:plus-alternate").toUTF8() + "icon"), "m-1.5", PenguinUiWidgetTheme::BtnAlternateWithIcon), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:plus-alternate\").toUTF8() + \"icon\"), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnAlternateWithIcon);");
    setCopyToClipboardAction(table->elementAt(8, 4)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:plus-inverse").toUTF8() + "icon"), "m-1.5", PenguinUiWidgetTheme::BtnInverseWithIcon), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:plus-inverse\").toUTF8() + \"icon\"), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnInverseWithIcon);");

    setCopyToClipboardAction(table->elementAt(1, 5)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:plus-primary").toUTF8()), "m-1.5", PenguinUiWidgetTheme::BtnPrimaryAction), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:plus-primary\").toUTF8()), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnPrimaryAction);");
    setCopyToClipboardAction(table->elementAt(2, 5)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:plus-seccondary").toUTF8()), "m-1.5", PenguinUiWidgetTheme::BtnSecondaryAction), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:plus-seccondary\").toUTF8()), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnSecondaryAction);");
    setCopyToClipboardAction(table->elementAt(3, 5)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:plus-success").toUTF8()), "m-1.5", PenguinUiWidgetTheme::BtnSuccessAction), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:plus-success\").toUTF8()), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnSuccessAction);");
    setCopyToClipboardAction(table->elementAt(4, 5)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:plus-danger").toUTF8()), "m-1.5", PenguinUiWidgetTheme::BtnDangerAction), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:plus-danger\").toUTF8()), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnDangerAction);");
    setCopyToClipboardAction(table->elementAt(5, 5)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:plus-warning").toUTF8()), "m-1.5", PenguinUiWidgetTheme::BtnWarningAction), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:plus-warning\").toUTF8()), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnWarningAction);");
    setCopyToClipboardAction(table->elementAt(6, 5)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:plus-info").toUTF8()), "m-1.5", PenguinUiWidgetTheme::BtnInfoAction), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:plus-info\").toUTF8()), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnInfoAction);");
    setCopyToClipboardAction(table->elementAt(7, 5)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:plus-alternate").toUTF8()), "m-1.5", PenguinUiWidgetTheme::BtnAlternateAction), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:plus-alternate\").toUTF8()), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnAlternateAction);");
    setCopyToClipboardAction(table->elementAt(8, 5)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:plus-inverse").toUTF8()), "m-1.5", PenguinUiWidgetTheme::BtnInverseAction), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:plus-inverse\").toUTF8()), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnInverseAction);");

    setCopyToClipboardAction(table->elementAt(1, 6)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:loader-primary").toUTF8() + "loader"), "m-1.5", PenguinUiWidgetTheme::BtnPrimaryLoader), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:loader-primary\").toUTF8() + \"loader\"), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnPrimaryLoader);");
    setCopyToClipboardAction(table->elementAt(2, 6)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:loader-seccondary").toUTF8() + "loader"), "m-1.5", PenguinUiWidgetTheme::BtnSecondaryLoader), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:loader-seccondary\").toUTF8() + \"loader\"), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnSecondaryLoader);");
    setCopyToClipboardAction(table->elementAt(3, 6)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:loader-success").toUTF8() + "loader"), "m-1.5", PenguinUiWidgetTheme::BtnSuccessLoader), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:loader-success\").toUTF8() + \"loader\"), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnSuccessLoader);");
    setCopyToClipboardAction(table->elementAt(4, 6)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:loader-danger").toUTF8() + "loader"), "m-1.5", PenguinUiWidgetTheme::BtnDangerLoader), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:loader-danger\").toUTF8() + \"loader\"), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnDangerLoader);");
    setCopyToClipboardAction(table->elementAt(5, 6)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:loader-warning").toUTF8() + "loader"), "m-1.5", PenguinUiWidgetTheme::BtnWarningLoader), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:loader-warning\").toUTF8() + \"loader\"), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnWarningLoader);");
    setCopyToClipboardAction(table->elementAt(6, 6)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:loader-info").toUTF8() + "loader"), "m-1.5", PenguinUiWidgetTheme::BtnInfoLoader), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:loader-info\").toUTF8() + \"loader\"), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnInfoLoader);");
    setCopyToClipboardAction(table->elementAt(7, 6)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:loader-alternate").toUTF8() + "loader"), "m-1.5", PenguinUiWidgetTheme::BtnAlternateLoader), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:loader-alternate\").toUTF8() + \"loader\"), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnAlternateLoader);");
    setCopyToClipboardAction(table->elementAt(8, 6)->addNew<Button>(std::string(Wt::WString::tr("penguin-ui-svg:loader-inverse").toUTF8() + "loader"), "m-1.5", PenguinUiWidgetTheme::BtnInverseLoader), size_combobox, "->addNew<Button>(std::string(Wt::WString::tr(\"penguin-ui-svg:loader-inverse\").toUTF8() + \"loader\"), \"m-1.5 text-", " \", PenguinUiWidgetTheme::BtnInverseLoader);");

    size_combobox->activated().connect([=](int index) {
        table->removeStyleClass("text-" + size_combobox->itemText(selected_size_).toUTF8(), true);
        table->addStyleClass("text-" + size_combobox->itemText(index).toUTF8());
        selected_size_ = ButtonSize(index);
    });

    // Set default size
    size_combobox->activated().emit(selected_size_);
}

void ComponentsDisplay::setCopyToClipboardAction(Wt::WInteractWidget  *widget, Wt::WComboBox* combo_box, const std::string &text_start, const std::string &text_end)
{
    widget->clicked().connect([=]() { 
        widget->doJavaScript("navigator.clipboard.writeText('"+text_start+combo_box->itemText(selected_size_).toUTF8()+text_end+"');"); 
    });
}

void ComponentsDisplay::createBigWorkWidget()
{
    auto wrapper = addWidget(std::make_unique<Wt::WContainerWidget>());
    wrapper->addStyleClass("mb-4");
    
    auto title = wrapper->addWidget(std::make_unique<Wt::WText>("Server Push & Background Processing Demo"));
    title->addStyleClass("text-xl font-bold mb-2 text-on-surface-strong block");
    
    auto description = wrapper->addWidget(std::make_unique<Wt::WText>(
        "Demonstrates real-time UI updates from background threads using Wt's server push functionality."));
    description->addStyleClass("text-sm text-on-surface mb-4 block");
    
    wrapper->addNew<BigWorkWidget>();
}

void ComponentsDisplay::createBroadcastExample()
{
    auto wrapper = addWidget(std::make_unique<Wt::WContainerWidget>());
    wrapper->addStyleClass("mb-4");
    wrapper->addNew<BroadcastExample>();
}

void ComponentsDisplay::createCheckboxBroadcastExample()
{
    auto wrapper = addWidget(std::make_unique<Wt::WContainerWidget>());
    wrapper->addStyleClass("mb-4");
    wrapper->addNew<CheckboxBroadcastExample>();
}

