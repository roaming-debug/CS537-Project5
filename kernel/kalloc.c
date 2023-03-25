// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "spinlock.h"

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;

  /*
  P5 changes
  */
  uint free_pages; //track free pages
  uint ref_cnt[PHYSTOP / PGSIZE]; //track reference count

} kmem;

extern char end[]; // first address after kernel loaded from ELF file

// Initialize free list of physical pages.
void
kinit(void)
{
  char *p;

  initlock(&kmem.lock, "kmem");
  kmem.free_pages = 0;
  p = (char*)PGROUNDUP((uint)end);
  for(; p + PGSIZE <= (char*)PHYSTOP; p += PGSIZE) {
    kmem.ref_cnt[refCountIndex(p)] = 0;
    kfree(p);
  }
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || (uint)v >= PHYSTOP) 
    panic("kfree");

  acquire(&kmem.lock);
  r = (struct run*)v;
  if (kmem.ref_cnt[refCountIndex(v)] > 0)
    kmem.ref_cnt[refCountIndex(v)]--;
  if (kmem.ref_cnt[refCountIndex(v)] == 0) {
    memset(v, 1, PGSIZE); // Fill with junk to catch dangling refs.
    r->next = kmem.freelist;
    kmem.freelist = r;
    kmem.free_pages++;
  }
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;
  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r) {
    kmem.freelist = r->next;
    kmem.ref_cnt[(uint)r / PGSIZE] = 1;
    kmem.free_pages--;
  }
  release(&kmem.lock);
  return (char*)r;
}

// retrieve the total number of free pages in the system
int getFreePagesCount(void)
{
  return kmem.free_pages;
}

// returns index in ref_cnt array for the given page
int refCountIndex(char* v)
{
  return (uint)v / PGSIZE;
}

// return number of references for the given page
int getRefCount(char* v)
{
  return kmem.ref_cnt[refCountIndex(v)];
}

// increment reference count for the given page
void incRefCount(char* v)
{
  acquire(&kmem.lock);
  kmem.ref_cnt[refCountIndex(v)]++;
  release(&kmem.lock);
}

// decrement reference count for the given page
void decRefCount(char *v)
{
  acquire(&kmem.lock);
  kmem.ref_cnt[refCountIndex(v)]--;
  release(&kmem.lock);
}