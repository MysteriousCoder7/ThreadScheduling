#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "llvm/Support/CommandLine.h"
#include <set>
#include <map>
#include <fstream>
#include <iostream>

using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;

std::map<std::string, std::set<std::string>> callGraph;

class FunctionCallGraphVisitor : public RecursiveASTVisitor<FunctionCallGraphVisitor> {
public:
    explicit FunctionCallGraphVisitor(ASTContext *Context) : Context(Context) {}

    bool VisitFunctionDecl(FunctionDecl *Func) {
        if (!Func->hasBody()) return true;

        currentFunc = Func->getNameAsString();

        Stmt *Body = Func->getBody();
        TraverseStmt(Body);
        return true;
    }

    bool VisitCallExpr(CallExpr *Call) {
        if (FunctionDecl *Callee = Call->getDirectCallee()) {
            callGraph[currentFunc].insert(Callee->getNameAsString());
        }
        return true;
    }

private:
    ASTContext *Context;
    std::string currentFunc;
};

class CallGraphConsumer : public ASTConsumer {
public:
    explicit CallGraphConsumer(ASTContext *Context) : Visitor(Context) {}

    void HandleTranslationUnit(ASTContext &Context) override {
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());

        std::ofstream outFile("callgraph.txt"); // Output file

        if (!outFile.is_open()) {
            std::cerr << "Error: could not open file for writing call graph.\n";
            return;
        }

        for (auto &[caller, callees] : callGraph) {
            for (const auto &callee : callees) {
                outFile << caller << " -> " << callee << std::endl;
            }
        }

        outFile.close();
    }

private:
    FunctionCallGraphVisitor Visitor;
};

class CallGraphAction : public ASTFrontendAction {
public:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
        return std::make_unique<CallGraphConsumer>(&CI.getASTContext());
    }
};

static llvm::cl::OptionCategory ToolCategory("call-graph options");

int main(int argc, const char **argv) {
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, ToolCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }

    ClangTool Tool(ExpectedParser->getCompilations(), ExpectedParser->getSourcePathList());
    return Tool.run(newFrontendActionFactory<CallGraphAction>().get());
}
