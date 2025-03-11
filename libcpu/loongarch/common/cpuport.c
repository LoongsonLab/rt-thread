
#include <rthw.h>
#include <rtthread.h>

/**
 * @brief from thread used interrupt context switch
 *
 */
volatile rt_ubase_t rt_interrupt_from_thread = 0;
/**
 * @brief to thread used interrupt context switch
 *
 */
volatile rt_ubase_t rt_interrupt_to_thread = 0;
/**
 * @brief flag to indicate context switch in interrupt or not
 *
 */
volatile rt_ubase_t rt_thread_switch_interrupt_flag = 0;

/*
 * #ifdef RT_USING_SMP
 * void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from, rt_ubase_t to, struct rt_thread *to_thread);
 * #else
 * void rt_hw_context_switch_interrupt(rt_ubase_t from, rt_ubase_t to);
 * #endif
 */
#ifndef RT_USING_SMP
void rt_hw_context_switch_interrupt(rt_ubase_t from, rt_ubase_t to, rt_thread_t from_thread, rt_thread_t to_thread)
{
    if (rt_thread_switch_interrupt_flag == 0)
        rt_interrupt_from_thread = from;

    rt_interrupt_to_thread = to;
    rt_thread_switch_interrupt_flag = 1;

    return;
}
#endif /* end of RT_USING_SMP */



