#include <dlfcn.h>
#include <stdio.h>

int main(){

    int current = 0;

    const char* gcf[2] = {"./libdynamic_euclid_gcf.so", "./libdynamic_naive_gcf.so"};
    const char* sin[2] = {"./libdynamic_rectangle_sin.so", "./libdynamic_trapezoid_sin.so"};

    void* gcf_handler = dlopen(gcf[current], RTLD_LAZY);
    void* sin_handler = dlopen(sin[current], RTLD_LAZY);

    if (!gcf_handler && !sin_handler){
        fprintf(stderr, "dlopen() error: %s\n", dlerror());
        return -1;
    }

    int (*gcf_function)(int A, int B);
    float (*sin_function)(float A, float B, float e);

    gcf_function = dlsym(gcf_handler, "GCF");
    sin_function = dlsym(sin_handler, "SinIntegral");

    if (!gcf_function && !sin_function){
        fprintf(stderr, "dlsym() error: %s\n", dlerror());
        return -1;
    }

    int mode;
    printf("> ");
    while (scanf("%d", &mode) != EOF){
        if (mode == 0){

            dlclose(gcf_handler);
            dlclose(sin_handler);

            gcf_handler = dlopen(gcf[1 - current], RTLD_LAZY);
            sin_handler = dlopen(sin[1 - current], RTLD_LAZY);

            if (!gcf_handler && !sin_handler){
                fprintf(stderr, "dlopen() error: %s\n", dlerror());
                return -1;
            }

            gcf_function = dlsym(gcf_handler, "GCF");
            sin_function = dlsym(sin_handler, "SinIntegral");

            if (!gcf_function && !sin_function){
                fprintf(stderr, "dlsym() error: %s\n", dlerror());
                return -1;
            }

            current = 1 - current;
            printf("implementation switched\n> ");
            continue;

        }
        if (mode == 1){
            float a, b, e;
            if (scanf("%f %f %f", &a, &b, &e) == EOF){ break; };
            printf("integral of sin(x) on [%f, %f] is %f\n", a, b, (*sin_function)(a, b, e));
        } else if (mode == 2){
            int a, b;
            if (scanf("%d %d", &a, &b) == EOF){ break; };
            printf("gcf of %d and %d is %d\n", a, b, (*gcf_function)(a, b));
        } else {
            printf("invalid argument \"%d\"\n", mode);
        }
        printf("> ");
    }

}