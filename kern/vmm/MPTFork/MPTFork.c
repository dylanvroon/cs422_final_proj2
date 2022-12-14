#include <lib/gcc.h>
#include <lib/x86.h>
#include <lib/debug.h>

#include "import.h"

#define PT_PERM_UP  0
#define PT_PERM_PTU (PTE_P | PTE_W | PTE_U)
#define PT_PERM_COW (PTE_P | PTE_COW | PTE_U)

#define ADDR_MASK(x) ((unsigned int) x & 0xfffff000)
#define PERM_MASK(x) ((unsigned int) x & 0x00000fff)

#define PDE_ADDR(x) (x >> 22)
#define PTE_ADDR(x) ((x >> 12) & 0x3ff)

#define PAGESIZE      4096
#define PDIRSIZE      (PAGESIZE * 1024)
#define VM_USERLO     0x40000000
#define VM_USERHI     0xF0000000
#define VM_USERLO_PDE (VM_USERLO / PDIRSIZE)
#define VM_USERHI_PDE (VM_USERHI / PDIRSIZE)

/**
 * Step 1 of the spec
 */
void setup_child_pdir(unsigned int parent, unsigned int child) 
{
    //ATTENTION:::: I think we need to allocate new space for page tables here (which I haven't done yet)
    //NOTE: Fixed this ^
    unsigned int pde_index, pte_index, ptbl_entry, chi_page_index, par_page_index, perm;

    for (pde_index = VM_USERLO_PDE; pde_index < VM_USERHI_PDE) {
        if (get_pdir_entry(parent,pde_index) != 0) {
            vaddr = pde_index << 22;
            chi_page_index = container_alloc(child);
            if (chi_page_index == 0) {
                return; //Seet err_no to E_MEM too maybe?
            }
            set_pdir_entry(child, pde_index, chi_page_index);
            for (pte_index = 0; pte_index < 1024; pte_index++) {
                if (get_ptbl_entry(parent, pde_index, pte_index) != 0) {
                    ptbl_entry = get_ptbl_entry(parent, pde_index, pte_index);
                    page_index = ptbl_entry >> 12;
                    perm = PERM_MASK(ptbl_entry);
                    if (perm & PTE_W > 0 && perm & PTE_COW == 0) {
                        perm -= PTE_W;
                        perm += PTW_COW;
                    } else if (perm & PTE_W > 0 && perm & PTW_COW > 0) {
                        perm -= PTE_W;
                    }
                    set_ptbl_entry(parent, pde_index, pte_index, page_index, PTE_PERM_COW);
                    set_ptbl_entry(child, pde_index, pte_index, page_index, PTE_PERM_COW);
                } else {
                    rmv_ptbl_entry(child, pde_index, pte_index);
                }
            }
        } else {
            rmv_pdir_entry(child, pde_index);
        }
    }
}

unsigned int copy_on_write(unsigned int pid, unsigned int vaddr) {
    //This is slightly/majorly wrong since I need to assume already allocated page directory
    //NOTE: Fixed this ^
    unsigned int paddr = get_ptbl_entry_by_va(pid, vaddr);
    unsigned int perm = PERM_MASK(paddr);
    unsigned int page_index = container_alloc(pid);
    if (paddr == 0 || page_index == 0) {
        return MagicNumber;
    }
    if (perm & PTE_COW > 0) {
        perm -= PTE_COW;
    }
    if (perm & PTE_W == 0) {
        perm += PTE_W;
    }
    return map_page(pid, vaddr, page_index, perm);
    // set_ptbl_entry_by_va(pid, vaddr, page_index, perm);
    // return pde_page_index;
}

