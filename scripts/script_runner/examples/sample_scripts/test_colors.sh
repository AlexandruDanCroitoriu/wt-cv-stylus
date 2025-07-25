#!/bin/bash

# Test script to demonstrate ANSI colors
echo "Testing ANSI color support:"
echo ""

# Standard colors
echo -e "\033[31mRed text\033[0m"
echo -e "\033[32mGreen text\033[0m"
echo -e "\033[33mYellow text\033[0m"
echo -e "\033[34mBlue text\033[0m"
echo -e "\033[35mMagenta text\033[0m"
echo -e "\033[36mCyan text\033[0m"
echo -e "\033[37mWhite text\033[0m"

echo ""
echo "Bright colors:"
echo -e "\033[91mBright Red text\033[0m"
echo -e "\033[92mBright Green text\033[0m"
echo -e "\033[93mBright Yellow text\033[0m"
echo -e "\033[94mBright Blue text\033[0m"
echo -e "\033[95mBright Magenta text\033[0m"
echo -e "\033[96mBright Cyan text\033[0m"
echo -e "\033[97mBright White text\033[0m"

echo ""
echo "Bold and mixed styles:"
echo -e "\033[1;31mBold Red text\033[0m"
echo -e "\033[1;32mBold Green text\033[0m"
echo -e "\033[1;33mBold Yellow text\033[0m"

echo ""
echo "Simulating typical build output:"
echo -e "\033[32m[INFO]\033[0m Compiling source files..."
echo -e "\033[33m[WARNING]\033[0m Deprecated function used in file.cpp:42"
echo -e "\033[31m[ERROR]\033[0m Compilation failed: undefined symbol 'missing_function'"
echo -e "\033[92m[SUCCESS]\033[0m Build completed successfully!"

echo ""
echo "Panel resizing test:"
echo "Try using Ctrl+Left/Right arrows to resize the script list panel!"
