#include <stdio.h>


void sanitize(int *x){
    if(*x != 0){
        *x = 0;
    }
}

int main(void){
    int x;
    int y;
    scanf("%d", &x);
    y = x;
    sanitize(&x);
    sanitize(&y);

    return 0;
}