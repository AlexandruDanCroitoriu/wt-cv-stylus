#include "002-Theme/Theme.h"
#include <Wt/WAbstractItemView.h>
#include <Wt/WAbstractSpinBox.h>
#include <Wt/WApplication.h>
#include <Wt/WCalendar.h>
#include <Wt/WCssTheme.h>
#include <Wt/WDateEdit.h>
#include <Wt/WDialog.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLinkedCssStyleSheet.h>
#include <Wt/WMessageResourceBundle.h>
#include <Wt/WPanel.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WProgressBar.h>
#include <Wt/WPushButton.h>
#include <Wt/WSuggestionPopup.h>
#include <Wt/WTabWidget.h>
#include <Wt/WTimeEdit.h>
#include <Wt/DomElement.h>
#include <Wt/WJavaScriptPreamble.h>
#include <Wt/WRandom.h>
#include <Wt/WTableCell.h>
#include <Wt/WTableColumn.h>
#include <Wt/WTableRow.h>
#include "001-App/App.h"

// /usr/local/include/Wt/WJavaScriptPreamble.h
// /home/alex/libs/wt-11-release/src/Wt/Chart/WCartesianChart.C
namespace skeletons
{
    const char *AuthCssTheme_xml = "default";
}

// namespace {
//   using namespace Wt;
//   WJavaScriptPreamble wtjs1() {
//     return WJavaScriptPreamble(WtClassScope, JavaScriptConstructor, "WCartesianChart", skeletons::AuthCssTheme_xml);
//   }
// }

Theme::Theme(Session& session, ThemeConfig theme_config)
    : Wt::WTheme(),
    // : Wt::WCssTheme("default"),
      session_(session)
{

    widgetThemeClasses_ = {
        {PenguinUiWidgetTheme::WComboBox, ""},
        {PenguinUiWidgetTheme::WLineEdit, ""},
        {PenguinUiWidgetTheme::BtnPrimary, ""},
        {PenguinUiWidgetTheme::BtnSecondary, ""},
        {PenguinUiWidgetTheme::BtnSuccess, ""},
        {PenguinUiWidgetTheme::BtnDanger, ""},
        {PenguinUiWidgetTheme::BtnWarning, ""},
        {PenguinUiWidgetTheme::BtnInfo, ""},
        {PenguinUiWidgetTheme::BtnAlternate, ""},
        {PenguinUiWidgetTheme::BtnInverse, ""},
        {PenguinUiWidgetTheme::BtnPrimaryOutline, ""},
        {PenguinUiWidgetTheme::BtnSecondaryOutline, ""},
        {PenguinUiWidgetTheme::BtnSuccessOutline, ""},
        {PenguinUiWidgetTheme::BtnDangerOutline, ""},
        {PenguinUiWidgetTheme::BtnWarningOutline, ""},
        {PenguinUiWidgetTheme::BtnInfoOutline, ""},
        {PenguinUiWidgetTheme::BtnAlternateOutline, ""},
        {PenguinUiWidgetTheme::BtnInverseOutline, ""},
        {PenguinUiWidgetTheme::BtnPrimaryGhost, ""},
        {PenguinUiWidgetTheme::BtnSecondaryGhost, ""},
        {PenguinUiWidgetTheme::BtnSuccessGhost, ""},
        {PenguinUiWidgetTheme::BtnDangerGhost, ""},
        {PenguinUiWidgetTheme::BtnWarningGhost, ""},
        {PenguinUiWidgetTheme::BtnInfoGhost, ""},
        {PenguinUiWidgetTheme::BtnAlternateGhost, ""},
        {PenguinUiWidgetTheme::BtnInverseGhost, ""},
        {PenguinUiWidgetTheme::BtnPrimaryWithIcon, ""},
        {PenguinUiWidgetTheme::BtnSecondaryWithIcon, ""},
        {PenguinUiWidgetTheme::BtnSuccessWithIcon, ""},
        {PenguinUiWidgetTheme::BtnDangerWithIcon, ""},
        {PenguinUiWidgetTheme::BtnWarningWithIcon, ""},
        {PenguinUiWidgetTheme::BtnInfoWithIcon, ""},
        {PenguinUiWidgetTheme::BtnAlternateWithIcon, ""},
        {PenguinUiWidgetTheme::BtnInverseWithIcon, ""},
        {PenguinUiWidgetTheme::BtnPrimaryAction, ""},
        {PenguinUiWidgetTheme::BtnSecondaryAction, ""},
        {PenguinUiWidgetTheme::BtnSuccessAction, ""},
        {PenguinUiWidgetTheme::BtnDangerAction, ""},
        {PenguinUiWidgetTheme::BtnWarningAction, ""},
        {PenguinUiWidgetTheme::BtnInfoAction, ""},
        {PenguinUiWidgetTheme::BtnAlternateAction, ""},
        {PenguinUiWidgetTheme::BtnInverseAction, ""},
        {PenguinUiWidgetTheme::BtnPrimaryLoader, ""},
        {PenguinUiWidgetTheme::BtnSecondaryLoader, ""},
        {PenguinUiWidgetTheme::BtnSuccessLoader, ""},
        {PenguinUiWidgetTheme::BtnDangerLoader, ""},
        {PenguinUiWidgetTheme::BtnWarningLoader, ""},
        {PenguinUiWidgetTheme::BtnInfoLoader, ""},
        {PenguinUiWidgetTheme::BtnAlternateLoader, ""},
        {PenguinUiWidgetTheme::BtnInverseLoader, ""},
        {PenguinUiWidgetTheme::TableCell, ""},
        {PenguinUiWidgetTheme::TableRow, ""},
        {PenguinUiWidgetTheme::TableColumn, ""}
    };


    wApp->setHtmlAttribute("data-theme", getThemeName(theme_config));

    dynamic_cast<App*>(wApp)->theme_changed_.connect(this, [=](ThemeConfig theme_config) {
        current_theme_ = theme_config;
    });

    session_.login().changed().connect([=]() {
      if(!session_.login().loggedIn()) return;

      Wt::Dbo::Transaction transaction(session_);
      auto user = session_.user(session_.login().user());
      if (user) {
        wApp->setHtmlClass(user->ui_dark_mode_ ? "dark" : "");
        wApp->setHtmlAttribute("data-theme", user->ui_penguin_theme_name_);
        dynamic_cast<App*>(wApp)->dark_mode_changed_.emit(user->ui_dark_mode_);
        dynamic_cast<App*>(wApp)->theme_changed_.emit(getThemeConfig(user->ui_penguin_theme_name_));
      }
      transaction.commit();
    });
}

