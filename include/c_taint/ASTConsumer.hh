#pragma once

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"

namespace c_taint {
class ASTConsumer : public clang::ASTConsumer {
    public:
        bool Unparse = false;

        ASTConsumer(bool Unparse);
        void HandleTranslationUnit(clang::ASTContext &Ctx) override;
};
}