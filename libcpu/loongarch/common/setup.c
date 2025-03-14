/*
 * Copyright (C) 2020-2025 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-10     LoongsonLab  the first version
 */


unsigned long fw_arg0;
unsigned long fw_arg1;
unsigned long fw_arg2;


static unsigned long drdtime(void)
{
	unsigned long val = 0;

	__asm__ __volatile__(
		"rdtime.d %0, $zero\n\t"
		: "=r"(val)
		:
		);
	return val;
}

extern void trap_init(void);

void init_cpu_early(void)
{
	trap_init();
}