void Theme::setWidgetThemeClasses(PenguinUiWidgetTheme widgetTheme, const std::string &styleClasses)
{
    widgetThemeClasses_[widgetTheme] = styleClasses;
}
void Theme::addWidgetThemeClasses(PenguinUiWidgetTheme widgetTheme, const std::string &styleClasses)
{
    auto &currentClasses = widgetThemeClasses_[widgetTheme];
    if (!currentClasses.empty())
        currentClasses += " ";
    currentClasses += styleClasses;
}

std::vector<Wt::WLinkedCssStyleSheet> Theme::styleSheets() const
{
    std::vector<Wt::WLinkedCssStyleSheet> result;
    std::string themeDir = resourcesUrl();

    current_tailwind_file_path_ = "static/tailwind.css?" + Wt::WRandom::generateId();

    result.push_back(Wt::WLinkedCssStyleSheet(Wt::WLink(current_tailwind_file_path_)));
    result.push_back(Wt::WLinkedCssStyleSheet(Wt::WLink(themeDir + "wt.css")));

    if (wApp->environment().agentIsIElt(9))
      result.push_back(Wt::WLinkedCssStyleSheet(Wt::WLink(themeDir + "wt_ie.css")));

    if (wApp->environment().agent() == Wt::UserAgent::IE6)
      result.push_back(Wt::WLinkedCssStyleSheet(Wt::WLink(themeDir + "wt_ie6.css")));

    return result;
}

