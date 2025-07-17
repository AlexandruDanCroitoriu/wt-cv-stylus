# Widget Reference Guide

This comprehensive guide covers all major Wt widgets with practical examples and usage patterns. Widgets are the building blocks of Wt applications, forming a hierarchical tree structure that renders to HTML/JavaScript.

## Table of Contents

1. [Basic Widgets](#basic-widgets)
2. [Input Widgets](#input-widgets)
3. [Container Widgets](#container-widgets)
4. [Layout Widgets](#layout-widgets)
5. [Advanced Widgets](#advanced-widgets)
6. [Custom Widgets](#custom-widgets)
7. [Widget Properties](#widget-properties)

## Basic Widgets

### WText - Text Display

The most fundamental widget for displaying text content.

```cpp
#include <Wt/WText.h>
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>

// Basic text
auto text = std::make_unique<Wt::WText>("Hello, World!");

// HTML content (with proper escaping)
auto htmlText = std::make_unique<Wt::WText>(
    "<h2>Welcome</h2><p>This is <em>emphasized</em> text.</p>");
htmlText->setTextFormat(Wt::TextFormat::XHTML);

// Rich text with formatting
auto richText = std::make_unique<Wt::WText>();
richText->setText("This text contains <strong>bold</strong> and <em>italic</em> formatting.");
richText->setTextFormat(Wt::TextFormat::XHTML);

// Dynamic text updates
auto dynamicText = std::make_unique<Wt::WText>("Click count: 0");
int clickCount = 0;

auto button = std::make_unique<Wt::WPushButton>("Click me");
button->clicked().connect([&dynamicText, &clickCount]() {
    clickCount++;
    dynamicText->setText("Click count: " + std::to_string(clickCount));
});
```

**Key Properties:**
- `setText()` - Set text content
- `setTextFormat()` - Plain, XHTML, or UnsafeXHTML
- `setWordWrap()` - Enable/disable word wrapping
- `setInline()` - Inline vs block display

### WImage - Image Display

Display images with various sources and properties.

```cpp
#include <Wt/WImage.h>
#include <Wt/WLink.h>

// Static image from URL
auto image = std::make_unique<Wt::WImage>("https://example.com/image.png");

// Local resource
auto localImage = std::make_unique<Wt::WImage>(Wt::WLink("images/logo.png"));

// Image with alt text and dimensions
auto styledImage = std::make_unique<Wt::WImage>("images/photo.jpg");
styledImage->setAlternateText("A beautiful landscape");
styledImage->resize(300, 200);

// Responsive image
auto responsiveImage = std::make_unique<Wt::WImage>("images/banner.jpg");
responsiveImage->setMaximumSize(Wt::WLength::Auto, 400);
responsiveImage->addStyleClass("img-responsive");

// Image with click handler
auto clickableImage = std::make_unique<Wt::WImage>("images/thumbnail.jpg");
clickableImage->clicked().connect([clickableImage = clickableImage.get()]() {
    clickableImage->setImageLink(Wt::WLink("images/fullsize.jpg"));
});
```

### WPushButton - Interactive Button

Buttons for user interactions and form submissions.

```cpp
#include <Wt/WPushButton.h>
#include <Wt/WMessageBox.h>

// Basic button
auto button = std::make_unique<Wt::WPushButton>("Click Me");

// Button with icon
auto iconButton = std::make_unique<Wt::WPushButton>();
iconButton->setIcon("icons/save.png");
iconButton->setText("Save");

// Styled button
auto primaryButton = std::make_unique<Wt::WPushButton>("Primary Action");
primaryButton->addStyleClass("btn btn-primary");

// Button with click handler
auto actionButton = std::make_unique<Wt::WPushButton>("Show Alert");
actionButton->clicked().connect([this]() {
    auto messageBox = std::make_unique<Wt::WMessageBox>(
        "Information", 
        "Button was clicked!", 
        Wt::Icon::Information, 
        Wt::StandardButton::Ok);
    messageBox->show();
});

// Submit button for forms
auto submitButton = std::make_unique<Wt::WPushButton>("Submit");
submitButton->setAttributeValue("type", "submit");

// Disabled button
auto disabledButton = std::make_unique<Wt::WPushButton>("Disabled");
disabledButton->setEnabled(false);
```

## Input Widgets

### WLineEdit - Single Line Text Input

Text input for short strings and data entry.

```cpp
#include <Wt/WLineEdit.h>
#include <Wt/WValidator.h>
#include <Wt/WRegExpValidator.h>

// Basic text input
auto nameInput = std::make_unique<Wt::WLineEdit>();
nameInput->setPlaceholderText("Enter your name");

// Password input
auto passwordInput = std::make_unique<Wt::WLineEdit>();
passwordInput->setEchoMode(Wt::EchoMode::Password);
passwordInput->setPlaceholderText("Password");

// Email input with validation
auto emailInput = std::make_unique<Wt::WLineEdit>();
emailInput->setPlaceholderText("email@example.com");
auto emailValidator = std::make_shared<Wt::WRegExpValidator>(
    "[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}");
emailValidator->setMandatory(true);
emailInput->setValidator(emailValidator);

// Number input
auto ageInput = std::make_unique<Wt::WLineEdit>();
ageInput->setPlaceholderText("Age");
auto ageValidator = std::make_shared<Wt::WIntValidator>(1, 120);
ageInput->setValidator(ageValidator);

// Input with change handler
auto searchInput = std::make_unique<Wt::WLineEdit>();
searchInput->setPlaceholderText("Search...");
searchInput->keyPressed().connect([searchInput = searchInput.get()](const Wt::WKeyEvent& event) {
    if (event.key() == Wt::Key::Enter) {
        std::string query = searchInput->text().toUTF8();
        // Perform search
        performSearch(query);
    }
});

// Real-time validation
auto validatedInput = std::make_unique<Wt::WLineEdit>();
validatedInput->changed().connect([validatedInput = validatedInput.get()]() {
    if (validatedInput->validate() == Wt::ValidationState::Valid) {
        validatedInput->removeStyleClass("is-invalid");
        validatedInput->addStyleClass("is-valid");
    } else {
        validatedInput->removeStyleClass("is-valid");
        validatedInput->addStyleClass("is-invalid");
    }
});
```

### WTextArea - Multi-line Text Input

Text area for longer text content and descriptions.

```cpp
#include <Wt/WTextArea.h>

// Basic text area
auto description = std::make_unique<Wt::WTextArea>();
description->setPlaceholderText("Enter description...");
description->setRows(5);
description->setColumns(50);

// Rich text editor (requires configuration)
auto richEditor = std::make_unique<Wt::WTextArea>();
richEditor->setExtraPlugins("table,contextmenu");
richEditor->setToolBar(0, "Bold,Italic,Underline,|,NumberedList,BulletedList");

// Auto-resizing text area
auto autoTextArea = std::make_unique<Wt::WTextArea>();
autoTextArea->resize(Wt::WLength::Auto, 100);
autoTextArea->textInput().connect([autoTextArea = autoTextArea.get()]() {
    // Auto-resize based on content
    int lines = std::count(autoTextArea->text().begin(), 
                          autoTextArea->text().end(), '\n') + 1;
    autoTextArea->setRows(std::max(3, std::min(lines, 20)));
});
```

### WComboBox - Dropdown Selection

Dropdown lists for single-choice selections.

```cpp
#include <Wt/WComboBox.h>

// Basic combo box
auto countryCombo = std::make_unique<Wt::WComboBox>();
countryCombo->addItem("United States");
countryCombo->addItem("Canada");
countryCombo->addItem("United Kingdom");
countryCombo->addItem("Germany");
countryCombo->addItem("France");

// Combo box with data
auto roleCombo = std::make_unique<Wt::WComboBox>();
roleCombo->addItem("User", std::make_any<int>(1));
roleCombo->addItem("Moderator", std::make_any<int>(2));
roleCombo->addItem("Administrator", std::make_any<int>(3));

// Selection handler
roleCombo->changed().connect([roleCombo = roleCombo.get()]() {
    int selectedRole = std::any_cast<int>(roleCombo->currentData());
    handleRoleSelection(selectedRole);
});

// Searchable combo box
auto searchableCombo = std::make_unique<Wt::WComboBox>();
searchableCombo->setNoSelectionText("Type to search...");
searchableCombo->setMatcherCallback([](const std::string& input) {
    return performSearchQuery(input);
});
```

### WCheckBox and WRadioButton - Boolean Selections

```cpp
#include <Wt/WCheckBox.h>
#include <Wt/WRadioButton.h>
#include <Wt/WButtonGroup.h>

// Checkbox
auto agreeCheckbox = std::make_unique<Wt::WCheckBox>("I agree to the terms");
agreeCheckbox->changed().connect([agreeCheckbox = agreeCheckbox.get()]() {
    bool agreed = agreeCheckbox->isChecked();
    enableSubmitButton(agreed);
});

// Radio button group
auto genderGroup = std::make_shared<Wt::WButtonGroup>();

auto maleRadio = std::make_unique<Wt::WRadioButton>("Male");
auto femaleRadio = std::make_unique<Wt::WRadioButton>("Female");
auto otherRadio = std::make_unique<Wt::WRadioButton>("Other");

genderGroup->addButton(maleRadio.get(), 0);
genderGroup->addButton(femaleRadio.get(), 1);
genderGroup->addButton(otherRadio.get(), 2);

genderGroup->checkedChanged().connect([genderGroup](Wt::WRadioButton* button) {
    if (button) {
        int selection = genderGroup->id(button);
        handleGenderSelection(selection);
    }
});
```

## Container Widgets

### WContainerWidget - Generic Container

The fundamental container for organizing other widgets.

```cpp
#include <Wt/WContainerWidget.h>
#include <Wt/WVBoxLayout.h>

// Basic container
auto container = std::make_unique<Wt::WContainerWidget>();

// Add widgets to container
auto title = std::make_unique<Wt::WText>("<h2>User Form</h2>");
title->setTextFormat(Wt::TextFormat::XHTML);

auto nameInput = std::make_unique<Wt::WLineEdit>();
nameInput->setPlaceholderText("Name");

auto submitButton = std::make_unique<Wt::WPushButton>("Submit");

container->addWidget(std::move(title));
container->addWidget(std::move(nameInput));
container->addWidget(std::move(submitButton));

// Container with CSS styling
auto styledContainer = std::make_unique<Wt::WContainerWidget>();
styledContainer->addStyleClass("card");
styledContainer->addStyleClass("p-3");

// Container with layout
auto layoutContainer = std::make_unique<Wt::WContainerWidget>();
auto layout = std::make_unique<Wt::WVBoxLayout>();
layout->addWidget(std::make_unique<Wt::WText>("Header"));
layout->addWidget(std::make_unique<Wt::WLineEdit>(), 1); // Stretch factor 1
layout->addWidget(std::make_unique<Wt::WPushButton>("Footer"));
layoutContainer->setLayout(std::move(layout));
```

### WStackedWidget - Tabbed Content

Stack widgets on top of each other, showing one at a time.

```cpp
#include <Wt/WStackedWidget.h>
#include <Wt/WMenu.h>

// Create stacked widget
auto stack = std::make_unique<Wt::WStackedWidget>();

// Add pages
auto page1 = std::make_unique<Wt::WContainerWidget>();
page1->addWidget(std::make_unique<Wt::WText>("This is page 1 content"));

auto page2 = std::make_unique<Wt::WContainerWidget>();
page2->addWidget(std::make_unique<Wt::WText>("This is page 2 content"));

auto page3 = std::make_unique<Wt::WContainerWidget>();
page3->addWidget(std::make_unique<Wt::WText>("This is page 3 content"));

stack->addWidget(std::move(page1));
stack->addWidget(std::move(page2));
stack->addWidget(std::move(page3));

// Create menu for navigation
auto menu = std::make_unique<Wt::WMenu>(stack.get());
menu->addItem("Page 1");
menu->addItem("Page 2");
menu->addItem("Page 3");

// Set initial page
stack->setCurrentIndex(0);

// Programmatic navigation
auto nextButton = std::make_unique<Wt::WPushButton>("Next");
nextButton->clicked().connect([stack = stack.get()]() {
    int current = stack->currentIndex();
    int next = (current + 1) % stack->count();
    stack->setCurrentIndex(next);
});
```

### WGroupBox - Grouped Content

Group related widgets with an optional title.

```cpp
#include <Wt/WGroupBox.h>

// Basic group box
auto personalInfo = std::make_unique<Wt::WGroupBox>("Personal Information");

auto nameInput = std::make_unique<Wt::WLineEdit>();
nameInput->setPlaceholderText("Full Name");

auto emailInput = std::make_unique<Wt::WLineEdit>();
emailInput->setPlaceholderText("Email");

auto phoneInput = std::make_unique<Wt::WLineEdit>();
phoneInput->setPlaceholderText("Phone Number");

personalInfo->addWidget(std::move(nameInput));
personalInfo->addWidget(std::move(emailInput));
personalInfo->addWidget(std::move(phoneInput));

// Collapsible group box
auto advancedOptions = std::make_unique<Wt::WGroupBox>("Advanced Options");
advancedOptions->setCollapsible(true);
advancedOptions->setCollapsed(true);

// Group box with layout
auto settingsGroup = std::make_unique<Wt::WGroupBox>("Settings");
auto layout = std::make_unique<Wt::WGridLayout>();

layout->addWidget(std::make_unique<Wt::WText>("Theme:"), 0, 0);
layout->addWidget(std::make_unique<Wt::WComboBox>(), 0, 1);

layout->addWidget(std::make_unique<Wt::WText>("Language:"), 1, 0);
layout->addWidget(std::make_unique<Wt::WComboBox>(), 1, 1);

settingsGroup->setLayout(std::move(layout));
```

## Layout Widgets

### WVBoxLayout and WHBoxLayout - Linear Layouts

Arrange widgets vertically or horizontally.

```cpp
#include <Wt/WVBoxLayout.h>
#include <Wt/WHBoxLayout.h>

// Vertical layout
auto vLayout = std::make_unique<Wt::WVBoxLayout>();

vLayout->addWidget(std::make_unique<Wt::WText>("Header"));
vLayout->addWidget(std::make_unique<Wt::WLineEdit>(), 1); // Stretch
vLayout->addWidget(std::make_unique<Wt::WPushButton>("Submit"));

// Set spacing and margins
vLayout->setSpacing(10);
vLayout->setContentsMargins(20, 20, 20, 20);

// Horizontal layout with alignment
auto hLayout = std::make_unique<Wt::WHBoxLayout>();

hLayout->addWidget(std::make_unique<Wt::WText>("Label:"));
hLayout->addWidget(std::make_unique<Wt::WLineEdit>(), 1);
hLayout->addWidget(std::make_unique<Wt::WPushButton>("Go"));

// Custom stretch and alignment
hLayout->addStretch(1); // Add stretchable space
hLayout->addWidget(std::make_unique<Wt::WPushButton>("Help"), 
                   0, Wt::AlignmentFlag::Right);
```

### WGridLayout - Grid-based Layout

Arrange widgets in a grid pattern.

```cpp
#include <Wt/WGridLayout.h>

auto gridLayout = std::make_unique<Wt::WGridLayout>();

// Add widgets with row, column positions
gridLayout->addWidget(std::make_unique<Wt::WText>("Name:"), 0, 0);
gridLayout->addWidget(std::make_unique<Wt::WLineEdit>(), 0, 1);

gridLayout->addWidget(std::make_unique<Wt::WText>("Email:"), 1, 0);
gridLayout->addWidget(std::make_unique<Wt::WLineEdit>(), 1, 1);

gridLayout->addWidget(std::make_unique<Wt::WText>("Message:"), 2, 0);
gridLayout->addWidget(std::make_unique<Wt::WTextArea>(), 2, 1);

// Spanning multiple columns
auto submitButton = std::make_unique<Wt::WPushButton>("Submit");
gridLayout->addWidget(std::move(submitButton), 3, 0, 1, 2); // row, col, rowspan, colspan

// Set column stretch
gridLayout->setColumnStretch(1, 1); // Second column stretches

// Set row and column spacing
gridLayout->setRowSpacing(10);
gridLayout->setColumnSpacing(15);
```

## Advanced Widgets

### WTable - Data Tables

Display tabular data with sorting and selection.

```cpp
#include <Wt/WTable.h>
#include <Wt/WTableCell.h>

// Basic table
auto table = std::make_unique<Wt::WTable>();
table->addStyleClass("table table-striped");

// Add headers
table->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("Name"));
table->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("Email"));
table->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("Actions"));

// Add data rows
std::vector<User> users = getUsers();
for (size_t i = 0; i < users.size(); ++i) {
    int row = i + 1;
    
    table->elementAt(row, 0)->addWidget(
        std::make_unique<Wt::WText>(users[i].name));
    
    table->elementAt(row, 1)->addWidget(
        std::make_unique<Wt::WText>(users[i].email));
    
    auto deleteButton = std::make_unique<Wt::WPushButton>("Delete");
    deleteButton->clicked().connect([this, userId = users[i].id]() {
        deleteUser(userId);
        refreshTable();
    });
    table->elementAt(row, 2)->addWidget(std::move(deleteButton));
}

// Table with selection
table->clicked().connect([table = table.get()](const Wt::WModelIndex& index) {
    if (index.isValid()) {
        int row = index.row();
        selectTableRow(row);
    }
});
```

### WTree - Hierarchical Data

Display tree-structured data with expand/collapse functionality.

```cpp
#include <Wt/WTree.h>
#include <Wt/WTreeNode.h>

// Create tree
auto tree = std::make_unique<Wt::WTree>();

// Root node
auto rootNode = std::make_unique<Wt::WTreeNode>("Root");
tree->setTreeRoot(std::move(rootNode));
auto root = tree->treeRoot();

// Add child nodes
auto folder1 = std::make_unique<Wt::WTreeNode>("Documents");
auto folder2 = std::make_unique<Wt::WTreeNode>("Images");
auto folder3 = std::make_unique<Wt::WTreeNode>("Videos");

// Add files to Documents folder
auto file1 = std::make_unique<Wt::WTreeNode>("document1.pdf");
auto file2 = std::make_unique<Wt::WTreeNode>("report.docx");

folder1->addChildNode(std::move(file1));
folder1->addChildNode(std::move(file2));

root->addChildNode(std::move(folder1));
root->addChildNode(std::move(folder2));
root->addChildNode(std::move(folder3));

// Node selection handler
tree->itemSelectionChanged().connect([tree = tree.get()]() {
    auto selected = tree->selectedNodes();
    if (!selected.empty()) {
        auto node = *selected.begin();
        handleNodeSelection(node->label()->text().toUTF8());
    }
});

// Expand/collapse handlers
root->expanded().connect([](){ 
    log("info") << "Root node expanded"; 
});
root->collapsed().connect([](){ 
    log("info") << "Root node collapsed"; 
});
```

### WMenu - Navigation Menu

Create navigation menus with hierarchical structure.

```cpp
#include <Wt/WMenu.h>
#include <Wt/WMenuItem.h>
#include <Wt/WSubMenuItem.h>

// Basic menu
auto menu = std::make_unique<Wt::WMenu>();

// Add menu items
auto homeItem = menu->addItem("Home");
auto aboutItem = menu->addItem("About");
auto contactItem = menu->addItem("Contact");

// Menu with stack
auto stack = std::make_unique<Wt::WStackedWidget>();
auto menuWithStack = std::make_unique<Wt::WMenu>(stack.get());

auto homePage = std::make_unique<Wt::WContainerWidget>();
homePage->addWidget(std::make_unique<Wt::WText>("Welcome to the home page"));

auto aboutPage = std::make_unique<Wt::WContainerWidget>();
aboutPage->addWidget(std::make_unique<Wt::WText>("About our company"));

menuWithStack->addItem("Home", std::move(homePage));
menuWithStack->addItem("About", std::move(aboutPage));

// Submenu
auto productsMenu = std::make_unique<Wt::WSubMenuItem>("Products", 
                                                      std::make_unique<Wt::WContainerWidget>());
auto subMenu = std::make_unique<Wt::WMenu>();
subMenu->addItem("Software");
subMenu->addItem("Hardware");
subMenu->addItem("Services");

productsMenu->setSubMenu(std::move(subMenu));
menu->addItem(std::move(productsMenu));

// Menu item selection
menu->itemSelected().connect([](Wt::WMenuItem* item) {
    if (item) {
        std::string selected = item->text().toUTF8();
        handleMenuSelection(selected);
    }
});
```

## Custom Widgets

### Creating Custom Widgets

Extend existing widgets or create completely new ones.

```cpp
#include <Wt/WCompositeWidget.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WVBoxLayout.h>

// Custom composite widget
class UserCard : public Wt::WCompositeWidget {
public:
    UserCard(const User& user) : user_(user) {
        auto container = std::make_unique<Wt::WContainerWidget>();
        
        // Create layout
        auto layout = std::make_unique<Wt::WVBoxLayout>();
        
        // User avatar
        auto avatar = std::make_unique<Wt::WImage>(user_.avatarUrl);
        avatar->addStyleClass("user-avatar");
        avatar->resize(64, 64);
        
        // User name
        auto name = std::make_unique<Wt::WText>(user_.name);
        name->addStyleClass("user-name");
        
        // User email
        auto email = std::make_unique<Wt::WText>(user_.email);
        email->addStyleClass("user-email");
        
        // Action buttons
        auto buttonLayout = std::make_unique<Wt::WHBoxLayout>();
        
        auto viewButton = std::make_unique<Wt::WPushButton>("View");
        viewButton->clicked().connect([this]() { viewUser(); });
        
        auto editButton = std::make_unique<Wt::WPushButton>("Edit");
        editButton->clicked().connect([this]() { editUser(); });
        
        buttonLayout->addWidget(std::move(viewButton));
        buttonLayout->addWidget(std::move(editButton));
        
        auto buttonContainer = std::make_unique<Wt::WContainerWidget>();
        buttonContainer->setLayout(std::move(buttonLayout));
        
        // Add to main layout
        layout->addWidget(std::move(avatar));
        layout->addWidget(std::move(name));
        layout->addWidget(std::move(email));
        layout->addWidget(std::move(buttonContainer));
        
        container->setLayout(std::move(layout));
        container->addStyleClass("user-card");
        
        setImplementation(std::move(container));
    }
    
    // Custom signals
    Wt::Signal<int>& userViewed() { return userViewed_; }
    Wt::Signal<int>& userEdited() { return userEdited_; }
    
private:
    User user_;
    Wt::Signal<int> userViewed_;
    Wt::Signal<int> userEdited_;
    
    void viewUser() {
        userViewed_.emit(user_.id);
    }
    
    void editUser() {
        userEdited_.emit(user_.id);
    }
};

// Usage
auto userCard = std::make_unique<UserCard>(user);
userCard->userViewed().connect([](int userId) {
    showUserProfile(userId);
});
userCard->userEdited().connect([](int userId) {
    showUserEditDialog(userId);
});
```

### Custom Widget with Resources

```cpp
// Custom widget with CSS and JavaScript resources
class InteractiveChart : public Wt::WCompositeWidget {
public:
    InteractiveChart() {
        auto container = std::make_unique<Wt::WContainerWidget>();
        container->addStyleClass("interactive-chart");
        
        // Add required resources
        Wt::WApplication::instance()->requireJQuery();
        Wt::WApplication::instance()->require("chart.js");
        Wt::WApplication::instance()->useStyleSheet("css/chart-styles.css");
        
        // Create canvas for chart
        auto canvas = std::make_unique<Wt::WWidget>();
        canvas->setId("chart-canvas-" + std::to_string(nextId_++));
        
        container->addWidget(std::move(canvas));
        setImplementation(std::move(container));
        
        // Initialize chart after widget is rendered
        doJavaScript(R"(
            $(document).ready(function() {
                initializeChart('" + canvas->id() + "');
            });
        )");
    }
    
    void updateData(const std::vector<ChartDataPoint>& data) {
        std::string jsonData = serializeChartData(data);
        doJavaScript("updateChartData('" + implementation()->id() + "', " + jsonData + ");");
    }
    
private:
    static int nextId_;
    
    std::string serializeChartData(const std::vector<ChartDataPoint>& data) {
        // Convert data to JSON format
        std::ostringstream json;
        json << "[";
        for (size_t i = 0; i < data.size(); ++i) {
            if (i > 0) json << ",";
            json << "{\"label\":\"" << data[i].label 
                 << "\",\"value\":" << data[i].value << "}";
        }
        json << "]";
        return json.str();
    }
};

int InteractiveChart::nextId_ = 0;
```

## Widget Properties

### Common Properties

All widgets inherit common properties and methods:

```cpp
// Visibility
widget->setHidden(true);
widget->show();
widget->hide();

// Styling
widget->addStyleClass("my-class");
widget->removeStyleClass("old-class");
widget->setStyleClass("new-class");

// Attributes
widget->setAttributeValue("data-id", "123");
widget->setId("unique-id");

// Size and positioning
widget->resize(200, 100);
widget->setMaximumSize(400, 300);
widget->setMinimumSize(100, 50);

// Margins and padding
widget->setMargin(10);
widget->setMargin(Wt::WLength(10), Wt::Side::Top | Wt::Side::Bottom);

// Enable/disable
widget->setEnabled(false);
widget->setDisabled(true);

// Focus
widget->setFocusPolicy(Wt::FocusPolicy::StrongFocus);
widget->setTabIndex(1);

// Tool tips
widget->setToolTip("This is a helpful tooltip");

// Custom JavaScript
widget->doJavaScript("console.log('Widget created');");
```

### Responsive Design

```cpp
// Responsive utilities
auto responsiveContainer = std::make_unique<Wt::WContainerWidget>();

// Bootstrap-style responsive classes
responsiveContainer->addStyleClass("col-lg-6 col-md-8 col-sm-12");

// Media query based styling
responsiveContainer->addStyleClass("responsive-widget");

// Programmatic responsive behavior
auto updateLayout = [responsiveContainer = responsiveContainer.get()]() {
    if (Wt::WApplication::instance()->environment().screenWidth() < 768) {
        responsiveContainer->addStyleClass("mobile-layout");
        responsiveContainer->removeStyleClass("desktop-layout");
    } else {
        responsiveContainer->addStyleClass("desktop-layout");
        responsiveContainer->removeStyleClass("mobile-layout");
    }
};

// Call on resize
Wt::WApplication::instance()->globalEscapePressed().connect(updateLayout);
```

### Theming and Styling

```cpp
// Theme-aware widgets
auto themedButton = std::make_unique<Wt::WPushButton>("Themed Button");

// Apply theme-specific classes
if (getCurrentTheme() == Theme::Dark) {
    themedButton->addStyleClass("btn-dark");
} else {
    themedButton->addStyleClass("btn-light");
}

// CSS custom properties
themedButton->setAttributeValue("style", 
    "--primary-color: #007bff; --secondary-color: #6c757d;");

// Dynamic theming
auto themeToggle = std::make_unique<Wt::WPushButton>("Toggle Theme");
themeToggle->clicked().connect([this]() {
    toggleApplicationTheme();
    updateAllWidgetThemes();
});
```

---

*This widget reference provides comprehensive coverage of Wt's widget system. For layout-specific details, see [Layout System](layout-system.md). For event handling patterns, see [Event Handling](event-handling.md).*
