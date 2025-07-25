#!/bin/bash
echo "=== Script Runner Test Suite ==="
echo "Testing the C++ Terminal Script Runner Application"
echo

# Test 1: Check if binary exists
echo "Test 1: Binary existence"
if [ -f "./build/bin/script_runner" ]; then
    echo "✅ Binary exists"
    ls -la ./build/bin/script_runner
else
    echo "❌ Binary not found"
    exit 1
fi
echo

# Test 2: Test version command
echo "Test 2: Version command"
./build/bin/script_runner --version
echo

# Test 3: Test help command  
echo "Test 3: Help command"
./build/bin/script_runner --help | head -10
echo

# Test 4: Test short UI run (should not produce leaked output)
echo "Test 4: Short UI test (3 seconds)"
echo "Looking for any output that leaks outside the UI..."
timeout 3s ./build/bin/script_runner 2>&1 | wc -l
echo "If the line count above is 0 or very low, the UI output leak is fixed!"
echo

echo "=== Test Suite Complete ==="
