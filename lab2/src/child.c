#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argv, char* argc[]){

    int src_fd[2], err_fd[2];
    src_fd[0] = atoi(argc[1]);
    src_fd[1] = atoi(argc[2]);
    err_fd[0] = atoi(argc[3]);
    err_fd[1] = atoi(argc[4]);

    char* name = argc[0];
    int output_fd = open(name, O_WRONLY | O_CREAT);
    bool file_error = false;
    if (output_fd < 0){ file_error = true; }
    write(err_fd[1], &file_error, sizeof(bool));
    if (file_error){
        close(src_fd[0]); close(src_fd[1]);
        close(err_fd[0]); close(err_fd[1]);
    }

    char str[256];
    read(src_fd[0], &str, sizeof(str));
    read(src_fd[0], &str, sizeof(str));
    while(strcmp(str, "_quit") != 0){
        bool err;
        if (str[0] >= 'A' && str[0] <= 'Z'){
            err = false;
            write(output_fd, str, strlen(str) * sizeof(char));
            write(output_fd, "\n", sizeof "\n");
        } else {
            err = true;
        }
        write(err_fd[1], &err, sizeof(bool));
        read(src_fd[0], &str, sizeof(str));
    }
    close(src_fd[0]); close(src_fd[1]);
    close(err_fd[0]); close(err_fd[1]);
}
