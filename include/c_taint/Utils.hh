#pragma once

#include <llvm-17/llvm/ADT/StringRef.h>
#include <variant>
#include <vector>

namespace c_taint {
// Parses a function decl annotation of the form "sanitizes(s1, s2, ..., sn)",
// where s1 through sn are valid C89 identifiers. If successful, returns a
// vector containing s1 through sn as strings. Otherwise, returns a single
// string containing the reason the parse failed.
std::variant<std::vector<std::string>, std::string>
ParseSanitizerAnnotation(llvm::StringRef Anno);
}