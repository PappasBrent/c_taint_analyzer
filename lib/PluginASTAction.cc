#include "c_taint/ASTConsumer.hh"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/ADT/StringRef.h"
#include <memory>

namespace c_taint {

class PluginASTAction : public clang::PluginASTAction {
    public:
        PluginASTAction()
                : clang::PluginASTAction() {
        }

        PluginASTAction(PluginASTAction const &) = delete;

        PluginASTAction &operator=(PluginASTAction const &) = delete;

    private:
        // Hook for defining custom command line argument parsing for this
        // plugin.
        bool ParseArgs(clang::CompilerInstance const &,
                       std::vector<std::string> const &Args) override {
                return true;
        }

        // Create the plugin's AST consumer so that it can do its thing.
        std::unique_ptr<clang::ASTConsumer>
        CreateASTConsumer(clang::CompilerInstance &CI,
                          llvm::StringRef InFile) override {
                return std::make_unique<ASTConsumer>(CI);
        }

        // We use this to tell Clang when we want to perform our action. In this
        // case, we perform our action before the main action so that we can run
        // our taint analysis even if we don't compile the target translation
        // unit (e.g. with the command line option -fsyntax-only).
        clang::PluginASTAction::ActionType getActionType() override {
                return clang::PluginASTAction::ActionType::AddBeforeMainAction;
        }
};

// Register the plugin with clang.
static clang::FrontendPluginRegistry::Add<PluginASTAction>
        X("c_taint_analyzer", "C89 taint analyzer");
}
