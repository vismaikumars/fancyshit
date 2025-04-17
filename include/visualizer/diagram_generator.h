#pragma once

#include <string>
#include <vector>
#include <memory>
#include <graphviz/gvc.h>
#include "parser/ast_parser.h"

namespace cpp_diagram {

class DiagramGenerator {
public:
    DiagramGenerator();
    ~DiagramGenerator();

    // Generate a class diagram from parsed AST information
    bool generateClassDiagram(const std::vector<ClassInfo>& classes,
                            const std::vector<RelationshipInfo>& relationships,
                            const std::string& outputFile);

    // Generate a function call graph
    bool generateCallGraph(const std::vector<FunctionInfo>& functions,
                          const std::string& outputFile);

    // Generate a component diagram
    bool generateComponentDiagram(const std::vector<ClassInfo>& classes,
                                const std::string& outputFile);

    // Set diagram style options
    void setStyle(const std::string& styleName);
    void setOutputFormat(const std::string& format);

private:
    // Graphviz context
    GVC_t* gvc_;
    
    // Current style settings
    std::string style_;
    std::string outputFormat_;

    // Helper methods for graph creation
    Agraph_t* createClassGraph(const std::vector<ClassInfo>& classes,
                             const std::vector<RelationshipInfo>& relationships);
    Agraph_t* createCallGraph(const std::vector<FunctionInfo>& functions);
    Agraph_t* createComponentGraph(const std::vector<ClassInfo>& classes);

    // Helper methods for node and edge creation
    Agnode_t* createClassNode(Agraph_t* graph, const ClassInfo& classInfo);
    Agnode_t* createFunctionNode(Agraph_t* graph, const FunctionInfo& functionInfo);
    Agedge_t* createRelationshipEdge(Agraph_t* graph, Agnode_t* from, Agnode_t* to,
                                   const RelationshipInfo& relationship);
};

} // namespace cpp_diagram 