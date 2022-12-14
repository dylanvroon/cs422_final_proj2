#include <proc.h>
#include <syscall.h>
#include <types.h>

pid_t spawn(uintptr_t exec, unsigned int quota)
{
    return sys_spawn(exec, quota);
}

void yield(void)
{
    sys_yield();
}

int brk(void *addr, unsigned int n) 
{
    return sys_brk(addr, n);
}

int free(void *addr, unsigned int n)
{
    return sys_free(addr, n);
}