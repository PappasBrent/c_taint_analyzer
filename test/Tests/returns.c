// RUN: unparse %s | clang-format | FileCheck %s --color --check-prefix=UNPARSE
// RUN: c_taint %s | FileCheck %s --color --check-prefix=C_TAINT

#include <stdio.h>

void sanitize(int *x) {
        return;
        return;
}

int main(void) {
        return 1;
        return 0;
}

// UNPARSE: void sanitize(int *x) {
// UNPARSE:         return;
// UNPARSE:         return;
// UNPARSE: }
// UNPARSE: int main(void) {
// UNPARSE:         return 1;
// UNPARSE:         return 0;
// UNPARSE: }

// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | Label | Block                          | Entry(Label)                   |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | 0     | return 1;                      | {}                             |
// C_TAINT: | 1     | return 0;                      | {}                             |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | Label | Block                          | Exit(Label)                    |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | 0     | return 1;                      | {}                             |
// C_TAINT: | 1     | return 0;                      | {}                             |
// C_TAINT: |-------|--------------------------------|--------------------------------|