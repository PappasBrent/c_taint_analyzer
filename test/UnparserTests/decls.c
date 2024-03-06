// RUN: unparse %s | clang-format | FileCheck %s --color

#include <stdio.h>

void sanitize(int *x) {
}

int main(void) {
        int a;
        int b;
        int c;
        int d;
        int e;
        return 0;
}

// CHECK: void sanitize(int *x) {
// CHECK: }
// CHECK: int main(void) {
// CHECK:         int a;
// CHECK:         int b;
// CHECK:         int c;
// CHECK:         int d;
// CHECK:         int e;
// CHECK:         return 0;
// CHECK: }
