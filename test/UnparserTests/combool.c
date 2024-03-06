// RUN: unparse %s | clang-format | FileCheck %s --color

#include <stdio.h>

void sanitize(int *x) {
        if (*x == -2 || (*x > 10 && (*x + *x < 39))) {
                *x = *x + *x;
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
// CHECK:         if (*x == -2 || (*x > 10 && (*x + *x < 39))) {
// CHECK:                 *x = *x + *x;
// CHECK:         } else {
// CHECK:         }
// CHECK: }
// CHECK: int main(void) {
// CHECK:         int x;
// CHECK:         scanf("%d", &x);
// CHECK:         sanitize(&x);
// CHECK:         return 0;
// CHECK: }
