

#include <rtthread.h>
#include <rthw.h>
#include <mmu.h>

#include "loongarch.h"

#define MAX_HANDLERS 128


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

/**
 * This function will un-mask a interrupt.
 * @param vector the interrupt number
 */
void rt_hw_interrupt_umask(int vector)
{

}

/**
 * This function will install a interrupt service routine to a interrupt.
 * @param vector the interrupt number
 * @param new_handler the interrupt service routine to be installed
 * @param old_handler the old interrupt service routine
 */
rt_isr_handler_t rt_hw_interrupt_install(int vector, rt_isr_handler_t handler,
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
}

// default function
static void rt_hw_exception_default_handle(void)
{
    rt_kprintf("UN-handled reserved exception occurred!!!\n");
}

void *exception_table[EXCCODE_INT_START] = {
	[0 ... EXCCODE_INT_START - 1] = rt_hw_exception_default_handle,

	[EXCCODE_TLBI]		= handle_tlb_load_ptw,
	[EXCCODE_TLBL]		= handle_tlb_load_ptw,
	[EXCCODE_TLBS]		= handle_tlb_store_ptw,
	[EXCCODE_TLBM]		= handle_tlb_modify_ptw,

	[EXCCODE_ADE]		= handle_ade,
	[EXCCODE_ALE]		= handle_ale,
	[EXCCODE_BCE]		= handle_bce,
	[EXCCODE_SYS]		= handle_sys,
	[EXCCODE_BP]		= handle_bp,
	[EXCCODE_INE]		= handle_ri,
	[EXCCODE_IPE]		= handle_ri,
	[EXCCODE_FPDIS]		= handle_fpu,
	[EXCCODE_FPE]		= handle_fpe,
};


void do_ade(void) 
{
	rt_kprintf("UN-handled do_ade exception occurred!!!\n");
	while(1);
}

void do_ale(void) 
{
	rt_kprintf("UN-handled do_ale exception occurred!!!\n");
	while(1);
}

void do_bce(void) 
{
	rt_kprintf("UN-handled do_bce exception occurred!!!\n");
	while(1);
}

void do_bp(void) 
{
	rt_kprintf("UN-handled do_bp exception occurred!!!\n");
	while(1);
}

void do_fpe(void) 
{
	rt_kprintf("UN-handled do_fpe exception occurred!!!\n");
	while(1);
}

void do_fpu(void) 
{
	rt_kprintf("UN-handled do_fpu exception occurred!!!\n");
	while(1);
}

void do_ri(void) 
{
	rt_kprintf("UN-handled do_ri exception occurred!!!\n");
	while(1);
}


void do_vint(void) 
{
	rt_kprintf("UN-handled do_ri exception occurred!!!\n");
	while(1);
}


#define SZ_4K		0x00001000
#define SZ_8K		0x00002000
#define SZ_16K		0x00004000
#define SZ_32K		0x00008000
#define SZ_64K		0x00010000
#define VECSIZE     0x200

long exception_handlers[VECSIZE * 128 / sizeof(long)] __attribute__((__aligned__(SZ_64K)));

unsigned long eentry;
unsigned long tlbrentry;

static void setup_vint_size(unsigned int size)
{
	unsigned int vs;
	//TODO:
	vs = 7;
	csr_xchg32(vs<<CSR_ECFG_VS_SHIFT, CSR_ECFG_VS, LOONGARCH_CSR_ECFG);
}

static void configure_exception_vector(void)
{
	eentry    = (unsigned long)exception_handlers;
	csr_write64(eentry, LOONGARCH_CSR_EENTRY);
}

/* Install CPU exception handler */
void set_handler(unsigned long offset, void *addr, unsigned long size)
{
	memcpy((void *)(eentry + offset), addr, size);
	// clear ICache
	__asm__ volatile ("\tibar 0\n"::);
}

void trap_init(void) {
	long i;

	setup_vint_size(7);

	configure_exception_vector();

	// copy exception handler to exception vector

	/* Set interrupt vector handler */
	for (i = EXCCODE_INT_START; i <= EXCCODE_INT_END; i++)
		set_handler(i * VECSIZE, handle_vint, VECSIZE);

	/* Set exception vector handler */
	for (i = EXCCODE_ADE; i <= EXCCODE_BTDIS; i++)
		set_handler(i * VECSIZE, exception_table[i], VECSIZE);
}


