#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// 사용자 영역에서 freepages() 시스템 호출의 반환값을 출력한다.
int main(void) {
    // usys.pl이 만든 스텁을 거쳐 커널이 집계한 빈 페이지 수를 받는다.
    printf("free pages : %lu\n", freepages());
    exit(0);
}
