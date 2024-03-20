// RUN: unparse %s | clang-format | FileCheck %s --color --check-prefix=UNPARSE
// RUN: c_taint %s | FileCheck %s --color --check-prefix=C_TAINT

#include <stdio.h>

void sanitize(int *x) {
        if (*x != 0) {
                *x = 0;
        } else {
        }
}

int main(void) {
        int x;
        int y;
        scanf("%d", &x);

        y = x;
        y = 5;
        return 0;
}

// UNPARSE: void sanitize(int *x) {
// UNPARSE:         if (*x != 0) {
// UNPARSE:                 *x = 0;
// UNPARSE:         } else {
// UNPARSE:         }
// UNPARSE: }
// UNPARSE: int main(void) {
// UNPARSE:         int x;
// UNPARSE:         int y;
// UNPARSE:         scanf("%d", &x);
// UNPARSE:         y = x;
// UNPARSE:         y = 5;
// UNPARSE:         return 0;
// UNPARSE: }


// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | Label | Block                          | Entry(Label)                   |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | 0     | scanf("%d", &x)                | {}                             |
// C_TAINT: | 1     | y = x                          | {x}                            |
// C_TAINT: | 2     | y = 5                          | {x, y}                         |
// C_TAINT: | 3     | return 0;                      | {x}                            |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | Label | Block                          | Exit(Label)                    |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | 0     | scanf("%d", &x)                | {x}                            |
// C_TAINT: | 1     | y = x                          | {x, y}                         |
// C_TAINT: | 2     | y = 5                          | {x}                            |
// C_TAINT: | 3     | return 0;                      | {x}                            |
// C_TAINT: |-------|--------------------------------|--------------------------------|