// RUN: unparse %s | clang-format | FileCheck %s --color --check-prefix=UNPARSE

#include <stdio.h>

void sanitize(int *x) {
        return;
        return;
}

int main(void) {
        return 1;
        return 0;
}

// UNPARSE: void sanitize(int *x) {
// UNPARSE:         return;
// UNPARSE:         return;
// UNPARSE: }
// UNPARSE: int main(void) {
// UNPARSE:         return 1;
// UNPARSE:         return 0;
// UNPARSE: }
