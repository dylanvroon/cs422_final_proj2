#ifndef _KERN_PMM_MATINIT_H_
#define _KERN_PMM_MATINIT_H_

#ifdef _KERN_

void pmem_init(unsigned int mbi_addr);
unsigned int get_log2(unsigned int start);

#endif  /* _KERN_ */

#endif  /* !_KERN_PMM_MATINIT_H_ */
