/*
 * Copyright (C) 2020-2025 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-10     LoongsonLab  the first version
 * 2025-03-22     LoongsonLab  fix tlb refill and page fault
 * 2025-03-23     LoongsonLab  add soft and hardware ptw
 */

#include <rtthread.h>
#include <rthw.h>
#include <mmu.h>

#ifdef RT_USING_SMART
#include <lwp.h>
#endif

#include "loongarch.h"
#include "stack.h"
#include "timer.h"
#include "tlb.h"

#define MAX_HANDLERS 128

#define DEBUG_TRAP_TRACE 0

/* Exception and interrupt handler table */
struct rt_irq_desc irq_desc[MAX_HANDLERS];

// default function
static rt_isr_handler_t rt_hw_interrupt_default_handle(rt_uint32_t vector, void *param)
{
    rt_kprintf("UN-handled interrupt %d occurred!!!\n", vector);
    return RT_NULL;
}

extern void handle_vint(void);

extern void handle_ade(void);
extern void handle_ale(void);
extern void handle_bce(void);
extern void handle_bp(void);
extern void handle_fpe(void);
extern void handle_fpu(void);
extern void handle_ri(void);

extern void handle_sys(void);

extern void handle_tlb_load_ptw(void);
extern void handle_tlb_store_ptw(void);
extern void handle_tlb_modify_ptw(void);

extern void handle_tlb_sw_refill(void);

/**
 * This function will un-mask a interrupt.
 * @param vector the interrupt number
 */
rt_weak void rt_hw_interrupt_umask(int vector)
{

}

/**
 * This function will install a interrupt service routine to a interrupt.
 * @param vector the interrupt number
 * @param new_handler the interrupt service routine to be installed
 * @param old_handler the old interrupt service routine
 */
rt_weak rt_isr_handler_t rt_hw_interrupt_install(int vector, rt_isr_handler_t handler,
        void *param, const char *name)
{
    rt_isr_handler_t old_handler = RT_NULL;

    if(vector < MAX_HANDLERS)
    {
        old_handler = irq_desc[vector].handler;
        if (handler != RT_NULL)
        {
            irq_desc[vector].handler = (rt_isr_handler_t)handler;
            irq_desc[vector].param = param;
#ifdef RT_USING_INTERRUPT_INFO
            rt_snprintf(irq_desc[vector].name, RT_NAME_MAX - 1, "%s", name);
            irq_desc[vector].counter = 0;
#endif
        }
    }

    return old_handler;
}

rt_weak void platform_irq_init()
{
}

rt_weak void platform_generic_irq()
{
	rt_uint64_t hw_irq_pending = (read_csr_estat() & CSR_ESTAT_IS_HW);
	rt_uint64_t hw_irq_index = 0;

	while (hw_irq_pending) {
		rt_uint64_t bit = ffs(hw_irq_pending);
		hw_irq_index = bit - 1 ;

		irq_desc[hw_irq_index].handler(hw_irq_index, irq_desc[hw_irq_index].param);
		hw_irq_pending &= ~(1UL << ( bit - 1));

#ifdef RT_USING_INTERRUPT_INFO0
        rt_snprintf(irq_desc[hw_irq_index].name, RT_NAME_MAX - 1, "%s", name);
        irq_desc[hw_irq_index].counter ++;
#endif

#if DEBUG_TRAP_TRACE
// #if 1
    	rt_kprintf("     Hardware Exception ID: %d\n", hw_irq_index);
#endif
	}
}


void rt_hw_interrupt_init()
{
    /* Enable machine external interrupts. */
    int idx = 0;
    /* init exceptions table */
    for (idx = 0; idx < MAX_HANDLERS; idx++)
    {
        irq_desc[idx].handler = (rt_isr_handler_t)rt_hw_interrupt_default_handle;
        irq_desc[idx].param = RT_NULL;
#ifdef RT_USING_INTERRUPT_INFO
        rt_snprintf(irq_desc[idx].name, RT_NAME_MAX - 1, "default");
        irq_desc[idx].counter = 0;
#endif
    }

    platform_irq_init();
}

// default function
static void rt_hw_exception_default_handle(void)
{
    rt_kprintf("UN-handled reserved exception occurred!!!\n");
}

void *exception_table[EXCCODE_INT_START] = {
	[0 ... EXCCODE_INT_START - 1] = rt_hw_exception_default_handle,

#ifdef RT_USING_SMART
	[EXCCODE_TLBI]		= handle_tlb_load_ptw,
	[EXCCODE_TLBL]		= handle_tlb_load_ptw,
	[EXCCODE_TLBS]		= handle_tlb_store_ptw,
	[EXCCODE_TLBM]		= handle_tlb_modify_ptw,
	[EXCCODE_SYS]		= handle_sys,
#endif

	[EXCCODE_ADE]		= handle_ade,
	[EXCCODE_ALE]		= handle_ale,
	[EXCCODE_BCE]		= handle_bce,
	[EXCCODE_BP]		= handle_bp,
	[EXCCODE_INE]		= handle_ri,
	[EXCCODE_IPE]		= handle_ri,
	[EXCCODE_FPDIS]		= handle_fpu,
	[EXCCODE_FPE]		= handle_fpe,
};


