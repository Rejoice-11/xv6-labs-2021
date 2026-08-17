// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKET 13

struct bucket {
  struct spinlock lock;
  struct buf head;
};

struct bucket bcache[NBUCKET];
struct spinlock bcache_lock;
struct buf buf[NBUF];

static int
hash(uint blockno)
{
  return blockno % NBUCKET;
}

void
binit(void)
{
  struct buf *b;

  initlock(&bcache_lock, "bcache");
  for(int i = 0; i < NBUCKET; i++){
    initlock(&bcache[i].lock, "bcache.bucket");
    bcache[i].head.prev = &bcache[i].head;
    bcache[i].head.next = &bcache[i].head;
  }

  // Put all buffers into bucket 0.
  for(b = buf; b < buf+NBUF; b++){
    b->dev = -1;
    b->blockno = -1;
    b->valid = 0;
    b->refcnt = 0;
    b->next = bcache[0].head.next;
    b->prev = &bcache[0].head;
    b->ticks = 0;
    initsleeplock(&b->lock, "buffer");
    bcache[0].head.next->prev = b;
    bcache[0].head.next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int h = hash(blockno);

  acquire(&bcache[h].lock);

  // Is the block already cached?
  for(b = bcache[h].head.next; b != &bcache[h].head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache[h].lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached; serialize eviction and re-check.
  release(&bcache[h].lock);
  acquire(&bcache_lock);
  acquire(&bcache[h].lock);
  for(b = bcache[h].head.next; b != &bcache[h].head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache[h].lock);
      release(&bcache_lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Reuse the first unused buffer found, unlinking it while
  // still holding its bucket lock so no other CPU can take it.
  struct buf *victim = 0;
  for(int i = 0; i < NBUCKET; i++){
    if(i != h)
      acquire(&bcache[i].lock);
    for(b = bcache[i].head.next; b != &bcache[i].head; b = b->next){
      if(b->refcnt == 0){
        victim = b;
        break;
      }
    }
    if(victim){
      victim->next->prev = victim->prev;
      victim->prev->next = victim->next;
      if(i != h)
        release(&bcache[i].lock);
      break;
    }
    if(i != h)
      release(&bcache[i].lock);
  }
  if(victim == 0){
    release(&bcache[h].lock);
    release(&bcache_lock);
    panic("bget: no buffers");
  }

  b = victim;
  b->dev = dev;
  b->blockno = blockno;
  b->valid = 0;
  b->refcnt = 1;
  b->ticks = ticks;
  b->next = bcache[h].head.next;
  b->prev = &bcache[h].head;
  bcache[h].head.next->prev = b;
  bcache[h].head.next = b;
  release(&bcache[h].lock);
  release(&bcache_lock);
  acquiresleep(&b->lock);
  return b;
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  int h = hash(b->blockno);
  acquire(&bcache[h].lock);
  b->refcnt--;
  if(b->refcnt == 0)
    b->ticks = ticks;
  release(&bcache[h].lock);
}

void
bpin(struct buf *b) {
  int h = hash(b->blockno);
  acquire(&bcache[h].lock);
  b->refcnt++;
  release(&bcache[h].lock);
}

void
bunpin(struct buf *b) {
  int h = hash(b->blockno);
  acquire(&bcache[h].lock);
  b->refcnt--;
  release(&bcache[h].lock);
}