void Theme::apply(Wt::WWidget *widget, Wt::WWidget *child, int widgetRole) const
{
    if (!widget->isThemeStyleEnabled())
        return;

    switch (widgetRole)
    {
    // case WidgetThemeRoleRest::WComboBox:
        // child->addStyleClass("");
    case Wt::WidgetThemeRole::MenuItemIcon:
        child->addStyleClass("Wt-icon");
        break;
    case Wt::WidgetThemeRole::MenuItemCheckBox:
        child->addStyleClass("Wt-chkbox");
        break;
    case Wt::WidgetThemeRole::MenuItemClose:
        widget->addStyleClass("Wt-closable");
        child->addStyleClass("closeicon");
        break;

    case Wt::WidgetThemeRole::DialogCoverWidget:
        child->setStyleClass("Wt-dialogcover in");
        break;
    case Wt::WidgetThemeRole::DialogTitleBar:
        child->addStyleClass("rounded-radius cursor-move text-md font-semibold text-on-surface-strong p-2 border-b border-outline bg-primary/40 text-center");
        break;
    case Wt::WidgetThemeRole::DialogBody:
        child->addStyleClass("body");
        break;
    case Wt::WidgetThemeRole::DialogFooter:
        child->addStyleClass("footer");
        break;
    case Wt::WidgetThemeRole::DialogCloseIcon:
        child->addStyleClass("closeicon");
        break;

    case Wt::WidgetThemeRole::TableViewRowContainer:
    {
        Wt::WAbstractItemView *view = dynamic_cast<Wt::WAbstractItemView *>(widget);

        std::string backgroundImage;

        if (view->alternatingRowColors())
            backgroundImage = "stripes/stripe-";
        else
            backgroundImage = "no-stripes/no-stripe-";

        backgroundImage = resourcesUrl() + backgroundImage + std::to_string(static_cast<int>(view->rowHeight().toPixels())) + "px.gif";

        child->decorationStyle().setBackgroundImage(Wt::WLink(backgroundImage));

        break;
    }

    case Wt::WidgetThemeRole::DatePickerPopup:
        child->addStyleClass("Wt-datepicker");
        break;
    case Wt::WidgetThemeRole::DatePickerIcon:
    {
        auto icon = dynamic_cast<Wt::WImage *>(child);
        icon->setImageLink(Wt::WApplication::relativeResourcesUrl() + "date.gif");
        icon->setVerticalAlignment(Wt::AlignmentFlag::Middle);
        icon->resize(16, 16);
        break;
    }

    case Wt::WidgetThemeRole::PanelTitleBar:
        child->addStyleClass("titlebar");
        break;
    case Wt::WidgetThemeRole::PanelBody:
        child->addStyleClass("body");
        break;
    case Wt::WidgetThemeRole::PanelCollapseButton:
        child->setFloatSide(Wt::Side::Left);
        break;

    case Wt::WidgetThemeRole::AuthWidgets:
        // Wt::WApplication *app = Wt::WApplication::instance();
        // app->useStyleSheet(Wt::WApplication::relativeResourcesUrl() + "form.css");
        // app->builtinLocalizedStrings().useBuiltin(skeletons::AuthCssTheme_xml);
        break;
    }
}


