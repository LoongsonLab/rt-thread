/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020-2022 Loongson Technology Corporation Limited
 */
#ifndef _ASM_TIMER_H
#define _ASM_TIMER_H


#define CPU_HZ       (200 * 1000 * 1000)

void rt_hw_timer_init(void);

void rt_hw_timer_handler(void);

#endif /* _ASM_TIMER_H */






