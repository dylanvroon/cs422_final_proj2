#ifndef _KERN_PMM_MCONTAINER_H_
#define _KERN_PMM_MCONTAINER_H_

#ifdef _KERN_

unsigned int get_nps(void);
unsigned int at_is_norm(unsigned int page_index);
unsigned int at_is_allocated(unsigned int page_index);
void pmem_init(unsigned int mbi_addr);
unsigned int palloc(void);
unsigned int palloc_multi(unsigned int size);
unsigned int palloc_multi_bb(unsigned int size);
void pfree(unsigned int pfree_index);
void pfree_bb(unsigned int pfree_index);

unsigned int bb_get_used(unsigned int page_index);
unsigned int get_bb_total_size(void);
unsigned int get_bb_offset(void);

#endif  /* _KERN_ */

#endif  /* !_KERN_PMM_MCONTAINER_H_ */
