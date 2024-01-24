#pragma once

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/Frontend/CompilerInstance.h"

namespace c_taint {
class ASTConsumer : public clang::ASTConsumer {
    public:
        ASTConsumer(clang::CompilerInstance &CI);
};
}