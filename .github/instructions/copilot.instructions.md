---
applyTo: '**'
---

# Project context reference:
## Project build, run, ... via scripts. Read  [script-creation.instructions.md](./script-creation.instructions.md)
## Add new widget class instructions: [widget-class.instructions.md](./widget-class.instructions.md)
## add new scripts: [script-creation.instructions.md](./script-creation.instructions.md)

## Available Copilot Prompts as Tools

The project includes specialized prompt files in `.github/prompts/` that serve as tools for common development tasks:

### 📋 **Script Creation**
- **File**: `script-creation.prompt.md`
- **Purpose**: Create new scripts following project standards and templates
- **Usage**: Use this prompt when creating any new bash script for the project

### 🏗️ **Widget Creation** 
- **File**: `create-widget.prompt.md`
- **Purpose**: Create new widget classes following Wt CV Stylus conventions
- **Usage**: Use this prompt when adding new UI components or widgets

### 🗑️ **Widget Deletion**
- **File**: `delete-widget.prompt.md` 
- **Purpose**: Safely remove widget classes and clean up related files
- **Usage**: Use this prompt when removing existing widget classes

**Note**: These prompts are designed to be invoked as VS Code Copilot tools and include proper YAML frontmatter with tool configurations and input variables.

## Adding New Prompts

When creating a new prompt file in `.github/prompts/`, remember to:
1. **Update this documentation** by adding the new prompt to the list above
2. **Follow the established naming pattern** (`action-target.prompt.md`)
3. **Include proper YAML frontmatter** with mode, tools, and description fields



## Project Overview [README.md](../../README.md)
