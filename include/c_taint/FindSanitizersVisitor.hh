#pragma once

#include "clang/AST/Decl.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace c_taint {
class FindSanitizersVisitor
        : public clang::RecursiveASTVisitor<FindSanitizersVisitor> {
    public:
        // Maps each function decl annotated with a "sanitizes" annotation to
        // the names of the parameters it sanitizes.
        std::map<clang::FunctionDecl *, std::vector<std::string> >
                SanitizersToTargets;

        // Contains a failure message if the visitor failed somehow; otherwise,
        // if the visitor succeeds, contains nothing (i.e., is std::nullopt, the
        // empty option).
        std::optional<std::string> FailureMessage = std::nullopt;

        // Overload Clang's RecursiveASTVisitor's VisitFunctionDecl() method to
        // define our own logic for visiting function decls.
        bool VisitFunctionDecl(clang::FunctionDecl *FD);
};
}