void Theme::apply(Wt::WWidget *widget, Wt::DomElement& element, int elementRole) const
{
  bool creating = element.mode() == Wt::DomElement::Mode::Create;

  if (!widget->isThemeStyleEnabled())
    return;

  {
    Wt::WPopupWidget *popup = dynamic_cast<Wt::WPopupWidget *>(widget);
    if (popup)
      element.addPropertyWord(Wt::Property::Class, "Wt-outset");
  }

  switch (element.type()) {
  case Wt::DomElementType::BUTTON:
    if (creating) {
      element.addPropertyWord(Wt::Property::Class, "Wt-btn");
      Wt::WPushButton *b = dynamic_cast<Wt::WPushButton *>(widget);
      if (b) {  
        element.addPropertyWord(Wt::Property::Class, widgetThemeClasses_.at(PenguinUiWidgetTheme::BtnDefault));
        if (b->isDefault())
          element.addPropertyWord(Wt::Property::Class, "Wt-btn-default");

        if (!b->text().empty())
          element.addPropertyWord(Wt::Property::Class, "with-label");
      }
    }
    break;

  case Wt::DomElementType::UL:
    if (dynamic_cast<Wt::WPopupMenu *>(widget))
      element.addPropertyWord(Wt::Property::Class, "Wt-popupmenu Wt-outset");
    else {
      Wt::WTabWidget *tabs
        = dynamic_cast<Wt::WTabWidget *>(widget->parent()->parent());

      if (tabs)
        element.addPropertyWord(Wt::Property::Class, "Wt-tabs");
      else {
        Wt::WSuggestionPopup *suggestions
          = dynamic_cast<Wt::WSuggestionPopup *>(widget);

        if (suggestions)
          element.addPropertyWord(Wt::Property::Class, "Wt-suggest");
      }
    }
    break;

  case Wt::DomElementType::LI:
    {
      Wt::WMenuItem *item = dynamic_cast<Wt::WMenuItem *>(widget);
      if (item) {
        if (item->isSeparator())
          element.addPropertyWord(Wt::Property::Class, "Wt-separator");
           if (item->isSectionHeader())
          element.addPropertyWord(Wt::Property::Class, "Wt-sectheader");
        if (item->menu())
          element.addPropertyWord(Wt::Property::Class, "submenu");
      }
    }
    break;

  case Wt::DomElementType::DIV:
    {
      Wt::WDialog *dialog = dynamic_cast<Wt::WDialog *>(widget);
      if (dialog) {
        element.addPropertyWord(Wt::Property::Class, "rounded-radius border-outline bg-surface-alt text-on-surface-alt");
        return;
      }

      Wt::WPanel *panel = dynamic_cast<Wt::WPanel *>(widget);
      if (panel) {
        element.addPropertyWord(Wt::Property::Class, "Wt-panel Wt-outset");
        return;
      }

      Wt::WProgressBar *bar = dynamic_cast<Wt::WProgressBar *>(widget);
      if (bar) {
        switch (elementRole) {
        case Wt::MainElement:
          element.addPropertyWord(Wt::Property::Class, "Wt-progressbar");
          break;
        case Wt::ProgressBarBar:
          element.addPropertyWord(Wt::Property::Class, "Wt-pgb-bar");
          break;
        case Wt::ProgressBarLabel:
          element.addPropertyWord(Wt::Property::Class, "Wt-pgb-label");
        }
        return;
      }
    }

    break;

  case Wt::DomElementType::INPUT:
    {
      Wt::WAbstractSpinBox *spinBox = dynamic_cast<Wt::WAbstractSpinBox *>(widget);
      if (spinBox) {
        element.addPropertyWord(Wt::Property::Class, "Wt-spinbox");
        return;
      }

      Wt::WDateEdit *dateEdit = dynamic_cast<Wt::WDateEdit *>(widget);
      if (dateEdit) {
        element.addPropertyWord(Wt::Property::Class, "Wt-dateedit");
        return;
      }

      Wt::WTimeEdit *timeEdit = dynamic_cast<Wt::WTimeEdit *>(widget);
      if (timeEdit) {
        element.addPropertyWord(Wt::Property::Class, "Wt-timeedit");
        return;
      }
    }
    break;
  default:
    break;
  }
  // Added extra from default configuration
  Wt::WLineEdit *lineEdit = dynamic_cast<Wt::WLineEdit *>(widget);
  if (lineEdit) {
    element.addPropertyWord(Wt::Property::Class, widgetThemeClasses_.at(PenguinUiWidgetTheme::WLineEdit));
  }
  Wt::WComboBox *comboBox = dynamic_cast<Wt::WComboBox *>(widget);
  if (comboBox) {
    element.addPropertyWord(Wt::Property::Class, widgetThemeClasses_.at(PenguinUiWidgetTheme::WComboBox));
  }

  // if(dynamic_cast<Wt::WTableColumn *>(widget)) {
  //   element.addPropertyWord(Wt::Property::Class, "Wt-table-column");
  // }else if(dynamic_cast<Wt::WTableRow *>(widget)) {
  //   element.addPropertyWord(Wt::Property::Class, "Wt-table-row");
  // }else if(dynamic_cast<Wt::WTableCell *>(widget)) {
  //   element.addPropertyWord(Wt::Property::Class, widgetThemeClasses_.at(PenguinUiWidgetTheme::TableCell));
  // }
  // if(element.type() == Wt::DomElementType::TD){
  //   if (dynamic_cast<Wt::WTableColumn *>(widget)) {
  //     element.addPropertyWord(Wt::Property::Class, widgetThemeClasses_.at(PenguinUiWidgetTheme::TableColumn));
  //   }else if(dynamic_cast<Wt::WTableRow *>(widget)) {
  //     element.addPropertyWord(Wt::Property::Class, widgetThemeClasses_.at(PenguinUiWidgetTheme::TableRow));
  //   }else if(dynamic_cast<Wt::WTableCell *>(widget)) {
  //     element.addPropertyWord(Wt::Property::Class, widgetThemeClasses_.at(PenguinUiWidgetTheme::TableCell));
  //   }
  // }
  // end of custom added code

}
void Theme::applyTheme(Wt::WWidget *widget, PenguinUiWidgetTheme widgetTheme) const
{
   widget->addStyleClass(widgetThemeClasses_.at(widgetTheme));
}


