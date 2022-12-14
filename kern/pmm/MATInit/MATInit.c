#include <lib/debug.h>
#include <lib/types.h>
#include "import.h"

#define PAGESIZE     4096
#define VM_USERLO    0x40000000
#define VM_USERHI    0xF0000000
#define VM_USERLO_PI (VM_USERLO / PAGESIZE)
#define VM_USERHI_PI (VM_USERHI / PAGESIZE)

unsigned int get_log2(unsigned int start) {
    unsigned int count = 0;
    while (start > 1) {
        start /= 2;
        count++;
    }
    return count;
}

/**
 * The initialization function for the allocation table AT.
 * It contains two major parts:
 * 1. Calculate the actual physical memory of the machine, and sets the number
 *    of physical pages (NUM_PAGES).
 * 2. Initializes the physical allocation table (AT) implemented in the MATIntro layer
 *    based on the information available in the physical memory map table.
 *    Review import.h in the current directory for the list of available
 *    getter and setter functions.
 */
void pmem_init(unsigned int mbi_addr)
{
    unsigned int nps;
    unsigned int pg_idx, pmmap_size, cur_addr, highest_addr;
    unsigned int entry_idx, flag, isnorm, start, len;
    unsigned int max_block_size = 0;
    unsigned int curr_block_size = 0;
    unsigned int max_start = 0;
    unsigned int curr_start = 0;

    // Calls the lower layer initialization primitive.
    // The parameter mbi_addr should not be used in the further code.
    devinit(mbi_addr);
    mem_spinlock_init();

    /**
     * Calculate the total number of physical pages provided by the hardware and
     * store it into the local variable nps.
     * Hint: Think of it as the highest address in the ranges of the memory map table,
     *       divided by the page size.
     */
    nps = 0;
    entry_idx = 0;
    pmmap_size = get_size();
    while (entry_idx < pmmap_size) {
        cur_addr = get_mms(entry_idx) + get_mml(entry_idx);
        if (nps < cur_addr) {
            nps = cur_addr;
        }
        entry_idx++;
    }

    nps = ROUNDDOWN(nps, PAGESIZE) / PAGESIZE;
    set_nps(nps);  // Setting the value computed above to NUM_PAGES.

    /**
     * Initialization of the physical allocation table (AT).
     *
     * In CertiKOS, all addresses < VM_USERLO or >= VM_USERHI are reserved by the kernel.
     * That corresponds to the physical pages from 0 to VM_USERLO_PI - 1,
     * and from VM_USERHI_PI to NUM_PAGES - 1.
     * The rest of the pages that correspond to addresses [VM_USERLO, VM_USERHI)
     * can be used freely ONLY IF the entire page falls into one of the ranges in
     * the memory map table with the permission marked as usable.
     *
     * Hint:
     * 1. You have to initialize AT for all the page indices from 0 to NPS - 1.
     * 2. For the pages that are reserved by the kernel, simply set its permission to 1.
     *    Recall that the setter at_set_perm also marks the page as unallocated.
     *    Thus, you don't have to call another function to set the allocation flag.
     * 3. For the rest of the pages, explore the memory map table to set its permission
     *    accordingly. The permission should be set to 2 only if there is a range
     *    containing the entire page that is marked as available in the memory map table.
     *    Otherwise, it should be set to 0. Note that the ranges in the memory map are
     *    not aligned by pages, so it may be possible that for some pages, only some of
     *    the addresses are in a usable range. Currently, we do not utilize partial pages,
     *    so in that case, you should consider those pages as unavailable.
     */
    pg_idx = 0;
    while (pg_idx < nps) {
        if (pg_idx < VM_USERLO_PI || VM_USERHI_PI <= pg_idx) {
            at_set_perm(pg_idx, 1);
        } else {
            entry_idx = 0;
            flag = 0;
            isnorm = 0;
            while (entry_idx < pmmap_size && !flag) {
                isnorm = is_usable(entry_idx);
                start = get_mms(entry_idx);
                len = get_mml(entry_idx);
                if (start <= pg_idx * PAGESIZE && (pg_idx + 1) * PAGESIZE <= start + len) {
                    flag = 1;
                }
                entry_idx++;
            }

            if (flag && isnorm) {
                at_set_perm(pg_idx, 2);
                if (curr_block_size == 0) {
                    curr_start = pg_idx;
                }
                curr_block_size++;
                if (curr_block_size > max_block_size) {
                    max_block_size = curr_block_size;
                    max_start = curr_start;
                }
            } else {
                at_set_perm(pg_idx, 0);
                curr_block_size = 0;
                curr_start = 0;
            }
        }
        pg_idx++;
    }

    //finds longest contiguous block of memory and makes
    // a buddy block system with that size rounded down to nearest power of 2
    //mark these parts of memory as unavailable

    unsigned int block_expo = get_log2(max_block_size);
    set_bb_total_size(1 << block_expo);
    set_bb_offset(max_start);
    bb_set_size(0, get_bb_total_size());
    bb_set_used(0, 0);
    at_set_perm(get_bb_offset(), 0);
    for (unsigned int i = 1; i < get_bb_total_size(); i++) { 
        bb_set_size(i, 0);
        bb_set_used(i, 0);
        at_set_perm(i + get_bb_offset(), 0);
    }
    

    // KERN_DEBUG("max block size: %u\n", max_block_size);
    // KERN_DEBUG("max block start: %u\n", max_start);
    // KERN_DEBUG("test count: %d\n", get_log2(max_block_size));
}
