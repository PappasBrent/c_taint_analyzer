#include <stdio.h>

void sanitize(int *x) {
        if (*x != 0) {
                *x = 0;
        }
}

int main(void) {
        int x;

        sanitize(&x);

        return 0;
}