std::string Theme::disabledClass() const
{
  return "Wt-disabled";
}

std::string Theme::activeClass() const
{
  return "Wt-selected";
}

std::string Theme::utilityCssClass(int utilityCssClassRole) const
{
  switch (utilityCssClassRole) {
  case Wt::ToolTipOuter:
    return "Wt-tooltip";
  default:
    return "";
  }
}

std::string Theme::name() const
{
  return "default";
}

bool Theme::canStyleAnchorAsButton() const
{
  return false;
}

void Theme::applyValidationStyle(Wt::WWidget *widget,
                                     const Wt::WValidator::Result& validation,
                                     Wt::WFlags<Wt::ValidationStyleFlag> styles) const
{
//   Wt::WApplication *app = Wt::WApplication::instance();

//   LOAD_JAVASCRIPT(app, "js/CssThemeValidate.js", "validate", wtjs1);
//   LOAD_JAVASCRIPT(app, "js/CssThemeValidate.js", "setValidationState", wtjs2);

//   if (app->environment().ajax()) {
//     Wt::WStringStream js;
//     js << WT_CLASS ".setValidationState(" << widget->jsRef() << ","
//        << (validation.state() == Wt::ValidationState::Valid) << ","
//        << validation.message().jsStringLiteral() << ","
//        << styles.value() << ");";

//     widget->doJavaScript(js.str());
//   } else {
//     bool validStyle
//       = (validation.state() == Wt::ValidationState::Valid) &&
//       styles.test(Wt::ValidationStyleFlag::ValidStyle);
//     bool invalidStyle
//       = (validation.state() != Wt::ValidationState::Valid) &&
//       styles.test(Wt::ValidationStyleFlag::InvalidStyle);

//     widget->toggleStyleClass("Wt-valid", validStyle);
//     widget->toggleStyleClass("Wt-invalid", invalidStyle);
//   }
}

bool Theme::canBorderBoxElement(WT_MAYBE_UNUSED const Wt::DomElement& element) const
{
  return true;

}

