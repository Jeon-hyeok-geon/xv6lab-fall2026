#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
// 커널과 사용자 프로그램이 공유하는 sysinfo 구조체 형식을 사용한다.
#include "sysinfo.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if (addr + n < addr)
      return -1;
    if (addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep_prepare(&ticks);
    release(&tickslock);
    sleep();
    acquire(&tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// 커널에서 수집한 시스템 정보를 호출한 프로세스의 사용자 메모리로 전달한다.
uint64
sys_sysinfo(void) {
  // 사용자에게 보낼 값을 먼저 커널 스택의 구조체에 모은다.
  struct sysinfo info;
  uint64 addr;
  struct proc *p = myproc();

  // 사용자 함수가 첫 번째 인자로 넘긴 구조체의 가상 주소를 읽는다.
  argaddr(0, &addr);
  info.freepages = freepages();
  info.nproc = nproc();
  // 현재 프로세스의 페이지 테이블을 이용해 커널 구조체 전체를 안전하게 복사한다.
  if (copyout(p->pagetable, p->sz, addr, (char *)&info, sizeof(info)) < 0) {
    // 주소가 유효하지 않거나 매핑되지 않은 경우 실패를 반환한다.
    return -1;
  }
  return 0;
}
