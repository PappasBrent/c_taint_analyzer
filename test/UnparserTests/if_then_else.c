// RUN: unparse %s | clang-format | FileCheck %s --color

#include <stdio.h>

void sanitize(int *x) {
        if (*x < 0) {
                *x = 0;
        } else {
        }
}

int main(void) {
        int x;

        scanf("%d", &x);
        sanitize(&x);

        return 0;
}

// CHECK: void sanitize(int *x) {
// CHECK:         if (*x < 0) {
// CHECK:                 *x = 0;
// CHECK:         } else {
// CHECK:         }
// CHECK: }
// CHECK: int main(void) {
// CHECK:         int x;
// CHECK:         scanf("%d", &x);
// CHECK:         sanitize(&x);
// CHECK:         return 0;
// CHECK: }
