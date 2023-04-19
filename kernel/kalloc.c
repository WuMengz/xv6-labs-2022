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

struct spinlock reflock, ptelock;
int ref_cnt[PHYSTOP >> PGSHIFT];

void
kinit()
{
  initlock(&reflock, "refcnt");
  initlock(&ptelock, "pte");
  for (int i = 0; i < (PHYSTOP >> PGSHIFT); ++i)
    ref_cnt[i] = 1;
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
add_refcnt(uint64 addr) {
  if (addr >= PHYSTOP)
    panic("add_refcnt: unknown address!");
  acquire(&reflock);
  ++ref_cnt[addr >> PGSHIFT];
  release(&reflock);
}

void
sub_refcnt(uint64 addr) {
  if (addr >= PHYSTOP)
    panic("sub_refcnt: unknown address!");
  acquire(&reflock);
  --ref_cnt[addr >> PGSHIFT];
  release(&reflock);
}

int
query_refcnt(uint64 addr) {
  return ref_cnt[addr >> PGSHIFT];
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
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

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  acquire(&ptelock);
  sub_refcnt((uint64)pa);
  if (query_refcnt((uint64)pa) == 0) {
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
  release(&ptelock);
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
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r) {
    memset((char*)r, 5, PGSIZE); // fill with junk
    if (query_refcnt((uint64)r) != 0)
      panic("kalloc: refcnt failed.\n");
    add_refcnt((uint64)r);
  }
  return (void*)r;
}
