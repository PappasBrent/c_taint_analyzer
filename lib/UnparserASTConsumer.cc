#include "c_taint/UnparserASTConsumer.hh"
#include "clang/AST/ASTContext.h"
#include <llvm/Support/raw_ostream.h>

namespace c_taint {
void UnparserASTConsumer::HandleTranslationUnit(clang::ASTContext &Ctx) {
        const auto TUD = Ctx.getTranslationUnitDecl();
        const auto &PP = Ctx.getPrintingPolicy();
        TUD->print(llvm::outs(), PP);
        return;
}
}