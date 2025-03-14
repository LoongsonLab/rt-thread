/*
 * Copyright (C) 2020-2025 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-10     LoongsonLab  the first version
 */

#ifndef __ASM_CPUPORT_H__
#define __ASM_CPUPORT_H__

#include <rttypes.h>
#include <rtcompiler.h>

void trap_init(void);
void init_smp(void);
void init_mmu(void);

rt_inline rt_ubase_t drdtime(void)
{
	rt_ubase_t val = 0;

	__asm__ __volatile__(
		"rdtime.d %0, $zero\n\t"
		: "=r"(val)
		:
		);
	return val;
}

#endif /* __ASM_CPUPORT_H__ */
