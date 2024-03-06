// RUN: unparse %s | clang-format | FileCheck %s --color

#include <stdio.h>

void sanitize(int *x) {
        if (*x != 0) {
                *x = 0;
        } else {
        }
}

int main(void) {
        int x;
        int y;
        scanf("%d", &x);
        while (x < 10) {
                y = x + 5;
                x = x + 1;
        }

        sanitize(&x);
        sanitize(&y);

        return 0;
}

// CHECK: void sanitize(int *x) {
// CHECK:         if (*x != 0) {
// CHECK:                 *x = 0;
// CHECK:         } else {
// CHECK:         }
// CHECK: }
// CHECK: int main(void) {
// CHECK:         int x;
// CHECK:         int y;
// CHECK:         scanf("%d", &x);
// CHECK:         while (x < 10) {
// CHECK:                 y = x + 5;
// CHECK:                 x = x + 1;
// CHECK:         }
// CHECK:         sanitize(&x);
// CHECK:         sanitize(&y);
// CHECK:         return 0;
// CHECK: }
