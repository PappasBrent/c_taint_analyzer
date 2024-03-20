#pragma once

#include "clang/AST/Decl.h"
#include "clang/AST/OperationKinds.h"
#include "clang/AST/Stmt.h"
#include "clang/Basic/SourceManager.h"

namespace c_taint {
/* Checks that the sanitizer definition conforms to our grammar subset. */
bool CheckSanitizerDefinition(const clang::FunctionDecl &FD);

/* Checks that the main function definition conforms to our grammar subset. */
bool CheckMainDefinition(const clang::FunctionDecl &FD);

/* Checks that the program defines a main and sanitizer function. */
bool HasSanitizerAndMain(const clang::TranslationUnitDecl &TUD,
                         const clang::SourceManager &SM);

using BOK = clang::BinaryOperatorKind;
static const std::vector<BOK> AllowedBOKs = {
        BOK::BO_Add, BOK::BO_Sub,  BOK::BO_Mul,   BOK::BO_Div, BOK::BO_EQ,
        BOK::BO_NE,  BOK::BO_LT,   BOK::BO_GT,    BOK::BO_LE,  BOK::BO_GE,
        BOK::BO_LOr, BOK::BO_LAnd, BOK::BO_Assign
};

/* Checks that the given statement conforms to our grammar. */
bool IsInGrammar(const clang::Stmt *S);

/* Checks that the given translation unit conforms to our grammar. */
bool IsInGrammar(const clang::TranslationUnitDecl *TUD,
                 const clang::SourceManager &SM);
}