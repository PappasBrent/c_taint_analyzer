#include "c_taint/GrammarChecks.hh"
#include "clang/AST/Expr.h"

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
}