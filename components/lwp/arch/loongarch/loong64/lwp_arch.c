

#include <rthw.h>
#include <rtthread.h>
#include <lwp_internal.h>
#ifdef ARCH_MM_MMU

#define DBG_TAG "lwp.arch"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#include "stack.h"

#include <lwp_arch.h>
#include <lwp_user_mm.h>

extern size_t MMUTable[];



int arch_user_space_init(struct rt_lwp *lwp)
{
    rt_ubase_t *mmu_table;

    mmu_table = rt_hw_mmu_pgtbl_create();
    if (!mmu_table)
    {
        return -RT_ENOMEM;
    }

    lwp->end_heap = USER_HEAP_VADDR;
    lwp->aspace = rt_aspace_create(
        (void *)USER_VADDR_START, USER_VADDR_TOP - USER_VADDR_START, mmu_table);
    if (!lwp->aspace)
    {
        return -RT_ERROR;
    }

    return 0;
}

void *arch_get_user_sp(void)
{
    /* user sp saved in interrupt context */
    rt_thread_t self = rt_thread_self();
    rt_uint8_t *stack_top = (rt_uint8_t *)self->stack_addr + self->stack_size;
    struct pt_regs *frame = (struct pt_regs *)(stack_top - sizeof(struct pt_regs));

    return (void *)frame->r_u_stack;
}

void *arch_kernel_mmu_table_get(void)
{
    return (void *)((char *)MMUTable);
}


void arch_user_space_free(struct rt_lwp *lwp)
{
    if (lwp)
    {
        RT_ASSERT(lwp->aspace);

        void *pgtbl = lwp->aspace->page_table;
        rt_aspace_delete(lwp->aspace);

        /* must be freed after aspace delete, pgtbl is required for unmap */
        rt_hw_mmu_pgtbl_delete(pgtbl);
        lwp->aspace = RT_NULL;
    }
    else
    {
        LOG_W("%s: NULL lwp as parameter", __func__);
        RT_ASSERT(0);
    }
}

#endif /* ARCH_MM_MMU */


/**
 * set exec context for fork/clone.
 */
int arch_set_thread_context(void (*exit)(void), void *new_thread_stack,
                            void *user_stack, void **thread_sp)
{
    
    return 0;
}


#define ALGIN_BYTES (16)

struct signal_ucontext
{
    rt_int64_t sigreturn;
    lwp_sigset_t save_sigmask;

    siginfo_t si;

    rt_align(16)
    struct pt_regs frame;
};


void *arch_signal_ucontext_restore(rt_base_t user_sp, rt_base_t kernel_sp)
{
    struct signal_ucontext *new_sp;
    new_sp = (void *)user_sp;

    if (lwp_user_accessable(new_sp, sizeof(*new_sp)))
    {
        lwp_thread_signal_mask(rt_thread_self(), LWP_SIG_MASK_CMD_SET_MASK, &new_sp->save_sigmask, RT_NULL);
        arch_signal_post_action(new_sp, kernel_sp);
    }
    else
    {
        LOG_I("User frame corrupted during signal handling\nexiting...");
        sys_exit_group(EXIT_FAILURE);
    }

    return (char *)&new_sp->frame;
}


void *arch_signal_ucontext_save(rt_base_t user_sp, siginfo_t *psiginfo,
                                struct pt_regs *exp_frame,
                                lwp_sigset_t *save_sig_mask)
{
    struct signal_ucontext *new_sp;
    new_sp = (void *)((user_sp - sizeof(struct signal_ucontext)) & ~0xf);

    if (lwp_user_accessable(new_sp, sizeof(*new_sp)))
    {
        /* push psiginfo */
        if (psiginfo)
        {
            lwp_memcpy(&new_sp->si, psiginfo, sizeof(*psiginfo));
        }

        /* exp frame is already aligned as AAPCS64 required */
        lwp_memcpy(&new_sp->frame, exp_frame, sizeof(*exp_frame));

        /* copy the save_sig_mask */
        lwp_memcpy(&new_sp->save_sigmask, save_sig_mask, sizeof(lwp_sigset_t));

        /* copy lwp_sigreturn */
        const size_t lwp_sigreturn_bytes = 8;
        extern void lwp_sigreturn(void);
        /* -> ensure that the sigreturn start at the outer most boundary */
        lwp_memcpy(&new_sp->sigreturn,  &lwp_sigreturn, lwp_sigreturn_bytes);
    }
    else
    {
        LOG_I("%s: User stack overflow", __func__);
        sys_exit_group(EXIT_FAILURE);
    }

    return new_sp;
}


int arch_backtrace_uthread(rt_thread_t thread)
{
    struct rt_hw_backtrace_frame frame;
    struct pt_regs *stack;

    if (thread && thread->lwp)
    {
        stack = thread->user_ctx.ctx;
        if ((long)stack > (unsigned long)thread->stack_addr
            && (long)stack < (unsigned long)thread->stack_addr + thread->stack_size)
        {
            frame.pc = stack->r_era;
            frame.fp = stack->r_fp;
            lwp_backtrace_frame(thread, &frame);
            return 0;
        }
        else
            return -1;
    }
    return -1;
}

