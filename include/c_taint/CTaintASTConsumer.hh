#pragma once

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"

namespace c_taint {
class CTaintASTConsumer : public clang::ASTConsumer {
    public:
        void HandleTranslationUnit(clang::ASTContext &Ctx) override;
};
}