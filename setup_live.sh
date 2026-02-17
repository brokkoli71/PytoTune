#!/bin/bash
# Setup script for PytoTune Live Mode

echo "=================================================="
echo "PytoTune Live Mode Setup"
echo "=================================================="
echo ""

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: Please run this script from the PytoTune root directory"
    exit 1
fi

echo "Step 1: Building PytoTune C++ library..."
echo "--------------------------------------------------"
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
if [ $? -ne 0 ]; then
    echo "Error: CMake configuration failed"
    exit 1
fi

cmake --build cmake-build-release -j$(nproc)
if [ $? -ne 0 ]; then
    echo "Error: Build failed"
    exit 1
fi

echo ""
echo "Step 2: Installing Python dependencies..."
echo "--------------------------------------------------"
pip install -r requirements_live.txt
if [ $? -ne 0 ]; then
    echo "Error: Failed to install Python dependencies"
    exit 1
fi

echo ""
echo "Step 3: Running tests..."
echo "--------------------------------------------------"
python test_bindings.py
if [ $? -ne 0 ]; then
    echo "Warning: Tests failed, but continuing..."
fi

echo ""
echo "=================================================="
echo "Setup complete!"
echo "=================================================="
echo ""
echo "Try the examples:"
echo "  python example_array_api.py"
echo ""
echo "Start live autotune:"
echo "  python live_autotune.py --scale \"C major\""
echo ""
echo "List audio devices:"
echo "  python live_autotune.py --list-devices"
echo ""
echo "For more information, see LIVE_MODE.md"
echo ""
