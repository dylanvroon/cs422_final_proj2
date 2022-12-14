#ifndef _KERN_VMM_MPTFORK_H_
#define _KERN_VMM_MPTFORK_H_

#ifdef _KERN_

unsigned int get_pdir_entry(unsigned int proc_index, unsigned int pde_index);
unsigned int get_ptbl_entry(unsigned int proc_index, unsigned int pde_index,
void rmv_pdir_entry(unsigned int proc_index, unsigned int pde_index);
void set_ptbl_entry(unsigned int proc_index, unsigned int pde_index,
void set_pdir_entry(unsigned int proc_index, unsigned int pde_index,
void rmv_ptbl_entry(unsigned int proc_index, unsigned int pde_index,
unsigned int container_alloc(unsigned int id);
unsigned int get_pdir_entry_by_va(unsigned int proc_index, unsigned int vaddr);
unsigned int get_ptbl_entry_by_va(unsigned int proc_index, unsigned int vaddr);
void set_pdir_entry_by_va(unsigned int proc_index, unsigned int vaddr,
                          unsigned int page_index);
unsigned int map_page(unsigned int proc_index, unsigned int vaddr, unsigned int page_index, unsigned int perm);

#endif  /* _KERN_ */

#endif  /* !_KERN_MM_MPTCOMM_H_ */
