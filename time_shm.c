#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s command [arg1 arg2 ...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *shm_name = "/time_shm_obj";
    int fd;

   fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    printf("shm_open: Success\n");

   
    if (ftruncate(fd, sizeof(struct timeval)) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    
    struct timeval *shared_start = mmap(
        NULL,
        sizeof(struct timeval),
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0
    );
    if (shared_start == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }


    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {

        printf("Child: before gettimeofday.\n");

        if (gettimeofday(shared_start, NULL) == -1) {
            perror("gettimeofday (child)");
            exit(EXIT_FAILURE);
        }

        printf("Child: before execvp.\n");
        fflush(stdout);

        execvp(argv[1], &argv[1]);


        perror("execvp");
        exit(EXIT_FAILURE);

    } else {

        struct timeval end;

        printf("Parent: waiting for child...\n");
        fflush(stdout);

        if (wait(NULL) == -1) {
            perror("wait");
            exit(EXIT_FAILURE);
        }

        printf("Parent: child finished, taking end time...\n");

        if (gettimeofday(&end, NULL) == -1) {
            perror("gettimeofday (parent)");
            exit(EXIT_FAILURE);
        }

        double elapsed =
            (end.tv_sec  - shared_start->tv_sec) +
            (end.tv_usec - shared_start->tv_usec) / 1e6;

        printf("Elapsed time: %f seconds\n", elapsed);
        fflush(stdout);

        munmap(shared_start, sizeof(struct timeval));
        close(fd);
        shm_unlink(shm_name);
    }

    printf("Parent finished normally.\n");
    return 0;
}

