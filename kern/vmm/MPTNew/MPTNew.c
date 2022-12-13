#include <lib/x86.h>

#include "import.h"

/**
 * This function will be called when there's no mapping found in the page structure
 * for the given virtual address [vaddr], e.g., by the page fault handler when
 * a page fault happened because the user process accessed a virtual address
 * that is not mapped yet.
 * The task of this function is to allocate a physical page and use it to register
 * a mapping for the virtual address with the given permission.
 * It should return the physical page index registered in the page directory, the
 * return value from map_page.
 * In the case of error, it should return the constant MagicNumber.
 */
unsigned int alloc_page(unsigned int proc_index, unsigned int vaddr,
                        unsigned int perm)
{
    unsigned int page_index = container_alloc(proc_index);
    if (page_index != 0) {
        return map_page(proc_index, vaddr, page_index, perm);
    } else {
        return MagicNumber;
    }
}

//TODO alloc_multipage
unsigned int alloc_page_multi(unsigned int proc_index, unsigned int vaddr,
                        unsigned int perm, unsigned int size)
{
    unsigned int page_index = container_alloc_multi(proc_index, size);
    if (page_index != 0) {
        return map_page_multi(proc_index, vaddr, page_index, perm);
    } else {
        return MagicNumber;
    }
}

//TODO alloc_superpage
unsigned int alloc_page_super(unsigned int proc_index, unsigned int vaddr,
                        unsigned int perm)
{
    unsigned int page_index = container_alloc_multi(proc_index, SUPERPAGE_SIZE);
    if (page_index != 0) {
        return map_page_super(proc_index, vaddr, page_index, perm);
    } else {
        return MagicNumber;
    }
}


/**
 * Designate some memory quota for the next child process.
 */
unsigned int alloc_mem_quota(unsigned int id, unsigned int quota)
{
    unsigned int child;
    child = container_split(id, quota);
    return child;
}
