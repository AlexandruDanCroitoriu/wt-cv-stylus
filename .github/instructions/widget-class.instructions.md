---
applyTo: "src/*-*/**"
---

# Widget Class Instructions - Wt CV Stylus Project

## Overview
Guidelines for creating new widget classes in the Wt CV Stylus project using numbered folders and Tailwind CSS styling.

## File Structure
- **Folder**: `src/XXX-ClassName/` (where XXX is next available 3-digit number 000-998)
- **Header**: `src/XXX-ClassName/ClassName.h`
- **Implementation**: `src/XXX-ClassName/ClassName.cpp`

## Templates

### Header File (`ClassName.h`)
```cpp
#pragma once
#include <Wt/WContainerWidget.h> // Replace with appropriate Wt base class

class ClassName : public Wt::WContainerWidget // Replace with appropriate base class
{
public:
    ClassName();
    
protected:
    // Protected members if needed

private:
    // Private members
};
```

### Implementation File (`ClassName.cpp`)
```cpp
#include "xxx-ClassName/ClassName.h" // or "xxx-ClassName/MemberClassName.h"
#include <Wt/WApplication.h>

ClassName::ClassName() // or MemberClassName::MemberClassName
    : Wt::WContainerWidget()
{
    
    // Default styling
    addStyleClass("bg-surface-alt text-on-surface-alt");
    
    // Additional initialization logic here
}
```

## Best Practices

### Constructor Parameters
1. **Content parameter**: Primary content (text, data, etc.) with default
2. **Tailwind classes**: CSS styling with empty default
3. Always provide sensible defaults

### Code Style
- Use PascalCase for class names
- Use camelCase for method names
- Include `<Wt/WApplication.h>` if using wApp
- Prefer `const std::string&` for string parameters

### Common Wt Base Classes
- `Wt::WContainerWidget` - General container (default choice)
- `Wt::WPushButton` - Clickable buttons
- `Wt::WLineEdit` - Text input fields
- `Wt::WComboBox` - Dropdown selections
- `Wt::WText` - Text display

### Tailwind CSS Integration
```cpp

// Apply default project styling
addStyleClass("bg-surface-alt text-on-surface-alt");
```

## Integration

### CMakeLists.txt Integration (Required)
After creating widget files, **MUST** update CMakeLists.txt:

1. **Add source files** to `MAIN_APP_SOURCES` section:
```cmake
# In CMakeLists.txt, add to MAIN_APP_SOURCES:
${SOURCE_DIR}/XXX-ClassName/ClassName.cpp
```

2. **Follow existing pattern** by adding entries in numerical order:
```cmake
# Example: if creating 010-MyWidget/MyWidget.cpp
set(MAIN_APP_SOURCES
    ${SOURCE_DIR}/main.cpp
    # ... existing entries ...
    ${SOURCE_DIR}/009-SomeExistingWidget/SomeExistingWidget.cpp
    ${SOURCE_DIR}/010-MyWidget/MyWidget.cpp  # <-- Add here
    ${SOURCE_DIR}/011-AnotherWidget/AnotherWidget.cpp
    # ... rest of entries ...
)
```

### Widget Removal Process
To completely remove a widget from the project:

1. **Remove from CMakeLists.txt**:
   - Delete the line `${SOURCE_DIR}/XXX-ClassName/ClassName.cpp` from `MAIN_APP_SOURCES`

2. **Delete widget files**:
   - Remove `src/XXX-ClassName/ClassName.h`
   - Remove `src/XXX-ClassName/ClassName.cpp`

3. **Remove folder if empty**:
   - Delete `src/XXX-ClassName/` directory if no other files remain

4. **Clean references**:
   - Remove any `#include "XXX-ClassName/ClassName.h"` from other files
   - Remove instantiations and usage of the widget class
