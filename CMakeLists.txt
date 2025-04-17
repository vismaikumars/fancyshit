cmake_minimum_required(VERSION 3.15)
project(CPPDiagramVisualizer VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)
find_package(Graphviz REQUIRED)

# Add executable
add_executable(cpp_diagram_visualizer
    src/main.cpp
    src/parser/ast_parser.cpp
    src/visualizer/diagram_generator.cpp
    src/analysis/code_analyzer.cpp
)

# Include directories
target_include_directories(cpp_diagram_visualizer PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
    ${GRAPHVIZ_INCLUDE_DIRS}
    include
)

# Link libraries
target_link_libraries(cpp_diagram_visualizer PRIVATE
    ${LLVM_LIBS}
    ${CLANG_LIBS}
    ${GRAPHVIZ_LIBS}
)

# Install target
install(TARGETS cpp_diagram_visualizer
    RUNTIME DESTINATION bin
) 