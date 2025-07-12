#include "003-Components/Button.h"
#include <Wt/WApplication.h>

Button::Button(const std::string& text, std::string style_classes, PenguinUiWidgetTheme widget_theme)
    : Wt::WPushButton(text, Wt::TextFormat::UnsafeXHTML)
{
    dynamic_cast<Theme *>(wApp->theme().get())->applyTheme(this, widget_theme);
    addStyleClass(style_classes);
        
}