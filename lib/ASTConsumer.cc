#include "c_taint/ASTConsumer.hh"
#include "c_taint/FindSanitizersVisitor.hh"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/Frontend/CompilerInstance.h"
#include <llvm-17/llvm/Support/raw_ostream.h>

namespace c_taint {
ASTConsumer::ASTConsumer(clang::CompilerInstance &CI) {
        (void)CI;
}

void ASTConsumer::HandleTranslationUnit(clang::ASTContext &Ctx) {
        // Create a recursive AST visitor to find all sanitizer functions.
        auto Finder = FindSanitizersVisitor();
        // Get the AST's translation unit decl (the program's top-level decl).
        auto TUD = Ctx.getTranslationUnitDecl();
        // Make the sanitizer finder visit the translation unit to find all
        // sanitizers in the program.
        Finder.TraverseTranslationUnitDecl(TUD);

        // Check if the Finder encountered an error, and exit early if so.
        if (Finder.FailureMessage) {
                llvm::errs() << *Finder.FailureMessage << "\n";
                return;
        }

        // Iterate the found sanitizer functions and their arguments that they
        // sanitize.
        for (auto [FD, Targets] : Finder.SanitizersToTargets) {
                // NOTE(bpp): Right now we just print the found sanitizers for
                // debugging purposes.
                llvm::outs() << "Sanitizer function `" << FD->getName()
                             << "` has targets: [";
                int i = 0;
                for (auto Target : Targets) {
                        if (i > 0) {
                                llvm::outs() << ", ";
                        }
                        llvm::outs() << Target;
                        i++;
                }
                llvm::outs() << "]\n";
        }
}
}