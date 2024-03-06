// RUN: unparse %s | clang-format | FileCheck %s --color

#include <stdio.h>

void sanitize(int *x) {
}

int main(void) {
        int x;

        x = 1;

        return 0;
}

// CHECK: void sanitize(int *x) {
// CHECK: }
// CHECK: int main(void) {
// CHECK:         int x;
// CHECK:         x = 1;
// CHECK:         return 0;
// CHECK: }
