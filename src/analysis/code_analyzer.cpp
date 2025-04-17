#include "analysis/code_analyzer.h"
#include "parser/ast_types.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace cpp_diagram {

CodeAnalyzer::CodeAnalyzer() = default;
CodeAnalyzer::~CodeAnalyzer() = default;

CodeSummary CodeAnalyzer::analyzeClass(const ClassInfo& classInfo) {
    CodeSummary summary;
    summary.metrics = calculateMetrics(classInfo);
    summary.designPatterns = identifyDesignPatterns(classInfo);
    
    // Generate purpose based on class characteristics
    std::stringstream purpose;
    purpose << "The " << classInfo.name << " class ";
    
    if (classInfo.isAbstract) {
        purpose << "is an abstract base class ";
    } else if (classInfo.isTemplate) {
        purpose << "is a template class ";
    }
    
    if (!classInfo.baseClasses.empty()) {
        purpose << "that inherits from " << classInfo.baseClasses[0];
        for (size_t i = 1; i < classInfo.baseClasses.size(); ++i) {
            purpose << ", " << classInfo.baseClasses[i];
        }
        purpose << " ";
    }
    
    purpose << "with " << classInfo.methods.size() << " methods and "
            << classInfo.fields.size() << " fields.";
    
    summary.purpose = purpose.str();
    return summary;
}

CodeSummary CodeAnalyzer::analyzeFunction(const FunctionInfo& functionInfo) {
    CodeSummary summary;
    summary.metrics = calculateMetrics(functionInfo);
    summary.keyAlgorithms = identifyAlgorithms(functionInfo);
    
    // Generate purpose based on function characteristics
    std::stringstream purpose;
    purpose << "The " << functionInfo.name << " function ";
    
    if (functionInfo.isTemplate) {
        purpose << "is a template function ";
    }
    
    purpose << "takes " << functionInfo.parameters.size() << " parameters "
            << "and returns " << functionInfo.returnType << ".";
    
    summary.purpose = purpose.str();
    return summary;
}

CodeSummary CodeAnalyzer::analyzeCodebase(const std::vector<ClassInfo>& classes,
                                        const std::vector<FunctionInfo>& functions) {
    CodeSummary summary;
    
    // Calculate overall metrics
    summary.metrics.cyclomaticComplexity = 0;
    summary.metrics.linesOfCode = 0;
    summary.metrics.numberOfMethods = 0;
    summary.metrics.numberOfAttributes = 0;
    summary.metrics.coupling = 0.0;
    summary.metrics.cohesion = 0.0;
    
    for (const auto& classInfo : classes) {
        auto classMetrics = calculateMetrics(classInfo);
        summary.metrics.cyclomaticComplexity += classMetrics.cyclomaticComplexity;
        summary.metrics.linesOfCode += classMetrics.linesOfCode;
        summary.metrics.numberOfMethods += classMetrics.numberOfMethods;
        summary.metrics.numberOfAttributes += classMetrics.numberOfAttributes;
        summary.metrics.coupling += classMetrics.coupling;
        summary.metrics.cohesion += classMetrics.cohesion;
    }
    
    // Calculate averages
    if (!classes.empty()) {
        summary.metrics.coupling /= classes.size();
        summary.metrics.cohesion /= classes.size();
    }
    
    // Generate overall purpose
    std::stringstream purpose;
    purpose << "The codebase contains " << classes.size() << " classes and "
            << functions.size() << " functions. ";
    
    if (!classes.empty()) {
        purpose << "The average class has " << summary.metrics.numberOfMethods / classes.size()
                << " methods and " << summary.metrics.numberOfAttributes / classes.size()
                << " attributes.";
    }
    
    summary.purpose = purpose.str();
    return summary;
}

std::string CodeAnalyzer::generateSummary(const CodeSummary& summary, int detailLevel) {
    std::stringstream ss;
    
    // Always include the purpose
    ss << "Purpose:\n" << summary.purpose << "\n\n";
    
    if (detailLevel >= 2) {
        // Include metrics
        ss << "Metrics:\n";
        ss << "  Cyclomatic Complexity: " << summary.metrics.cyclomaticComplexity << "\n";
        ss << "  Lines of Code: " << summary.metrics.linesOfCode << "\n";
        ss << "  Number of Methods: " << summary.metrics.numberOfMethods << "\n";
        ss << "  Number of Attributes: " << summary.metrics.numberOfAttributes << "\n";
        ss << "  Coupling: " << std::fixed << std::setprecision(2) << summary.metrics.coupling << "\n";
        ss << "  Cohesion: " << std::fixed << std::setprecision(2) << summary.metrics.cohesion << "\n\n";
    }
    
    if (detailLevel >= 3) {
        // Include design patterns and algorithms
        if (!summary.designPatterns.empty()) {
            ss << "Design Patterns:\n";
            for (const auto& pattern : summary.designPatterns) {
                ss << "  - " << pattern << "\n";
            }
            ss << "\n";
        }
        
        if (!summary.keyAlgorithms.empty()) {
            ss << "Key Algorithms:\n";
            for (const auto& algorithm : summary.keyAlgorithms) {
                ss << "  - " << algorithm << "\n";
            }
            ss << "\n";
        }
    }
    
    return ss.str();
}

