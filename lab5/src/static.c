#include "headers/gcf.h"
#include "headers/sin.h"
#include <stdio.h>

int main(){

    int mode;
    printf("> ");
    while (scanf("%d", &mode) != EOF){
        if (mode == 0){
            printf("this is static program, can't switch implementation\n> ");
            continue;
        }
        if (mode == 1){
            float a, b, e;
            if (scanf("%f %f %f", &a, &b, &e) == EOF){ break; };
            printf("integral of sin(x) on [%f, %f] is %f\n", a, b, SinIntegral(a, b, e));
        } else if (mode == 2){
            int a, b;
            if (scanf("%d %d", &a, &b) == EOF){ break; };
            printf("gcf of %d and %d is %d\n", a, b, GCF(a, b));
        } else {
            printf("invalid argument \"%d\"\n", mode);
        }
        printf("> ");
    }

}