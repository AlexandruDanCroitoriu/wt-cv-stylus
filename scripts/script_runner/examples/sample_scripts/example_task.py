#!/usr/bin/env python3
"""
Example Python script demonstrating good practices for Script Runner.

This script shows how to:
- Provide informative output
- Handle errors gracefully
- Report progress for long operations
- Use proper exit codes
"""

import sys
import time
import random
from datetime import datetime

def log_message(message, level="INFO"):
    """Log a message with timestamp and level."""
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    print(f"[{timestamp}] {level}: {message}")
    sys.stdout.flush()

def simulate_work(duration, task_name):
    """Simulate work with progress reporting."""
    log_message(f"Starting {task_name}")
    
    steps = 10
    step_duration = duration / steps
    
    for i in range(steps + 1):
        progress = (i / steps) * 100
        log_message(f"{task_name} progress: {progress:.1f}%")
        
        if i < steps:
            time.sleep(step_duration)
            
            # Simulate occasional warnings
            if random.random() < 0.1:
                log_message(f"Minor issue in {task_name} (step {i+1})", "WARNING")
    
    log_message(f"Completed {task_name}")

def main():
    """Main execution function."""
    try:
        log_message("Python example script starting")
        log_message(f"Python version: {sys.version}")
        
        # Simulate different types of work
        tasks = [
            ("Data Processing", 3),
            ("File Analysis", 2),
            ("Report Generation", 4)
        ]
        
        for task_name, duration in tasks:
            simulate_work(duration, task_name)
            
            # Small delay between tasks
            time.sleep(0.5)
        
        # Simulate final summary
        log_message("All tasks completed successfully")
        log_message("Summary: 3 tasks executed, 0 errors")
        
        return 0
        
    except KeyboardInterrupt:
        log_message("Script interrupted by user", "WARNING")
        return 130  # Standard exit code for Ctrl+C
        
    except Exception as e:
        log_message(f"Unexpected error: {e}", "ERROR")
        return 1

if __name__ == "__main__":
    exit_code = main()
    sys.exit(exit_code)
