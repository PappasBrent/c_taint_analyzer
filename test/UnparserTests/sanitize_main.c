// RUN: unparse %s | clang-format | FileCheck %s --color

#include <stdio.h>

void sanitize(int *x) {
}

int main(void) {
        return 0;
}

// CHECK: void sanitize(int *x) {
// CHECK: }
// CHECK: int main(void) {
// CHECK:         return 0;
// CHECK: }
