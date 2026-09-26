#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

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

// kernel/sysproc.c  — 파일 맨 아래에 추가

uint64
sys_va2pa(void)
{
  uint64 va;
  pte_t *pte;
  struct proc *p = myproc();

  argaddr(0, &va);                    // 0번째 인자를 주소로 읽는다
  if (va >= MAXVA)
    return 0;                         // 이대로 walk 에 주면 커널이 panic 한다

  // PX(level, va)는 level에 해당하는 9비트 페이지 테이블 인덱스를 구한다.
  // va의 하위 12비트는 4 KiB 페이지 안에서의 오프셋이다.
  // printk의 %d는 int, %x는 uint를 받으므로 각 값을 알맞은 형으로 변환한다.
  printk("va %p : L2=%d L1=%d L0=%d off=0x%x\n",
         (void *)va,
         (int)PX(2, va), (int)PX(1, va), (int)PX(0, va),
         (uint)(va & 0xFFF));

  pte = walk(p->pagetable, va, 0);    // alloc = 0 : 찾기만 하고 만들지 않는다
  if (pte == 0 || (*pte & PTE_V) == 0)
    return 0;                         // 매핑이 없다
  if ((*pte & PTE_U) == 0)
    return 0;                         // 매핑은 있지만 사용자용이 아니다

  // PTE2PA(*pte)는 매핑된 물리 페이지의 시작 주소를 준다.
  // 여기에 가상 주소의 페이지 내부 오프셋을 더해 정확한 물리 주소를 만든다.
  return PTE2PA(*pte) + (va & 0xFFF);
}
