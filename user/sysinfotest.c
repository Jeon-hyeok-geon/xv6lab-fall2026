#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

int
main(void)
{
    // 커널이 copyout()으로 결과를 채울 사용자 영역 구조체를 준비한다.
    struct sysinfo info;

    // 첫 호출로 현재 남은 페이지 수와 프로세스 수를 가져온다.
    if (sysinfo(&info) < 0) {
        printf("sysinfo failed\n");
        exit(1);
    }
    printf("freepages = %lu pages\n", info.freepages);
    printf("nproc     = %lu\n", info.nproc);

    // 새 프로세스 슬롯이 생겼을 때 nproc 값이 증가하는지 확인한다.
    int pid = fork();

    // fork()가 음수를 반환하면 자식 생성에 실패한 경우다.
    if (pid < 0) {
    printf("fork failed\n");
    exit(1);
    }
    // 자식은 종료되지만 부모가 wait()하기 전까지 ZOMBIE 상태로 남는다.
    if (pid == 0) {
    exit(0);
    }

    // 부모는 자식이 RUNNABLE, RUNNING, ZOMBIE 중 하나일 때 다시 센다.
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
