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
