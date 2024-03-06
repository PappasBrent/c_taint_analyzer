// RUN: unparse %s | clang-format | FileCheck %s --color

#include <stdio.h>

void sanitize(int *x) {
        return;
        return;
}

int main(void) {
        return 1;
        return 0;
}

// CHECK: void sanitize(int *x) {
// CHECK:         return;
// CHECK:         return;
// CHECK: }
// CHECK: int main(void) {
// CHECK:         return 1;
// CHECK:         return 0;
// CHECK: }
