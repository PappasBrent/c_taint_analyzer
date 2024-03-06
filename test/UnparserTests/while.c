// RUN: unparse %s | clang-format | FileCheck %s --color

#include <stdio.h>

void sanitize(int *x) {
        while (*x > 0) {
                *x = *x - 1;
        }
}

int main(void) {
        int x;

        scanf("%d", &x);
        sanitize(&x);

        return 0;
}

// CHECK: void sanitize(int *x) {
// CHECK:         while (*x > 0) {
// CHECK:                 *x = *x - 1;
// CHECK:         }
// CHECK: }
// CHECK: int main(void) {
// CHECK:         int x;
// CHECK:         scanf("%d", &x);
// CHECK:         sanitize(&x);
// CHECK:         return 0;
// CHECK: }
