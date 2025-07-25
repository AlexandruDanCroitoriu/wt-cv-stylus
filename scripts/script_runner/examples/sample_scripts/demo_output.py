#!/usr/bin/env python3
"""
Demo script that produces various types of output to test the script runner's
real-time output capture and display capabilities.
"""

import time
import sys
import os

def main():
    print("=== Script Runner Demo ===")
    print(f"Script: {os.path.basename(__file__)}")
    print(f"PID: {os.getpid()}")
    print()
    
    # Test immediate output
    print("Starting demonstration...")
    time.sleep(1)
    
    # Test progress output
    for i in range(1, 11):
        print(f"Progress: {i}/10 ({i*10}%)")
        sys.stdout.flush()
        time.sleep(0.5)
    
    print()
    print("Testing different output types:")
    
    # Test stderr
    print("This goes to stdout", file=sys.stdout)
    print("This goes to stderr", file=sys.stderr)
    sys.stderr.flush()
    
    # Test long output
    print("\nGenerating long output:")
    for i in range(20):
        print(f"Line {i+1}: Lorem ipsum dolor sit amet, consectetur adipiscing elit.")
        if i % 5 == 4:
            time.sleep(0.3)
    
    # Test final status
    print("\n✓ Demo completed successfully!")
    print("Total execution time: ~8 seconds")

if __name__ == "__main__":
    main()