void Theme::setPenguinUiConfig()
{

    setWidgetThemeClasses(PenguinUiWidgetTheme::WComboBox, "appearance-none rounded-radius border border-outline bg-surface-alt px-4 py-2 text-sm focus-visible:outline-2 text-on-surface focus-visible:outline-offset-2 focus-visible:outline-primary disabled:cursor-not-allowed disabled:opacity-75");
    setWidgetThemeClasses(PenguinUiWidgetTheme::WLineEdit, "w-full rounded-radius border border-outline bg-surface-alt px-2 py-2 text-sm focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-primary disabled:cursor-not-allowed disabled:opacity-75");


    // setWidgetThemeClasses(PenguinUiWidgetTheme::TableCell, "");
    // setWidgetThemeClasses(PenguinUiWidgetTheme::TableRow, "");
    // setWidgetThemeClasses(PenguinUiWidgetTheme::TableColumn, "");

    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnDefault, "font-paragraph cursor-pointer inline-flex items-center justify-center border whitespace-nowrap px-4 py-2 font-medium text-center transition tracking-whide disabled:cursor-not-allowed focus-visible:outline-2 focus-visible:outline-offset-2 active:opacity-100 disabled:opacity-75");

    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnPrimary, "rounded-radius bg-primary border-primary text-on-primary hover:opacity-75 focus-visible:outline-primary active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnSecondary, "rounded-radius bg-secondary border-secondary text-on-secondary hover:opacity-75 focus-visible:outline-secondary active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnAlternate, "rounded-radius bg-surface-alt border-surface-alt text-on-surface-strong hover:opacity-75 focus-visible:outline-surface-alt active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnInverse, "rounded-radius hover:opacity-75 active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnInfo, "rounded-radius bg-info border-info text-on-info hover:opacity-75 focus-visible:outline-info active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnDanger, "rounded-radius bg-danger border-danger text-on-danger hover:opacity-75 focus-visible:outline-danger active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnWarning, "rounded-radius bg-warning border-warning text-on-warning hover:opacity-75 focus-visible:outline-warning active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnSuccess, "rounded-radius bg-success border-success text-on-success hover:opacity-75 focus-visible:outline-success active:outline-offset-0");

    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnPrimaryOutline, "bg-transparent rounded-radius border-primary text-primary hover:opacity-75 focus-visible:outline-primary active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnSecondaryOutline, "bg-transparent rounded-radius border-secondary text-secondary hover:opacity-75 focus-visible:outline-secondary active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnAlternateOutline, "bg-transparent rounded-radius border-outline text-outline hover:opacity-75 focus-visible:outline-outline active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnInfoOutline, "bg-transparent rounded-radius border-info text-info hover:opacity-75 focus-visible:outline-info active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnDangerOutline, "bg-transparent rounded-radius border-danger text-danger hover:opacity-75 focus-visible:outline-danger active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnWarningOutline, "bg-transparent rounded-radius border-warning text-warning hover:opacity-75 focus-visible:outline-warning active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnSuccessOutline, "bg-transparent rounded-radius border-success text-success hover:opacity-75 focus-visible:outline-success active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnInverseOutline, "bg-transparent rounded-radius hover:opacity-75 active:outline-offset-0");

    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnPrimaryGhost, "border-none bg-transparent rounded-radius text-primary hover:opacity-75 focus-visible:outline-primary active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnSecondaryGhost, "border-none bg-transparent rounded-radius text-secondary hover:opacity-75 focus-visible:outline-secondary active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnAlternateGhost, "border-none bg-transparent rounded-radius text-outline hover:opacity-75 focus-visible:outline-outline active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnInverseGhost, "border-none bg-transparent rounded-radius hover:opacity-75 active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnInfoGhost, "border-none bg-transparent rounded-radius text-info hover:opacity-75 focus-visible:outline-info active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnDangerGhost, "border-none bg-transparent rounded-radius text-danger hover:opacity-75 focus-visible:outline-danger active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnWarningGhost, "border-none bg-transparent rounded-radius text-warning hover:opacity-75 focus-visible:outline-warning active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnSuccessGhost, "border-none bg-transparent rounded-radius text-success hover:opacity-75 focus-visible:outline-success active:outline-offset-0");

    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnPrimaryWithIcon, "gap-2 rounded-radius bg-primary border-primary text-on-primary hover:opacity-75 focus-visible:outline focus-visible:outline-primary active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnSecondaryWithIcon, "gap-2 rounded-radius bg-secondary border-secondary text-on-secondary hover:opacity-75 focus-visible:outline focus-visible:outline-secondary active:outline-offset-0 ");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnAlternateWithIcon, "gap-2 rounded-radius bg-surface-alt border-surface-alt text-on-surface-strong hover:opacity-75 focus-visible:outline focus-visible:outline-surface-alt active:outline-offset-0-strong");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnInverseWithIcon, "gap-2 rounded-radius hover:opacity-75 focus-visible:outline active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnInfoWithIcon, "gap-2 rounded-radius bg-info border-info text-on-info hover:opacity-75 focus-visible:outline focus-visible:outline-info active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnDangerWithIcon, "gap-2 rounded-radius bg-danger border-danger text-on-danger hover:opacity-75 focus-visible:outline focus-visible:outline-danger active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnWarningWithIcon, "gap-2 rounded-radius bg-warning border-warning text-on-warning hover:opacity-75 focus-visible:outline focus-visible:outline-warning active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnSuccessWithIcon, "gap-2 rounded-radius bg-success border-success text-on-success hover:opacity-75 focus-visible:outline focus-visible:outline-success active:outline-offset-0");

    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnPrimaryAction, "aspect-square !p-2 rounded-full border-primary bg-primary p-2 text-on-primary hover:opacity-75 focus-visible:outline-primary active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnSecondaryAction, "aspect-square !p-2 rounded-full border-secondary bg-secondary p-2 text-on-secondary hover:opacity-75 focus-visible:outline-secondary active:outline-offset-0 ");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnAlternateAction, "aspect-square !p-2 rounded-full border-surface-alt bg-surface-alt p-2 text-on-surface-strong hover:opacity-75 focus-visible:outline-surface-alt active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnInverseAction, "aspect-square !p-2 rounded-full p-2 hover:opacity-75 active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnInfoAction, "aspect-square !p-2 rounded-full border-info bg-info p-2 text-on-info hover:opacity-75 focus-visible:outline-info active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnDangerAction, "aspect-square !p-2 rounded-full border-danger bg-danger p-2 text-on-danger hover:opacity-75 focus-visible:outline-danger active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnWarningAction, "aspect-square !p-2 rounded-full border-warning bg-warning p-2 text-on-warning hover:opacity-75 focus-visible:outline-warning active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnSuccessAction, "aspect-square !p-2 rounded-full border-success bg-success p-2 text-on-success hover:opacity-75 focus-visible:outline-success active:outline-offset-0");

    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnPrimaryLoader, "gap-2 rounded-radius bg-primary border-primary text-on-primary hover:opacity-75 focus-visible:outline-primary active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnSecondaryLoader, "gap-2 rounded-radius bg-secondary border-secondary text-on-secondary hover:opacity-75 focus-visible:outline-secondary active:outline-offset-0 ");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnAlternateLoader, "gap-2 rounded-radius bg-surface-alt border-surface-alt text-on-surface-strong hover:opacity-75 focus-visible:outline-surface-alt active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnInverseLoader, "gap-2 rounded-radius hover:opacity-75 active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnInfoLoader, "gap-2 rounded-radius bg-info border-info text-on-info hover:opacity-75 focus-visible:outline-info active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnDangerLoader, "gap-2 rounded-radius bg-danger border-danger text-on-danger hover:opacity-75 focus-visible:outline-danger active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnWarningLoader, "gap-2 rounded-radius bg-warning border-warning text-on-warning hover:opacity-75 focus-visible:outline-warning active:outline-offset-0");
    setWidgetThemeClasses(PenguinUiWidgetTheme::BtnSuccessLoader, "gap-2 rounded-radius bg-success border-success text-on-success hover:opacity-75 focus-visible:outline-success active:outline-offset-0");


}


std::string Theme::getThemeName(ThemeConfig theme_config)
{
    switch (theme_config)
    {
    case ThemeConfig::Arctic:
        return "arctic";
    case ThemeConfig::Modern:
        return "modern";
    case ThemeConfig::Pastel:
        return "pastel";
    case ThemeConfig::News:
        return "news";
    default:
        return "arctic"; // Default theme if none matches
    }
}

ThemeConfig Theme::getThemeConfig(std::string theme_name)
{
    if (theme_name == "arctic")
        return ThemeConfig::Arctic;
    else if (theme_name == "modern")
        return ThemeConfig::Modern;
    else if (theme_name == "pastel")
        return ThemeConfig::Pastel;
    else if (theme_name == "news")
        return ThemeConfig::News;

    // Default theme
    return ThemeConfig::Arctic;
}

