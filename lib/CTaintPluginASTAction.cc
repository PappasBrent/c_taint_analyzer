#include "c_taint/CTaintASTConsumer.hh"
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
        bool ShouldPrintBlockLabels = false;
        bool ShouldPrintInitTable = false;
        bool ShouldPrintFinalsTable = false;
        bool ShouldPrintFlowTable = false;
        bool ShouldPrintKillTable = false;
        bool ShouldPrintGenTable = false;
        bool ShouldPrintEntryTable = true;
        bool ShouldPrintExitTable = true;

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
                        ShouldPrintBlockLabels |= Arg == "--print-block-labels";
                        ShouldPrintInitTable |= Arg == "--print-init";
                        ShouldPrintFinalsTable |= Arg == "--print-finals";
                        ShouldPrintFlowTable |= Arg == "--print-flow";
                        ShouldPrintKillTable |= Arg == "--print-kill";
                        ShouldPrintGenTable |= Arg == "--print-gen";
                        ShouldPrintEntryTable |= Arg == "--print-entry";
                        ShouldPrintExitTable |= Arg == "--print-exit";

                        ShouldPrintBlockLabels &=
                                (1 - (Arg == "--no-print-block-labels"));
                        ShouldPrintInitTable &=
                                (1 - (Arg == "--no-print-init"));
                        ShouldPrintFinalsTable &=
                                (1 - (Arg == "--no-print-finals"));
                        ShouldPrintFlowTable &=
                                (1 - (Arg == "--no-print-flow"));
                        ShouldPrintKillTable &=
                                (1 - (Arg == "--no-print-kill"));
                        ShouldPrintGenTable &= (1 - (Arg == "--no-print-gen"));
                        ShouldPrintEntryTable &=
                                (1 - (Arg == "--no-print-entry"));
                        ShouldPrintExitTable &=
                                (1 - (Arg == "--no-print-exit"));
                }
                return true;
        }

        std::unique_ptr<clang::ASTConsumer>
        CreateASTConsumer(clang::CompilerInstance &CI,
                          llvm::StringRef InFile) override {
                (void)CI;
                (void)InFile;
                if (Unparse) {
                        return std::make_unique<UnparserASTConsumer>();
                }
                return std::make_unique<CTaintASTConsumer>(
                        ShouldPrintBlockLabels, ShouldPrintInitTable,
                        ShouldPrintFinalsTable, ShouldPrintFlowTable,
                        ShouldPrintKillTable, ShouldPrintGenTable,
                        ShouldPrintEntryTable, ShouldPrintExitTable);
        }

        clang::PluginASTAction::ActionType getActionType() override {
                return clang::PluginASTAction::ActionType::AddBeforeMainAction;
        }
};

static clang::FrontendPluginRegistry::Add<PluginASTAction>
        X("c_taint_analyzer", "C89 taint analyzer");
}
