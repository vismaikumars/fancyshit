#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <cxxopts.hpp>
#include "parser/ast_parser.h"
#include "visualizer/diagram_generator.h"
#include "analysis/code_analyzer.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    try {
        cxxopts::Options options("cpp_diagram_visualizer",
                               "C++ Code to Diagram Visualization Tool");

        options.add_options()
            ("i,input", "Input C++ source files", cxxopts::value<std::vector<std::string>>())
            ("o,output", "Output directory for diagrams", cxxopts::value<std::string>())
            ("t,type", "Diagram type (class, call, component)", cxxopts::value<std::string>())
            ("f,format", "Output format (png, svg, pdf)", cxxopts::value<std::string>()->default_value("png"))
            ("s,style", "Diagram style", cxxopts::value<std::string>()->default_value("default"))
            ("d,detail", "Detail level (1-3)", cxxopts::value<int>()->default_value("2"))
            ("h,help", "Print usage");

        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            return 0;
        }

        if (!result.count("input") || !result.count("output") || !result.count("type")) {
            std::cerr << "Error: Missing required arguments" << std::endl;
            std::cout << options.help() << std::endl;
            return 1;
        }

        // Create output directory if it doesn't exist
        fs::path outputDir(result["output"].as<std::string>());
        if (!fs::exists(outputDir)) {
            fs::create_directories(outputDir);
        }

        // Initialize components
        cpp_diagram::ASTParser parser;
        cpp_diagram::DiagramGenerator diagramGenerator;
        cpp_diagram::CodeAnalyzer analyzer;

        // Parse input files
        auto inputFiles = result["input"].as<std::vector<std::string>>();
        if (!parser.parseFiles(inputFiles)) {
            std::cerr << "Error: Failed to parse input files" << std::endl;
            return 1;
        }

        // Get parsed information
        auto classes = parser.getClassInfo();
        auto functions = parser.getFunctionInfo();
        auto relationships = parser.getRelationships();

        // Set diagram style and format
        diagramGenerator.setStyle(result["style"].as<std::string>());
        diagramGenerator.setOutputFormat(result["format"].as<std::string>());

        // Generate requested diagram type
        std::string diagramType = result["type"].as<std::string>();
        std::string outputFile = (outputDir / (diagramType + "." + result["format"].as<std::string>())).string();

        bool success = false;
        if (diagramType == "class") {
            success = diagramGenerator.generateClassDiagram(classes, relationships, outputFile);
        } else if (diagramType == "call") {
            success = diagramGenerator.generateCallGraph(functions, outputFile);
        } else if (diagramType == "component") {
            success = diagramGenerator.generateComponentDiagram(classes, outputFile);
        } else {
            std::cerr << "Error: Unknown diagram type: " << diagramType << std::endl;
            return 1;
        }

        if (!success) {
            std::cerr << "Error: Failed to generate diagram" << std::endl;
            return 1;
        }

        // Generate code analysis summary
        auto summary = analyzer.analyzeCodebase(classes, functions);
        std::string summaryText = analyzer.generateSummary(summary, result["detail"].as<int>());

        // Write summary to file
        std::ofstream summaryFile(outputDir / "summary.txt");
        if (summaryFile.is_open()) {
            summaryFile << summaryText;
            summaryFile.close();
        }

        std::cout << "Successfully generated " << diagramType << " diagram and analysis summary" << std::endl;
        return 0;

    } catch (const cxxopts::OptionException& e) {
        std::cerr << "Error parsing options: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
} 