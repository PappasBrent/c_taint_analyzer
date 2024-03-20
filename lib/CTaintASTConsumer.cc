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
        // PrintBlockLabels(BL, PP);

        InitFunction Init;
        ComputeInit(Init, Body, LB, BL);

        /* Print init labels. */
        // PrintInitLabelsTable(Body, Init, PP);

        FinalsFunction Finals;
        ComputeFinals(Finals, Body, LB, BL);

        /* Print final labels. */
        // PrintFinalsLabelsTable(Body, Finals, PP);

        FlowFunction Flow;
        ComputeFlow(Flow, Body, Init, Finals);

        /* Print flow. */
        // PrintFlowTable(Body, Flow, PP);

        KillFunction Kill;
        ComputeKill(Kill, Body);

        /* Print kill. */
        // PrintKillTable(Body, Kill, PP);

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

        /* Print the least solution. */
        PrintEntryOrExitTable(Body, Entry, "Entry(Label)", BL, PP);
        llvm::errs() << "\n\n";
        PrintEntryOrExitTable(Body, Exit, "Exit(Label)", BL, PP);
}
}
