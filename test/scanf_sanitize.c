#include <stdio.h>

void sanitize(int *x) {
}

int main(void) {
        int x;

        scanf("%d", &x);
        sanitize(&x);

        return 0;
}
