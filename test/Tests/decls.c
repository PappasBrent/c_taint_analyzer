// RUN: unparse %s | clang-format | FileCheck %s --color --check-prefix=UNPARSE

#include <stdio.h>

void sanitize(int *x) {
}

int main(void) {
        int a;
        int b;
        int c;
        int d;
        int e;
        return 0;
}

// UNPARSE: void sanitize(int *x) {
// UNPARSE: }
// UNPARSE: int main(void) {
// UNPARSE:         int a;
// UNPARSE:         int b;
// UNPARSE:         int c;
// UNPARSE:         int d;
// UNPARSE:         int e;
// UNPARSE:         return 0;
// UNPARSE: }
