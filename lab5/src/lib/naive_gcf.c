#include "../headers/gcf.h"
#include <stdio.h>

int gcf(int a, int b){
    if (a < b){
        int tmp = b;
        b = a;
        a = tmp;
    }
    for (int i = b; i > 0; --i){
        if (a % i == 0 && b % i == 0){
            return i;
        }
    }
    return -1;
}

int GCF(int A, int B){

    printf("|naive algorithm|\n");
    return gcf(A, B);

}