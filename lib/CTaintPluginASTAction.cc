#include "c_taint/UnparserASTConsumer.hh"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/ADT/StringRef.h"
#include <memory>

namespace c_taint {

class PluginASTAction : public clang::PluginASTAction {
    public:
        bool Unparse = false;

        PluginASTAction()
                : clang::PluginASTAction() {
        }

        PluginASTAction(PluginASTAction const &) = delete;

        PluginASTAction &operator=(PluginASTAction const &) = delete;

    private:
        bool ParseArgs(clang::CompilerInstance const &,
                       std::vector<std::string> const &Args) override {
                for (auto &Arg : Args) {
                        Unparse |= Arg == "--unparse";
                }
                return true;
        }

        std::unique_ptr<clang::ASTConsumer>
        CreateASTConsumer(clang::CompilerInstance &CI,
                          llvm::StringRef InFile) override {
                (void)CI;
                (void)InFile;
                return std::make_unique<UnparserASTConsumer>();
        }

        clang::PluginASTAction::ActionType getActionType() override {
                return clang::PluginASTAction::ActionType::AddBeforeMainAction;
        }
};

static clang::FrontendPluginRegistry::Add<PluginASTAction>
        X("c_taint_analyzer", "C89 taint analyzer");
}
