// RUN: unparse %s | clang-format | FileCheck %s --color --check-prefix=UNPARSE

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
