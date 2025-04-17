#pragma once

#include <string>
#include <vector>
#include <memory>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>

namespace cpp_diagram {

// Forward declarations
struct ClassInfo;
struct FunctionInfo;
struct RelationshipInfo;

class ASTParser {
public:
    ASTParser();
    ~ASTParser();

    // Parse a single C++ source file
    bool parseFile(const std::string& filename);

    // Parse multiple C++ source files
    bool parseFiles(const std::vector<std::string>& filenames);

    // Get parsed class information
    std::vector<ClassInfo> getClassInfo() const;

    // Get parsed function information
    std::vector<FunctionInfo> getFunctionInfo() const;

    // Get relationships between classes
    std::vector<RelationshipInfo> getRelationships() const;

private:
    class ASTConsumer : public clang::ASTConsumer {
    public:
        explicit ASTConsumer(ASTParser& parser) : parser_(parser) {}
        void HandleTranslationUnit(clang::ASTContext& context) override;

    private:
        ASTParser& parser_;
    };

    class ASTVisitor : public clang::RecursiveASTVisitor<ASTVisitor> {
    public:
        explicit ASTVisitor(ASTParser& parser) : parser_(parser) {}
        bool VisitCXXRecordDecl(clang::CXXRecordDecl* decl);
        bool VisitFunctionDecl(clang::FunctionDecl* decl);

    private:
        ASTParser& parser_;
    };

    class ASTFrontendAction : public clang::ASTFrontendAction {
    public:
        explicit ASTFrontendAction(ASTParser& parser) : parser_(parser) {}
        std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
            clang::CompilerInstance& compiler, llvm::StringRef file) override;

    private:
        ASTParser& parser_;
    };

    std::vector<ClassInfo> classes_;
    std::vector<FunctionInfo> functions_;
    std::vector<RelationshipInfo> relationships_;
};

} // namespace cpp_diagram 