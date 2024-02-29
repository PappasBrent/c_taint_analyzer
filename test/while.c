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
