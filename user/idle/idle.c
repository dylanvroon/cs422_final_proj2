#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <x86.h>

int main(int argc, char **argv)
{
    printf("idle\n");

    pid_t memtest_pid;

    if ((memtest_pid = spawn(4, 20000)) != -1)
        printf("memtest in process %d.\n", memtest_pid);
    else
        printf("Failed to launch memtest.\n");

    return 0;
}
