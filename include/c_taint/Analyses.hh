#pragma once

#include "c_taint/types.hh"

namespace c_taint {
/* Generates a fresh label. */
Label FreshLabel(void);

/* Computes the labels for the given stmt, and nested stmts for compound, if,
and while stmts. Returns true is successful, false otherwise. */
void GetLabels(const clang::Stmt *S, LabelBlockMap &LB, BlockLabelMap &BL);

/* Computes the init() function result for each grammatical construct, as
defined in our formalism. */
void ComputeInit(InitFunction &Init, clang::Stmt *S, LabelBlockMap &LB,
                 BlockLabelMap &BL);

/* Computes the finals() function result for each grammatical construct, as
defined in our formalism. */
void ComputeFinals(FinalsFunction &Finals, clang::Stmt *S, LabelBlockMap &LB,
                   BlockLabelMap &BL);

/* Computes the flow() function result for each grammatical construct, as
defined in our formalism. */
void ComputeFlow(FlowFunction &Flow, clang::Stmt *S, InitFunction &Init,
                 FinalsFunction &Finals);

/* Computes the kill() function result for each grammatical construct, as
defined in our formalism. */
void ComputeKill(KillFunction &Kill, clang::Stmt *S);

void GetReferencedVarDecls(const clang::Stmt *S, VarSet &VDs);

/* Computes the gen() function result for each grammatical construct, as defined
in our formalism. */
void ComputeGen(GenFunction &Gen, const clang::Stmt *S, BlockLabelMap &BL,
                EntryFunction &Entry);

/* Computes the Entry() function result for the given label. */
void ComputeEntry(EntryFunction &Entry, Label L, ExitFunction &Exit,
                  FlowSet &FlowStar);

/* Computes the Exit() function result for the given label. */
void ComputeExit(ExitFunction &Exit, Label L, EntryFunction &Entry,
                 LabelBlockMap &LB, KillFunction &Kill, GenFunction &Gen);

/* Computes the least fixed point for the gen() for each grammatical construct
and the Entry(), and Exit() functions for each label. */
void ComputeLeastFixedPoint(Block *B, EntryFunction &Entry, ExitFunction &Exit,
                            GenFunction &Gen, LabelBlockMap &LB,
                            BlockLabelMap &BL, FlowFunction &Flow,
                            KillFunction &Kill);

/* Prints each block and its label. */
void PrintBlockLabels(const BlockLabelMap &BL, const clang::PrintingPolicy &PP);

/* Prints the init labels for each block in a table. */
void PrintInitLabelsTable(const Block *B, InitFunction &Init,
                          const clang::PrintingPolicy &PP);

/* Prints the finals labels for each block in a table. */
void PrintFinalsLabelsTable(const Block *B, FinalsFunction &Finals,
                            const clang::PrintingPolicy &PP);

/* Prints the flow for each block in a table. */
void PrintFlowTable(const Block *B, FlowFunction &Flow,
                    const clang::PrintingPolicy &PP);

/* Print each block's set of killed variables in a table. */
void PrintKillTable(const Block *B, KillFunction &Kill,
                    const clang::PrintingPolicy &PP);

/* Print each block's set of generated variables in a table. */
void PrintGenTable(const Block *B, GenFunction &Gen,
                   const clang::PrintingPolicy &PP);

/* Prints the entry or exit function for each block in a table. */
void PrintEntryOrExitTable(const Block *B, LabelVarSetMap &EntryOrExit,
                           std::string ColumnName, BlockLabelMap &BL,
                           const clang::PrintingPolicy &PP);
}