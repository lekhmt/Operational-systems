#define _GNU_SOURCE

#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

void handle_error(bool expr, char* msg){
    if (expr){
        perror(msg);
        exit(-1);
    }
}

void clean(char* str){
    for (int i = 0 ; i < strlen(str); ++i){
        if (str[i] == '\n'){ str[i] = '\0'; }
    }
}

int main(){

    const char* SOURCE_SEMAPHORE_NAME = "source_sem";
    const char* RESPONSE_SEMAPHORE_NAME = "response_sem";

    sem_unlink(SOURCE_SEMAPHORE_NAME);
    sem_unlink(RESPONSE_SEMAPHORE_NAME);

    const char* SOURCE_NAME = "source_shm";
    const char* RESPONSE_NAME = "response_shm";
    const int SIZE = 4096;

    shm_unlink(SOURCE_NAME);
    shm_unlink(RESPONSE_NAME);

    int source_fd = shm_open(SOURCE_NAME, O_RDWR | O_CREAT, 0644);
    int response_fd = shm_open(RESPONSE_NAME, O_RDWR | O_CREAT, 0644);
    handle_error(source_fd == -1 || response_fd == -1, "can't open shared memory object");

    handle_error(ftruncate(source_fd, SIZE) == -1 ||
                 ftruncate(response_fd, SIZE) == -1,
                 "can't resize shared memory object");

    void* source_ptr = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, source_fd, 0);
    void* response_ptr = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, response_fd, 0);
    handle_error(source_ptr == MAP_FAILED || response_ptr == MAP_FAILED, "can't mmap shared memory object");

    sem_t* source_semaphore = sem_open(SOURCE_SEMAPHORE_NAME, O_RDWR | O_CREAT, 0644, 0);
    sem_t* response_semaphore = sem_open(RESPONSE_SEMAPHORE_NAME, O_RDWR | O_CREAT, 0644, 0);
    handle_error(source_semaphore == SEM_FAILED ||
                 response_semaphore == SEM_FAILED,
                 "can't open semaphore");

    pid_t pid = fork();
    handle_error(pid == -1, "fork error");

    if (pid == 0){

        sem_wait(source_semaphore);
        char* name = source_ptr;
        int output_fd = open(name, O_WRONLY | O_CREAT, 0644);
        char* file_error = "0";
        if (output_fd < 0){ file_error = "1"; }
        strcpy(response_ptr, file_error);
        sem_post(response_semaphore);

        sem_wait(source_semaphore);
        char* str = source_ptr;
        while (strcmp(str, "\0") != 0){
            char* error = "0";
            if (str[0] >= 'A' && str[0] <= 'Z'){
                write(output_fd, str, strlen(str) * sizeof(char));
                write(output_fd, "\n", sizeof "\n");
            } else {
                error = "1";
            }
            strcpy(response_ptr, error);
            sem_post(response_semaphore);
            sem_wait(source_semaphore);
            str = source_ptr;
        }

    } else {

        const int parent = getpid();
        printf("[%d] Enter the name of file to write: ", parent);
        fflush(stdout);
        char name[256];
        read(fileno(stdin), name, 256); clean(name);
        strcpy(source_ptr, name);
        sem_post(source_semaphore);

        sem_wait(response_semaphore);
        if (strcmp(response_ptr, "1") == 0){
            close(source_fd);
            close(response_fd);
            shm_unlink(SOURCE_NAME);
            shm_unlink(RESPONSE_NAME);
            munmap(source_ptr, SIZE);
            munmap(response_ptr, SIZE);
            sem_destroy(source_semaphore);
            sem_destroy(response_semaphore);
            sem_unlink(SOURCE_SEMAPHORE_NAME);
            sem_unlink(RESPONSE_SEMAPHORE_NAME);
            handle_error(true, "file error");
        }

        char str[256];
        printf("[%d] Enter string: ", parent);
        fflush(stdout);
        while (read(fileno(stdin), str, 256) != 0){
            clean(str);
            strcpy(source_ptr, str);
            sem_post(source_semaphore);
            sem_wait(response_semaphore);
            char* error = response_ptr;
            if (strcmp(error, "1") == 0){
                printf("Error: \"%s\" is not valid.\n", str);
            }
            printf("[%d] Enter string: ", parent);
            fflush(stdout);
        }

        printf("\n");
        fflush(stdout);

    }

    close(source_fd);
    close(response_fd);
    shm_unlink(SOURCE_NAME);
    shm_unlink(RESPONSE_NAME);
    munmap(source_ptr, SIZE);
    munmap(response_ptr, SIZE);
    sem_destroy(source_semaphore);
    sem_destroy(response_semaphore);
    sem_unlink(SOURCE_SEMAPHORE_NAME);
    sem_unlink(RESPONSE_SEMAPHORE_NAME);

}
