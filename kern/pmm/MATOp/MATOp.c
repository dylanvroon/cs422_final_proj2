#include <lib/debug.h>
#include <lib/types.h>
#include "import.h"

#define PAGESIZE     4096
#define VM_USERLO    0x40000000
#define VM_USERHI    0xF0000000
#define VM_USERLO_PI (VM_USERLO / PAGESIZE)
#define VM_USERHI_PI (VM_USERHI / PAGESIZE)

static unsigned int last_palloc_index = VM_USERLO_PI;

/**
 * Allocate a physical page.
 *
 * 1. First, implement a naive page allocator that scans the allocation table (AT)
 *    using the functions defined in import.h to find the first unallocated page
 *    with normal permissions.
 *    (Q: Do you have to scan the allocation table from index 0? Recall how you have
 *    initialized the table in pmem_init.)
 *    Then mark the page as allocated in the allocation table and return the page
 *    index of the page found. In the case when there is no available page found,
 *    return 0.
 * 2. Optimize the code using memoization so that you do not have to
 *    scan the allocation table from scratch every time.
 */
unsigned int palloc()
{
    unsigned int nps;
    unsigned int palloc_index;
    unsigned int palloc_free_index;
    bool first;

    mem_lock();

    nps = get_nps();
    palloc_index = last_palloc_index;
    palloc_free_index = nps;
    first = TRUE;

    while ((palloc_index != last_palloc_index || first) && palloc_free_index == nps) {
        first = FALSE;
        if (at_is_norm(palloc_index) && !at_is_allocated(palloc_index)) {
            palloc_free_index = palloc_index;
        }
        palloc_index++;
        if (palloc_index >= VM_USERHI_PI) {
            palloc_index = VM_USERLO_PI;
        }
    }

    if (palloc_free_index == nps) {
        palloc_free_index = 0;
        last_palloc_index = VM_USERLO_PI;
    } else {
        at_set_allocated(palloc_free_index, 1);
        last_palloc_index = palloc_free_index;
    }

    mem_unlock();

    return palloc_free_index;
}

unsigned int palloc_multi(unsigned int size)
{
    unsigned int nps;
    unsigned int palloc_index;
    unsigned int palloc_free_index;
    unsigned int count = 0;
    bool first;
    bool found = FALSE;

    mem_lock();
    KERN_DEBUG("check1\n");

    nps = get_nps();
    palloc_index = last_palloc_index;
    palloc_free_index = nps;
    first = TRUE;
    // KERN_DEBUG(found ? "true\n":"false\n");
    // KERN_DEBUG("Palloc index: %u\n", palloc_index);
    // KERN_DEBUG("Last Palloc index: %u\n", last_palloc_index);
    // KERN_DEBUG("Palloc index: %u\n", palloc_index);
    while ((palloc_index != last_palloc_index || first) && found == FALSE) {
        first = FALSE;
        // KERN_DEBUG("check2\n");
        if (at_is_norm(palloc_index) && !at_is_allocated(palloc_index)) {
            if (count == 0) {
                palloc_free_index = palloc_index;
            }
            if (++count >= size) {
                found = TRUE;
            }
            // KERN_DEBUG("count: %d\n", count);
        }
        palloc_index++;
        if (palloc_index >= VM_USERHI_PI) {
            palloc_index = VM_USERLO_PI;
            count = 0;
        }
    }

    if (found) {
        for (count = 0; count < size; count++) {
            at_set_allocated(palloc_free_index + count, 1);
        }
        last_palloc_index = palloc_free_index + count;
    } else {
        palloc_free_index = 0;
        last_palloc_index = VM_USERLO_PI;
    }

    mem_unlock();

    return palloc_free_index;
}

unsigned int palloc_multi_bb(unsigned int size) {

    unsigned int count;
    unsigned int min_up_size;
    unsigned int min_up_index;
    unsigned int testing_count = 0;

    
    mem_lock();
    if (size != get_log2(size)) {
        size = 1 << (get_log2(size) + 1);
    }
    KERN_DEBUG("got here\n");
    // KERN_DEBUG("get_bb_total size: %u\n", get_bb_total_size());
    KERN_DEBUG("goal size: %u\n", size);
    while(TRUE) {
        testing_count++;
        count = 0;
        min_up_size = get_bb_total_size() + 1;
        min_up_index = get_bb_total_size() + 1;
        while(count < get_bb_total_size()) {
            testing_count++;
            // if (testing_count > 50) {
            //     mem_unlock();
            //     return 0;
            // }
            KERN_DEBUG("\n ----- count: %u\n", count);
            KERN_DEBUG("bb size of count: %u\n", bb_get_size(count));
            KERN_DEBUG("min_up_size: %u\n", min_up_size);
            // KERN_DEBUG("current size: %u\n", size);
            KERN_DEBUG("is used? %u\n", bb_get_used(count));
            if (bb_get_size(count) == size && bb_get_used(count) == 0) {
                // KERN_DEBUG("got here?onvgojqbeovbnqoeno\n");
                bb_set_used(count,1);
                mem_unlock();
                KERN_DEBUG("\n\ncount: %u\n", count);
                KERN_DEBUG("get result: %u\n", get_bb_offset() + count);
                return count + get_bb_offset();
            }
            
            if ((bb_get_size(count) < min_up_size) && (bb_get_size(count) > size) && (bb_get_used(count) == 0)) {
                min_up_size = bb_get_size(count);
                min_up_index = count;
            }
            count += bb_get_size(count);
        }
        KERN_DEBUG("min_up_size: %u\n", min_up_size);
        KERN_DEBUG("Min up index: %u\n", min_up_index);
        if (min_up_index > get_bb_total_size()) {
            mem_unlock();
            return 0;
        }
        
        // KERN_DEBUG("Min up size: %u\n", min_up_size);
        bb_split(min_up_index);
    }

    mem_unlock();
    return 0;
    
}


/**
 * Free a physical page.
 *
 * This function marks the page with given index as unallocated
 * in the allocation table.
 *
 * Hint: Simple.
 */
void pfree(unsigned int pfree_index)
{
    mem_lock();
    at_set_allocated(pfree_index, 0);
    mem_unlock();
}

void pfree_bb(unsigned int pfree_index) {
    unsigned int count;
    unsigned int buddy;
    unsigned int temp;

    mem_lock();
    count = pfree_index - get_bb_offset();
    if (bb_get_used(count) == 0) {
        KERN_DEBUG("freeing unused buddy block\n");
    }
    KERN_DEBUG("freeing mi amigo %u\n", count);
    while(TRUE) {
        bb_set_used(count, 0);
        buddy = bb_get_buddy(count, bb_get_size(count));
        KERN_DEBUG("freeing mi buddy %u\n", buddy);
        if (buddy >= get_bb_total_size()) {
            mem_unlock();
            return;
        }
        if (bb_get_used(buddy) == 0 && bb_get_size(buddy) == bb_get_size(count)) {
            if (buddy < count) {
                temp = count;
                count = buddy;
                buddy = temp;
            }
            bb_set_size(buddy, 0);
            bb_set_size(count, bb_get_size(count)*2);
        } else {
            mem_unlock();
            return;
        }
    }
    mem_unlock();
    return;
}
