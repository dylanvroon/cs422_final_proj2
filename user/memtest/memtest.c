#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <x86.h>
#include <file.h>
#include <gcc.h>

#define exit(...) return __VA_ARGS__

// NOTE: in this an in subsequent tests the location "750*1024*4096" is used. This may look arbitrary, but it is just for the purpose of being in a non-kernel reserved space. 
void superpage_locations(void) {
    printf("=====superpage test=====\n");

    // Can allocate in available space
    void *vaddr = (void *)(750*1024*4096);
    if (brk(vaddr, 0) == -1) {
        printf("failed to allocate in free space%d\n", vaddr); 
        exit();
    }
    free(vaddr, 0);


    // Can't allocate in kernel space
    vaddr = (void *)(1*1024*4096);
    if (brk(vaddr, 0) == 0) {
        printf("failed to protect against allocating in kernel space%d\n", vaddr); 
        exit();
    }
    free(vaddr, 0);

    printf("=====superpage finish=====\n\n");
}

void multipage_small(void) {
    printf("=====multipage small test=====\n");

    // Can allocate in available space (not using buddy)
    void *vaddr = (void *)(750*1024*4096);
    if (brk(vaddr, 12) == -1) {
        printf("failed to allocate size 12 multi page%d\n", vaddr); 
        exit();
    }
    free(vaddr, 0);

    // Can allocate in available space (using buddy)
    vaddr = (void *)(750*1024*4096);
    if (brk(vaddr, 32) == -1) {
        printf("failed to allocate size 32 multi page%d\n", vaddr); 
        exit();
    }
    free(vaddr, 0);

    printf("=====multipage small finish=====\n\n");
}

void multipage_big(void) {
    printf("=====multipage big test=====\n");

    // Can allocate big chunk in available space using buddy
    void *vaddr = (void *)(750*1024*4096);
    if (brk(vaddr, 128) == -1) {
        printf("failed to allocate size 100 multi%d\n", vaddr); 
        exit();
    }
    free(vaddr, 0);

    // Can allocate in available space (using buddy)
    vaddr = (void *)(750*1024*4096);
    if (brk(vaddr, 1024) == -1) {
        printf("failed to allocate size 1000 multi%d\n", vaddr); 
        exit();
    }
    free(vaddr, 0);

    printf("=====multipage big finish=====\n\n");

}

// Also serves as robustness test to see if can handle allocating 20,000 pages. 
void overload(void) {
    printf("=====overload test=====\n");

    // Allocate 19 superpages (= 19456 pages), meaning next one will go over quota
    for (int i = 0; i < 19; i++) {
        int vaddr = (750*1024*4096);
        brk((void *) (vaddr + i*1024*4096), 0);
    }

    // Check if 20th page brings over quota or not
    if (brk((void *)(770*1024*4096), 0) != -1){
        printf("Surpassed 20,000 page quota\n"); 
        exit();
    }

    printf("=====overload finish=====\n\n");
}

int main(int argc, char *argv[])
{
    printf("*******usertests starting*******\n\n");
    superpage_locations();
    multipage_small();
    multipage_big();
    overload();
    printf("*******end of tests*******\n");
    return 0;
}










