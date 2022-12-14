#include <lib/gcc.h>
#include <lib/spinlock.h>
#include <lib/debug.h>

static spinlock_t mem_lk;


// Number of physical pages that are actually available in the machine.
static unsigned int NUM_PAGES;
static unsigned int BB_OFFSET;
static unsigned int BB_SIZE;

/**
 * Structure representing information for one physical page.
 */
struct ATStruct {
    /**
     * The permission of the page.
     * 0: Reserved by the BIOS.
     * 1: Kernel only.
     * >1: Normal (available).
     */
    unsigned int perm;
    /**
     * Whether the page is allocated.
     * 0: unallocated
     * >0: allocated
     */
    unsigned int allocated;
};

struct BuddyBlock {
    unsigned int size;
    unsigned int used;
    // unsigned int start_index;
    // unsigned int next_index;
};

/**
 * A 32 bit machine may have up to 4GB of memory.
 * So it may have up to 2^20 physical pages,
 * with the page size being 4KB.
 */
static struct ATStruct AT[1 << 20];


/** 
 * through testing, we found 2^17 to be the largest consecutive part of memory
 * we reduce the size in practice if there happens to be a smaller part of consecutive memory
 */
static struct BuddyBlock BB[1 << 17];



void mem_spinlock_init(void) {
    spinlock_init(&mem_lk);
}

void mem_lock(void) {
    spinlock_acquire(&mem_lk);
}

void mem_unlock(void) {
    spinlock_release(&mem_lk);
}

// The getter function for NUM_PAGES.
unsigned int get_nps(void)
{
    return NUM_PAGES;
}

// The setter function for NUM_PAGES.
void set_nps(unsigned int nps)
{
    NUM_PAGES = nps;
}

/**
 * The getter function for the page permission.
 * If the page with the given index has the normal permission,
 * then returns 1, otherwise returns 0.
 */
unsigned int at_is_norm(unsigned int page_index)
{
    unsigned int perm;

    perm = AT[page_index].perm;
    if (perm > 1) {
        perm = 1;
    } else {
        perm = 0;
    }

    return perm;
}

/**
 * The setter function for the physical page permission.
 * Sets the permission of the page with given index.
 * It also marks the page as unallocated.
 */
void at_set_perm(unsigned int page_index, unsigned int perm)
{
    AT[page_index].perm = perm;
    AT[page_index].allocated = 0;
}

/**
 * The getter function for the physical page allocation flag.
 * Returns 0 if the page is not allocated, otherwise returns 1.
 */
unsigned int at_is_allocated(unsigned int page_index)
{
    unsigned int allocated;

    allocated = AT[page_index].allocated;
    if (allocated > 0) {
        allocated = 1;
    }

    return allocated;
}

/**
 * The setter function for the physical page allocation flag.
 * Set the flag of the page with given index to the given value.
 */
void at_set_allocated(unsigned int page_index, unsigned int allocated)
{
    AT[page_index].allocated = allocated;
}

void set_bb_offset(unsigned int page_index) {
    BB_OFFSET = page_index;
}

unsigned int get_bb_offset(void) {
    return BB_OFFSET;
}

void set_bb_total_size(unsigned int size) {
    if (size > 1>> 17) {
        size = 1>>17;
    }
    BB_SIZE = size;
}

unsigned int get_bb_total_size(void) {
    return BB_SIZE;
}

void bb_set_size(unsigned int page_index, unsigned int size) {
    BB[page_index].size = size;
}

// void bb_set_start(unsigned int page_index, unsigned int start_index) {
//     BB[page_index+BB_OFFSET].start_index = start_index;
// }

// void bb_set_next(unsigned int page_index, unsigned int next_index) {
//     BB[page_index+BB_OFFSET].start_index = next_index;
// }

unsigned int bb_get_size(unsigned int page_index) {
    return BB[page_index].size;
}

// unsigned int bb_get_start(unsigned int page_index) {
//     return BB[page_index+BB_OFFSET].start_index;
// }

// unsigned int bb_get_next(unsigned int page_index) {
//     return BB[page_index+BB_OFFSET].next_index;
// }

void bb_set_used(unsigned int page_index, unsigned int used) {
    BB[page_index].used = used;
}

unsigned int bb_get_used(unsigned int page_index) {
    return BB[page_index].used;
}

unsigned int bb_get_buddy(unsigned int page_index, unsigned int size) {
    return page_index ^ size;
}

void bb_split(unsigned int page_index) {
    if (BB[page_index].size <= 1) {
        KERN_DEBUG("size at given page_index is 0 (or 1)\n");
    }
    if (BB[page_index].used != 0) {
        KERN_DEBUG("current buddy block in use can't split\n");
    }
    unsigned int new_pi = BB[page_index].size/2 + page_index;
    BB[page_index].size = BB[page_index].size/2;
    BB[new_pi].size = BB[page_index].size/2;
}

