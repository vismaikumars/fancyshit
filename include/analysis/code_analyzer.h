#pragma once

#include <string>
#include <vector>
#include <memory>
#include "parser/ast_parser.h"

namespace cpp_diagram {

struct CodeMetrics {
    int cyclomaticComplexity;
    int linesOfCode;
    int numberOfMethods;
    int numberOfAttributes;
    double coupling;
    double cohesion;
};

struct CodeSummary {
    std::string purpose;
    std::vector<std::string> keyAlgorithms;
    std::vector<std::string> designPatterns;
    CodeMetrics metrics;
};

class CodeAnalyzer {
public:
    CodeAnalyzer();
    ~CodeAnalyzer();

    // Analyze a single class
    CodeSummary analyzeClass(const ClassInfo& classInfo);

    // Analyze a single function
    CodeSummary analyzeFunction(const FunctionInfo& functionInfo);

    // Analyze the entire codebase
    CodeSummary analyzeCodebase(const std::vector<ClassInfo>& classes,
                              const std::vector<FunctionInfo>& functions);

    // Generate natural language summary
    std::string generateSummary(const CodeSummary& summary, int detailLevel);

    // Calculate code metrics
    CodeMetrics calculateMetrics(const ClassInfo& classInfo);
    CodeMetrics calculateMetrics(const FunctionInfo& functionInfo);

private:
    // Helper methods for analysis
    std::vector<std::string> identifyDesignPatterns(const ClassInfo& classInfo);
    std::vector<std::string> identifyAlgorithms(const FunctionInfo& functionInfo);
    int calculateCyclomaticComplexity(const FunctionInfo& functionInfo);
    double calculateCoupling(const ClassInfo& classInfo,
                           const std::vector<ClassInfo>& allClasses);
    double calculateCohesion(const ClassInfo& classInfo);

    // Helper methods for summary generation
    std::string generateClassSummary(const ClassInfo& classInfo,
                                   const CodeMetrics& metrics,
                                   int detailLevel);
    std::string generateFunctionSummary(const FunctionInfo& functionInfo,
                                      const CodeMetrics& metrics,
                                      int detailLevel);
};

} // namespace cpp_diagram 