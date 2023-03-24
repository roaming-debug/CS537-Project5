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
  for(; p + PGSIZE <= (char*)PHYSTOP; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  kmem.free_pages++;
  struct run *r;

  int i = ((uint)v - (uint)end) / PGSIZE;
  if (kmem.ref_cnt[i] != 1) {
    ; // debug here
  }

  if((uint)v % PGSIZE || v < end || (uint)v >= PHYSTOP) 
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  acquire(&kmem.lock);
  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;
  kmem.free_pages--;
  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  int i = ((uint)r - (uint)end) / PGSIZE;
  kmem.ref_cnt[i] = 1;
  release(&kmem.lock);
  return (char*)r;
}

// retrieve the total number of free pages in the system
int getFreePagesCount(void)
{
  return kmem.free_pages;
}

// returns index in ref_cnt array for given page
int refCountIndex(char* v)
{
  return ((uint)v - (uint)end) / PGSIZE;
}

int getRefCount(char* v)
{
  return kmem.ref_cnt[refCountIndex(v)];
}

// increment reference count for the given page
void incRefCount(char* v)
{
  acquire(&kmem.lock);
  int i = refCountIndex(v);
  kmem.ref_cnt[i]++;
  release(&kmem.lock);
}

// decrement reference count for the given page
void decRefCount(char *v)
{
  acquire(&kmem.lock);
  int i = refCountIndex(v);
  kmem.ref_cnt[i]--;
  release(&kmem.lock);
}