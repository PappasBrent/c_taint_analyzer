#include <stdio.h>

void sanitize(int *x) {
        if (*x == -2 || (*x > 10 && (x + x < 39))) {
                *x = *x + *x;
        }
}

int main(void) {
        int x;

        scanf("%d", &x);
        sanitize(&x);

        return 0;
}
