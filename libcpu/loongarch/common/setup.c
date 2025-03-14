/*
 * Copyright (C) 2020-2025 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-10     LoongsonLab  the first version
 */

#include "cpuport.h"

// Save parameters passed by loaders
unsigned long fw_arg0;
unsigned long fw_arg1;
unsigned long fw_arg2;


void init_cpu_early(void)
{
	trap_init();
}



