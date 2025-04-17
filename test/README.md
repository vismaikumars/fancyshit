# C++ Code to Diagram Visualization Tool - Test Suite

This directory contains test files and scripts to verify the functionality of the C++ Code to Diagram Visualization Tool.

## Test Files

- `example.cpp`: A comprehensive C++ file containing various class relationships and patterns
- `run_tests.sh`: Shell script to run all test cases

## Test Cases

The test suite includes the following test cases:

1. **Class Diagram Generation**
   - Input: `example.cpp`
   - Output: SVG format class diagram
   - Tests inheritance, composition, and template relationships

2. **Call Graph Generation**
   - Input: `example.cpp`
   - Output: PNG format call graph
   - Tests method call relationships

3. **Component Diagram Generation**
   - Input: `example.cpp`
   - Output: PDF format component diagram
   - Tests component relationships and dependencies

4. **Detailed Analysis**
   - Input: `example.cpp`
   - Output: Text format analysis
   - Tests detailed code analysis capabilities

## Running the Tests

1. Make sure the tool is built and available in the parent directory
2. Make the test script executable:
   ```bash
   chmod +x run_tests.sh
   ```
3. Run the test script:
   ```bash
   ./run_tests.sh
   ```
4. Check the `output` directory for generated diagrams and analysis

## Expected Results

- `output/class_diagram.svg`: Should show class relationships including:
  - Animal (abstract) → Dog (inheritance)
  - Container<T> (template)
  - Logger (singleton)
  - ShapeFactory → Circle (factory pattern)
  - Observer pattern classes

- `output/call_graph.png`: Should show method call relationships

- `output/component_diagram.pdf`: Should show component relationships

- `output/analysis.txt`: Should contain detailed code analysis

## Troubleshooting

If any test fails:
1. Check that the tool is properly built
2. Verify that all required dependencies are installed
3. Ensure the output directory has write permissions
4. Check the tool's error messages for specific issues 