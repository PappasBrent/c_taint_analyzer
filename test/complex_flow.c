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
    while(x < 10){
        y = x + 5;
        x = x + 1;
    }
    
    sanitize(&x);
    sanitize(&y);

    return 0;
}