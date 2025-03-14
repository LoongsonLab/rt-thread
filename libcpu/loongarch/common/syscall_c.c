/*
 * Copyright (C) 2020-2025 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-10     LoongsonLab  the first version
 */

#include <rthw.h>
#include <rtthread.h>

#include "stack.h"

#ifdef RT_USING_SMART
#include <lwp_syscall.h>


typedef rt_ubase_t (*syscall_func)( rt_ubase_t, 
	                                rt_ubase_t, 
	                                rt_ubase_t, 
	                                rt_ubase_t, 
	                                rt_ubase_t, 
	                                rt_ubase_t, 
	                                rt_ubase_t);

void rt_dispatch_syscall(struct pt_regs *regs)
{
	int syscall_num = regs->r_a7;
	if (syscall_num <= 0)
	{
		LOG_E("syscall num = 0!\n");
		while (1);
	}

	syscall_func syscallfunc = (syscall_func)lwp_get_sys_api(syscall_num);

	if (syscallfunc == RT_NULL)
    {
        LOG_E("unsupported syscall!\n");
        sys_exit_group(-1);
    }

    char *syscall_name;
#if DBG_LVL >= DBG_INFO
    syscall_name = lwp_get_syscall_name(syscall_num);
#endif
    regs->r_a0 = syscallfunc(regs->r_a0, regs->r_a1, regs->r_a2, 
    	                     regs->r_a3, regs->r_a4, regs->r_a5, regs->r_a6);
    regs->r_a7 = 0;

    regs->r_era += 4; // skip ecall instruction
    LOG_I("[0x%lx] %s ret: 0x%lx", rt_thread_self(), syscall_name, regs->r_a0);
}

#else

void rt_dispatch_syscall(struct pt_regs *regs) 
{
	rt_kprintf("UN-IMPL Syscall %d occurred!!!\n", regs->r_a7);
}

#endif /* RT_USING_SMART */

