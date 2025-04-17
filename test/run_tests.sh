#!/bin/bash

# Create output directory
mkdir -p output

# Test 1: Generate class diagram
echo "Test 1: Generating class diagram..."
./cpp_diagram_visualizer -i test/example.cpp -o output/class_diagram.svg -t class -f svg -d 2

# Test 2: Generate call graph
echo "Test 2: Generating call graph..."
./cpp_diagram_visualizer -i test/example.cpp -o output/call_graph.png -t call -f png -d 2

# Test 3: Generate component diagram
echo "Test 3: Generating component diagram..."
./cpp_diagram_visualizer -i test/example.cpp -o output/component_diagram.pdf -t component -f pdf -d 2

# Test 4: Generate detailed analysis
echo "Test 4: Generating detailed analysis..."
./cpp_diagram_visualizer -i test/example.cpp -o output/analysis.txt -t analysis -f text -d 3

echo "Tests completed. Check the output directory for results." 