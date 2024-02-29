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
    y = 5;

    sanitize(&x);
    
    return 0;
}