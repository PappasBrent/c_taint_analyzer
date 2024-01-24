#include "c_taint/ASTConsumer.hh"
#include <llvm-17/llvm/Support/raw_ostream.h>

namespace c_taint {
ASTConsumer::ASTConsumer(clang::CompilerInstance &CI) {
        (void)CI;
        llvm::outs() << "Hello, world!\n";
}
}