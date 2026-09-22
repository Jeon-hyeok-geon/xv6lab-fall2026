// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void *)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char *)PGROUNDUP((uint64)pa_start);
  for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run *)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if (r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if (r)
    memset((char *)r, 5, PGSIZE); // fill with junk
  return (void *)r;
}

// free list를 순회해 현재 할당 가능한 물리 페이지 수를 반환한다.
// 순회 중 kalloc()이나 kfree()가 목록을 바꾸지 못하도록 kmem.lock을 사용한다.
uint64
freepages(void) {
  struct run *r;
  uint64 n = 0;

  // free list 전체를 일관된 상태로 읽기 위해 할당기 락을 획득한다.
  acquire(&kmem.lock);

  // struct run 하나가 빈 물리 페이지 하나를 나타낸다.
  r = kmem.freelist;
  while (r) {
    n++;          // 현재 노드가 가리키는 빈 페이지를 하나 센다.
    r = r->next;  // 다음 빈 페이지로 이동한다.
  }

  // 순회가 끝났으므로 다른 CPU가 free list를 수정할 수 있게 락을 해제한다.
  release(&kmem.lock);

  return n;
}
