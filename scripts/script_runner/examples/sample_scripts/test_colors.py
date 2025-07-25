#!/usr/bin/env python3

# Test script for ANSI colors in Python
import time
import sys

def print_colored(text, color_code):
    print(f"\033[{color_code}m{text}\033[0m")

def main():
    print("Python ANSI Color Test")
    print("=" * 25)
    
    # Standard colors
    colors = [
        ("Red", "31"),
        ("Green", "32"), 
        ("Yellow", "33"),
        ("Blue", "34"),
        ("Magenta", "35"),
        ("Cyan", "36"),
        ("White", "37"),
    ]
    
    for name, code in colors:
        print_colored(f"{name} text", code)
        time.sleep(0.5)
    
    print("\nBright colors:")
    bright_colors = [
        ("Bright Red", "91"),
        ("Bright Green", "92"),
        ("Bright Yellow", "93"),
        ("Bright Blue", "94"),
        ("Bright Magenta", "95"),
        ("Bright Cyan", "96"),
        ("Bright White", "97"),
    ]
    
    for name, code in bright_colors:
        print_colored(f"{name} text", code)
        time.sleep(0.5)
    
    print("\nReal-time log simulation:")
    log_entries = [
        ("INFO", "32", "Application started successfully"),
        ("DEBUG", "36", "Loading configuration file"),
        ("WARNING", "33", "Configuration file not found, using defaults"),
        ("ERROR", "31", "Failed to connect to database"),
        ("INFO", "32", "Retrying connection..."),
        ("SUCCESS", "92", "Connected to database successfully"),
    ]
    
    for level, color, message in log_entries:
        print_colored(f"[{level}]", color) # This should show in color
        print(f" {message}")
        time.sleep(1)
    
    print("\nColor test completed!")

if __name__ == "__main__":
    main()
