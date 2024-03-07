// RUN: unparse %s | clang-format | FileCheck %s --color --check-prefix=UNPARSE

#include <stdio.h>

void sanitize(int *x) {
}

int main(void) {
        ;
        ;
        ;
        ;
        ;
        return 0;
}

// UNPARSE: void sanitize(int *x) {
// UNPARSE: }
// UNPARSE: int main(void) {
// UNPARSE:         ;
// UNPARSE:         ;
// UNPARSE:         ;
// UNPARSE:         ;
// UNPARSE:         ;
// UNPARSE:         return 0;
// UNPARSE: }
