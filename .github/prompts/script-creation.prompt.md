---
mode: agent
tools: ['context7']
description: "Guidelines for creating scripts following project standards"
---

# Script Creation Prompt

You are tasked with creating a new script for the Wt CV Stylus project. This script must follow the established patterns and standards defined in the project.

## Instructions Reference

Please follow all guidelines specified in:
📋 **[Script Creation Instructions](.github/instructions/script-creation.instructions.md)**

## Key Requirements Summary

### Mandatory Elements
- **Use the exact template structure** provided in the instructions
- **Implement all required logging functions** with proper color coding
- **Follow naming conventions** (kebab-case, descriptive names)
- **Include proper error handling** with `set -e`
- **Add colorized help function** with usage examples
- **Log all output** to `scripts/output/[script-name].log`

### Integration Principles
- **Reuse existing scripts** instead of duplicating functionality
- **Use proper permission handling** for system-level operations
- **Follow established patterns** for build/run/Docker scripts
- **Maintain consistency** with existing script ecosystem

### Script Categories
Choose the appropriate category and follow its specific patterns:
- **Core Scripts** (build.sh, run.sh) - Essential workflow
- **Utility Scripts** - Supporting development tasks  
- **System Setup Scripts** - Environment preparation
- **Docker Scripts** - Container management

### Permission Handling
If your script requires system permissions:
- **Implement smart detection** (try without sudo first)
- **Provide clear feedback** when privileges are needed
- **Use environment detection** for Docker vs host systems
- **Follow established permission patterns** from the instructions

### Before Submitting
Validate your script against the checklist in the instructions:
- Template structure compliance
- Required functions implementation
- Error handling and logging
- Permission handling (if applicable)
- Naming conventions
- Integration with existing scripts
- Documentation updates (if needed)

## Script Context

The Wt CV Stylus project uses a **script-driven development philosophy** where all operations are performed through unified scripts. Your new script should integrate seamlessly with this ecosystem and follow the established patterns for consistency and maintainability.

Refer to existing scripts in the `scripts/` directory and the comprehensive documentation in [Script Creation Instructions](.github/instructions/script-creation.instructions.md) for examples and patterns.
