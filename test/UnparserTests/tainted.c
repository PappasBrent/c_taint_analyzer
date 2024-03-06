// RUN: unparse %s | clang-format | FileCheck %s --color

#include <stdio.h>

void sanitize(int *x) {
}

int main(void) {
        int x;
        scanf("%d", &x);

        return 0;
}

// CHECK: void sanitize(int *x) {
// CHECK: }
// CHECK: int main(void) {
// CHECK:         int x;
// CHECK:         scanf("%d", &x);
// CHECK:         return 0;
// CHECK: }
