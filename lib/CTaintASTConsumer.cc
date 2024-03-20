#include "c_taint/CTaintASTConsumer.hh"
#include "c_taint/Analyses.hh"
#include "c_taint/GrammarChecks.hh"
#include "c_taint/types.hh"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Type.h"
#include "clang/Basic/LLVM.h"
#include "clang/Basic/SourceManager.h"
#include <cassert>
#include <llvm/Support/Errc.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/raw_ostream.h>
#include <string>

namespace c_taint {
const clang::FunctionDecl *GetMainFunctionDecl(clang::ASTContext &Ctx) {
        auto TUD = Ctx.getTranslationUnitDecl();
        auto &SM = Ctx.getSourceManager();
        for (auto D : TUD->decls()) {
                auto Loc = D->getLocation();
                if (!SM.isWrittenInMainFile(Loc)) {
                        continue;
                }
                const auto FD = clang::dyn_cast<clang::FunctionDecl>(D);
                if (!FD) {
                        continue;
                }
                if (FD->getName() != "main") {
                        continue;
                }
                return FD;
        }
        return nullptr;
}

CTaintASTConsumer::CTaintASTConsumer(
        bool ShouldPrintBlockLabels, bool ShouldPrintInitTable,
        bool ShouldPrintFinalsTable, bool ShouldPrintFlowTable,
        bool ShouldPrintKillTable, bool ShouldPrintGenTable,
        bool ShouldPrintEntryTable, bool ShouldPrintExitTable)
        : ShouldPrintBlockLabels(ShouldPrintBlockLabels)
        , ShouldPrintInitTable(ShouldPrintInitTable)
        , ShouldPrintFinalsTable(ShouldPrintFinalsTable)
        , ShouldPrintFlowTable(ShouldPrintFlowTable)
        , ShouldPrintKillTable(ShouldPrintKillTable)
        , ShouldPrintGenTable(ShouldPrintGenTable)
        , ShouldPrintEntryTable(ShouldPrintEntryTable)
        , ShouldPrintExitTable(ShouldPrintExitTable) {
}

void CTaintASTConsumer::HandleTranslationUnit(clang::ASTContext &Ctx) {
        auto TUD = Ctx.getTranslationUnitDecl();
        clang::SourceManager &SM = Ctx.getSourceManager();
        auto PP = clang::PrintingPolicy(Ctx.getLangOpts());
        PP.Indentation = 1;
        PP.IncludeNewlines = 0;

        if (!IsInGrammar(TUD, SM)) {
                llvm::errs() << "error: given program not in grammar\n";
                return;
        }

        auto Main = GetMainFunctionDecl(Ctx);
        auto Body = Main->getBody();
        assert(Main && Body);

        LabelBlockMap LB;
        BlockLabelMap BL;
        GetLabels(Body, LB, BL);

        /* Print blocks and their labels. */
        if (ShouldPrintBlockLabels) {
                PrintBlockLabels(BL, PP);
                llvm::errs() << "\n\n";
        }

        InitFunction Init;
        ComputeInit(Init, Body, LB, BL);

        /* Print init labels. */
        if (ShouldPrintInitTable) {
                PrintInitTable(Body, Init, PP);
                llvm::errs() << "\n\n";
        }

        FinalsFunction Finals;
        ComputeFinals(Finals, Body, LB, BL);

        /* Print final labels. */
        if (ShouldPrintFinalsTable) {
                PrintFinalsTable(Body, Finals, PP);
                llvm::errs() << "\n\n";
        }

        FlowFunction Flow;
        ComputeFlow(Flow, Body, Init, Finals);

        /* Print flow. */
        if (ShouldPrintFlowTable) {
                PrintFlowTable(Body, Flow, PP);
                llvm::errs() << "\n\n";
        }

        KillFunction Kill;
        ComputeKill(Kill, Body);

        /* Print kill. */
        if (ShouldPrintKillTable) {
                PrintKillTable(Body, Kill, PP);
                llvm::errs() << "\n\n";
        }

        GenFunction Gen;

        /* Initialize the entry and exit functions to the greatest solution (in
        this case the empty set.) */
        EntryFunction Entry;
        ExitFunction Exit;
        for (auto [L, _] : LB) {
                Entry[L] = VarSet{};
                Exit[L] = VarSet{};
        }

        ComputeLeastFixedPoint(Body, Entry, Exit, Gen, LB, BL, Flow, Kill);

        /* Print gen. */
        if (ShouldPrintGenTable) {
                PrintGenTable(Body, Gen, PP);
                llvm::errs() << "\n\n";
        }

        /* Print the least solution. */
        if (ShouldPrintEntryTable) {
                PrintEntryOrExitTable(Body, Entry, "Entry(Label)", BL, PP);
                llvm::errs() << "\n\n";
        }
        if (ShouldPrintExitTable) {
                PrintEntryOrExitTable(Body, Exit, "Exit(Label)", BL, PP);
        }
}
}
