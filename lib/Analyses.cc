#include "c_taint/Analyses.hh"
#include "clang/AST/Expr.h"

namespace c_taint {
Label FreshLabel(void) {
        static Label LastLabel = 0;
        auto Result = LastLabel;
        LastLabel++;
        return Result;
}

void GetLabels(const clang::Stmt *S, LabelBlockMap &LB, BlockLabelMap &BL) {
        if (auto CS = clang::dyn_cast<clang::CompoundStmt>(S)) {
                /* Get the labels of all the compound stmt's children. This
                handles our sequencing grammar rule. */
                for (auto Child : CS->body()) {
                        const auto ChildS = clang::dyn_cast<clang::Stmt>(Child);
                        GetLabels(ChildS, LB, BL);
                }
        } else if (auto CE = clang::dyn_cast<clang::CallExpr>(S)) {
                /* Either a call to scanf() or sanitize(). */
                auto L = FreshLabel();
                LB[L] = S;
                BL[S] = L;
        } else if (auto BO = clang::dyn_cast<clang::BinaryOperator>(S)) {
                /* Only assign labels to assignment operations. */
                if (BO->getOpcode() != clang::BinaryOperatorKind::BO_Assign) {
                        return;
                }
                auto L = FreshLabel();
                LB[L] = S;
                BL[S] = L;
        } else if (auto RS = clang::dyn_cast<clang::ReturnStmt>(S)) {
                /* Return stmt. */
                auto L = FreshLabel();
                LB[L] = S;
                BL[S] = L;
        } else if (auto If = clang::dyn_cast<clang::IfStmt>(S)) {
                /* Assign a label to the condition, then get the labels for the
                then branch and else branch. */
                auto Cond = If->getCond();
                auto L = FreshLabel();
                LB[L] = Cond;
                BL[Cond] = L;

                auto Then = If->getThen();
                GetLabels(Then, LB, BL);

                auto Else = If->getElse();
                GetLabels(Else, LB, BL);
        } else if (auto W = clang::dyn_cast<clang::WhileStmt>(S)) {
                /* Assign a label to the condition, then get the labels for the
                body. */
                auto Cond = W->getCond();
                auto L = FreshLabel();
                LB[L] = Cond;
                BL[Cond] = L;

                auto Body = W->getBody();
                GetLabels(Body, LB, BL);

        } else {
        }
}

void ComputeInit(InitFunction &Init, clang::Stmt *S, LabelBlockMap &LB,
                 BlockLabelMap &BL) {
        if (auto CS = clang::dyn_cast<clang::CompoundStmt>(S)) {
                /* Visit children first. */
                for (auto Child : CS->body()) {
                        ComputeInit(Init, Child, LB, BL);
                        /* Then get init label for first non-decl stmt of
                         * compound stmt.
                         */
                        if (!clang::isa<clang::DeclStmt>(Child) &&
                            !Init.contains(S)) {
                                auto FirstChildInitLabel = Init[Child];
                                Init[S] = FirstChildInitLabel;
                        }
                }
        } else if (auto CE = clang::dyn_cast<clang::CallExpr>(S)) {
                auto L = BL.at(S);
                Init[S] = L;
        } else if (auto BO = clang::dyn_cast<clang::BinaryOperator>(S)) {
                auto L = BL.at(S);
                Init[S] = L;
        } else if (auto RS = clang::dyn_cast<clang::ReturnStmt>(S)) {
                auto L = BL.at(S);
                Init[S] = L;
        } else if (auto If = clang::dyn_cast<clang::IfStmt>(S)) {
                auto CondLabel = BL.at(If->getCond());
                Init[S] = CondLabel;
                ComputeInit(Init, If->getThen(), LB, BL);
                ComputeInit(Init, If->getElse(), LB, BL);
        } else if (auto W = clang::dyn_cast<clang::WhileStmt>(S)) {
                auto CondLabel = BL.at(W->getCond());
                Init[S] = CondLabel;
                ComputeInit(Init, W->getBody(), LB, BL);
        } else {
        }
}

void ComputeFinals(FinalsFunction &Finals, clang::Stmt *S, LabelBlockMap &LB,
                   BlockLabelMap &BL) {
        if (auto CS = clang::dyn_cast<clang::CompoundStmt>(S)) {
                /* Visit children first. */
                for (auto Child : CS->body()) {
                        ComputeFinals(Finals, Child, LB, BL);
                }
                /* Then get final labels for last non-decl stmt of compound
                 * stmt.
                 */
                if (CS->body_empty()) {
                        return;
                }
                auto LastChild = CS->body_back();
                if (!clang::isa<clang::DeclStmt>(LastChild) &&
                    !Finals.contains(S)) {
                        auto LastChildFinals = Finals[LastChild];
                        Finals[S] = LastChildFinals;
                }
        } else if (auto CE = clang::dyn_cast<clang::CallExpr>(S)) {
                auto L = BL.at(S);
                Finals[S].insert(L);
        } else if (auto BO = clang::dyn_cast<clang::BinaryOperator>(S)) {
                auto L = BL.at(S);
                Finals[S].insert(L);
        } else if (auto RS = clang::dyn_cast<clang::ReturnStmt>(S)) {
                auto L = BL.at(S);
                Finals[S].insert(L);
        } else if (auto If = clang::dyn_cast<clang::IfStmt>(S)) {
                /* Compute finals for then and else branches. */
                auto Then = If->getThen();
                auto Else = If->getElse();
                ComputeFinals(Finals, Then, LB, BL);
                ComputeFinals(Finals, Else, LB, BL);

                /* Union these finals to form finals for if stmt. We have to
                check if the Finals map actually contains finals for these
                branches because branches that are empty compound stmts will not
                have any finals. */
                if (Finals.contains(Then)) {
                        for (auto L : Finals.at(Then)) {
                                Finals[S].insert(L);
                        }
                }
                if (Finals.contains(Else)) {
                        for (auto L : Finals.at(Else)) {
                                Finals[S].insert(L);
                        }
                }
        } else if (auto W = clang::dyn_cast<clang::WhileStmt>(S)) {
                /* A while stmt's final is just its condition's final. */
                auto CondLabel = BL.at(W->getCond());
                Finals[S].insert(CondLabel);
                /* Compute finals for body stmts. */
                ComputeFinals(Finals, W->getBody(), LB, BL);
        } else {
        }
}

void ComputeFlow(FlowFunction &Flow, clang::Stmt *S, InitFunction &Init,
                 FinalsFunction &Finals) {
        if (auto CS = clang::dyn_cast<clang::CompoundStmt>(S)) {
                /* Compute flow for each child.. */
                for (auto Child : CS->body()) {
                        ComputeFlow(Flow, Child, Init, Finals);
                }
                /* Link flows of adjacent children together. */
                for (auto it = CS->body_begin(); it + 1 < CS->body_end();
                     it++) {
                        auto B1 = *it;
                        auto B2 = *(it + 1);
                        if (!(Init.contains(B2) && Finals.contains(B1))) {
                                continue;
                        }
                        auto L2 = Init.at(B2);
                        for (auto L1 : Finals.at(B1)) {
                                Flow[B1].insert({ L1, L2 });
                        }
                }
        } else if (auto CE = clang::dyn_cast<clang::CallExpr>(S)) {
                /* No flow. */
        } else if (auto BO = clang::dyn_cast<clang::BinaryOperator>(S)) {
                /* No flow. */
        } else if (auto RS = clang::dyn_cast<clang::ReturnStmt>(S)) {
                /* No flow. */
        } else if (auto If = clang::dyn_cast<clang::IfStmt>(S)) {
                /* Compute flow for then branch. */
                auto Then = If->getThen();
                ComputeFlow(Flow, Then, Init, Finals);
                if (Flow.contains(Then)) {
                        for (auto F : Flow.at(Then)) {
                                Flow[S].insert(F);
                        }
                }

                /* Compute flow for else branch. */
                auto Else = If->getElse();
                if (Flow.contains(Else)) {
                        for (auto F : Flow.at(Else)) {
                                Flow[S].insert(F);
                        }
                }

                ComputeFlow(Flow, Else, Init, Finals);
                /* Add flows connecting condition label to then and else branch
                flows. We can get the if stmt's condition label by accessing its
                init label. */
                auto L1 = Init[S];
                if (Init.contains(Then)) {
                        auto L2 = Init.at(Then);
                        Flow[S].insert({ L1, L2 });
                }
                if (Init.contains(Else)) {
                        auto L2 = Init.at(Else);
                        Flow[S].insert({ L1, L2 });
                }
        } else if (auto W = clang::dyn_cast<clang::WhileStmt>(S)) {
                /* Compute flow for body. */
                auto Body = W->getBody();
                ComputeFlow(Flow, Body, Init, Finals);
                if (Flow.contains(Body)) {
                        for (auto F : Flow.at(Body)) {
                                Flow[S].insert(F);
                        }
                }

                /* Connect the condition label to the init of the body. */
                auto L1 = Init.at(W);
                if (Init.contains(Body)) {
                        auto L2 = Init.at(Body);
                        Flow[S].insert({ L1, L2 });
                }
                /* Connect finals of body to condition label. */
                if (Finals.contains(Body)) {
                        auto L2s = Finals.at(Body);
                        for (auto L2 : L2s) {
                                Flow[S].insert({ L2, L1 });
                        }
                }

        } else {
        }
}

void ComputeKill(KillFunction &Kill, clang::Stmt *S) {
        if (auto CS = clang::dyn_cast<clang::CompoundStmt>(S)) {
                /* Compute kill of children. */
                for (const auto Child : CS->body()) {
                        ComputeKill(Kill, Child);
                }
        } else if (auto CE = clang::dyn_cast<clang::CallExpr>(S)) {
                /* Check if this is a call to sanitize(). */
                auto D = CE->getCalleeDecl();
                auto FD = clang::dyn_cast<clang::FunctionDecl>(D);
                if (!FD) {
                        return;
                }
                auto Name = FD->getName();
                if ("sanitize" != Name) {
                        return;
                }
                /* Kills its first and only argument. */
                auto Arg = CE->getArg(0);
                /* Get the address of expression in the argument. */
                auto UO = dyn_cast<clang::UnaryOperator>(Arg->IgnoreImplicit());
                auto DRE = clang::dyn_cast<clang::DeclRefExpr>(
                        UO->getSubExpr()->IgnoreCasts());
                auto KilledDecl =
                        clang::dyn_cast<clang::VarDecl>(DRE->getDecl());
                Kill[S].insert(KilledDecl);
        } else if (auto BO = clang::dyn_cast<clang::BinaryOperator>(S)) {
                /* Check if this is an assignment. */
                if (BO->getOpcode() != clang::BinaryOperatorKind::BO_Assign) {
                        return;
                }
                /* Kills the left-hand side. */
                auto LHS = BO->getLHS()->IgnoreCasts();
                /* Get the decl referenced in the LHS. */
                auto DRE = clang::dyn_cast<clang::DeclRefExpr>(LHS);
                auto KilledDecl =
                        clang::dyn_cast<clang::VarDecl>(DRE->getDecl());
                Kill[S].insert(KilledDecl);
        } else if (auto RS = clang::dyn_cast<clang::ReturnStmt>(S)) {
                /* Kills nothing. */
        } else if (auto If = clang::dyn_cast<clang::IfStmt>(S)) {
                /* Compute kill for branches. */
                ComputeKill(Kill, If->getThen());
                ComputeKill(Kill, If->getElse());
        } else if (auto W = clang::dyn_cast<clang::WhileStmt>(S)) {
                /* Compute kill for body. */
                ComputeKill(Kill, W->getBody());
        } else {
        }
}

void GetReferencedVarDecls(const clang::Stmt *S, VarSet &VDs) {
        if (auto DRE = clang::dyn_cast<clang::DeclRefExpr>(S)) {
                auto D = DRE->getDecl();
                if (auto VD = clang::dyn_cast<clang::VarDecl>(D)) {
                        VDs.insert(VD);
                }
        }

        for (const auto Child : S->children()) {
                GetReferencedVarDecls(Child, VDs);
        }
}

void ComputeGen(GenFunction &Gen, const clang::Stmt *S, BlockLabelMap &BL,
                EntryFunction &Entry) {
        if (auto CS = clang::dyn_cast<clang::CompoundStmt>(S)) {
                /* Compute gen of children. */
                for (const auto Child : CS->body()) {
                        ComputeGen(Gen, Child, BL, Entry);
                }
        } else if (auto CE = clang::dyn_cast<clang::CallExpr>(S)) {
                /* Check if this is a call to scanf(). */
                auto D = CE->getCalleeDecl();
                auto FD = clang::dyn_cast<clang::FunctionDecl>(D);
                if (!FD) {
                        return;
                }
                auto Name = FD->getName();
                if ("scanf" != Name) {
                        return;
                }
                /* Generates its second argument. */
                auto Arg = CE->getArg(1);
                /* Get the address of expression in the argument. */
                auto UO = dyn_cast<clang::UnaryOperator>(Arg->IgnoreImplicit());
                auto DRE = clang::dyn_cast<clang::DeclRefExpr>(
                        UO->getSubExpr()->IgnoreCasts());
                auto GeneratedDecl =
                        clang::dyn_cast<clang::VarDecl>(DRE->getDecl());
                Gen[S].insert(GeneratedDecl);
        } else if (auto BO = clang::dyn_cast<clang::BinaryOperator>(S)) {
                /* Check if this is an assignment. */
                if (BO->getOpcode() != clang::BinaryOperatorKind::BO_Assign) {
                        return;
                }

                /* Get the decl assigned to in the LHS. */
                auto LHS = BO->getLHS()->IgnoreCasts();
                auto DRE = clang::dyn_cast<clang::DeclRefExpr>(LHS);
                auto Assignee = clang::dyn_cast<clang::VarDecl>(DRE->getDecl());

                /* Get the decls referenced in the right hand side. */
                VarSet FV;
                GetReferencedVarDecls(BO->getRHS(), FV);

                /* Generate the decl in the LHS if the RHS contains tainted
                decls. */
                auto L = BL.at(S);
                VarSet TaintedHere;
                TaintedHere = Entry.contains(L) ? Entry.at(L) : TaintedHere;
                if (std::any_of(FV.begin(), FV.end(),
                                [&TaintedHere](const clang::VarDecl *FVD) {
                                        return TaintedHere.contains(FVD);
                                })) {
                        Gen[S].insert(Assignee);
                }
        } else if (auto RS = clang::dyn_cast<clang::ReturnStmt>(S)) {
                /* Generates nothing. */
        } else if (auto If = clang::dyn_cast<clang::IfStmt>(S)) {
                /* Compute gen for branches. */
                ComputeGen(Gen, If->getThen(), BL, Entry);
                ComputeGen(Gen, If->getElse(), BL, Entry);
        } else if (auto W = clang::dyn_cast<clang::WhileStmt>(S)) {
                /* Compute Gen for body. */
                ComputeGen(Gen, W->getBody(), BL, Entry);
        } else {
        }
}

void ComputeEntry(EntryFunction &Entry, Label L, ExitFunction &Exit,
                  FlowSet &FlowStar) {
        /* Entry[l] = {} when l = init(S*) */
        if (0 == L) {
                Entry[L] = VarSet{};
                return;
        }

        /* Entry[l] = U {Exit(l') | (l', l) in Flow(S*) } */
        for (auto [L1, L2] : FlowStar) {
                if (L2 == L) {
                        for (auto VD : Exit.at(L1)) {
                                Entry[L].insert(VD);
                        }
                }
        }
}

void ComputeExit(ExitFunction &Exit, Label L, EntryFunction &Entry,
                 LabelBlockMap &LB, KillFunction &Kill, GenFunction &Gen) {
        Exit[L] = Entry.at(L);
        auto B = LB.at(L);
        if (Kill.contains(B)) {
                for (auto Killed : Kill.at(B)) {
                        Exit[L].erase(Killed);
                }
        }
        if (Gen.contains(B)) {
                for (auto Generated : Gen.at(B)) {
                        Exit[L].insert(Generated);
                }
        }
}

void ComputeLeastFixedPoint(Block *B, EntryFunction &Entry, ExitFunction &Exit,
                            GenFunction &Gen, LabelBlockMap &LB,
                            BlockLabelMap &BL, FlowFunction &Flow,
                            KillFunction &Kill) {
        FlowSet FlowStar;
        for (auto &[_, Fs] : Flow) {
                for (auto [L1, L2] : Fs) {
                        FlowStar.insert({ L1, L2 });
                }
        }
        while (true) {
                EntryFunction NextEntry;
                ExitFunction NextExit;
                for (auto [L, _] : LB) {
                        NextEntry[L] = VarSet{};
                        NextExit[L] = VarSet{};
                }
                GenFunction NextGen;

                for (auto [L, _] : LB) {
                        ComputeEntry(NextEntry, L, Exit, FlowStar);
                        ComputeExit(NextExit, L, Entry, LB, Kill, Gen);
                }
                ComputeGen(NextGen, B, BL, Entry);
                if (Entry == NextEntry && Exit == NextExit && Gen == NextGen) {
                        break;
                } else {
                        Entry = NextEntry;
                        Exit = NextExit;
                        Gen = NextGen;
                }
        }
}

void PrintBlockLabels(const BlockLabelMap &BL,
                      const clang::PrintingPolicy &PP) {
        for (auto [B, L] : BL) {
                B->printPretty(llvm::outs(), nullptr, PP, 0, "");
                llvm::outs() << " " << L << "\n";
        }
}

void PrintInitLabelsTable(const Block *B, InitFunction &Init,
                          const clang::PrintingPolicy &PP) {
        if (Init.contains(B)) {
                auto L = Init.at(B);
                B->printPretty(llvm::outs(), nullptr, PP, 0, "");
                llvm::outs() << " " << L << "\n";
        }
        for (auto Child : B->children()) {
                PrintInitLabelsTable(Child, Init, PP);
        }
}

void PrintFinalsLabelsTable(const Block *B, FinalsFunction &Finals,
                            const clang::PrintingPolicy &PP) {
        if (Finals.contains(B)) {
                auto Ls = Finals.at(B);
                B->printPretty(llvm::outs(), nullptr, PP, 0, "");
                llvm::outs() << " {";
                unsigned i = 0;
                for (auto L : Ls) {
                        if (i != 0) {
                                llvm::outs() << ", ";
                        }
                        llvm::outs() << L;
                        i++;
                }
                llvm::outs() << "}\n";
        }
        for (auto Child : B->children()) {
                PrintFinalsLabelsTable(Child, Finals, PP);
        }
}

void PrintFlowTable(const Block *B, FlowFunction &Flow,
                    const clang::PrintingPolicy &PP) {
        if (Flow.contains(B)) {
                auto Flows = Flow.at(B);
                B->printPretty(llvm::outs(), nullptr, PP, 0, "");
                llvm::outs() << " {";
                unsigned i = 0;
                for (auto [L1, L2] : Flows) {
                        if (i != 0) {
                                llvm::outs() << ", ";
                        }
                        llvm::outs() << "(" << L1 << ", " << L2 << ")";
                        i++;
                }
                llvm::outs() << "}\n";
        }
        for (auto Child : B->children()) {
                PrintFlowTable(Child, Flow, PP);
        }
}

void PrintKillTable(const Block *B, KillFunction &Kill,
                    const clang::PrintingPolicy &PP) {
        if (Kill.contains(B)) {
                auto Killed = Kill.at(B);
                B->printPretty(llvm::outs(), nullptr, PP, 0, "");
                llvm::outs() << " {";
                unsigned i = 0;
                for (auto K : Killed) {
                        if (i != 0) {
                                llvm::outs() << ", ";
                        }
                        llvm::outs() << K->getName();
                        i++;
                }
                llvm::outs() << "}\n";
        }
        for (auto Child : B->children()) {
                PrintKillTable(Child, Kill, PP);
        }
}

void PrintGenTable(const Block *B, GenFunction &Gen,
                   const clang::PrintingPolicy &PP) {
        if (Gen.contains(B)) {
                auto Killed = Gen.at(B);
                B->printPretty(llvm::outs(), nullptr, PP, 0, "");
                llvm::outs() << " {";
                unsigned i = 0;
                for (auto K : Killed) {
                        if (i != 0) {
                                llvm::outs() << ", ";
                        }
                        llvm::outs() << K->getName();
                        i++;
                }
                llvm::outs() << "}\n";
        }
        for (auto Child : B->children()) {
                PrintGenTable(Child, Gen, PP);
        }
}

void PrintEntryOrExitTable(const Block *B, LabelVarSetMap &EntryOrExit,
                           std::string ColumnName, BlockLabelMap &BL,
                           const clang::PrintingPolicy &PP) {
        auto Table = std::vector<std::array<std::string, 3> >();
        Table.push_back({ "Label", "Block", ColumnName });

        std::function<void(const clang::Stmt *S)> Visit =
                [&](const clang::Stmt *S) -> void {
                if (BL.contains(S)) {
                        auto L = BL.at(S);

                        if (EntryOrExit.contains(L)) {
                                auto LStr = std::to_string(L);
                                std::string BlockStr;
                                auto BlockOS =
                                        llvm::raw_string_ostream(BlockStr);
                                S->printPretty(BlockOS, nullptr, PP, 0, "");

                                std::string ExitVarsStr;
                                auto ExitVarsOS =
                                        llvm::raw_string_ostream(ExitVarsStr);
                                auto ExitVars = EntryOrExit.at(L);
                                ExitVarsOS << "{";
                                unsigned i = 0;
                                for (auto VD : ExitVars) {
                                        if (i != 0) {
                                                ExitVarsOS << ", ";
                                        }
                                        ExitVarsOS << VD->getName();
                                        i++;
                                }
                                ExitVarsOS << "}";

                                Table.push_back(
                                        { LStr, BlockStr, ExitVarsStr });
                        }
                }
                for (auto Child : S->children()) {
                        Visit(Child);
                }
        };

        Visit(B);
        const unsigned LabelFieldWidth = 5;
        const unsigned BlockFieldWith = 30;
        const unsigned ExitVarsFieldWith = 30;
        bool FirstRow = true;
        auto PrintLine = [&]() {
                llvm::outs() << "|";
                for (unsigned i = 0; i < LabelFieldWidth + 2; i++) {
                        llvm::outs() << "-";
                }
                llvm::outs() << "|";
                for (unsigned i = 0; i < BlockFieldWith + 2; i++) {
                        llvm::outs() << "-";
                }
                llvm::outs() << "|";
                for (unsigned i = 0; i < ExitVarsFieldWith + 2; i++) {
                        llvm::outs() << "-";
                }
                llvm::outs() << "|";
                llvm::outs() << "\n";
        };
        PrintLine();
        for (auto [Label, Block, ExitVars] : Table) {
                llvm::outs() << "| ";
                llvm::outs() << Label;
                for (unsigned i = Label.length(); i < LabelFieldWidth; i++) {
                        llvm::outs() << " ";
                }
                llvm::outs() << " | ";
                llvm::outs() << Block;
                for (unsigned i = Block.length(); i < BlockFieldWith; i++) {
                        llvm::outs() << " ";
                }
                llvm::outs() << " | ";
                llvm::outs() << ExitVars;
                for (unsigned i = ExitVars.length(); i < ExitVarsFieldWith;
                     i++) {
                        llvm::outs() << " ";
                }
                llvm::outs() << " |";
                llvm::outs() << "\n";
                if (FirstRow) {
                        PrintLine();
                        FirstRow = false;
                }
        }
        PrintLine();
}
}