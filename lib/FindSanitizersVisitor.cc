#include "c_taint/FindSanitizersVisitor.hh"
#include "c_taint/Utils.hh"
#include "clang/AST/Attrs.inc"
#include "clang/AST/Decl.h"
#include <llvm-17/llvm/ADT/StringRef.h>
#include <string>
#include <variant>
#include <vector>

namespace c_taint {
bool FindSanitizersVisitor::VisitFunctionDecl(clang::FunctionDecl *FD) {
        // Check that the function decl we are visiting has an annotate
        // attribute.
        if (auto AnnoAttr = FD->getAttr<clang::AnnotateAttr>()) {
                // Get the annotation itself.
                auto Anno = AnnoAttr->getAnnotation();
                // Try to parse the annotation.
                auto MaybeTargets = ParseSanitizerAnnotation(Anno);

                if (auto Targets = std::get_if<std::vector<std::string> >(
                            &MaybeTargets)) {
                        // If we succesfully parsed the annotation, then map its
                        // corresponding function decl to the names of its
                        // sanitization targets.
                        SanitizersToTargets.insert({ FD, *Targets });
                } else {
                        // Otherwise, populate the visitor's failure message and
                        // stop visiting the AST early by returning false.
                        auto Name = FD->getName().str();
                        FailureMessage =
                                "error parsing sanitizer annotation for \"" +
                                Name +
                                "\": " + std::get<std::string>(MaybeTargets);
                        return false;
                }
        }
        return true;
}
}