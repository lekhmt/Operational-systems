#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>

typedef struct {
    bool* sieve;
    long long size;
    long long prime;
} arg_t;

void handle_error(bool expr, char* msg){
    if (expr){
        perror(msg);
        exit(-1);
    }
}

void* func(void* args){
    arg_t* arg = (arg_t*) args;
    printf("# thread %lu work with prime %lld\n", pthread_self(), arg->prime);
    for (int i = 2; i * arg->prime <= arg->size; ++i){
        arg->sieve[i * arg->prime] = true;
    }
    free(arg); arg = NULL;
    return NULL;
}


int main(int argc, char** argv){

    handle_error(argc < 2, "specify number of threads");

    long long threads_max = strtol(argv[1], NULL, 10);
    handle_error(threads_max < 1, "number of threads can't be less than 1");
    if (threads_max > 8){
        printf("The number of threads can't be more than 8. Set to 8.\n");
        threads_max = 8;
    }

    printf("Enter a natural number: ");
    long long n; char* cmd = malloc(64 * sizeof(char));
    fgets(cmd, 1024, stdin);
    for (int i = 0; i < 1024; ++i){
        if (cmd[i] == '\n'){ cmd[i] = '\0'; break; }
        if (cmd[i] == '\0'){ break; }
        if (cmd[i] < '0' || cmd[i] > '9'){
            printf("invalid number\n");
            return 0;
        }
    }

    n = strtol(cmd, NULL, 10);

    bool* sieve = malloc((n + 1) * sizeof(bool));
    memset(sieve, false, n + 1);

    pthread_t* threads = calloc(threads_max, sizeof(pthread_t));

    struct timeval start, stop;
    gettimeofday(&start, NULL);


    long long primes[9] = {2, 3, 5, 7, 11, 13, 17, 19, 23};

    printf("1 iteration:\n");

    for (int i = 0; i < threads_max; ++i){
        arg_t* arg = malloc(sizeof(arg_t));
        arg->sieve = sieve;
        arg->size = n;
        arg->prime = primes[i];
        if (pthread_create(&threads[i], NULL, func, arg) != 0){
            printf("Unable to create %d-th thread\n", i + 1);
            free(threads);
            free(arg);
            exit(-1);
        }
    }

    for (int j = 0; j < threads_max; ++j){
        if (pthread_join(threads[j], NULL) != 0){
            printf("Unable to join %d-th thread\n", j + 1);
            free(threads);
            exit(-1);
        };
    }

    long long prime = primes[threads_max]; int counter = 2;

    while (prime * prime <= n){

        printf("%d iteration:\n", counter);
        ++counter;
        int threads_num = 0;
        while (threads_num < threads_max){
            if (sieve[prime]){ ++prime; continue; }
            arg_t* arg = malloc(sizeof(arg_t));
            arg->sieve = sieve;
            arg->size = n;
            arg->prime = prime;
            if (pthread_create(&threads[threads_num], NULL, func, arg) != 0){
                printf("Unable to create %d-th thread\n", threads_num + 1);
                free(threads);
                free(arg);
                exit(-1);
            }
            ++threads_num;
            ++prime;
        }

        for (int j = 0; j < threads_max; ++j){
            if (pthread_join(threads[j], NULL) != 0){
                printf("Unable to join %d-th thread\n", j + 1);
                free(threads);
                exit(-1);
            };
        }

    }

    gettimeofday(&stop, NULL);

    sieve[n] == 0 ? printf("Result: %lld is prime ", n) : printf("Result: %lld is composite ", n);
    printf("(in %lu ms)\n", stop.tv_sec - start.tv_sec);

    free(threads); free(sieve); free(cmd);

    return 0;

}