CodeMetrics CodeAnalyzer::calculateMetrics(const ClassInfo& classInfo) {
    CodeMetrics metrics;
    
    // Calculate cyclomatic complexity
    metrics.cyclomaticComplexity = 0;
    for (const auto& method : classInfo.methods) {
        metrics.cyclomaticComplexity += calculateCyclomaticComplexity(method);
    }
    
    // Calculate lines of code (estimated)
    metrics.linesOfCode = classInfo.methods.size() * 10; // Rough estimate
    
    // Count methods and attributes
    metrics.numberOfMethods = classInfo.methods.size();
    metrics.numberOfAttributes = classInfo.fields.size();
    
    // Calculate coupling and cohesion
    metrics.coupling = calculateCoupling(classInfo, {classInfo});
    metrics.cohesion = calculateCohesion(classInfo);
    
    return metrics;
}

CodeMetrics CodeAnalyzer::calculateMetrics(const FunctionInfo& functionInfo) {
    CodeMetrics metrics;
    
    // Calculate cyclomatic complexity
    metrics.cyclomaticComplexity = calculateCyclomaticComplexity(functionInfo);
    
    // Calculate lines of code (estimated)
    metrics.linesOfCode = 10; // Rough estimate
    
    // Other metrics are not applicable for functions
    metrics.numberOfMethods = 0;
    metrics.numberOfAttributes = 0;
    metrics.coupling = 0.0;
    metrics.cohesion = 0.0;
    
    return metrics;
}

std::vector<std::string> CodeAnalyzer::identifyDesignPatterns(const ClassInfo& classInfo) {
    std::vector<std::string> patterns;
    
    // Check for Singleton pattern
    if (classInfo.methods.size() == 1 && 
        classInfo.methods[0].isStatic && 
        classInfo.methods[0].name == "getInstance") {
        patterns.push_back("Singleton");
    }
    
    // Check for Factory pattern
    if (classInfo.name.find("Factory") != std::string::npos) {
        patterns.push_back("Factory");
    }
    
    // Check for Observer pattern
    bool hasAttach = false;
    bool hasDetach = false;
    bool hasNotify = false;
    for (const auto& method : classInfo.methods) {
        if (method.name == "attach") hasAttach = true;
        if (method.name == "detach") hasDetach = true;
        if (method.name == "notify") hasNotify = true;
    }
    if (hasAttach && hasDetach && hasNotify) {
        patterns.push_back("Observer");
    }
    
    return patterns;
}

std::vector<std::string> CodeAnalyzer::identifyAlgorithms(const FunctionInfo& functionInfo) {
    std::vector<std::string> algorithms;
    
    // Simple pattern matching for common algorithms
    if (functionInfo.name.find("sort") != std::string::npos) {
        algorithms.push_back("Sorting");
    }
    if (functionInfo.name.find("search") != std::string::npos) {
        algorithms.push_back("Searching");
    }
    if (functionInfo.name.find("traverse") != std::string::npos) {
        algorithms.push_back("Tree/Graph Traversal");
    }
    
    return algorithms;
}

int CodeAnalyzer::calculateCyclomaticComplexity(const FunctionInfo& functionInfo) {
    // This is a simplified version - in a real implementation,
    // we would analyze the function body for control flow statements
    return 1; // Base complexity
}

double CodeAnalyzer::calculateCoupling(const ClassInfo& classInfo,
                                     const std::vector<ClassInfo>& allClasses) {
    // Calculate coupling based on relationships with other classes
    int couplingCount = 0;
    for (const auto& otherClass : allClasses) {
        if (otherClass.qualifiedName != classInfo.qualifiedName) {
            // Check for inheritance
            if (std::find(classInfo.baseClasses.begin(), classInfo.baseClasses.end(),
                         otherClass.qualifiedName) != classInfo.baseClasses.end()) {
                couplingCount++;
            }
            
            // Check for method parameters and return types
            for (const auto& method : classInfo.methods) {
                for (const auto& param : method.parameters) {
                    if (param.find(otherClass.name) != std::string::npos) {
                        couplingCount++;
                    }
                }
                if (method.returnType.find(otherClass.name) != std::string::npos) {
                    couplingCount++;
                }
            }
        }
    }
    
    return static_cast<double>(couplingCount) / allClasses.size();
}

double CodeAnalyzer::calculateCohesion(const ClassInfo& classInfo) {
    // Calculate cohesion based on method interactions with class fields
    int totalInteractions = 0;
    int possibleInteractions = classInfo.methods.size() * classInfo.fields.size();
    
    if (possibleInteractions == 0) {
        return 1.0; // Perfect cohesion for empty classes
    }
    
    // This is a simplified version - in a real implementation,
    // we would analyze method bodies for field access
    return static_cast<double>(totalInteractions) / possibleInteractions;
}

} // namespace cpp_diagram 