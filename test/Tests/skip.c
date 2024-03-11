// RUN: unparse %s | clang-format | FileCheck %s --color --check-prefix=UNPARSE

#include <stdio.h>

void sanitize(int *x) {
}

int main(void) {
        ;
        ;
        ;
        ;
        ;
        return 0;
}

// UNPARSE: void sanitize(int *x) {
// UNPARSE: }
// UNPARSE: int main(void) {
// UNPARSE:         ;
// UNPARSE:         ;
// UNPARSE:         ;
// UNPARSE:         ;
// UNPARSE:         ;
// UNPARSE:         return 0;
// UNPARSE: }

// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | Label | Block                          | Entry(Label)                   |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | 0     | ;                              | {}                             |
// C_TAINT: | 1     | ;                              | {}                             |
// C_TAINT: | 2     | ;                              | {}                             |
// C_TAINT: | 3     | ;                              | {}                             |
// C_TAINT: | 4     | ;                              | {}                             |
// C_TAINT: | 5     | return 0                       | {}                             |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | Label | Block                          | Exit(Label)                    |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | 0     | ;                              | {}                             |
// C_TAINT: | 1     | ;                              | {}                             |
// C_TAINT: | 2     | ;                              | {}                             |
// C_TAINT: | 3     | ;                              | {}                             |
// C_TAINT: | 4     | ;                              | {}                             |
// C_TAINT: | 5     | return 0                       | {}                             |
// C_TAINT: |-------|--------------------------------|--------------------------------|