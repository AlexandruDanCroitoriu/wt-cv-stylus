# Development Guide

## Introduction
This guide provides detailed instructions on how to further develop this WT (C++) web application, including setting up the development environment, understanding the project structure, and following best practices.

## Project Structure

## Setup Instructions

### Debug Build
```sh
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build/debug && cd build/debug && make -j$(nproc)
```
### Release Build
```sh
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build/release && cd build/release && make -j$(nproc)
```
### Running the application

⚠️ **CRITICAL RULE**: The application MUST be run from the `build/debug` (or `build/release`) directory.

**CORRECT way to run (USE ABSOLUTE PATHS TO AVOID TERMINAL ISSUES):**
```sh
# ALWAYS use absolute paths to avoid terminal switching issues
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd && make run
```

**Alternative - Manual run with absolute paths:**
```sh
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd && ./app --docroot ../../ -c ../../wt_config.xml --http-address 0.0.0.0 --http-port 9020
```

**⚠️ CRITICAL**: Always use the FULL absolute path in a single command chain with `&&`

**⚠️ NEVER run the app from the project root or any other directory!**
- The app expects specific relative paths to models, config, and resources
- Library paths are configured relative to the build directory
- **ALWAYS use `pwd` to verify you're in the correct directory before running any command**

## ⚠️ PREVENTING TERMINAL SWITCHING ISSUES

**The Problem**: AI assistants may open new terminals instead of using the current directory.

**The Solution**: Always use **absolute paths** and **single command chains**:

```sh
# ✅ CORRECT - Single command with absolute path
cd /absolute/path/to/project/build/debug && pwd && make run

# ❌ WRONG - Relative paths or separate commands
cd build/debug
make run

# ❌ WRONG - May cause terminal switching
pwd
cd build/debug
make run
```

**Rule**: Combine directory change + verification + command in ONE line with `&&`

## Development Workflow

## Coding Standards

## Troubleshooting