void do_ade(struct pt_regs *regs) 
{
	rt_kprintf("UN-handled do_ade exception occurred!!!\n");
    rt_kprintf("Ra :\n");
    rt_kprintf("    0x%lx\n", regs->r_ra);
    rt_kprintf("Era:\n");
    rt_kprintf("    0x%lx\n", regs->r_era);
	while(1);
}

void do_ale(struct pt_regs *regs) 
{
	rt_kprintf("UN-handled do_ale exception occurred!!!\n");
    rt_kprintf("Ra :\n");
    rt_kprintf("    0x%lx\n", regs->r_ra);
    rt_kprintf("Era:\n");
    rt_kprintf("    0x%lx\n", regs->r_era);
	while(1);
}

void do_bce(struct pt_regs *regs) 
{
	rt_kprintf("UN-handled do_bce exception occurred!!!\n");
    rt_kprintf("Ra :\n");
    rt_kprintf("    0x%lx\n", regs->r_ra);
    rt_kprintf("Era:\n");
    rt_kprintf("    0x%lx\n", regs->r_era);
	while(1);
}

void do_bp(struct pt_regs *regs) 
{
	rt_kprintf("UN-handled do_bp exception occurred!!!\n");
	while(1);
}

void do_fpe(struct pt_regs *regs) 
{
	rt_kprintf("UN-handled do_fpe exception occurred!!!\n");
	while(1);
}

void do_fpu(struct pt_regs *regs) 
{
	rt_kprintf("UN-handled do_fpu exception occurred!!!\n");
	while(1);
}

void do_ri(struct pt_regs *regs) 
{
	rt_kprintf("UN-handled do_ri exception occurred!!!\n");
    rt_kprintf(" Ra:\n");
    rt_kprintf("    0x%lx\n", regs->r_ra);
    rt_kprintf("Era:\n");
    rt_kprintf("    0x%lx\n", regs->r_era);
	while (1);
}


void do_rt_dispatch_trap(struct pt_regs *regs) 
{
	rt_uint64_t estat;
	rt_thread_t thread = rt_thread_self();

	estat = read_csr_estat() & CSR_ESTAT_IS;
#if DEBUG_TRAP_TRACE
	rt_kprintf("Exception Thread: 0x%lx, pt_regs: 0x%lx\n", thread, regs);
#endif
	if ((estat & CSR_ESTAT_IS_IPI))
	{
	#if DEBUG_TRAP_TRACE
		rt_kprintf("---------- Enter Interrupt ----------\n");
		rt_kprintf("      IPI Exception occurred!        \n");
	#endif
		while (1);
	} else if ((estat & CSR_ESTAT_IS_TI))
	{
	#if DEBUG_TRAP_TRACE
		rt_kprintf("---------- Enter Interrupt ----------\n");
		rt_kprintf("     Timer Exception occurred!       \n");
	#endif
		write_csr_tintclear(CSR_TINTCLR_TI);
		rt_interrupt_enter();
		rt_hw_timer_handler();
		rt_interrupt_leave();
		return;
	} else if ((estat & CSR_ESTAT_IS_HW))
	{
		write_csr_estat(estat & ~CSR_ESTAT_IS_HW);
		rt_interrupt_enter();
		platform_generic_irq();
		rt_interrupt_leave();
	#if DEBUG_TRAP_TRACE
		rt_kprintf("---------- Enter Interrupt ----------\n");
		rt_kprintf("     Hardware Exception occurred: 0x%lx\n", estat);
	#endif
		return;
	} else {
	#if DEBUG_TRAP_TRACE
		rt_kprintf("---------- Enter Interrupt ----------\n");
		rt_kprintf("UN-handled rt_dispatch_trap exception occurred!!!\n");
	#endif
		while(1);
	}
}

