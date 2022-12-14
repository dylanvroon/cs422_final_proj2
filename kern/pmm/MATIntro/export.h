#ifndef _KERN_PMM_MATINTRO_H_
#define _KERN_PMM_MATINTRO_H_

#ifdef _KERN_

void mem_spinlock_init(void);
void mem_lock(void);
void mem_unlock(void);

unsigned int get_nps(void);
void set_nps(unsigned int page_index);

unsigned int at_is_norm(unsigned int page_index);
void at_set_perm(unsigned int page_index, unsigned int perm);

unsigned int at_is_allocated(unsigned int page_index);
void at_set_allocated(unsigned int page_index, unsigned int allocated);

unsigned int get_bb_offset(void);
void set_bb_offset(unsigned int page_index);

unsigned int get_bb_total_size(void);
void set_bb_total_size(unsigned int size);

unsigned int bb_get_size(unsigned int page_index);
void bb_set_size(unsigned int page_index, unsigned int size);

unsigned int bb_get_used(unsigned int page_index);
void bb_set_used(unsigned int page_index, unsigned int used);

// unsigned int bb_get_start(unsigned int page_index);
// void bb_set_start(unsigned int page_index, unsigned int start);

// unsigned int bb_get_next(unsigned int page_index);
// void bb_set_next(unsigned int page_index, unsigned int next);

unsigned int bb_get_buddy(unsigned int page_index, unsigned int size);
void bb_split(unsigned int page_index);

#endif  /* _KERN_ */

#endif  /* !_KERN_PMM_MATINTRO_H_ */
