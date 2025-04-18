#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "llvm/Support/CommandLine.h"
#include <iostream>

using namespace clang;
using namespace clang::tooling;

class ThreadVisitor : public RecursiveASTVisitor<ThreadVisitor> {
public:
    explicit ThreadVisitor(ASTContext *Context) : Context(Context) {}

    bool VisitCXXConstructExpr(CXXConstructExpr *Expr) {
        if (auto CTor = Expr->getConstructor()) {
            if (auto RD = CTor->getParent()) {
                if (RD->getQualifiedNameAsString() == "std::thread") {
                    std::cout << "std::thread created at ";
                    Expr->getExprLoc().print(llvm::outs(), Context->getSourceManager());
                    std::cout << std::endl;
                    threadCount++;
                }
            }
        }
        return true;
    }

    bool VisitCallExpr(CallExpr *Call) {
        if (FunctionDecl *FD = Call->getDirectCallee()) {
            if (FD->getNameAsString() == "pthread_create") {
                std::cout << "pthread_create found at ";
                Call->getExprLoc().print(llvm::outs(), Context->getSourceManager());
                std::cout << std::endl;
                threadCount++;
            }
        }
        return true;
    }

    static int threadCount;

private:
    ASTContext *Context;
};

int ThreadVisitor::threadCount = 0;

class ThreadDetectorConsumer : public ASTConsumer {
public:
    explicit ThreadDetectorConsumer(ASTContext *Context) : Visitor(Context) {}

    void HandleTranslationUnit(ASTContext &Context) override {
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());
        std::cout << "\nTotal threads found: " << ThreadVisitor::threadCount << std::endl;
    }

private:
    ThreadVisitor Visitor;
};

class ThreadDetectorAction : public ASTFrontendAction {
public:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
        return std::make_unique<ThreadDetectorConsumer>(&CI.getASTContext());
    }
};

static llvm::cl::OptionCategory ThreadToolCategory("thread-detector options");

int main(int argc, const char **argv) {
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, ThreadToolCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }

    ClangTool Tool(ExpectedParser->getCompilations(), ExpectedParser->getSourcePathList());
    return Tool.run(newFrontendActionFactory<ThreadDetectorAction>().get());
}

