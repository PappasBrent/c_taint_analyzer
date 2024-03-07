#include "c_taint/CTaintASTConsumer.hh"
#include "c_taint/types.hh"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/OperationKinds.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Type.h"
#include "clang/Basic/LLVM.h"
#include "clang/Basic/SourceManager.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <llvm-17/llvm/Support/Errc.h>
#include <llvm-17/llvm/Support/Error.h>
#include <llvm/Support/raw_ostream.h>
#include <set>
#include <string>
#include <vector>

namespace c_taint {

bool CheckSanitizerDefinition(const clang::FunctionDecl &FD) {
        if (!FD.getReturnType()->isVoidType()) {
                llvm::errs() << "error: sanitize() should return void\n";
                return false;
        }
        if (1 != FD.getNumParams()) {
                llvm::errs() << "error: sanitize() should have one parameter\n";
                return false;
        }
        auto FirstParam = FD.getParamDecl(0);
        auto T = FirstParam->getType();
        if (!(T->isPointerType() && T->getPointeeType()->isIntegerType())) {
                llvm::errs()
                        << "error: sanitize() parameter should have type *int\n";
                return false;
        }
        if (FirstParam->getName() != "x") {
                llvm::errs()
                        << "error: sanitize() parameter should be named x\n";
                return false;
        }
        return true;
}

bool CheckMainDefinition(const clang::FunctionDecl &FD) {
        if (!FD.getReturnType()->isIntegerType()) {
                llvm::errs() << "error: main should return an int\n";
                return false;
        }
        if (0 != FD.getNumParams()) {
                llvm::errs() << "error: main should have no parameters\n";
                return false;
        }
        return true;
}

bool HasSanitizerAndMain(const clang::TranslationUnitDecl &TUD,
                         const clang::SourceManager &SM) {
        bool HasSanitizer = false;
        bool HasMain = false;
        bool HasFunctionNotInGrammar = false;

        for (auto D : TUD.decls()) {
                auto FD = clang::dyn_cast_or_null<clang::FunctionDecl>(D);
                if (!FD) {
                        continue;
                }
                auto Loc = FD->getLocation();
                if (!SM.isWrittenInMainFile(Loc)) {
                        continue;
                }
                auto Name = FD->getName();
                if (Name == "sanitize") {
                        HasSanitizer = CheckSanitizerDefinition(*FD);
                } else if (Name == "main") {
                        HasMain = CheckMainDefinition(*FD);
                } else {
                        llvm::errs() << "error: function \"" << Name
                                     << "\" defined but not in grammar\n";
                }
        }
        if (!HasSanitizer) {
                llvm::errs() << "error: no sanitize function\n";
        }
        if (!HasMain) {
                llvm::errs() << "error: no main function\n";
        }
        if (HasFunctionNotInGrammar) {
                llvm::errs() << "error: function not in grammar\n";
        }
        return HasSanitizer && HasMain && !HasFunctionNotInGrammar;
}

using BOK = clang::BinaryOperatorKind;
static const std::vector<BOK> AllowedBOKs = {
        BOK::BO_Add, BOK::BO_Sub,  BOK::BO_Mul,   BOK::BO_Div, BOK::BO_EQ,
        BOK::BO_NE,  BOK::BO_LT,   BOK::BO_GT,    BOK::BO_LE,  BOK::BO_GE,
        BOK::BO_LOr, BOK::BO_LAnd, BOK::BO_Assign
};

bool IsInGrammar(const clang::Stmt *S) {
        if (!clang::isa_and_nonnull<clang::CompoundStmt, clang::DeclStmt,
                                    clang::IfStmt, clang::WhileStmt,
                                    clang::ReturnStmt, clang::CallExpr,
                                    clang::IntegerLiteral, clang::DeclRefExpr,
                                    clang::StringLiteral, clang::BinaryOperator,
                                    clang::UnaryOperator>(S)) {
                llvm::errs() << "error: stmt " << S->getStmtClassName()
                             << " not in grammar\n";
                return false;
        };
        if (auto DS = clang::dyn_cast_or_null<clang::DeclStmt>(S)) {
                auto VD = clang::dyn_cast_or_null<clang::VarDecl>(
                        DS->getSingleDecl());
                if (!VD) {
                        llvm::errs() << "error: invalid variable declaration\n";
                        return false;
                }
                auto T = VD->getType();
                if (!T->isIntegerType()) {
                        llvm::errs()
                                << "error: variable declaration is not an integer\n";
                        return false;
                }
        }
        if (auto If = clang::dyn_cast_or_null<clang::IfStmt>(S)) {
                if (!If->getElse()) {
                        llvm::errs() << "error: if stmt requires else branch\n";
                        return false;
                }
        } else if (auto BO =
                           clang::dyn_cast_or_null<clang::BinaryOperator>(S)) {
                const auto OC = BO->getOpcode();
                if (std::find(AllowedBOKs.begin(), AllowedBOKs.end(), OC) ==
                    AllowedBOKs.end()) {
                        llvm::errs()
                                << "error: binary operator not in grammar\n";
                        return false;
                }
        } else if (auto UO = clang::dyn_cast_or_null<clang::UnaryOperator>(S)) {
                const auto OC = UO->getOpcode();
                if (!(OC == clang::UnaryOperatorKind::UO_Deref ||
                      OC == clang::UnaryOperatorKind::UO_AddrOf)) {
                        // NOTE(bpp): Technically we should only allow address
                        // of expressions in the arguments to calls to
                        // sanitize().
                        llvm::errs()
                                << "error: unary operator not in grammar\n";
                        return false;
                }
        } else if (auto C = clang::dyn_cast_or_null<clang::CallExpr>(S)) {
                auto D = C->getCalleeDecl();
                auto FD = clang::dyn_cast_or_null<clang::FunctionDecl>(D);
                if (!FD) {
                        return true;
                }
                auto Name = FD->getName();
                if ("scanf" == Name) {
                        if (C->getNumArgs() != 2) {
                                llvm::errs()
                                        << "error: call to scanf() passed an incorrect number of arguments\n";
                                return false;
                        }
                        auto A1 = C->getArg(0);
                        auto SL = clang::dyn_cast_or_null<clang::StringLiteral>(
                                A1->IgnoreImpCasts());
                        if (!SL) {
                                llvm::errs()
                                        << "error: call to scanf() not passed a string literal\n";
                                return false;
                        }
                        if (SL->getString() != "%d") {
                                llvm::errs()
                                        << "error: call to scanf() not passed \"%d\"\n";
                                return false;
                        }
                        auto A2 = C->getArg(1);
                        auto UO = clang::dyn_cast_or_null<clang::UnaryOperator>(
                                A2);
                        if (!(UO &&
                              UO->getOpcode() ==
                                      clang::UnaryOperatorKind::UO_AddrOf)) {
                                llvm::errs()
                                        << "error: call to scanf() not passed a pointer\n";
                                return false;
                        }
                        auto DRE = clang::dyn_cast_or_null<clang::DeclRefExpr>(
                                UO->getSubExpr());
                        if (!(DRE && DRE->getType()->isIntegerType())) {
                                llvm::errs()
                                        << "error: call to scanf() not passed a pointer to an integer variable\n";
                                return false;
                        }
                } else if ("sanitize" == Name) {
                        if (C->getNumArgs() != 1) {
                                llvm::errs()
                                        << "error: call to sanitize() passed an incorrect number of arguments\n";
                                return false;
                        }
                        auto A1 = C->getArg(0);
                        auto UO = clang::dyn_cast_or_null<clang::UnaryOperator>(
                                A1);
                        if (!(UO &&
                              UO->getOpcode() ==
                                      clang::UnaryOperatorKind::UO_AddrOf)) {
                                llvm::errs()
                                        << "error: call to sanitize() not passed a pointer\n";
                                return false;
                        }
                        auto DRE = clang::dyn_cast_or_null<clang::DeclRefExpr>(
                                UO->getSubExpr());
                        if (!(DRE && DRE->getType()->isIntegerType())) {
                                llvm::errs()
                                        << "error: call to sanitize() not passed a pointer to an integer variable\n";
                                return false;
                        }
                }
        }

        // NOTE(bpp): We should also check that "%d" is the only string literal
        // that appears, and that it only appears in calls to scanf().

        return true;
}

bool IsInGrammar(const clang::TranslationUnitDecl *TUD,
                 const clang::SourceManager &SM) {
        if (!HasSanitizerAndMain(*TUD, SM)) {
                llvm::errs() << "error: not all required functions defined\n";
                return false;
        }
        return true;
}

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

Label FreshLabel(void) {
        static Label LastLabel = 0;
        auto Result = LastLabel;
        LastLabel++;
        return Result;
}

/* Gets the labels for the given stmt, and nested stmts for compound, if, and
while stmts. Returns true is successful, false otherwise. */
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

/* Computes the init() function result for each grammatical construct, as
defined in our formalism. */
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

/* Computes the finals() function result for each grammatical construct, as
defined in our formalism. */
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

/* Computes the flow() function result for each grammatical construct, as
defined in our formalism. */
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

/* Computes the kill() function result for each grammatical construct, as
defined in our formalism. */
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

/* Computes the gen() function result for each grammatical construct, as defined
in our formalism. */
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

/* Computes the Entry() function result for the given label. */
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

/* Computes the Exit() function result for the given label. */
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

/* Computes the least fixed point for the gen() for each grammatical construct
and the Entry(), and Exit() functions for each label. */
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
        this the empty set.) */
        EntryFunction Entry;
        ExitFunction Exit;
        for (auto [L, _] : LB) {
                Entry[L] = VarSet{};
                Exit[L] = VarSet{};
        }

        ComputeLeastFixedPoint(Body, Entry, Exit, Gen, LB, BL, Flow, Kill);

        // PrintGenTable(Body, Gen, PP);
        // llvm::errs() << "\n\n";

        /* Print the least solution. */
        PrintEntryOrExitTable(Body, Entry, "Entry(Label)", BL, PP);
        llvm::errs() << "\n\n";
        PrintEntryOrExitTable(Body, Exit, "Exit(Label)", BL, PP);
}
}
