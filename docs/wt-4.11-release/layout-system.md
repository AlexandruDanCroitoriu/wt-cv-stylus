# Layout System Guide

Wt provides a comprehensive layout system that combines CSS-based styling with programmatic layout managers. This guide covers all layout approaches available in Wt applications.

## Table of Contents

1. [Layout Overview](#layout-overview)
2. [CSS-Based Layouts](#css-based-layouts)
3. [Layout Managers](#layout-managers)
4. [Template-Based Layouts](#template-based-layouts)
5. [Responsive Design](#responsive-design)
6. [Advanced Layout Techniques](#advanced-layout-techniques)
7. [Performance Considerations](#performance-considerations)

## Layout Overview

Wt supports multiple layout approaches:

- **CSS-based layouts**: Using style classes and external stylesheets
- **Layout managers**: Programmatic layouts (WBoxLayout, WGridLayout, WBorderLayout)
- **Template layouts**: HTML templates with widget placeholders
- **Hybrid approaches**: Combining multiple layout techniques

### Layout Philosophy

```cpp
// Wt's layout system follows these principles:
// 1. Widgets form a hierarchical tree
// 2. Parent widgets manage child positioning
// 3. CSS and layout managers can be combined
// 4. Responsive design is supported throughout

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WText.h>

class LayoutDemoApp : public Wt::WApplication {
public:
    LayoutDemoApp(const Wt::WEnvironment& env) : WApplication(env) {
        // Set page title
        setTitle("Layout System Demo");
        
        // Add global stylesheets
        useStyleSheet("css/bootstrap.min.css");
        useStyleSheet("css/custom-layout.css");
        
        // Create main layout
        createMainLayout();
    }
    
private:
    void createMainLayout() {
        auto container = std::make_unique<Wt::WContainerWidget>();
        container->addStyleClass("main-container");
        
        // Use layout manager for structure
        auto layout = std::make_unique<Wt::WVBoxLayout>();
        layout->addWidget(createHeader());
        layout->addWidget(createContent(), 1); // Stretch factor
        layout->addWidget(createFooter());
        
        container->setLayout(std::move(layout));
        root()->addWidget(std::move(container));
    }
    
    std::unique_ptr<Wt::WWidget> createHeader() {
        auto header = std::make_unique<Wt::WContainerWidget>();
        header->addStyleClass("header bg-primary text-white p-3");
        header->addWidget(std::make_unique<Wt::WText>("<h1>My Application</h1>"));
        return header;
    }
    
    std::unique_ptr<Wt::WWidget> createContent() {
        auto content = std::make_unique<Wt::WContainerWidget>();
        content->addStyleClass("content-area p-4");
        return content;
    }
    
    std::unique_ptr<Wt::WWidget> createFooter() {
        auto footer = std::make_unique<Wt::WContainerWidget>();
        footer->addStyleClass("footer bg-secondary text-white text-center p-2");
        footer->addWidget(std::make_unique<Wt::WText>("© 2024 My Company"));
        return footer;
    }
};
```

## CSS-Based Layouts

### External Stylesheets

```cpp
// In your application constructor
class StyledApplication : public Wt::WApplication {
public:
    StyledApplication(const Wt::WEnvironment& env) : WApplication(env) {
        // Load external CSS frameworks
        useStyleSheet("https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css");
        useStyleSheet("css/custom-styles.css");
        
        // Load theme-specific styles
        if (getUserTheme() == "dark") {
            useStyleSheet("css/dark-theme.css");
        }
        
        // Application-specific styles
        useStyleSheet("css/app-layout.css");
    }
};

// Custom CSS file: css/app-layout.css
/*
.main-container {
    min-height: 100vh;
    display: flex;
    flex-direction: column;
}

.header {
    flex-shrink: 0;
    background: linear-gradient(90deg, #667eea 0%, #764ba2 100%);
}

.content-area {
    flex: 1;
    display: grid;
    grid-template-columns: 250px 1fr;
    gap: 20px;
}

.sidebar {
    background-color: #f8f9fa;
    border-right: 1px solid #dee2e6;
    padding: 20px;
}

.main-content {
    padding: 20px;
    overflow-y: auto;
}

.footer {
    flex-shrink: 0;
    margin-top: auto;
}

@media (max-width: 768px) {
    .content-area {
        grid-template-columns: 1fr;
    }
    
    .sidebar {
        order: 2;
    }
}
*/
```

### Style Class Management

```cpp
#include <Wt/WContainerWidget.h>

class ResponsiveCard : public Wt::WContainerWidget {
public:
    ResponsiveCard() {
        // Base card styling
        addStyleClass("card shadow-sm");
        
        // Responsive behavior
        addStyleClass("col-lg-4 col-md-6 col-sm-12");
        
        createCardContent();
    }
    
    void setTheme(const std::string& theme) {
        // Remove existing theme classes
        removeStyleClass("card-light");
        removeStyleClass("card-dark");
        
        // Apply new theme
        addStyleClass("card-" + theme);
    }
    
    void setSize(CardSize size) {
        // Remove size classes
        removeStyleClass("card-sm");
        removeStyleClass("card-md");
        removeStyleClass("card-lg");
        
        // Apply new size
        switch (size) {
            case CardSize::Small:
                addStyleClass("card-sm");
                break;
            case CardSize::Medium:
                addStyleClass("card-md");
                break;
            case CardSize::Large:
                addStyleClass("card-lg");
                break;
        }
    }
    
private:
    enum class CardSize { Small, Medium, Large };
    
    void createCardContent() {
        // Card header
        auto header = std::make_unique<Wt::WContainerWidget>();
        header->addStyleClass("card-header");
        header->addWidget(std::make_unique<Wt::WText>("<h5>Card Title</h5>"));
        
        // Card body
        auto body = std::make_unique<Wt::WContainerWidget>();
        body->addStyleClass("card-body");
        body->addWidget(std::make_unique<Wt::WText>("Card content goes here."));
        
        // Card footer
        auto footer = std::make_unique<Wt::WContainerWidget>();
        footer->addStyleClass("card-footer");
        
        auto button = std::make_unique<Wt::WPushButton>("Action");
        button->addStyleClass("btn btn-primary");
        footer->addWidget(std::move(button));
        
        addWidget(std::move(header));
        addWidget(std::move(body));
        addWidget(std::move(footer));
    }
};
```

### Flexbox Layouts

```cpp
// Flexbox container widget
class FlexContainer : public Wt::WContainerWidget {
public:
    enum class Direction { Row, RowReverse, Column, ColumnReverse };
    enum class Justify { Start, End, Center, SpaceBetween, SpaceAround, SpaceEvenly };
    enum class Align { Start, End, Center, Baseline, Stretch };
    
    FlexContainer(Direction direction = Direction::Row) {
        addStyleClass("d-flex");
        setDirection(direction);
    }
    
    void setDirection(Direction direction) {
        removeStyleClass("flex-row flex-row-reverse flex-column flex-column-reverse");
        
        switch (direction) {
            case Direction::Row:
                addStyleClass("flex-row");
                break;
            case Direction::RowReverse:
                addStyleClass("flex-row-reverse");
                break;
            case Direction::Column:
                addStyleClass("flex-column");
                break;
            case Direction::ColumnReverse:
                addStyleClass("flex-column-reverse");
                break;
        }
    }
    
    void setJustifyContent(Justify justify) {
        removeStyleClass("justify-content-start justify-content-end justify-content-center "
                         "justify-content-between justify-content-around justify-content-evenly");
        
        switch (justify) {
            case Justify::Start:
                addStyleClass("justify-content-start");
                break;
            case Justify::End:
                addStyleClass("justify-content-end");
                break;
            case Justify::Center:
                addStyleClass("justify-content-center");
                break;
            case Justify::SpaceBetween:
                addStyleClass("justify-content-between");
                break;
            case Justify::SpaceAround:
                addStyleClass("justify-content-around");
                break;
            case Justify::SpaceEvenly:
                addStyleClass("justify-content-evenly");
                break;
        }
    }
    
    void setAlignItems(Align align) {
        removeStyleClass("align-items-start align-items-end align-items-center "
                         "align-items-baseline align-items-stretch");
        
        switch (align) {
            case Align::Start:
                addStyleClass("align-items-start");
                break;
            case Align::End:
                addStyleClass("align-items-end");
                break;
            case Align::Center:
                addStyleClass("align-items-center");
                break;
            case Align::Baseline:
                addStyleClass("align-items-baseline");
                break;
            case Align::Stretch:
                addStyleClass("align-items-stretch");
                break;
        }
    }
};

// Usage example
void createFlexLayout() {
    auto flexContainer = std::make_unique<FlexContainer>(FlexContainer::Direction::Row);
    flexContainer->setJustifyContent(FlexContainer::Justify::SpaceBetween);
    flexContainer->setAlignItems(FlexContainer::Align::Center);
    
    // Add flex items
    auto item1 = std::make_unique<Wt::WContainerWidget>();
    item1->addStyleClass("flex-grow-1 p-3 bg-light");
    item1->addWidget(std::make_unique<Wt::WText>("Flexible Item 1"));
    
    auto item2 = std::make_unique<Wt::WContainerWidget>();
    item2->addStyleClass("flex-shrink-0 p-3 bg-primary text-white");
    item2->addWidget(std::make_unique<Wt::WText>("Fixed Item"));
    
    auto item3 = std::make_unique<Wt::WContainerWidget>();
    item3->addStyleClass("flex-grow-2 p-3 bg-secondary text-white");
    item3->addWidget(std::make_unique<Wt::WText>("Flexible Item 2"));
    
    flexContainer->addWidget(std::move(item1));
    flexContainer->addWidget(std::move(item2));
    flexContainer->addWidget(std::move(item3));
}
```

## Layout Managers

### WVBoxLayout - Vertical Layout

```cpp
#include <Wt/WVBoxLayout.h>

class VerticalLayoutExample : public Wt::WContainerWidget {
public:
    VerticalLayoutExample() {
        auto layout = std::make_unique<Wt::WVBoxLayout>();
        
        // Add widgets with different stretch factors
        layout->addWidget(createHeader(), 0); // No stretch
        layout->addWidget(createToolbar(), 0); // No stretch
        layout->addWidget(createMainContent(), 1); // Stretch to fill
        layout->addWidget(createStatusBar(), 0); // No stretch
        
        // Set spacing and margins
        layout->setSpacing(0);
        layout->setContentsMargins(10, 10, 10, 10);
        
        setLayout(std::move(layout));
    }
    
private:
    std::unique_ptr<Wt::WWidget> createHeader() {
        auto header = std::make_unique<Wt::WContainerWidget>();
        header->addStyleClass("bg-primary text-white p-3");
        header->setHeight(60);
        header->addWidget(std::make_unique<Wt::WText>("<h3>Application Header</h3>"));
        return header;
    }
    
    std::unique_ptr<Wt::WWidget> createToolbar() {
        auto toolbar = std::make_unique<Wt::WContainerWidget>();
        toolbar->addStyleClass("bg-light border-bottom p-2");
        toolbar->setHeight(40);
        
        auto button1 = std::make_unique<Wt::WPushButton>("New");
        auto button2 = std::make_unique<Wt::WPushButton>("Open");
        auto button3 = std::make_unique<Wt::WPushButton>("Save");
        
        toolbar->addWidget(std::move(button1));
        toolbar->addWidget(std::move(button2));
        toolbar->addWidget(std::move(button3));
        
        return toolbar;
    }
    
    std::unique_ptr<Wt::WWidget> createMainContent() {
        auto content = std::make_unique<Wt::WContainerWidget>();
        content->addStyleClass("bg-white p-4");
        content->addWidget(std::make_unique<Wt::WText>("Main content area - this will expand to fill available space"));
        return content;
    }
    
    std::unique_ptr<Wt::WWidget> createStatusBar() {
        auto status = std::make_unique<Wt::WContainerWidget>();
        status->addStyleClass("bg-secondary text-white p-1");
        status->setHeight(25);
        status->addWidget(std::make_unique<Wt::WText>("Ready"));
        return status;
    }
};
```

### WHBoxLayout - Horizontal Layout

```cpp
#include <Wt/WHBoxLayout.h>

class HorizontalLayoutExample : public Wt::WContainerWidget {
public:
    HorizontalLayoutExample() {
        auto layout = std::make_unique<Wt::WHBoxLayout>();
        
        // Sidebar with fixed width
        layout->addWidget(createSidebar(), 0);
        
        // Main content that stretches
        layout->addWidget(createMainArea(), 1);
        
        // Right panel with fixed width
        layout->addWidget(createRightPanel(), 0);
        
        layout->setSpacing(5);
        setLayout(std::move(layout));
    }
    
private:
    std::unique_ptr<Wt::WWidget> createSidebar() {
        auto sidebar = std::make_unique<Wt::WContainerWidget>();
        sidebar->addStyleClass("bg-light border-end");
        sidebar->setWidth(200);
        sidebar->setMinimumSize(200, Wt::WLength::Auto);
        
        // Navigation menu
        auto nav = std::make_unique<Wt::WContainerWidget>();
        nav->addStyleClass("nav flex-column p-3");
        
        auto link1 = std::make_unique<Wt::WText>("<a href='#' class='nav-link'>Dashboard</a>");
        auto link2 = std::make_unique<Wt::WText>("<a href='#' class='nav-link'>Users</a>");
        auto link3 = std::make_unique<Wt::WText>("<a href='#' class='nav-link'>Settings</a>");
        
        nav->addWidget(std::move(link1));
        nav->addWidget(std::move(link2));
        nav->addWidget(std::move(link3));
        
        sidebar->addWidget(std::move(nav));
        return sidebar;
    }
    
    std::unique_ptr<Wt::WWidget> createMainArea() {
        auto main = std::make_unique<Wt::WContainerWidget>();
        main->addStyleClass("p-4");
        
        // This area will expand to fill available horizontal space
        auto content = std::make_unique<Wt::WText>(
            "<h2>Main Content Area</h2>"
            "<p>This area expands to fill available space.</p>");
        content->setTextFormat(Wt::TextFormat::XHTML);
        
        main->addWidget(std::move(content));
        return main;
    }
    
    std::unique_ptr<Wt::WWidget> createRightPanel() {
        auto panel = std::make_unique<Wt::WContainerWidget>();
        panel->addStyleClass("bg-light border-start p-3");
        panel->setWidth(150);
        panel->setMinimumSize(150, Wt::WLength::Auto);
        
        panel->addWidget(std::make_unique<Wt::WText>("<h5>Quick Info</h5>"));
        panel->addWidget(std::make_unique<Wt::WText>("Additional information panel"));
        
        return panel;
    }
};
```

### WGridLayout - Grid Layout

```cpp
#include <Wt/WGridLayout.h>

class GridLayoutExample : public Wt::WContainerWidget {
public:
    GridLayoutExample() {
        auto layout = std::make_unique<Wt::WGridLayout>();
        
        // Create form-like layout
        createFormLayout(layout.get());
        
        // Set layout properties
        layout->setColumnResizable(1, true);
        layout->setRowSpacing(10);
        layout->setColumnSpacing(15);
        
        setLayout(std::move(layout));
        addStyleClass("p-4");
    }
    
private:
    void createFormLayout(Wt::WGridLayout* layout) {
        // Row 0: Title spanning all columns
        auto title = std::make_unique<Wt::WText>("<h3>User Registration Form</h3>");
        title->setTextFormat(Wt::TextFormat::XHTML);
        layout->addWidget(std::move(title), 0, 0, 1, 3); // row, col, rowspan, colspan
        
        // Row 1: First Name
        layout->addWidget(std::make_unique<Wt::WText>("First Name:"), 1, 0);
        auto firstName = std::make_unique<Wt::WLineEdit>();
        firstName->setPlaceholderText("Enter first name");
        layout->addWidget(std::move(firstName), 1, 1);
        
        // Row 2: Last Name
        layout->addWidget(std::make_unique<Wt::WText>("Last Name:"), 2, 0);
        auto lastName = std::make_unique<Wt::WLineEdit>();
        lastName->setPlaceholderText("Enter last name");
        layout->addWidget(std::move(lastName), 2, 1);
        
        // Row 3: Email spanning two columns
        layout->addWidget(std::make_unique<Wt::WText>("Email:"), 3, 0);
        auto email = std::make_unique<Wt::WLineEdit>();
        email->setPlaceholderText("user@example.com");
        layout->addWidget(std::move(email), 3, 1, 1, 2); // Span 2 columns
        
        // Row 4: Phone and Country
        layout->addWidget(std::make_unique<Wt::WText>("Phone:"), 4, 0);
        auto phone = std::make_unique<Wt::WLineEdit>();
        phone->setPlaceholderText("Phone number");
        layout->addWidget(std::move(phone), 4, 1);
        
        auto country = std::make_unique<Wt::WComboBox>();
        country->addItem("USA");
        country->addItem("Canada");
        country->addItem("UK");
        layout->addWidget(std::move(country), 4, 2);
        
        // Row 5: Address (text area spanning multiple columns)
        layout->addWidget(std::make_unique<Wt::WText>("Address:"), 5, 0);
        auto address = std::make_unique<Wt::WTextArea>();
        address->setRows(3);
        address->setPlaceholderText("Enter full address");
        layout->addWidget(std::move(address), 5, 1, 1, 2);
        
        // Row 6: Buttons
        auto buttonContainer = std::make_unique<Wt::WContainerWidget>();
        auto buttonLayout = std::make_unique<Wt::WHBoxLayout>();
        
        auto submitBtn = std::make_unique<Wt::WPushButton>("Submit");
        submitBtn->addStyleClass("btn btn-primary me-2");
        
        auto cancelBtn = std::make_unique<Wt::WPushButton>("Cancel");
        cancelBtn->addStyleClass("btn btn-secondary");
        
        buttonLayout->addWidget(std::move(submitBtn));
        buttonLayout->addWidget(std::move(cancelBtn));
        buttonLayout->addStretch(1);
        
        buttonContainer->setLayout(std::move(buttonLayout));
        layout->addWidget(std::move(buttonContainer), 6, 0, 1, 3);
        
        // Set column stretch factors
        layout->setColumnStretch(0, 0); // Labels don't stretch
        layout->setColumnStretch(1, 1); // Input fields stretch
        layout->setColumnStretch(2, 0); // Right column fixed
    }
};
```

### WBorderLayout - Border Layout

```cpp
#include <Wt/WBorderLayout.h>

class BorderLayoutExample : public Wt::WContainerWidget {
public:
    BorderLayoutExample() {
        auto layout = std::make_unique<Wt::WBorderLayout>();
        
        // Add widgets to different regions
        layout->addWidget(createNorthWidget(), Wt::LayoutPosition::North);
        layout->addWidget(createSouthWidget(), Wt::LayoutPosition::South);
        layout->addWidget(createWestWidget(), Wt::LayoutPosition::West);
        layout->addWidget(createEastWidget(), Wt::LayoutPosition::East);
        layout->addWidget(createCenterWidget(), Wt::LayoutPosition::Center);
        
        setLayout(std::move(layout));
        setMinimumSize(800, 600);
    }
    
private:
    std::unique_ptr<Wt::WWidget> createNorthWidget() {
        auto north = std::make_unique<Wt::WContainerWidget>();
        north->addStyleClass("bg-primary text-white text-center p-3");
        north->setHeight(60);
        north->addWidget(std::make_unique<Wt::WText>("<h4>Header (North)</h4>"));
        return north;
    }
    
    std::unique_ptr<Wt::WWidget> createSouthWidget() {
        auto south = std::make_unique<Wt::WContainerWidget>();
        south->addStyleClass("bg-secondary text-white text-center p-2");
        south->setHeight(40);
        south->addWidget(std::make_unique<Wt::WText>("Footer (South)"));
        return south;
    }
    
    std::unique_ptr<Wt::WWidget> createWestWidget() {
        auto west = std::make_unique<Wt::WContainerWidget>();
        west->addStyleClass("bg-light border-end p-3");
        west->setWidth(200);
        west->addWidget(std::make_unique<Wt::WText>("<h5>Sidebar (West)</h5>"));
        return west;
    }
    
    std::unique_ptr<Wt::WWidget> createEastWidget() {
        auto east = std::make_unique<Wt::WContainerWidget>();
        east->addStyleClass("bg-light border-start p-3");
        east->setWidth(150);
        east->addWidget(std::make_unique<Wt::WText>("<h5>Panel (East)</h5>"));
        return east;
    }
    
    std::unique_ptr<Wt::WWidget> createCenterWidget() {
        auto center = std::make_unique<Wt::WContainerWidget>();
        center->addStyleClass("p-4");
        
        auto content = std::make_unique<Wt::WText>(
            "<h2>Main Content (Center)</h2>"
            "<p>This is the main content area that takes up the remaining space.</p>");
        content->setTextFormat(Wt::TextFormat::XHTML);
        
        center->addWidget(std::move(content));
        return center;
    }
};
```

## Template-Based Layouts

### WTemplate - HTML Templates

```cpp
#include <Wt/WTemplate.h>

class TemplateLayoutExample : public Wt::WContainerWidget {
public:
    TemplateLayoutExample() {
        createTemplateLayout();
    }
    
private:
    void createTemplateLayout() {
        // HTML template string
        const char* templateText = R"(
            <div class="container-fluid">
                <header class="row bg-primary text-white p-3">
                    <div class="col">
                        ${header-content}
                    </div>
                </header>
                
                <div class="row flex-grow-1">
                    <nav class="col-md-3 col-lg-2 bg-light sidebar">
                        <div class="p-3">
                            ${navigation}
                        </div>
                    </nav>
                    
                    <main class="col-md-9 col-lg-10 p-4">
                        <div class="row">
                            <div class="col-lg-8">
                                ${main-content}
                            </div>
                            <aside class="col-lg-4">
                                ${sidebar-content}
                            </aside>
                        </div>
                    </main>
                </div>
                
                <footer class="row bg-secondary text-white p-2">
                    <div class="col text-center">
                        ${footer-content}
                    </div>
                </footer>
            </div>
        )";
        
        auto template_ = std::make_unique<Wt::WTemplate>(templateText);
        
        // Bind widgets to template placeholders
        template_->bindWidget("header-content", createHeaderContent());
        template_->bindWidget("navigation", createNavigation());
        template_->bindWidget("main-content", createMainContent());
        template_->bindWidget("sidebar-content", createSidebarContent());
        template_->bindWidget("footer-content", createFooterContent());
        
        addWidget(std::move(template_));
    }
    
    std::unique_ptr<Wt::WWidget> createHeaderContent() {
        auto header = std::make_unique<Wt::WContainerWidget>();
        
        auto title = std::make_unique<Wt::WText>("<h2>Dashboard</h2>");
        title->setTextFormat(Wt::TextFormat::XHTML);
        
        auto userInfo = std::make_unique<Wt::WText>("Welcome, John Doe");
        userInfo->addStyleClass("float-end");
        
        header->addWidget(std::move(title));
        header->addWidget(std::move(userInfo));
        
        return header;
    }
    
    std::unique_ptr<Wt::WWidget> createNavigation() {
        auto nav = std::make_unique<Wt::WContainerWidget>();
        
        auto navList = std::make_unique<Wt::WContainerWidget>();
        navList->addStyleClass("nav flex-column");
        
        auto createNavItem = [](const std::string& text, const std::string& href = "#") {
            auto item = std::make_unique<Wt::WText>(
                "<a href='" + href + "' class='nav-link'>" + text + "</a>");
            item->setTextFormat(Wt::TextFormat::XHTML);
            return item;
        };
        
        navList->addWidget(createNavItem("Dashboard", "#dashboard"));
        navList->addWidget(createNavItem("Users", "#users"));
        navList->addWidget(createNavItem("Reports", "#reports"));
        navList->addWidget(createNavItem("Settings", "#settings"));
        
        nav->addWidget(std::move(navList));
        return nav;
    }
    
    std::unique_ptr<Wt::WWidget> createMainContent() {
        auto main = std::make_unique<Wt::WContainerWidget>();
        
        // Statistics cards
        auto cardRow = std::make_unique<Wt::WContainerWidget>();
        cardRow->addStyleClass("row mb-4");
        
        for (int i = 0; i < 3; ++i) {
            auto cardCol = std::make_unique<Wt::WContainerWidget>();
            cardCol->addStyleClass("col-md-4");
            
            auto card = std::make_unique<Wt::WContainerWidget>();
            card->addStyleClass("card text-center");
            
            auto cardBody = std::make_unique<Wt::WContainerWidget>();
            cardBody->addStyleClass("card-body");
            
            auto cardTitle = std::make_unique<Wt::WText>("<h5 class='card-title'>Metric " + std::to_string(i+1) + "</h5>");
            cardTitle->setTextFormat(Wt::TextFormat::XHTML);
            
            auto cardValue = std::make_unique<Wt::WText>("<h2 class='text-primary'>" + std::to_string((i+1) * 123) + "</h2>");
            cardValue->setTextFormat(Wt::TextFormat::XHTML);
            
            cardBody->addWidget(std::move(cardTitle));
            cardBody->addWidget(std::move(cardValue));
            card->addWidget(std::move(cardBody));
            cardCol->addWidget(std::move(card));
            cardRow->addWidget(std::move(cardCol));
        }
        
        main->addWidget(std::move(cardRow));
        
        // Chart placeholder
        auto chartContainer = std::make_unique<Wt::WContainerWidget>();
        chartContainer->addStyleClass("card");
        chartContainer->addWidget(std::make_unique<Wt::WText>("<div class='card-body'><h5>Chart Area</h5><p>Chart would go here</p></div>"));
        
        main->addWidget(std::move(chartContainer));
        
        return main;
    }
    
    std::unique_ptr<Wt::WWidget> createSidebarContent() {
        auto sidebar = std::make_unique<Wt::WContainerWidget>();
        
        // Recent activity
        auto activityCard = std::make_unique<Wt::WContainerWidget>();
        activityCard->addStyleClass("card");
        
        auto activityHeader = std::make_unique<Wt::WText>("<div class='card-header'><h6>Recent Activity</h6></div>");
        activityHeader->setTextFormat(Wt::TextFormat::XHTML);
        
        auto activityBody = std::make_unique<Wt::WContainerWidget>();
        activityBody->addStyleClass("card-body");
        
        auto activityList = std::make_unique<Wt::WText>(
            "<ul class='list-unstyled'>"
            "<li><small>User logged in</small></li>"
            "<li><small>Report generated</small></li>"
            "<li><small>Settings updated</small></li>"
            "</ul>");
        activityList->setTextFormat(Wt::TextFormat::XHTML);
        
        activityBody->addWidget(std::move(activityList));
        activityCard->addWidget(std::move(activityHeader));
        activityCard->addWidget(std::move(activityBody));
        
        sidebar->addWidget(std::move(activityCard));
        return sidebar;
    }
    
    std::unique_ptr<Wt::WWidget> createFooterContent() {
        auto footer = std::make_unique<Wt::WText>("© 2024 My Application. All rights reserved.");
        return footer;
    }
};
```

### External Template Files

```cpp
// Load template from file
class FileTemplateExample : public Wt::WContainerWidget {
public:
    FileTemplateExample() {
        try {
            // Load template from external file
            auto template_ = std::make_unique<Wt::WTemplate>(
                Wt::WString::tr("user-profile-template"));
            
            // Bind data to template
            bindUserData(template_.get());
            
            addWidget(std::move(template_));
            
        } catch (const std::exception& e) {
            // Fallback if template loading fails
            auto errorMsg = std::make_unique<Wt::WText>("Template loading failed");
            errorMsg->addStyleClass("alert alert-danger");
            addWidget(std::move(errorMsg));
        }
    }
    
private:
    void bindUserData(Wt::WTemplate* template_) {
        // Load user data
        User user = getCurrentUser();
        
        // Bind simple values
        template_->bindString("user-name", user.name);
        template_->bindString("user-email", user.email);
        template_->bindString("member-since", user.memberSince.toString());
        
        // Bind widgets
        template_->bindWidget("user-avatar", createAvatarWidget(user));
        template_->bindWidget("edit-button", createEditButton(user.id));
        template_->bindWidget("profile-form", createProfileForm(user));
        
        // Conditional content
        if (user.isAdmin) {
            template_->setCondition("admin-panel", true);
            template_->bindWidget("admin-controls", createAdminControls());
        }
    }
    
    std::unique_ptr<Wt::WWidget> createAvatarWidget(const User& user) {
        auto avatar = std::make_unique<Wt::WImage>(user.avatarUrl);
        avatar->addStyleClass("rounded-circle");
        avatar->setAttributeValue("width", "100");
        avatar->setAttributeValue("height", "100");
        return avatar;
    }
    
    std::unique_ptr<Wt::WWidget> createEditButton(int userId) {
        auto button = std::make_unique<Wt::WPushButton>("Edit Profile");
        button->addStyleClass("btn btn-primary");
        button->clicked().connect([this, userId]() {
            editUserProfile(userId);
        });
        return button;
    }
    
    std::unique_ptr<Wt::WWidget> createProfileForm(const User& user) {
        // Create and return profile editing form
        return std::make_unique<Wt::WContainerWidget>();
    }
    
    std::unique_ptr<Wt::WWidget> createAdminControls() {
        // Create and return admin control panel
        return std::make_unique<Wt::WContainerWidget>();
    }
    
    User getCurrentUser() { /* Implementation */ return User{}; }
    void editUserProfile(int userId) { /* Implementation */ }
};

// Template file: templates/user-profile-template.xml
/*
<?xml version="1.0" encoding="UTF-8"?>
<messages>
  <message id="user-profile-template">
    <div class="user-profile">
      <div class="row">
        <div class="col-md-4 text-center">
          ${user-avatar}
          <h3>${user-name}</h3>
          <p class="text-muted">${user-email}</p>
          <p><small>Member since: ${member-since}</small></p>
          ${edit-button}
        </div>
        <div class="col-md-8">
          ${profile-form}
          
          ${#admin-panel}
          <div class="admin-section mt-4">
            <h4>Admin Controls</h4>
            ${admin-controls}
          </div>
          ${/admin-panel}
        </div>
      </div>
    </div>
  </message>
</messages>
*/
```

## Responsive Design

### Media Queries and Breakpoints

```cpp
// Responsive layout manager
class ResponsiveLayoutManager {
public:
    enum class Breakpoint { XS, SM, MD, LG, XL };
    
    static Breakpoint getCurrentBreakpoint() {
        int width = Wt::WApplication::instance()->environment().screenWidth();
        
        if (width < 576) return Breakpoint::XS;
        if (width < 768) return Breakpoint::SM;
        if (width < 992) return Breakpoint::MD;
        if (width < 1200) return Breakpoint::LG;
        return Breakpoint::XL;
    }
    
    static std::string getBootstrapClass(const std::string& base, 
                                       const std::map<Breakpoint, std::string>& sizes) {
        std::string classes = base;
        
        for (const auto& [breakpoint, size] : sizes) {
            std::string prefix;
            switch (breakpoint) {
                case Breakpoint::XS: prefix = ""; break;
                case Breakpoint::SM: prefix = "sm-"; break;
                case Breakpoint::MD: prefix = "md-"; break;
                case Breakpoint::LG: prefix = "lg-"; break;
                case Breakpoint::XL: prefix = "xl-"; break;
            }
            classes += " " + base + "-" + prefix + size;
        }
        
        return classes;
    }
};

// Responsive grid component
class ResponsiveGrid : public Wt::WContainerWidget {
public:
    ResponsiveGrid() {
        addStyleClass("container-fluid");
    }
    
    void addResponsiveRow(std::vector<ResponsiveColumn> columns) {
        auto row = std::make_unique<Wt::WContainerWidget>();
        row->addStyleClass("row");
        
        for (auto& column : columns) {
            auto col = std::make_unique<Wt::WContainerWidget>();
            col->addStyleClass(column.getBootstrapClasses());
            col->addWidget(std::move(column.content));
            row->addWidget(std::move(col));
        }
        
        addWidget(std::move(row));
    }
};

struct ResponsiveColumn {
    std::unique_ptr<Wt::WWidget> content;
    std::map<ResponsiveLayoutManager::Breakpoint, int> sizes;
    
    ResponsiveColumn(std::unique_ptr<Wt::WWidget> widget) : content(std::move(widget)) {}
    
    ResponsiveColumn& xs(int size) { sizes[ResponsiveLayoutManager::Breakpoint::XS] = size; return *this; }
    ResponsiveColumn& sm(int size) { sizes[ResponsiveLayoutManager::Breakpoint::SM] = size; return *this; }
    ResponsiveColumn& md(int size) { sizes[ResponsiveLayoutManager::Breakpoint::MD] = size; return *this; }
    ResponsiveColumn& lg(int size) { sizes[ResponsiveLayoutManager::Breakpoint::LG] = size; return *this; }
    ResponsiveColumn& xl(int size) { sizes[ResponsiveLayoutManager::Breakpoint::XL] = size; return *this; }
    
    std::string getBootstrapClasses() const {
        std::string classes = "col";
        
        for (const auto& [breakpoint, size] : sizes) {
            std::string prefix;
            switch (breakpoint) {
                case ResponsiveLayoutManager::Breakpoint::XS: prefix = ""; break;
                case ResponsiveLayoutManager::Breakpoint::SM: prefix = "-sm"; break;
                case ResponsiveLayoutManager::Breakpoint::MD: prefix = "-md"; break;
                case ResponsiveLayoutManager::Breakpoint::LG: prefix = "-lg"; break;
                case ResponsiveLayoutManager::Breakpoint::XL: prefix = "-xl"; break;
            }
            classes += " col" + prefix + "-" + std::to_string(size);
        }
        
        return classes;
    }
};

// Usage example
void createResponsiveLayout() {
    auto grid = std::make_unique<ResponsiveGrid>();
    
    // Row 1: Hero section (full width on all devices)
    grid->addResponsiveRow({
        ResponsiveColumn(createHeroSection()).xs(12)
    });
    
    // Row 2: Main content and sidebar
    grid->addResponsiveRow({
        ResponsiveColumn(createMainContent())
            .xs(12)  // Full width on extra small
            .md(8),  // 8/12 width on medium and up
        ResponsiveColumn(createSidebar())
            .xs(12)  // Full width on extra small (stacks below main)
            .md(4)   // 4/12 width on medium and up
    });
    
    // Row 3: Footer cards
    grid->addResponsiveRow({
        ResponsiveColumn(createFooterCard("Services"))
            .xs(12)  // Full width on mobile
            .sm(6)   // Half width on small
            .lg(3),  // Quarter width on large
        ResponsiveColumn(createFooterCard("About"))
            .xs(12).sm(6).lg(3),
        ResponsiveColumn(createFooterCard("Contact"))
            .xs(12).sm(6).lg(3),
        ResponsiveColumn(createFooterCard("Support"))
            .xs(12).sm(6).lg(3)
    });
}
```

## Advanced Layout Techniques

### Nested Layouts

```cpp
class NestedLayoutExample : public Wt::WContainerWidget {
public:
    NestedLayoutExample() {
        // Main vertical layout
        auto mainLayout = std::make_unique<Wt::WVBoxLayout>();
        
        // Header
        mainLayout->addWidget(createHeader(), 0);
        
        // Content area with horizontal layout
        auto contentContainer = std::make_unique<Wt::WContainerWidget>();
        auto contentLayout = std::make_unique<Wt::WHBoxLayout>();
        
        // Left sidebar
        contentLayout->addWidget(createSidebar(), 0);
        
        // Main content area with nested vertical layout
        auto mainContentContainer = std::make_unique<Wt::WContainerWidget>();
        auto mainContentLayout = std::make_unique<Wt::WVBoxLayout>();
        
        // Toolbar
        mainContentLayout->addWidget(createToolbar(), 0);
        
        // Content with tabs
        auto tabContainer = std::make_unique<Wt::WContainerWidget>();
        auto tabLayout = createTabLayout();
        tabContainer->setLayout(std::move(tabLayout));
        mainContentLayout->addWidget(std::move(tabContainer), 1);
        
        mainContentContainer->setLayout(std::move(mainContentLayout));
        contentLayout->addWidget(std::move(mainContentContainer), 1);
        
        // Right panel with grid layout
        auto rightPanel = std::make_unique<Wt::WContainerWidget>();
        auto gridLayout = createDashboardGrid();
        rightPanel->setLayout(std::move(gridLayout));
        contentLayout->addWidget(std::move(rightPanel), 0);
        
        contentContainer->setLayout(std::move(contentLayout));
        mainLayout->addWidget(std::move(contentContainer), 1);
        
        // Footer
        mainLayout->addWidget(createFooter(), 0);
        
        setLayout(std::move(mainLayout));
    }
    
private:
    std::unique_ptr<Wt::WLayout> createTabLayout() {
        // Create layout for tabbed content
        auto layout = std::make_unique<Wt::WVBoxLayout>();
        
        // Tab navigation
        auto tabNav = std::make_unique<Wt::WContainerWidget>();
        tabNav->addStyleClass("nav nav-tabs");
        // Add tab buttons...
        
        // Tab content
        auto tabContent = std::make_unique<Wt::WStackedWidget>();
        // Add tab pages...
        
        layout->addWidget(std::move(tabNav), 0);
        layout->addWidget(std::move(tabContent), 1);
        
        return layout;
    }
    
    std::unique_ptr<Wt::WLayout> createDashboardGrid() {
        auto layout = std::make_unique<Wt::WGridLayout>();
        
        // Dashboard widgets in grid
        layout->addWidget(createMetricWidget("Users", "1,234"), 0, 0);
        layout->addWidget(createMetricWidget("Sales", "$56,789"), 0, 1);
        layout->addWidget(createMetricWidget("Orders", "890"), 1, 0);
        layout->addWidget(createMetricWidget("Revenue", "$12,345"), 1, 1);
        
        return layout;
    }
    
    // Implementation methods for header, sidebar, etc.
    std::unique_ptr<Wt::WWidget> createHeader() { return std::make_unique<Wt::WContainerWidget>(); }
    std::unique_ptr<Wt::WWidget> createSidebar() { return std::make_unique<Wt::WContainerWidget>(); }
    std::unique_ptr<Wt::WWidget> createToolbar() { return std::make_unique<Wt::WContainerWidget>(); }
    std::unique_ptr<Wt::WWidget> createFooter() { return std::make_unique<Wt::WContainerWidget>(); }
    std::unique_ptr<Wt::WWidget> createMetricWidget(const std::string& label, const std::string& value) {
        return std::make_unique<Wt::WContainerWidget>();
    }
};
```

### Dynamic Layout Changes

```cpp
class DynamicLayoutExample : public Wt::WContainerWidget {
public:
    DynamicLayoutExample() {
        createInitialLayout();
        setupLayoutControls();
    }
    
private:
    Wt::WContainerWidget* contentArea_;
    std::unique_ptr<Wt::WLayout> currentLayout_;
    
    void createInitialLayout() {
        auto mainLayout = std::make_unique<Wt::WVBoxLayout>();
        
        // Controls
        auto controls = createLayoutControls();
        mainLayout->addWidget(std::move(controls), 0);
        
        // Content area
        auto contentContainer = std::make_unique<Wt::WContainerWidget>();
        contentArea_ = contentContainer.get();
        contentArea_->addStyleClass("content-area border p-3");
        contentArea_->setMinimumSize(Wt::WLength::Auto, 400);
        
        // Set initial layout
        switchToLayout(LayoutType::Vertical);
        
        mainLayout->addWidget(std::move(contentContainer), 1);
        setLayout(std::move(mainLayout));
    }
    
    enum class LayoutType { Vertical, Horizontal, Grid, Border };
    
    std::unique_ptr<Wt::WWidget> createLayoutControls() {
        auto controls = std::make_unique<Wt::WContainerWidget>();
        controls->addStyleClass("mb-3");
        
        auto buttonLayout = std::make_unique<Wt::WHBoxLayout>();
        
        auto createLayoutButton = [this](const std::string& text, LayoutType type) {
            auto button = std::make_unique<Wt::WPushButton>(text);
            button->addStyleClass("btn btn-outline-primary me-2");
            button->clicked().connect([this, type]() {
                switchToLayout(type);
            });
            return button;
        };
        
        buttonLayout->addWidget(createLayoutButton("Vertical", LayoutType::Vertical));
        buttonLayout->addWidget(createLayoutButton("Horizontal", LayoutType::Horizontal));
        buttonLayout->addWidget(createLayoutButton("Grid", LayoutType::Grid));
        buttonLayout->addWidget(createLayoutButton("Border", LayoutType::Border));
        buttonLayout->addStretch(1);
        
        controls->setLayout(std::move(buttonLayout));
        return controls;
    }
    
    void switchToLayout(LayoutType type) {
        // Clear existing content but save widgets
        std::vector<std::unique_ptr<Wt::WWidget>> widgets;
        if (contentArea_->layout()) {
            // Extract widgets from current layout
            widgets = extractWidgetsFromLayout();
        }
        
        // Create sample widgets if none exist
        if (widgets.empty()) {
            widgets = createSampleWidgets();
        }
        
        // Clear current layout
        contentArea_->setLayout(nullptr);
        
        // Create new layout based on type
        std::unique_ptr<Wt::WLayout> newLayout;
        switch (type) {
            case LayoutType::Vertical:
                newLayout = createVerticalLayout(widgets);
                break;
            case LayoutType::Horizontal:
                newLayout = createHorizontalLayout(widgets);
                break;
            case LayoutType::Grid:
                newLayout = createGridLayout(widgets);
                break;
            case LayoutType::Border:
                newLayout = createBorderLayout(widgets);
                break;
        }
        
        contentArea_->setLayout(std::move(newLayout));
    }
    
    std::vector<std::unique_ptr<Wt::WWidget>> extractWidgetsFromLayout() {
        std::vector<std::unique_ptr<Wt::WWidget>> widgets;
        
        // Remove widgets from container without deleting them
        auto children = contentArea_->children();
        for (auto child : children) {
            auto widget = contentArea_->removeWidget(child);
            if (widget) {
                widgets.push_back(std::move(widget));
            }
        }
        
        return widgets;
    }
    
    std::vector<std::unique_ptr<Wt::WWidget>> createSampleWidgets() {
        std::vector<std::unique_ptr<Wt::WWidget>> widgets;
        
        for (int i = 1; i <= 5; ++i) {
            auto widget = std::make_unique<Wt::WContainerWidget>();
            widget->addStyleClass("bg-light border p-3 m-1");
            widget->addWidget(std::make_unique<Wt::WText>("Widget " + std::to_string(i)));
            widgets.push_back(std::move(widget));
        }
        
        return widgets;
    }
    
    std::unique_ptr<Wt::WLayout> createVerticalLayout(std::vector<std::unique_ptr<Wt::WWidget>>& widgets) {
        auto layout = std::make_unique<Wt::WVBoxLayout>();
        for (auto& widget : widgets) {
            layout->addWidget(std::move(widget));
        }
        widgets.clear();
        return layout;
    }
    
    std::unique_ptr<Wt::WLayout> createHorizontalLayout(std::vector<std::unique_ptr<Wt::WWidget>>& widgets) {
        auto layout = std::make_unique<Wt::WHBoxLayout>();
        for (auto& widget : widgets) {
            layout->addWidget(std::move(widget));
        }
        widgets.clear();
        return layout;
    }
    
    std::unique_ptr<Wt::WLayout> createGridLayout(std::vector<std::unique_ptr<Wt::WWidget>>& widgets) {
        auto layout = std::make_unique<Wt::WGridLayout>();
        
        int row = 0, col = 0;
        const int maxCols = 3;
        
        for (auto& widget : widgets) {
            layout->addWidget(std::move(widget), row, col);
            
            col++;
            if (col >= maxCols) {
                col = 0;
                row++;
            }
        }
        
        widgets.clear();
        return layout;
    }
    
    std::unique_ptr<Wt::WLayout> createBorderLayout(std::vector<std::unique_ptr<Wt::WWidget>>& widgets) {
        auto layout = std::make_unique<Wt::WBorderLayout>();
        
        if (widgets.size() >= 1) layout->addWidget(std::move(widgets[0]), Wt::LayoutPosition::North);
        if (widgets.size() >= 2) layout->addWidget(std::move(widgets[1]), Wt::LayoutPosition::South);
        if (widgets.size() >= 3) layout->addWidget(std::move(widgets[2]), Wt::LayoutPosition::West);
        if (widgets.size() >= 4) layout->addWidget(std::move(widgets[3]), Wt::LayoutPosition::East);
        if (widgets.size() >= 5) layout->addWidget(std::move(widgets[4]), Wt::LayoutPosition::Center);
        
        widgets.clear();
        return layout;
    }
    
    void setupLayoutControls() {
        // Additional setup if needed
    }
};
```

## Performance Considerations

### Layout Optimization

```cpp
class OptimizedLayoutManager {
public:
    // Minimize layout reflows
    static void optimizeLayout(Wt::WContainerWidget* container) {
        // Batch DOM updates
        container->doJavaScript("document.body.style.display = 'none';");
        
        // Perform layout changes
        performLayoutChanges(container);
        
        // Re-enable rendering
        container->doJavaScript("document.body.style.display = '';");
    }
    
    // Use CSS transforms for animations instead of changing layout
    static void animateWithTransform(Wt::WWidget* widget, const std::string& transform) {
        widget->doJavaScript(
            "this.style.transform = '" + transform + "';"
            "this.style.transition = 'transform 0.3s ease';");
    }
    
    // Lazy loading for complex layouts
    static void setupLazyLoading(Wt::WContainerWidget* container) {
        container->doJavaScript(R"(
            const observer = new IntersectionObserver((entries) => {
                entries.forEach(entry => {
                    if (entry.isIntersecting) {
                        entry.target.classList.add('visible');
                        observer.unobserve(entry.target);
                    }
                });
            });
            
            this.querySelectorAll('.lazy-load').forEach(el => {
                observer.observe(el);
            });
        )");
    }
    
private:
    static void performLayoutChanges(Wt::WContainerWidget* container) {
        // Implement layout changes
    }
};

// Efficient responsive behavior
class EfficientResponsiveLayout : public Wt::WContainerWidget {
public:
    EfficientResponsiveLayout() {
        setupBaseLayout();
        setupResizeHandler();
    }
    
private:
    void setupBaseLayout() {
        addStyleClass("efficient-layout");
        
        // Use CSS Grid for efficient responsive layout
        doJavaScript(R"(
            this.style.display = 'grid';
            this.style.gridTemplateColumns = 'repeat(auto-fit, minmax(300px, 1fr))';
            this.style.gap = '20px';
        )");
    }
    
    void setupResizeHandler() {
        // Throttled resize handler to avoid excessive reflows
        doJavaScript(R"(
            let resizeTimer;
            window.addEventListener('resize', () => {
                clearTimeout(resizeTimer);
                resizeTimer = setTimeout(() => {
                    this.classList.toggle('mobile', window.innerWidth < 768);
                }, 250);
            });
        )");
    }
};
```

---

*This layout system guide provides comprehensive coverage of Wt's layout capabilities. For widget-specific details, see [Widget Reference](widget-reference.md). For responsive design patterns, see [Best Practices](best-practices.md).*
