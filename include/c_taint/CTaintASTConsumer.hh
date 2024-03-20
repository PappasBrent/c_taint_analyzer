#pragma once

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"

namespace c_taint {
class CTaintASTConsumer : public clang::ASTConsumer {
    public:
        bool ShouldPrintBlockLabels = false;
        bool ShouldPrintInitTable = false;
        bool ShouldPrintFinalsTable = false;
        bool ShouldPrintFlowTable = false;
        bool ShouldPrintKillTable = false;
        bool ShouldPrintGenTable = false;
        bool ShouldPrintEntryTable = false;
        bool ShouldPrintExitTable = false;

        CTaintASTConsumer(bool ShouldPrintBlockLabels,
                          bool ShouldPrintInitTable,
                          bool ShouldPrintFinalsTable,
                          bool ShouldPrintFlowTable, bool ShouldPrintKillTable,
                          bool ShouldPrintGenTable, bool ShouldPrintEntryTable,
                          bool ShouldPrintExitTable);

        void HandleTranslationUnit(clang::ASTContext &Ctx) override;
};
}