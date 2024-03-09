// RUN: unparse %s | clang-format | FileCheck %s --color --check-prefix=UNPARSE

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
        while (x < 10) {
                y = x + 5;
                x = x + 1;
        }

        sanitize(&x);
        sanitize(&y);

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
// UNPARSE:         while (x < 10) {
// UNPARSE:                 y = x + 5;
// UNPARSE:                 x = x + 1;
// UNPARSE:         }
// UNPARSE:         sanitize(&x);
// UNPARSE:         sanitize(&y);
// UNPARSE:         return 0;
// UNPARSE: }

// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | Label | Block                          | Entry(Label)                   |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | 0     | scanf("%d", &x)                | {}                             |
// C_TAINT: | 1     | x < 10                         | {x, y}                         |
// C_TAINT: | 2     | y = x + 5                      | {x, y}                         |
// C_TAINT: | 3     | x = x + 1                      | {x, y}                         |
// C_TAINT: | 4     | sanitize(&x)                   | {x, y}                         |
// C_TAINT: | 5     | sanitize(&y)                   | {y}                            |
// C_TAINT: | 6     | return 0                       | {}                             |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | Label | Block                          | Exit(Label)                    |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | 0     | scanf("%d", &x)                | {x}                            |
// C_TAINT: | 1     | x < 10                         | {x, y}                         |
// C_TAINT: | 2     | y = x + 5                      | {x, y}                         |
// C_TAINT: | 3     | x = x + 1                      | {x, y}                         |
// C_TAINT: | 4     | sanitize(&x)                   | {y}                            |
// C_TAINT: | 5     | sanitize(&y)                   | {}                             |
// C_TAINT: | 6     | return 0                       | {}                             |
// C_TAINT: |-------|--------------------------------|--------------------------------|
