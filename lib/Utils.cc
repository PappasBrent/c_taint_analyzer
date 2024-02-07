#include "c_taint/Utils.hh"
#include <llvm-17/llvm/ADT/StringRef.h>
#include <variant>
#include <vector>

namespace c_taint {
std::variant<std::vector<std::string>, std::string>
ParseSanitizerAnnotation(llvm::StringRef Anno) {
        // TODO
        return "unimplemented";
}
}