#ifdef RT_USING_SMART
void do_page_fault(struct pt_regs *regs)
{
	rt_ubase_t id = (read_csr_estat() & CSR_ESTAT_EXC ) >> CSR_ESTAT_EXC_SHIFT;
	rt_ubase_t estat = read_csr_estat();
    struct rt_lwp *lwp;
    rt_base_t saved_stat;
	/* user page fault */
    enum rt_mm_fault_op fault_op;
    enum rt_mm_fault_type fault_type;
    switch (id)
    {
        case MMU_STAT_EXCODE_PIL: // load page invalid
            fault_op = MM_FAULT_OP_READ;
            fault_type = MM_FAULT_TYPE_PAGE_FAULT;
            break;
        case MMU_STAT_EXCODE_PIS: // store page invalid
            fault_op = MM_FAULT_OP_WRITE;
            fault_type = MM_FAULT_TYPE_PAGE_FAULT;
            break;
        case MMU_STAT_EXCODE_PIF: // fetch instruction page invalid
            fault_op = MM_FAULT_OP_EXECUTE;
            fault_type = MM_FAULT_TYPE_PAGE_FAULT;
            break;
        case MMU_STAT_EXCODE_PME: // page modified
            fault_op = MM_FAULT_OP_WRITE;
            fault_type = MM_FAULT_TYPE_PAGE_FAULT;
            break;
        default:
            fault_op = 0;
            fault_type = MM_FAULT_TYPE_GENERIC;
    }
    if (fault_op)
    {
        lwp = lwp_self();
        struct rt_aspace_fault_msg msg = {
            .fault_op = fault_op,
            .fault_type = fault_type,
            .fault_vaddr = (void *)regs->r_bvaddr,
        };

        saved_stat = rt_hw_interrupt_disable();
        if (!lwp)
        	goto bad_varea;
        if (rt_aspace_fault_try_fix(lwp->aspace, &msg)) {
            if (id == MMU_STAT_EXCODE_PME) {
                // Page existed
                rt_hw_mmu_update_modify_page(lwp->aspace, regs->r_bvaddr);
            }
            goto good_varea;
        }
    }

bad_varea:
	// if return from rt_aspace_fault_try_fix is false, 
    // but if PME, this seems as valid operation
	// if (fault_op == MM_FAULT_OP_WRITE)
	// 	goto good_varea;
	rt_hw_interrupt_enable(saved_stat);
    rt_thread_t cur_thr = rt_thread_self();
    struct rt_hw_backtrace_frame frame = {.fp = regs->r_fp, .pc = regs->r_era};
	rt_kprintf("fp = %p, era = %p\n", frame.fp, frame.pc);
    lwp_backtrace_frame(cur_thr, &frame);
    sys_exit_group(-1);

good_varea:
	rt_hw_interrupt_enable(saved_stat);
	// update tlb and validate page
    rt_hw_tlb_invalidate_all_local();
    return;
}

#else
void do_page_fault(struct pt_regs *regs)
{
	rt_kprintf("UN-handled do_page_fault exception occurred!!!\n");
	while(1);
}
#endif

#define SZ_4K		0x00001000
#define SZ_8K		0x00002000
#define SZ_16K		0x00004000
#define SZ_32K		0x00008000
#define SZ_64K		0x00010000
#define VECSIZE     0x200

rt_align(SZ_64K) long exception_handlers[VECSIZE * 128 / sizeof(long)];

unsigned long eentry;
unsigned long tlbrentry;

static void setup_vint_size(unsigned int size)
{
	unsigned int vs;
	//TODO:
	vs = size;
	csr_xchg32(vs<<CSR_ECFG_VS_SHIFT, CSR_ECFG_VS, LOONGARCH_CSR_ECFG);
}

static void configure_exception_vector(void)
{
	eentry    = (unsigned long)exception_handlers;
	tlbrentry = (unsigned long)exception_handlers + 80*VECSIZE;

	csr_write64(eentry, LOONGARCH_CSR_EENTRY);
	csr_write64(eentry, LOONGARCH_CSR_MERRENTRY);
#ifdef LOONGARCH_SOFTWARE_PTW
	csr_write64(tlbrentry, LOONGARCH_CSR_TLBRENTRY);
#endif
}

/* Install CPU exception handler */
void set_handler(unsigned long offset, void *addr, unsigned long size)
{
	memcpy((void *)(eentry + offset), addr, size);
	// clear ICache
	__asm__ volatile ("\tibar 0\n"::);
}

void trap_init(void) 
{
	unsigned int i;

	setup_vint_size(7);

	configure_exception_vector();

	// copy exception handler to exception vector

	/* Set interrupt vector handler */
	for (i = EXCCODE_INT_START; i <= EXCCODE_INT_END; i++)
		set_handler(i * VECSIZE, handle_vint, VECSIZE);

	/* Set exception vector handler */
	for (i = EXCCODE_ADE; i <= EXCCODE_BTDIS; i++)
		set_handler(i * VECSIZE, exception_table[i], VECSIZE);

	for (int i = EXCCODE_TLBL; i <= EXCCODE_TLBPE; i++)
		set_handler(i * VECSIZE, exception_table[i], VECSIZE);

#ifdef LOONGARCH_SOFTWARE_PTW
	memcpy((void *)tlbrentry, handle_tlb_sw_refill, 0x80);
	__asm__ volatile ("\tibar 0\n"::);
#endif

	set_csr_ecfg(ECFGF_SIP0 | ECFGF_IP0 | ECFGF_IP1 | ECFGF_IP2 | ECFGF_IPI | ECFGF_PMC);

}


