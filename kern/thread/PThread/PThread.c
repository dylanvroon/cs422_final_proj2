#include <lib/x86.h>
#include <lib/thread.h>
#include <lib/spinlock.h>
#include <lib/debug.h>
#include <dev/lapic.h>
#include <pcpu/PCPUIntro/export.h>
#include <kern/thread/PTCBIntro/export.h>

#include "import.h"

static spinlock_t sched_lk;

unsigned int sched_ticks[NUM_CPUS];

void thread_init(unsigned int mbi_addr)
{
    unsigned int i;
    for (i = 0; i < NUM_CPUS; i++) {
        sched_ticks[i] = 0;
    }

    spinlock_init(&sched_lk);
    tqueue_init(mbi_addr);
    set_curid(0);
    tcb_set_state(0, TSTATE_RUN);
}

/**
 * Allocates a new child thread context, sets the state of the new child thread
 * to ready, and pushes it to the ready queue.
 * It returns the child thread id.
 */
unsigned int thread_spawn(void *entry, unsigned int id, unsigned int quota)
{
    unsigned int pid;

    spinlock_acquire(&sched_lk);

    pid = kctx_new(entry, id, quota);
    if (pid != NUM_IDS) {
        tcb_set_state(pid, TSTATE_READY);
        tqueue_enqueue(NUM_IDS, pid);
    }

    spinlock_release(&sched_lk);

    return pid;
}

/**
 * Yield to the next thread in the ready queue.
 * You should set the currently running thread state as ready,
 * and push it back to the ready queue.
 * Then set the state of the popped thread as running, set the
 * current thread id, and switch to the new kernel context.
 * Hint: If you are the only thread that is ready to run,
 * do you need to switch to yourself?
 */
void thread_yield(void)
{
    unsigned int old_cur_pid;
    unsigned int new_cur_pid;

    spinlock_acquire(&sched_lk);

    old_cur_pid = get_curid();
    tcb_set_state(old_cur_pid, TSTATE_READY);
    tqueue_enqueue(NUM_IDS, old_cur_pid);

    new_cur_pid = tqueue_dequeue(NUM_IDS);
    tcb_set_state(new_cur_pid, TSTATE_RUN);
    set_curid(new_cur_pid);

    if (old_cur_pid != new_cur_pid) {
        spinlock_release(&sched_lk);
        kctx_switch(old_cur_pid, new_cur_pid);
    }
    else {
        spinlock_release(&sched_lk);
    }
}

void sched_update(void)
{
    spinlock_acquire(&sched_lk);
    sched_ticks[get_pcpu_idx()] += 1000 / LAPIC_TIMER_INTR_FREQ;
    if (sched_ticks[get_pcpu_idx()] >= SCHED_SLICE) {
        sched_ticks[get_pcpu_idx()] = 0;
        spinlock_release(&sched_lk);
        thread_yield();
    }
    else {
        spinlock_release(&sched_lk);
    }
}

/**
 * Atomically release lock and sleep on chan.
 * Reacquires lock when awakened.
 */
void thread_sleep(void *chan, spinlock_t *lk)
{
    unsigned int old_cur_pid;
    unsigned int new_cur_pid;

    // TODO: your local variables here.

    if (lk == 0)
        KERN_PANIC("sleep without lock");

   

    spinlock_acquire(&sched_lk);
    spinlock_release(lk);

    old_cur_pid = get_curid();
    tcb_set_state(old_cur_pid, TSTATE_SLEEP);
    tcb_set_chan(old_cur_pid, chan);
    tqueue_enqueue(0, old_cur_pid);

    new_cur_pid = tqueue_dequeue(NUM_IDS);
    tcb_set_state(new_cur_pid, TSTATE_RUN);
    set_curid(new_cur_pid);

    spinlock_release(&sched_lk);
    kctx_switch(old_cur_pid, new_cur_pid);

    spinlock_acquire(&sched_lk);
    spinlock_acquire(lk);
    spinlock_release(&sched_lk);

    // TODO:
    // Must acquire sched_lk in order to change the current thread's state and
    // then switch. Once we hold sched_lk, we can be guaranteed that we won't
    // miss any wakeup (wakeup runs with sched_lk locked), so it's okay to
    // release lock.

    // TODO: Go to sleep.

    // TODO: Context switch.

    // TODO: Tidy up.

    // TODO: Reacquire original lock.

}

/**
 * Wake up all processes sleeping on chan.
 */
void thread_wakeup(void *chan)
{
    // TODO
    unsigned int iter_pid;
    unsigned int chan_addy;

    spinlock_acquire(&sched_lk);

    iter_pid = tqueue_get_head(0);
    chan_addy = (unsigned int) chan;

    while (iter_pid != NUM_IDS) {
        if ((unsigned int) tcb_get_chan(iter_pid) == chan_addy) {
            
            //wake up
            tcb_set_state(iter_pid, TSTATE_READY);
            tqueue_enqueue(NUM_IDS, iter_pid);

            tcb_set_chan(iter_pid, 0);
            tqueue_remove(0, iter_pid);

        }
        iter_pid = tcb_get_next(iter_pid);
    }

    spinlock_release(&sched_lk);
}
