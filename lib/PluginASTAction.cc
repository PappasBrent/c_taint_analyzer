#include "c_taint/ASTConsumer.hh"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/ADT/StringRef.h"
#include <llvm-17/llvm/Support/raw_ostream.h>
#include <memory>

namespace c_taint {

// class ASTConsumer : public clang::ASTConsumer {
//     public:
//         ASTConsumer(clang::CompilerInstance &CI) {
//                 llvm::outs() << "Hello, world!\n";
//         }
// };

class PluginASTAction : public clang::PluginASTAction {
    public:
        PluginASTAction()
                : clang::PluginASTAction() {
        }

        PluginASTAction(PluginASTAction const &) = delete;

        PluginASTAction &operator=(PluginASTAction const &) = delete;

    private:
        bool ParseArgs(clang::CompilerInstance const &,
                       std::vector<std::string> const &Args) override {
                return true;
        }

        std::unique_ptr<clang::ASTConsumer>
        CreateASTConsumer(clang::CompilerInstance &CI,
                          llvm::StringRef InFile) override {
                return std::make_unique<ASTConsumer>(CI);
        }

        clang::PluginASTAction::ActionType getActionType() override {
                return clang::PluginASTAction::ActionType::AddBeforeMainAction;
        }
};

static clang::FrontendPluginRegistry::Add<PluginASTAction>
        X("c_taint_analyzer", "C89 taint analyzer");
}
