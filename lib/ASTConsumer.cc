#include "c_taint/ASTConsumer.hh"
#include "clang/AST/ASTContext.h"
#include <llvm/Support/raw_ostream.h>

namespace c_taint {

ASTConsumer::ASTConsumer(bool Unparse)
        : Unparse(Unparse) {
}

void ASTConsumer::HandleTranslationUnit(clang::ASTContext &Ctx) {
        if (Unparse) {
                const auto TUD = Ctx.getTranslationUnitDecl();
                const auto &PP = Ctx.getPrintingPolicy();
                TUD->print(llvm::outs(), PP);
        }
}
}