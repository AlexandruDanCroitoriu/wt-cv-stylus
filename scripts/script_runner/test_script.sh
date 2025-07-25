#!/bin/bash
echo "=== Test Script from Project Directory ==="
echo "Hello from test script!"
echo "Current directory: $(pwd)"
echo "Script location: $0"
echo "Testing output buffering..."
for i in {1..5}; do
    echo "Line $i: This is a test output line"
    sleep 0.5
done
echo "=== Test completed successfully! ==="
