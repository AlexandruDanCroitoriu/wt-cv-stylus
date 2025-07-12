#pragma once
#include <Wt/WPushButton.h>
#include "002-Theme/Theme.h"


class Button : public Wt::WPushButton
{
public:
    Button(const std::string& text = "", std::string style_classes = "text-sm",  PenguinUiWidgetTheme widget_theme = PenguinUiWidgetTheme::BtnPrimary);
private:
};