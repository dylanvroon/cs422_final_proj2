#ifndef _KERN_PMM_MATOP_H_
#define _KERN_PMM_MATOP_H_

#ifdef _KERN_

unsigned int palloc(void);
unsigned int palloc_multi(unsigned int size);
unsigned int palloc_multi_bb(unsigned int size);
void pfree(unsigned int pfree_index);
void pfree_bb(unsigned int pfree_index);

#endif  /* _KERN_ */

#endif  /* !_KERN_PMM_MATOP_H_ */
