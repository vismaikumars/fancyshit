# C++ Code to Diagram Visualization Tool

A tool for generating UML diagrams and visualizations from C++ source code.

## Features

- Parse C++ source files and extract class, function, and relationship information
- Generate various types of diagrams:
  - Class diagrams
  - Function call graphs
  - Component diagrams
- Analyze code metrics and generate summaries
- Support for multiple output formats (PNG, SVG, PDF)
- Customizable diagram styles

## Requirements

- C++17 compatible compiler
- CMake 3.15 or higher
- LLVM/Clang libraries
- Graphviz
- cxxopts (for command-line argument parsing)

## Building

1. Install dependencies:
   ```bash
   # On Ubuntu/Debian
   sudo apt-get install llvm clang graphviz

   # On Windows (using vcpkg)
   vcpkg install llvm:x64-windows
   vcpkg install graphviz:x64-windows
   ```

2. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/cpp-diagram-visualizer.git
   cd cpp-diagram-visualizer
   ```

3. Build the project:
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

## Usage

Basic usage:
```bash
cpp_diagram_visualizer -i input.cpp -o output_dir -t class
```

Command-line options:
- `-i, --input`: Input C++ source files (required)
- `-o, --output`: Output directory for diagrams (required)
- `-t, --type`: Diagram type (class, call, component) (required)
- `-f, --format`: Output format (png, svg, pdf) (default: png)
- `-s, --style`: Diagram style (default: default)
- `-d, --detail`: Detail level (1-3) (default: 2)
- `-h, --help`: Print usage information

## Examples

Generate a class diagram:
```bash
cpp_diagram_visualizer -i src/*.cpp -o diagrams -t class -f svg
```

Generate a call graph with high detail:
```bash
cpp_diagram_visualizer -i src/*.cpp -o diagrams -t call -d 3
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the LICENSE file for details. 