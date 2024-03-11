#pragma once

#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"
#include <map>
#include <set>
#include <sys/types.h>
#include <utility>

namespace c_taint {
using Label = uint;
using LabelSet = std::set<Label>;
using Block = clang::Stmt;
using LabelBlockMap = std::map<Label, const Block *>;
using BlockLabelMap = std::map<const Block *, Label>;
using InitFunction = std::map<const Block *, Label>;
using FinalsFunction = std::map<const Block *, LabelSet>;
using FlowSet = std::set<std::pair<Label, Label> >;
using FlowFunction = std::map<const Block *, FlowSet>;
using VarSet = std::set<const clang::VarDecl *>;
using KillFunction = std::map<const Block *, VarSet>;
using GenFunction = std::map<const Block *, VarSet>;
using LabelVarSetMap = std::map<Label, VarSet>;
using EntryFunction = LabelVarSetMap;
using ExitFunction = LabelVarSetMap;
}