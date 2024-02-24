#include <stdio.h>


void sanitize(int *x){
    if(*x != 0){
        *x = 0;
    }
}

int main(void){
    int x;
    scanf("%d", &x);
    sanitize(&x)
}