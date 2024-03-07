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
        y = x;
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
// UNPARSE:         y = x;
// UNPARSE:         sanitize(&x);
// UNPARSE:         sanitize(&y);
// UNPARSE:         return 0;
// UNPARSE: }
