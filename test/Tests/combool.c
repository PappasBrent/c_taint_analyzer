// RUN: unparse %s | clang-format | FileCheck %s --color --check-prefix=UNPARSE

#include <stdio.h>

void sanitize(int *x) {
        if (*x == -2 || (*x > 10 && (*x + *x < 39))) {
                *x = *x + *x;
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
// UNPARSE:         if (*x == -2 || (*x > 10 && (*x + *x < 39))) {
// UNPARSE:                 *x = *x + *x;
// UNPARSE:         } else {
// UNPARSE:         }
// UNPARSE: }
// UNPARSE: int main(void) {
// UNPARSE:         int x;
// UNPARSE:         scanf("%d", &x);
// UNPARSE:         sanitize(&x);
// UNPARSE:         return 0;
// UNPARSE: }
