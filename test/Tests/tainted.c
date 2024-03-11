// RUN: unparse %s | clang-format | FileCheck %s --color --check-prefix=UNPARSE

#include <stdio.h>

void sanitize(int *x) {
}

int main(void) {
        int x;
        scanf("%d", &x);

        return 0;
}

// UNPARSE: void sanitize(int *x) {
// UNPARSE: }
// UNPARSE: int main(void) {
// UNPARSE:         int x;
// UNPARSE:         scanf("%d", &x);
// UNPARSE:         return 0;
// UNPARSE: }


// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | Label | Block                          | Entry(Label)                   |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | 0     | scanf("%d", &x)                | {}                             |
// C_TAINT: | 1     | return 0                       | {x}                            |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | Label | Block                          | Exit(Label)                    |
// C_TAINT: |-------|--------------------------------|--------------------------------|
// C_TAINT: | 0     | scanf("%d", &x)                | {x}                            |
// C_TAINT: | 1     | return 0                       | {x}                            |
// C_TAINT: |-------|--------------------------------|--------------------------------|