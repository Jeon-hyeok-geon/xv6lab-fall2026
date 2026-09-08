#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NCMHILD 5
#define NSTEP 3

int main(void) {
    for (int i = 0; i < NCMHILD; i++) {
        int pid = fork();

        if (pid < 0) {
            printf("fork failed\n");
            exit(1);
        }

        if (pid == 0) {
            for (int j = 0; j < NSTEP; j++) 
                printf("child %d (pid %d): step %d\n", i, getpid(), j);
            exit(0);
        }
    }

    for (int i = 0; i < NCMHILD; i++) {
        wait(0);
    }

    printf("parent: all children done\n");
    exit(0);
}