#include "clang/AST/AST.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;

// Matches variables that are either:
// 1. Directly of type std::thread
// 2. Templated with std::thread, e.g., std::vector<std::thread>
DeclarationMatcher threadVarMatcher = varDecl(
    anyOf(
        hasType(recordDecl(hasName("std::thread"))),
        hasType(hasUnqualifiedDesugaredType(
            templateSpecializationType(hasAnyTemplateArgument(
                refersToType(recordType(hasDeclaration(recordDecl(hasName("std::thread")))))
            ))
        ))
    )
).bind("threadVar");

class ThreadVarCallback : public MatchFinder::MatchCallback {
public:
    int threadCount = 0;

    virtual void run(const MatchFinder::MatchResult &Result) {
        const VarDecl *var = Result.Nodes.getNodeAs<VarDecl>("threadVar");
        if (var) {
            ++threadCount;
            llvm::outs() << "Found std::thread variable: " << var->getNameAsString()
                         << " at " << var->getBeginLoc().printToString(*Result.SourceManager)
                         << "\n";
        }
    }
};

class ThreadFrontendAction : public ASTFrontendAction {
public:
    void EndSourceFileAction() override {
        llvm::outs() << "Total threads found: " << threadCallback.threadCount << "\n";
    }

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                   StringRef file) override {
        finder.addMatcher(threadVarMatcher, &threadCallback);
        return finder.newASTConsumer();
    }

private:
    ThreadVarCallback threadCallback;
    MatchFinder finder;
};

static llvm::cl::OptionCategory MyToolCategory("thread-detector options");

int main(int argc, const char **argv) {
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }
    CommonOptionsParser &OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(),
                   OptionsParser.getSourcePathList());
    return Tool.run(newFrontendActionFactory<ThreadFrontendAction>().get());
}
