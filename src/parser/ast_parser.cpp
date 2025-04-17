#include "parser/ast_parser.h"
#include "parser/ast_types.h"
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <llvm/Support/CommandLine.h>
#include <iostream>

namespace cpp_diagram {

ASTParser::ASTParser() = default;
ASTParser::~ASTParser() = default;

bool ASTParser::parseFile(const std::string& filename) {
    std::vector<std::string> files = {filename};
    return parseFiles(files);
}

bool ASTParser::parseFiles(const std::vector<std::string>& filenames) {
    try {
        std::vector<const char*> args = {
            "cpp_diagram_visualizer",
            "-std=c++17",
            "-I/usr/include",
            "-I/usr/local/include"
        };

        for (const auto& file : filenames) {
            args.push_back(file.c_str());
        }

        clang::tooling::FixedCompilationDatabase compilations(".", args);
        clang::tooling::ClangTool tool(compilations, filenames);

        return tool.run(new clang::tooling::FrontendActionFactory<ASTFrontendAction>(*this));
    } catch (const std::exception& e) {
        std::cerr << "Error parsing files: " << e.what() << std::endl;
        return false;
    }
}

std::vector<ClassInfo> ASTParser::getClassInfo() const {
    return classes_;
}

std::vector<FunctionInfo> ASTParser::getFunctionInfo() const {
    return functions_;
}

std::vector<RelationshipInfo> ASTParser::getRelationships() const {
    return relationships_;
}

void ASTParser::ASTConsumer::HandleTranslationUnit(clang::ASTContext& context) {
    ASTVisitor visitor(parser_);
    visitor.TraverseDecl(context.getTranslationUnitDecl());
}

bool ASTParser::ASTVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl* decl) {
    if (!decl->isCompleteDefinition()) {
        return true;
    }

    ClassInfo classInfo;
    classInfo.name = decl->getNameAsString();
    classInfo.qualifiedName = decl->getQualifiedNameAsString();
    classInfo.isAbstract = decl->isAbstract();
    classInfo.isTemplate = decl->isTemplated();

    // Get template parameters if it's a template
    if (classInfo.isTemplate) {
        if (auto* templateDecl = decl->getDescribedClassTemplate()) {
            for (const auto* param : templateDecl->getTemplateParameters()) {
                classInfo.templateParameters.push_back(param->getNameAsString());
            }
        }
    }

    // Get base classes
    for (const auto& base : decl->bases()) {
        if (auto* baseType = base.getType()->getAs<clang::RecordType>()) {
            classInfo.baseClasses.push_back(baseType->getDecl()->getQualifiedNameAsString());
            
            // Add inheritance relationship
            RelationshipInfo relationship;
            relationship.fromClass = classInfo.qualifiedName;
            relationship.toClass = baseType->getDecl()->getQualifiedNameAsString();
            relationship.type = RelationshipType::Inheritance;
            relationship.isBidirectional = false;
            parser_.relationships_.push_back(relationship);
        }
    }

    // Get methods
    for (const auto* method : decl->methods()) {
        MethodInfo methodInfo;
        methodInfo.name = method->getNameAsString();
        methodInfo.returnType = method->getReturnType().getAsString();
        methodInfo.isVirtual = method->isVirtual();
        methodInfo.isPureVirtual = method->isPureVirtual();
        methodInfo.isStatic = method->isStatic();
        methodInfo.isConst = method->isConst();

        // Get access specifier
        if (method->getAccess() == clang::AS_public) {
            methodInfo.access = AccessSpecifier::Public;
        } else if (method->getAccess() == clang::AS_protected) {
            methodInfo.access = AccessSpecifier::Protected;
        } else {
            methodInfo.access = AccessSpecifier::Private;
        }

        // Get parameters
        for (const auto* param : method->parameters()) {
            methodInfo.parameters.push_back(param->getType().getAsString());
        }

        classInfo.methods.push_back(methodInfo);
    }

    // Get fields
    for (const auto* field : decl->fields()) {
        FieldInfo fieldInfo;
        fieldInfo.name = field->getNameAsString();
        fieldInfo.type = field->getType().getAsString();
        fieldInfo.isStatic = field->isStatic();

        // Get access specifier
        if (field->getAccess() == clang::AS_public) {
            fieldInfo.access = AccessSpecifier::Public;
        } else if (field->getAccess() == clang::AS_protected) {
            fieldInfo.access = AccessSpecifier::Protected;
        } else {
            fieldInfo.access = AccessSpecifier::Private;
        }

        classInfo.fields.push_back(fieldInfo);
    }

    parser_.classes_.push_back(classInfo);
    return true;
}

bool ASTParser::ASTVisitor::VisitFunctionDecl(clang::FunctionDecl* decl) {
    if (!decl->isThisDeclarationADefinition()) {
        return true;
    }

    FunctionInfo functionInfo;
    functionInfo.name = decl->getNameAsString();
    functionInfo.qualifiedName = decl->getQualifiedNameAsString();
    functionInfo.returnType = decl->getReturnType().getAsString();
    functionInfo.isTemplate = decl->isTemplated();

    // Get template parameters if it's a template
    if (functionInfo.isTemplate) {
        if (auto* templateDecl = decl->getDescribedFunctionTemplate()) {
            for (const auto* param : templateDecl->getTemplateParameters()) {
                functionInfo.templateParameters.push_back(param->getNameAsString());
            }
        }
    }

    // Get parameters
    for (const auto* param : decl->parameters()) {
        functionInfo.parameters.push_back(param->getType().getAsString());
    }

    // Get called functions
    if (auto* body = decl->getBody()) {
        for (auto it = body->child_begin(); it != body->child_end(); ++it) {
            if (auto* callExpr = llvm::dyn_cast<clang::CallExpr>(*it)) {
                if (auto* callee = callExpr->getDirectCallee()) {
                    functionInfo.calledFunctions.push_back(callee->getQualifiedNameAsString());
                }
            }
        }
    }

    parser_.functions_.push_back(functionInfo);
    return true;
}

std::unique_ptr<clang::ASTConsumer> ASTParser::ASTFrontendAction::CreateASTConsumer(
    clang::CompilerInstance& compiler, llvm::StringRef file) {
    return std::make_unique<ASTConsumer>(parser_);
}

} // namespace cpp_diagram 