---
description: "Create a widget class following Wt CV Stylus project conventions"
tools: ['changes', 'codebase', 'editFiles', 'extensions', 'fetch', 'findTestFiles', 'githubRepo', 'new', 'openSimpleBrowser', 'problems', 'runCommands', 'runNotebooks', 'runTasks', 'runTests', 'search', 'searchResults', 'terminalLastCommand', 'terminalSelection', 'testFailure', 'usages', 'vscodeAPI']
mode: "agent"
---

# Create Widget

Create a widget class for the Wt CV Stylus project following the comprehensive guidelines in [widget-class.instructions.md](../instructions/widget-class.instructions.md).

## Input Variables

- `${input:className}`: Name of the widget class (PascalCase, e.g., "CustomButton")
- `${input:baseClass:}`: Wt base class to inherit from (if empty, defaults to Wt::WContainerWidget)
- `${input:description}`: Brief description of the widget functionality
- `${input:tailwindClasses:}`: suggested Tailwind CSS styles to apply to the widget, written in human language (e.g., "should have shadow and rounded corners")

## Widget Creation Steps

1. **Determine Folder Structure**
   - Find next available 3-digit number (000-998) in `src/` directory
   - Create folder: `src/XXX-${input:className}/`
   - **Exception**: If widget belongs solely to another widget, use that widget's folder instead

2. **Create Files Using Templates**
   - Use header template from instructions to create `${input:className}.h`
   - Use implementation template from instructions to create `${input:className}.cpp`
   - Apply appropriate base class (${input:baseClass} or Wt::WContainerWidget if empty)

3. **Update Build System (REQUIRED)**
   - Add `${SOURCE_DIR}/XXX-${input:className}/${input:className}.cpp` to CMakeLists.txt
   - Add in numerical order within `MAIN_APP_SOURCES` section

4. **Test Integration**
   - Compile project: `cmake --build build`
   - Verify widget functionality
   - Add demonstration to `008-ComponentsDisplay/` if needed

## Templates Reference

All code templates, best practices, constructor patterns, and CMakeLists.txt integration details are provided in the [widget-class.instructions.md](../instructions/widget-class.instructions.md) file.

## Example Usage After Creation

```cpp
#include "XXX-${input:className}/${input:className}.h"

auto widget = std::make_unique<${input:className}>("Content", "${input:tailwindClasses}");
```

## Success Criteria

- [ ] Widget compiles without errors
- [ ] CMakeLists.txt properly updated
- [ ] Tailwind CSS classes applied correctly
- [ ] Widget follows project naming conventions
- [ ] Widget can be instantiated and displayed
