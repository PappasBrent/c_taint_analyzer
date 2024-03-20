// RUN: unparse %s | clang-format | FileCheck %s --color --check-prefix=UNPARSE
// RUN: c_taint %s | FileCheck %s --color --check-prefix=C_TAINT

#include <stdio.h>

void sanitize(int *x) {
}

int main(void) {
        int x;

        x = 1;

        return 0;
}

// UNPARSE: void sanitize(int *x) {
// UNPARSE: }
// UNPARSE: int main(void) {
// UNPARSE:         int x;
// UNPARSE:         x = 1;
// UNPARSE:         return 0;
// UNPARSE: }

// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | Label | Block                          | Entry(Label)                   |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | 0     | x = 1                          | {}                             |
// C_TAINT: | 1     | return 0;                      | {}                             |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | Label | Block                          | Exit(Label)                    |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | 0     | x = 1                          | {}                             |
// C_TAINT: | 1     | return 0;                      | {}                             |
// C_TAINT: |-------|--------------------------------|--------------------------------|