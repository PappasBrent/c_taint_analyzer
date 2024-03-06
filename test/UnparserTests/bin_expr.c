// RUN: unparse %s | clang-format | FileCheck %s --color

#include <stdio.h>

void sanitize(int *x) {
        return;
}

int main(void) {
        int x;

        x = 1 + 2 * 3 - 4;

        return 0;
}

// CHECK: void sanitize(int *x) {
// CHECK:         return;
// CHECK: }
// CHECK: int main(void) {
// CHECK:         int x;
// CHECK:         x = 1 + 2 * 3 - 4;
// CHECK:         return 0;
// CHECK: }
