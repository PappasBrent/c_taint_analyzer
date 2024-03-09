// RUN: unparse %s | clang-format | FileCheck %s --color --check-prefix=UNPARSE

#include <stdio.h>

void sanitize(int *x) {
        return;
}

int main(void) {
        int x;

        x = 1 + 2 * 3 - 4;

        return 0;
}

// UNPARSE: void sanitize(int *x) {
// UNPARSE:         return;
// UNPARSE: }
// UNPARSE: int main(void) {
// UNPARSE:         int x;
// UNPARSE:         x = 1 + 2 * 3 - 4;
// UNPARSE:         return 0;
// UNPARSE: }


// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | Label | Block                          | Entry(Label)                   |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | 0     | x = 1 + 2 * 3 - 4              | {}                             |
// C_TAINT: | 1     | return 0                       | {}                             |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | Label | Block                          | Exit(Label)                    |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | 0     | x = 1 + 2 * 3 - 4              | {}                             |
// C_TAINT: | 1     | return 0                       | {}                             |
// C_TAINT: |-------|--------------------------------|--------------------------------|