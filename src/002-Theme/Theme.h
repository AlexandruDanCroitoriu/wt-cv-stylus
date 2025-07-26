#pragma once
#include <unordered_map>
#include "004-Dbo/Session.h"

#include <Wt/WTheme.h>
#include <Wt/WCssTheme.h>

enum ThemeConfig
{
    Arctic,
    Modern,
    Pastel,
    News,
};


// enum WidgetThemeRole {
//   MenuItemIcon = 100,
//   MenuItemCheckBox = 101,
//   MenuItemClose = 102,
// 
//   DialogCoverWidget = 200,
//   DialogTitleBar = 201,
//   DialogBody = 202,
//   DialogFooter = 203,
//   DialogCloseIcon = 204,
//   DialogContent = 205, // For bootstrap 3 modal-content style
// 
//   TableViewRowContainer = 300,
// 
//   DatePickerPopup = 400,
//   DatePickerIcon = 401,
//   TimePickerPopup = 410,
//   TimePickerPopupContent = 411,

//   PanelTitleBar = 500,
//   PanelCollapseButton = 501,
//   PanelTitle = 502,
//   PanelBody = 503,
//   PanelBodyContent = 504,
// 
//   AuthWidgets = 600,
// 
//   InPlaceEditing = 700,
//   InPlaceEditingButtonsContainer = 701,
//   InPlaceEditingButton = 702,
// 
//   Navbar = 800,
//   NavCollapse = 801,
//   NavBrand = 802,
//   NavbarForm = 803,
//   NavbarSearchForm = 804,
//   NavbarSearchInput = 805,
//   NavbarMenu = 806,
//   NavbarBtn = 807,
//   NavbarAlignLeft = 808,
//   NavbarAlignRight = 809
// };

enum PenguinUiWidgetTheme {
    // widgets
    WComboBox = 10,
    WLineEdit = 11,

    // Buttons
    BtnDefault = 99,

    BtnPrimary = 100,
    BtnSecondary = 101,
    BtnSuccess = 102,
    BtnDanger = 103,
    BtnWarning = 104,
    BtnInfo = 105,
    BtnAlternate = 106,
    BtnInverse = 107,
    
    BtnPrimaryOutline = 108,
    BtnSecondaryOutline = 109,
    BtnSuccessOutline = 110,
    BtnDangerOutline = 111,
    BtnWarningOutline = 112,
    BtnInfoOutline = 113,
    BtnAlternateOutline = 114,
    BtnInverseOutline = 115,
    
    BtnPrimaryGhost = 116,
    BtnSecondaryGhost = 117,
    BtnSuccessGhost = 118,
    BtnDangerGhost = 119,
    BtnWarningGhost = 120,
    BtnInfoGhost = 121,
    BtnAlternateGhost = 122,
    BtnInverseGhost = 123,

    BtnPrimaryWithIcon = 124,
    BtnSecondaryWithIcon = 125,
    BtnSuccessWithIcon = 126,
    BtnDangerWithIcon = 127,
    BtnWarningWithIcon = 128,
    BtnInfoWithIcon = 129,
    BtnAlternateWithIcon = 130,
    BtnInverseWithIcon = 131,
    
    BtnPrimaryAction = 132,
    BtnSecondaryAction = 133,
    BtnSuccessAction = 134,
    BtnDangerAction = 135,
    BtnWarningAction = 136,
    BtnInfoAction = 137,
    BtnAlternateAction = 138,
    BtnInverseAction = 139,

    BtnPrimaryLoader = 140,
    BtnSecondaryLoader = 141,
    BtnSuccessLoader = 142,
    BtnDangerLoader = 143,
    BtnWarningLoader = 144,
    BtnInfoLoader = 145,
    BtnAlternateLoader = 146,
    BtnInverseLoader = 147,

    TableCell = 200,
    TableRow = 201,
    TableColumn = 202,

};

class Theme : public Wt::WTheme
{
public:
    Theme(Session& session, ThemeConfig theme_config = ThemeConfig::Arctic);

    void setWidgetThemeClasses(PenguinUiWidgetTheme widgetTheme, const std::string &styleClasses);
    void addWidgetThemeClasses(PenguinUiWidgetTheme widgetTheme, const std::string &styleClasses);

    mutable std::string current_tailwind_file_path_;

    virtual std::string name() const override;

    /*! \brief Returns the stylesheets.
     *
     * Returns wt.css, plus on IE wt_ie.css, plus on IE6 wt_ie6.css. The
     * style sheets are located in the theme directory in the resources
     * folder.
     */
    virtual std::vector<Wt::WLinkedCssStyleSheet> styleSheets() const override;

    virtual void apply(Wt::WWidget *widget, Wt::WWidget *child, int widgetRole) const override;
    virtual void apply(Wt::WWidget *widget, Wt::DomElement &element, int elementRole) const override;
    void applyTheme(Wt::WWidget *widget, PenguinUiWidgetTheme widgetTheme) const;

    /*! \brief Returns a generic CSS class name for a disabled element.
     *
     * The CSS class Wt-disabled is applied to disabled classes.
     */
    virtual std::string disabledClass() const override;

    /*! \brief Returns a generic CSS class name for an active element.
     *
     * The CSS class Wt-selected is applied to active classes.
     */
    virtual std::string activeClass() const override;

    virtual std::string utilityCssClass(int utilityCssClassRole) const override;

    /*! \brief Returns whether the theme allows for an anchor to be styled
     *         as a button.
     *
     * Returns false.
     */
    virtual bool canStyleAnchorAsButton() const override;

    virtual void applyValidationStyle(Wt::WWidget *widget,
                                      const Wt::WValidator::Result &validation,
                                      Wt::WFlags<Wt::ValidationStyleFlag> styles) const override;

    virtual bool canBorderBoxElement(const Wt::DomElement &element) const override;

    void setPenguinUiConfig();
    static std::string getThemeName(ThemeConfig theme_config);
    static ThemeConfig getThemeConfig(std::string theme_name);
    ThemeConfig current_theme_ = ThemeConfig::Arctic; // Default theme
private:
    std::unordered_map<int, std::string> widgetThemeClasses_;
    Session& session_;

};
