---
description: "Delete a widget class from the Wt CV Stylus project following proper cleanup procedures"
tools: ['changes', 'codebase', 'editFiles', 'extensions', 'fetch', 'findTestFiles', 'githubRepo', 'new', 'openSimpleBrowser', 'problems', 'runCommands', 'runNotebooks', 'runTasks', 'runTests', 'search', 'searchResults', 'terminalLastCommand', 'terminalSelection', 'testFailure', 'usages', 'vscodeAPI']
mode: "agent"
---

# Delete Widget

Safely remove a widget class from the Wt CV Stylus project following the comprehensive cleanup guidelines in [widget-class.instructions.md](../instructions/widget-class.instructions.md).

## Input Variables

- `${input:className}`: Name of the widget class to delete (PascalCase, e.g., "CustomButton")
- `${input:folderNumber:}`: 3-digit folder number (e.g., "010") - if empty, will search for the widget
- `${input:confirmDeletion:true}`: Safety confirmation (must be "true" to proceed with deletion)

## Widget Deletion Steps

1. **Locate Widget Files**
   - Find widget folder: `src/XXX-${input:className}/` (use ${input:folderNumber} if provided)
   - Verify files exist: `${input:className}.h` and `${input:className}.cpp`
   - Search for widget if folder number not provided

2. **Analyze Dependencies (CRITICAL)**
   - Search entire codebase for `#include "XXX-${input:className}/${input:className}.h"`
   - Find all instantiations of `${input:className}` class
   - Identify any derived classes that inherit from this widget
   - Check for references in documentation or comments

3. **Clean Dependencies First**
   - Remove all `#include` statements referencing the widget
   - Remove all instantiations and usage of the widget class
   - Update any code that depends on this widget
   - Remove from any component displays or demonstrations

4. **Update Build System (REQUIRED)**
   - Remove `${SOURCE_DIR}/XXX-${input:className}/${input:className}.cpp` from CMakeLists.txt
   - Remove from `MAIN_APP_SOURCES` section maintaining numerical order

5. **Delete Widget Files**
   - Delete `src/XXX-${input:className}/${input:className}.h`
   - Delete `src/XXX-${input:className}/${input:className}.cpp`
   - Remove `src/XXX-${input:className}/` directory if empty

6. **Verify Clean Removal**
   - Compile project: `cmake --build build`
   - Ensure no compilation errors
   - Verify no broken references remain

## Safety Checks

**BEFORE DELETION**:
- [ ] All dependencies identified and handled
- [ ] No other widgets inherit from this class
- [ ] Confirmation variable set to "true"
- [ ] Backup created if needed

**AFTER DELETION**:
- [ ] Project compiles without errors
- [ ] No broken includes or references
- [ ] CMakeLists.txt properly updated
- [ ] All dependent code updated or removed

## Dependency Search Commands

Use these patterns to find all references:
- Header includes: `#include "XXX-${input:className}/${input:className}.h"`
- Class usage: `${input:className}` (class name)
- Instantiation patterns: `std::make_unique<${input:className}>` or `new ${input:className}`

## Example Removal Process

```bash
# 1. Find all references (example)
grep -r "#include \"010-CustomButton/CustomButton.h\"" src/
grep -r "CustomButton" src/

# 2. After cleaning dependencies, verify build
cmake --build build
```

## Rollback Plan

If deletion causes issues:
1. Restore files from version control
2. Re-add CMakeLists.txt entry
3. Restore any removed dependencies
4. Rebuild project

## Success Criteria

- [ ] Widget files completely removed
- [ ] CMakeLists.txt properly updated
- [ ] All dependencies cleaned up
- [ ] Project compiles without errors
- [ ] No broken references remain
- [ ] Folder removed if empty

**⚠️ Warning**: This operation is destructive. Ensure all dependencies are properly handled before proceeding.
