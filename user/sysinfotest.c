#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

int
main(void)
{
    struct sysinfo info;

    if (sysinfo(&info) < 0) {
        printf("sysinfo failed\n");
        exit(1);
    }
    printf("freepages = %lu pages\n", info.freepages);
    printf("nproc     = %lu\n", info.nproc);

    int pid = fork();

    if (pid < 0) {
    printf("fork failed\n");
    exit(1);
    }
    if (pid == 0) {
    exit(0);
    }

    // 부모: 자식이 RUNNABLE, RUNNING, 또는 ZOMBIE인 동안 다시 센다.
    if (sysinfo(&info) < 0) {
    printf("sysinfo failed\n");
    exit(1);
    }
    printf("after fork, nproc = %lu\n", info.nproc);

    wait(0);  // 자식을 수거하면 해당 proc 슬롯은 UNUSED가 된다.

    if (sysinfo(&info) < 0) {
    printf("sysinfo failed\n");
    exit(1);
    }
    printf("after wait, nproc = %lu\n", info.nproc);


    exit(0);
}
