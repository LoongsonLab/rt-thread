/*
 * Copyright (C) 2020-2025 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-10     LoongsonLab  the first version
 */

#include "asm-generic.h"
#include "asm-offsets.h"
#include "cpuport.h"
#include "stackframe.h"


START_POINT(handle_syscall)
	move		t2, sp /* t2 save user sp*/
	csrrd		sp, EXCEPTION_KS2
	
	addi.d		sp, sp, -PT_SIZE
	LONG_ST		t2, sp, PT_R3
	LONG_ST		zero, sp, PT_R0
	csrrd		t2, LOONGARCH_CSR_PRMD
	LONG_ST		t2, sp, PT_PRMD
	csrrd		t2, LOONGARCH_CSR_CRMD
	LONG_ST		t2, sp, PT_CRMD
	csrrd		t2, LOONGARCH_CSR_EUEN
	LONG_ST		t2, sp, PT_EUEN
	csrrd		t2, LOONGARCH_CSR_ECFG
	LONG_ST		t2, sp, PT_ECFG
	csrrd		t2, LOONGARCH_CSR_ESTAT
	LONG_ST		t2, sp, PT_ESTAT
	LONG_ST		ra, sp, PT_R1
	LONG_ST		a0, sp, PT_R4
	LONG_ST		a1, sp, PT_R5
	LONG_ST		a2, sp, PT_R6
	LONG_ST		a3, sp, PT_R7
	LONG_ST		a4, sp, PT_R8
	LONG_ST		a5, sp, PT_R9
	LONG_ST		a6, sp, PT_R10
	LONG_ST		a7, sp, PT_R11
	csrrd		ra, LOONGARCH_CSR_ERA
	LONG_ST		ra, sp, PT_ERA

	LONG_ST		tp, sp, PT_R2
	LONG_ST		u0, sp, PT_R21
	LONG_ST		fp, sp, PT_R22

	SAVE_STATIC

	move        a0, sp 
	bl 			rt_dispatch_syscall

	/* sp need switch to user sp*/
	RESTORE_ALL_AND_RET

START_POINT_END(handle_syscall)

