#ifndef _KERN_VMM_MPTFORK_H_
#define _KERN_VMM_MPTFORK_H_

#ifdef _KERN_

void setup_child_pdir(unsigned int parent, unsigned int child);
unsigned int copy_for_pdir(unsigned int pid, unsigned int vaddr);


#endif  /* _KERN_ */

#endif  /* !_KERN_VMM_MPTKERN_H_ */
