#include "visualizer/diagram_generator.h"
#include "parser/ast_types.h"
#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>
#include <iostream>
#include <fstream>

namespace cpp_diagram {

DiagramGenerator::DiagramGenerator() {
    gvc_ = gvContext();
}

DiagramGenerator::~DiagramGenerator() {
    if (gvc_) {
        gvFreeContext(gvc_);
    }
}

void DiagramGenerator::setStyle(const std::string& styleName) {
    style_ = styleName;
}

void DiagramGenerator::setOutputFormat(const std::string& format) {
    outputFormat_ = format;
}

bool DiagramGenerator::generateClassDiagram(const std::vector<ClassInfo>& classes,
                                          const std::vector<RelationshipInfo>& relationships,
                                          const std::string& outputFile) {
    Agraph_t* graph = createClassGraph(classes, relationships);
    if (!graph) {
        return false;
    }

    // Set graph attributes
    agsafeset(graph, "rankdir", "TB", "");
    agsafeset(graph, "splines", "ortho", "");
    agsafeset(graph, "nodesep", "0.5", "");
    agsafeset(graph, "ranksep", "0.5", "");

    // Layout and render the graph
    gvLayout(gvc_, graph, "dot");
    gvRenderFilename(gvc_, graph, outputFormat_.c_str(), outputFile.c_str());
    gvFreeLayout(gvc_, graph);
    agclose(graph);

    return true;
}

bool DiagramGenerator::generateCallGraph(const std::vector<FunctionInfo>& functions,
                                       const std::string& outputFile) {
    Agraph_t* graph = createCallGraph(functions);
    if (!graph) {
        return false;
    }

    // Set graph attributes
    agsafeset(graph, "rankdir", "LR", "");
    agsafeset(graph, "splines", "ortho", "");
    agsafeset(graph, "nodesep", "0.5", "");
    agsafeset(graph, "ranksep", "0.5", "");

    // Layout and render the graph
    gvLayout(gvc_, graph, "dot");
    gvRenderFilename(gvc_, graph, outputFormat_.c_str(), outputFile.c_str());
    gvFreeLayout(gvc_, graph);
    agclose(graph);

    return true;
}

bool DiagramGenerator::generateComponentDiagram(const std::vector<ClassInfo>& classes,
                                              const std::string& outputFile) {
    Agraph_t* graph = createComponentGraph(classes);
    if (!graph) {
        return false;
    }

    // Set graph attributes
    agsafeset(graph, "rankdir", "TB", "");
    agsafeset(graph, "splines", "ortho", "");
    agsafeset(graph, "nodesep", "0.5", "");
    agsafeset(graph, "ranksep", "0.5", "");

    // Layout and render the graph
    gvLayout(gvc_, graph, "dot");
    gvRenderFilename(gvc_, graph, outputFormat_.c_str(), outputFile.c_str());
    gvFreeLayout(gvc_, graph);
    agclose(graph);

    return true;
}

Agraph_t* DiagramGenerator::createClassGraph(const std::vector<ClassInfo>& classes,
                                           const std::vector<RelationshipInfo>& relationships) {
    Agraph_t* graph = agopen("ClassDiagram", Agdirected, nullptr);
    if (!graph) {
        return nullptr;
    }

    // Create nodes for each class
    std::map<std::string, Agnode_t*> classNodes;
    for (const auto& classInfo : classes) {
        Agnode_t* node = createClassNode(graph, classInfo);
        if (node) {
            classNodes[classInfo.qualifiedName] = node;
        }
    }

    // Create edges for relationships
    for (const auto& relationship : relationships) {
        auto fromIt = classNodes.find(relationship.fromClass);
        auto toIt = classNodes.find(relationship.toClass);
        
        if (fromIt != classNodes.end() && toIt != classNodes.end()) {
            createRelationshipEdge(graph, fromIt->second, toIt->second, relationship);
        }
    }

    return graph;
}

Agraph_t* DiagramGenerator::createCallGraph(const std::vector<FunctionInfo>& functions) {
    Agraph_t* graph = agopen("CallGraph", Agdirected, nullptr);
    if (!graph) {
        return nullptr;
    }

    // Create nodes for each function
    std::map<std::string, Agnode_t*> functionNodes;
    for (const auto& functionInfo : functions) {
        Agnode_t* node = createFunctionNode(graph, functionInfo);
        if (node) {
            functionNodes[functionInfo.qualifiedName] = node;
        }
    }

    // Create edges for function calls
    for (const auto& functionInfo : functions) {
        auto fromIt = functionNodes.find(functionInfo.qualifiedName);
        if (fromIt != functionNodes.end()) {
            for (const auto& calledFunction : functionInfo.calledFunctions) {
                auto toIt = functionNodes.find(calledFunction);
                if (toIt != functionNodes.end()) {
                    Agedge_t* edge = agedge(graph, fromIt->second, toIt->second, nullptr, 1);
                    if (edge) {
                        agsafeset(edge, "label", "calls", "");
                    }
                }
            }
        }
    }

    return graph;
}

Agraph_t* DiagramGenerator::createComponentGraph(const std::vector<ClassInfo>& classes) {
    Agraph_t* graph = agopen("ComponentDiagram", Agdirected, nullptr);
    if (!graph) {
        return nullptr;
    }

    // Create nodes for each class as components
    for (const auto& classInfo : classes) {
        Agnode_t* node = agnode(graph, classInfo.qualifiedName.c_str(), 1);
        if (node) {
            agsafeset(node, "shape", "component", "");
            agsafeset(node, "label", classInfo.name.c_str(), "");
        }
    }

    return graph;
}

Agnode_t* DiagramGenerator::createClassNode(Agraph_t* graph, const ClassInfo& classInfo) {
    Agnode_t* node = agnode(graph, classInfo.qualifiedName.c_str(), 1);
    if (!node) {
        return nullptr;
    }

    // Set node attributes
    agsafeset(node, "shape", "record", "");
    agsafeset(node, "style", "filled", "");
    agsafeset(node, "fillcolor", "lightgray", "");

    // Create label with class name and members
    std::string label = "{ " + classInfo.name;
    if (classInfo.isTemplate) {
        label += "\\<";
        for (size_t i = 0; i < classInfo.templateParameters.size(); ++i) {
            if (i > 0) label += ", ";
            label += classInfo.templateParameters[i];
        }
        label += "\\>";
    }
    label += " | ";

    // Add fields
    for (const auto& field : classInfo.fields) {
        std::string accessSymbol;
        switch (field.access) {
            case AccessSpecifier::Public: accessSymbol = "+"; break;
            case AccessSpecifier::Protected: accessSymbol = "#"; break;
            case AccessSpecifier::Private: accessSymbol = "-"; break;
        }
        label += accessSymbol + field.name + " : " + field.type + "\\l";
    }

    // Add methods
    for (const auto& method : classInfo.methods) {
        std::string accessSymbol;
        switch (method.access) {
            case AccessSpecifier::Public: accessSymbol = "+"; break;
            case AccessSpecifier::Protected: accessSymbol = "#"; break;
            case AccessSpecifier::Private: accessSymbol = "-"; break;
        }
        label += accessSymbol + method.name + "(";
        for (size_t i = 0; i < method.parameters.size(); ++i) {
            if (i > 0) label += ", ";
            label += method.parameters[i];
        }
        label += ") : " + method.returnType;
        if (method.isVirtual) label += " (virtual)";
        if (method.isPureVirtual) label += " = 0";
        if (method.isStatic) label += " (static)";
        if (method.isConst) label += " const";
        label += "\\l";
    }

    label += "}";
    agsafeset(node, "label", label.c_str(), "");

    return node;
}

Agnode_t* DiagramGenerator::createFunctionNode(Agraph_t* graph, const FunctionInfo& functionInfo) {
    Agnode_t* node = agnode(graph, functionInfo.qualifiedName.c_str(), 1);
    if (!node) {
        return nullptr;
    }

    // Set node attributes
    agsafeset(node, "shape", "box", "");
    agsafeset(node, "style", "filled", "");
    agsafeset(node, "fillcolor", "lightblue", "");

    // Create label with function signature
    std::string label = functionInfo.name + "(";
    for (size_t i = 0; i < functionInfo.parameters.size(); ++i) {
        if (i > 0) label += ", ";
        label += functionInfo.parameters[i];
    }
    label += ") : " + functionInfo.returnType;
    if (functionInfo.isTemplate) {
        label += "\\<";
        for (size_t i = 0; i < functionInfo.templateParameters.size(); ++i) {
            if (i > 0) label += ", ";
            label += functionInfo.templateParameters[i];
        }
        label += "\\>";
    }

    agsafeset(node, "label", label.c_str(), "");

    return node;
}

Agedge_t* DiagramGenerator::createRelationshipEdge(Agraph_t* graph, Agnode_t* from, Agnode_t* to,
                                                const RelationshipInfo& relationship) {
    Agedge_t* edge = agedge(graph, from, to, nullptr, 1);
    if (!edge) {
        return nullptr;
    }

    // Set edge attributes based on relationship type
    switch (relationship.type) {
        case RelationshipType::Inheritance:
            agsafeset(edge, "arrowhead", "empty", "");
            break;
        case RelationshipType::Composition:
            agsafeset(edge, "arrowhead", "diamond", "");
            break;
        case RelationshipType::Aggregation:
            agsafeset(edge, "arrowhead", "odiamond", "");
            break;
        case RelationshipType::Association:
            agsafeset(edge, "arrowhead", "vee", "");
            break;
        case RelationshipType::Dependency:
            agsafeset(edge, "arrowhead", "vee", "");
            agsafeset(edge, "style", "dashed", "");
            break;
    }

    if (!relationship.label.empty()) {
        agsafeset(edge, "label", relationship.label.c_str(), "");
    }

    return edge;
}

} // namespace cpp_diagram 