#include <stdio.h>

void sanitize(int *x) {
        if (*x != 0) {
                *x = 0;
        } else {
        }
}

int main(void) {
        int x;

        scanf("%d", &x);
        sanitize(&x);
        scanf("%d", &x);

        return 0;
}
