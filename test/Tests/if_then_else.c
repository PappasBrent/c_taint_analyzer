// RUN: unparse %s | clang-format | FileCheck %s --color --check-prefix=UNPARSE
// RUN: c_taint %s | FileCheck %s --color --check-prefix=C_TAINT

#include <stdio.h>

void sanitize(int *x) {
        if (*x < 0) {
                *x = 0;
        } else {
        }
}

int main(void) {
        int x;

        scanf("%d", &x);
        sanitize(&x);

        return 0;
}

// UNPARSE: void sanitize(int *x) {
// UNPARSE:         if (*x < 0) {
// UNPARSE:                 *x = 0;
// UNPARSE:         } else {
// UNPARSE:         }
// UNPARSE: }
// UNPARSE: int main(void) {
// UNPARSE:         int x;
// UNPARSE:         scanf("%d", &x);
// UNPARSE:         sanitize(&x);
// UNPARSE:         return 0;
// UNPARSE: }

// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | Label | Block                          | Entry(Label)                   |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | 0     | scanf("%d", &x)                | {}                             |
// C_TAINT: | 1     | sanitize(&x)                   | {x}                            |
// C_TAINT: | 2     | return 0;                      | {}                             |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | Label | Block                          | Exit(Label)                    |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | 0     | scanf("%d", &x)                | {x}                            |
// C_TAINT: | 1     | sanitize(&x)                   | {}                             |
// C_TAINT: | 2     | return 0;                      | {}                             |
// C_TAINT: |-------|--------------------------------|--------------------------------|