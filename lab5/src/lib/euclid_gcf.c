#include "../headers/gcf.h"
#include <stdio.h>

int gcf(int a, int b){
    if (a % b == 0){
        return b;
    }
    return gcf(b, a % b);
}

int GCF(int A, int B){

    printf("|euclid algorithm|\n");
    return gcf(A, B);

}