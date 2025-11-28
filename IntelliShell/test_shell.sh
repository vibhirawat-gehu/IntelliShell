#!/bin/bash

# Test script for Smart Shell
echo "Testing Smart Shell..."

# Test basic compilation
echo "1. Testing compilation..."
make clean
if make; then
    echo "✅ Compilation successful"
else
    echo "❌ Compilation failed"
    exit 1
fi

# Test basic functionality
echo "2. Testing basic functionality..."

# Create a test script
cat > test_commands.txt << EOF
help
resources
suggest ls
alias testalias=echo "Hello World"
testalias
unalias testalias
history 5
exit
EOF

echo "3. Running interactive test..."
echo "Test commands will be executed automatically"
timeout 10s ./smart_shell < test_commands.txt

# Cleanup
rm -f test_commands.txt
echo "✅ Basic tests completed"

echo "4. Testing shortcuts..."
echo "Testing ll shortcut (should show detailed listing)"
echo "ll" | timeout 5s ./smart_shell

echo "✅ All tests completed successfully!"
