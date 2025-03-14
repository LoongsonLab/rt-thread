/*
 * Copyright (C) 2020-2025 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-10     LoongsonLab  the first version
 * 2025-03-12     LoongsonLab  add fpu support
 * 2025-03-13     LoongsonLab  fix save and restore
 */

#ifndef __ASM_STACKFRAME_H__
#define __ASM_STACKFRAME_H__

#include "regdef.h"
#include "asm-offsets.h"
#include "loongarch.h"

#ifdef RT_USING_FPU
#include "fpregdef.h"
#endif


#ifdef __ASSEMBLY__
.macro SAVE_FPU
	
.endm

.macro BACKUP_T0T1
	csrwr	t0, EXCEPTION_KS0
	csrwr	t1, EXCEPTION_KS1
.endm

.macro RELOAD_T0T1
	csrrd   t0, EXCEPTION_KS0
	csrrd   t1, EXCEPTION_KS1
.endm

.macro	SAVE_TEMP
	RELOAD_T0T1
	LONG_ST	t0, sp, PT_R12
	LONG_ST	t1, sp, PT_R13
	LONG_ST	t2, sp, PT_R14
	LONG_ST	t3, sp, PT_R15
	LONG_ST	t4, sp, PT_R16
	LONG_ST	t5, sp, PT_R17
	LONG_ST	t6, sp, PT_R18
	LONG_ST	t7, sp, PT_R19
	LONG_ST	t8, sp, PT_R20
.endm

.macro	SAVE_STATIC
	LONG_ST	s0, sp, PT_R23
	LONG_ST	s1, sp, PT_R24
	LONG_ST	s2, sp, PT_R25
	LONG_ST	s3, sp, PT_R26
	LONG_ST	s4, sp, PT_R27
	LONG_ST	s5, sp, PT_R28
	LONG_ST	s6, sp, PT_R29
	LONG_ST	s7, sp, PT_R30
	LONG_ST	s8, sp, PT_R31
.endm


.macro	SAVE_SOME
	csrrd	t1, LOONGARCH_CSR_PRMD
	andi	t1, t1, 0x3	/* extract pplv bit */
	move	t0, sp
	beqz	t1, 8f
	/* Called from user mode, new stack. */
	csrrd   sp, EXCEPTION_KS2
8:
	addi.d  sp, sp, -PT_SIZE
	LONG_ST	t0, sp, PT_R3
	LONG_ST	zero, sp, PT_R0
	csrrd	t0, LOONGARCH_CSR_PRMD
	LONG_ST	t0, sp, PT_PRMD
	csrrd	t0, LOONGARCH_CSR_CRMD
	LONG_ST	t0, sp, PT_CRMD
	csrrd	t0, LOONGARCH_CSR_EUEN
	LONG_ST t0, sp, PT_EUEN
	csrrd	t0, LOONGARCH_CSR_ECFG
	LONG_ST	t0, sp, PT_ECFG
	csrrd	t0, LOONGARCH_CSR_ESTAT
	LONG_ST	t0, sp, PT_ESTAT
	LONG_ST	ra, sp, PT_R1
	LONG_ST	a0, sp, PT_R4
	LONG_ST	a1, sp, PT_R5
	LONG_ST	a2, sp, PT_R6
	LONG_ST	a3, sp, PT_R7
	LONG_ST	a4, sp, PT_R8
	LONG_ST	a5, sp, PT_R9
	LONG_ST	a6, sp, PT_R10
	LONG_ST	a7, sp, PT_R11
	csrrd	ra, LOONGARCH_CSR_ERA
	LONG_ST	ra, sp, PT_ERA
	LONG_ST	tp, sp, PT_R2
	LONG_ST	fp, sp, PT_R22

	/* Set thread_info if we're coming from user mode */
	csrrd	t0, LOONGARCH_CSR_PRMD
	andi	t0, t0, 0x3	/* extract pplv bit */
	beqz	t0, 9f

	LONG_ST	u0, sp, PT_R21
	csrrd	u0, PERCPU_BASE_KS
9:
.endm


.macro SAVE_ALL
	SAVE_SOME
	SAVE_TEMP
	SAVE_STATIC
.endm


.macro	RESTORE_TEMP
	LONG_LD	t0, sp, PT_R12
	LONG_LD	t1, sp, PT_R13
	LONG_LD	t2, sp, PT_R14
	LONG_LD	t3, sp, PT_R15
	LONG_LD	t4, sp, PT_R16
	LONG_LD	t5, sp, PT_R17
	LONG_LD	t6, sp, PT_R18
	LONG_LD	t7, sp, PT_R19
	LONG_LD	t8, sp, PT_R20
.endm

.macro	RESTORE_STATIC
	LONG_LD	s0, sp, PT_R23
	LONG_LD	s1, sp, PT_R24
	LONG_LD	s2, sp, PT_R25
	LONG_LD	s3, sp, PT_R26
	LONG_LD	s4, sp, PT_R27
	LONG_LD	s5, sp, PT_R28
	LONG_LD	s6, sp, PT_R29
	LONG_LD	s7, sp, PT_R30
	LONG_LD	s8, sp, PT_R31
.endm


.macro	RESTORE_SOME
	LONG_LD	a0, sp, PT_PRMD
	andi    a0, a0, 0x3	/* extract pplv bit */
	beqz    a0, 8f
	LONG_LD  u0, sp, PT_R21
8:
	LONG_LD	a0, sp, PT_ERA
	csrwr	a0, LOONGARCH_CSR_ERA
	LONG_LD	a0, sp, PT_PRMD
	csrwr	a0, LOONGARCH_CSR_PRMD
	LONG_LD	ra, sp, PT_R1
	LONG_LD	a0, sp, PT_R4
	LONG_LD	a1, sp, PT_R5
	LONG_LD	a2, sp, PT_R6
	LONG_LD	a3, sp, PT_R7
	LONG_LD	a4, sp, PT_R8
	LONG_LD	a5, sp, PT_R9
	LONG_LD	a6, sp, PT_R10
	LONG_LD	a7, sp, PT_R11
	LONG_LD	fp, sp, PT_R22

	addi.d  tp, sp, PT_SIZE
	csrwr	tp, EXCEPTION_KS2

	LONG_LD	tp, sp, PT_R2
.endm

.macro	RESTORE_SP_AND_RET
	LONG_LD	sp, sp, PT_R3
	ertn
.endm

.macro RESTORE_ALL_AND_RET
	RESTORE_STATIC
	RESTORE_TEMP
	RESTORE_SOME
	RESTORE_SP_AND_RET
.endm


.macro	cpu_save_nonscratch thread
	LONG_ST	s0, \thread, RT_THREAD_S0
	LONG_ST	s1, \thread, RT_THREAD_S1
	LONG_ST	s2, \thread, RT_THREAD_S2
	LONG_ST	s3, \thread, RT_THREAD_S3
	LONG_ST	s4, \thread, RT_THREAD_S4
	LONG_ST	s5, \thread, RT_THREAD_S5
	LONG_ST	s6, \thread, RT_THREAD_S6
	LONG_ST	s7, \thread, RT_THREAD_S7
	LONG_ST	s8, \thread, RT_THREAD_S8
	LONG_ST	sp, \thread, RT_THREAD_SP
	LONG_ST	fp, \thread, RT_THREAD_FP
	LONG_ST	tp, \thread, RT_THREAD_TP
.endm

.macro	cpu_save_csr_reg thread
	csrrd	t1, LOONGARCH_CSR_PRMD
	stptr.d	t1, \thread, RT_THREAD_PRMD
	csrrd	t1, LOONGARCH_CSR_CRMD
	stptr.d	t1, \thread, RT_THREAD_CRMD
.endm


.macro	cpu_restore_nonscratch thread
	LONG_LD	ra, \thread, RT_THREAD_RA
	LONG_LD	s0, \thread, RT_THREAD_S0
	LONG_LD	s1, \thread, RT_THREAD_S1
	LONG_LD	s2, \thread, RT_THREAD_S2
	LONG_LD	s3, \thread, RT_THREAD_S3
	LONG_LD	s4, \thread, RT_THREAD_S4
	LONG_LD	s5, \thread, RT_THREAD_S5
	LONG_LD	s6, \thread, RT_THREAD_S6
	LONG_LD	s7, \thread, RT_THREAD_S7
	LONG_LD	s8, \thread, RT_THREAD_S8
	LONG_LD	fp, \thread, RT_THREAD_FP
	LONG_LD	tp, \thread, RT_THREAD_TP
.endm

.macro	cpu_restore_csr_reg thread
	ldptr.d	t1, \thread, RT_THREAD_PRMD
	csrwr	t1, LOONGARCH_CSR_PRMD
	ldptr.d	t1, \thread, RT_THREAD_CRMD
	csrwr	t1, LOONGARCH_CSR_CRMD
.endm

.macro	cpu_restore_sp thread
	LONG_LD	sp, \thread, RT_THREAD_SP
.endm

.macro fpu_save_csr thread tmp
	movfcsr2gr	\tmp, fcsr0
	stptr.w		\tmp, \thread, RT_THREAD_FCSR
.endm

.macro fpu_restore_csr thread tmp0
	ldptr.w		\tmp0, \thread, RT_THREAD_FCSR
	movgr2fcsr	fcsr0, \tmp0
.endm


.macro fpu_save_cc thread tmp0 tmp1
	movcf2gr	\tmp0, $fcc0
	move	\tmp1, \tmp0
	movcf2gr	\tmp0, $fcc1
	bstrins.d	\tmp1, \tmp0, 15, 8
	movcf2gr	\tmp0, $fcc2
	bstrins.d	\tmp1, \tmp0, 23, 16
	movcf2gr	\tmp0, $fcc3
	bstrins.d	\tmp1, \tmp0, 31, 24
	movcf2gr	\tmp0, $fcc4
	bstrins.d	\tmp1, \tmp0, 39, 32
	movcf2gr	\tmp0, $fcc5
	bstrins.d	\tmp1, \tmp0, 47, 40
	movcf2gr	\tmp0, $fcc6
	bstrins.d	\tmp1, \tmp0, 55, 48
	movcf2gr	\tmp0, $fcc7
	bstrins.d	\tmp1, \tmp0, 63, 56
	stptr.d		\tmp1, \thread, RT_THREAD_FCC
.endm

.macro fpu_restore_cc thread tmp0 tmp1
	ldptr.d	\tmp0, \thread, RT_THREAD_FCC
	bstrpick.d	\tmp1, \tmp0, 7, 0
	movgr2cf	$fcc0, \tmp1
	bstrpick.d	\tmp1, \tmp0, 15, 8
	movgr2cf	$fcc1, \tmp1
	bstrpick.d	\tmp1, \tmp0, 23, 16
	movgr2cf	$fcc2, \tmp1
	bstrpick.d	\tmp1, \tmp0, 31, 24
	movgr2cf	$fcc3, \tmp1
	bstrpick.d	\tmp1, \tmp0, 39, 32
	movgr2cf	$fcc4, \tmp1
	bstrpick.d	\tmp1, \tmp0, 47, 40
	movgr2cf	$fcc5, \tmp1
	bstrpick.d	\tmp1, \tmp0, 55, 48
	movgr2cf	$fcc6, \tmp1
	bstrpick.d	\tmp1, \tmp0, 63, 56
	movgr2cf	$fcc7, \tmp1
.endm

.macro	fpu_save_double thread tmp
	li.w	\tmp, RT_THREAD_FPR0
	PTR_ADD \tmp, \tmp, \thread
	fst.d	$f0, \tmp, RT_THREAD_FPR0  - RT_THREAD_FPR0
	fst.d	$f1, \tmp, RT_THREAD_FPR1  - RT_THREAD_FPR0
	fst.d	$f2, \tmp, RT_THREAD_FPR2  - RT_THREAD_FPR0
	fst.d	$f3, \tmp, RT_THREAD_FPR3  - RT_THREAD_FPR0
	fst.d	$f4, \tmp, RT_THREAD_FPR4  - RT_THREAD_FPR0
	fst.d	$f5, \tmp, RT_THREAD_FPR5  - RT_THREAD_FPR0
	fst.d	$f6, \tmp, RT_THREAD_FPR6  - RT_THREAD_FPR0
	fst.d	$f7, \tmp, RT_THREAD_FPR7  - RT_THREAD_FPR0
	fst.d	$f8, \tmp, RT_THREAD_FPR8  - RT_THREAD_FPR0
	fst.d	$f9, \tmp, RT_THREAD_FPR9  - RT_THREAD_FPR0
	fst.d	$f10, \tmp, RT_THREAD_FPR10 - RT_THREAD_FPR0
	fst.d	$f11, \tmp, RT_THREAD_FPR11 - RT_THREAD_FPR0
	fst.d	$f12, \tmp, RT_THREAD_FPR12 - RT_THREAD_FPR0
	fst.d	$f13, \tmp, RT_THREAD_FPR13 - RT_THREAD_FPR0
	fst.d	$f14, \tmp, RT_THREAD_FPR14 - RT_THREAD_FPR0
	fst.d	$f15, \tmp, RT_THREAD_FPR15 - RT_THREAD_FPR0
	fst.d	$f16, \tmp, RT_THREAD_FPR16 - RT_THREAD_FPR0
	fst.d	$f17, \tmp, RT_THREAD_FPR17 - RT_THREAD_FPR0
	fst.d	$f18, \tmp, RT_THREAD_FPR18 - RT_THREAD_FPR0
	fst.d	$f19, \tmp, RT_THREAD_FPR19 - RT_THREAD_FPR0
	fst.d	$f20, \tmp, RT_THREAD_FPR20 - RT_THREAD_FPR0
	fst.d	$f21, \tmp, RT_THREAD_FPR21 - RT_THREAD_FPR0
	fst.d	$f22, \tmp, RT_THREAD_FPR22 - RT_THREAD_FPR0
	fst.d	$f23, \tmp, RT_THREAD_FPR23 - RT_THREAD_FPR0
	fst.d	$f24, \tmp, RT_THREAD_FPR24 - RT_THREAD_FPR0
	fst.d	$f25, \tmp, RT_THREAD_FPR25 - RT_THREAD_FPR0
	fst.d	$f26, \tmp, RT_THREAD_FPR26 - RT_THREAD_FPR0
	fst.d	$f27, \tmp, RT_THREAD_FPR27 - RT_THREAD_FPR0
	fst.d	$f28, \tmp, RT_THREAD_FPR28 - RT_THREAD_FPR0
	fst.d	$f29, \tmp, RT_THREAD_FPR29 - RT_THREAD_FPR0
	fst.d	$f30, \tmp, RT_THREAD_FPR30 - RT_THREAD_FPR0
	fst.d	$f31, \tmp, RT_THREAD_FPR31 - RT_THREAD_FPR0
.endm

.macro	fpu_restore_double thread tmp
	li.w	\tmp, RT_THREAD_FPR0
	add.d   \tmp, \tmp, \thread
	fld.d	$f0, \tmp, RT_THREAD_FPR0  - RT_THREAD_FPR0
	fld.d	$f1, \tmp, RT_THREAD_FPR1  - RT_THREAD_FPR0
	fld.d	$f2, \tmp, RT_THREAD_FPR2  - RT_THREAD_FPR0
	fld.d	$f3, \tmp, RT_THREAD_FPR3  - RT_THREAD_FPR0
	fld.d	$f4, \tmp, RT_THREAD_FPR4  - RT_THREAD_FPR0
	fld.d	$f5, \tmp, RT_THREAD_FPR5  - RT_THREAD_FPR0
	fld.d	$f6, \tmp, RT_THREAD_FPR6  - RT_THREAD_FPR0
	fld.d	$f7, \tmp, RT_THREAD_FPR7  - RT_THREAD_FPR0
	fld.d	$f8, \tmp, RT_THREAD_FPR8  - RT_THREAD_FPR0
	fld.d	$f9, \tmp, RT_THREAD_FPR9  - RT_THREAD_FPR0
	fld.d	$f10, \tmp, RT_THREAD_FPR10 - RT_THREAD_FPR0
	fld.d	$f11, \tmp, RT_THREAD_FPR11 - RT_THREAD_FPR0
	fld.d	$f12, \tmp, RT_THREAD_FPR12 - RT_THREAD_FPR0
	fld.d	$f13, \tmp, RT_THREAD_FPR13 - RT_THREAD_FPR0
	fld.d	$f14, \tmp, RT_THREAD_FPR14 - RT_THREAD_FPR0
	fld.d	$f15, \tmp, RT_THREAD_FPR15 - RT_THREAD_FPR0
	fld.d	$f16, \tmp, RT_THREAD_FPR16 - RT_THREAD_FPR0
	fld.d	$f17, \tmp, RT_THREAD_FPR17 - RT_THREAD_FPR0
	fld.d	$f18, \tmp, RT_THREAD_FPR18 - RT_THREAD_FPR0
	fld.d	$f19, \tmp, RT_THREAD_FPR19 - RT_THREAD_FPR0
	fld.d	$f20, \tmp, RT_THREAD_FPR20 - RT_THREAD_FPR0
	fld.d	$f21, \tmp, RT_THREAD_FPR21 - RT_THREAD_FPR0
	fld.d	$f22, \tmp, RT_THREAD_FPR22 - RT_THREAD_FPR0
	fld.d	$f23, \tmp, RT_THREAD_FPR23 - RT_THREAD_FPR0
	fld.d	$f24, \tmp, RT_THREAD_FPR24 - RT_THREAD_FPR0
	fld.d	$f25, \tmp, RT_THREAD_FPR25 - RT_THREAD_FPR0
	fld.d	$f26, \tmp, RT_THREAD_FPR26 - RT_THREAD_FPR0
	fld.d	$f27, \tmp, RT_THREAD_FPR27 - RT_THREAD_FPR0
	fld.d	$f28, \tmp, RT_THREAD_FPR28 - RT_THREAD_FPR0
	fld.d	$f29, \tmp, RT_THREAD_FPR29 - RT_THREAD_FPR0
	fld.d	$f30, \tmp, RT_THREAD_FPR30 - RT_THREAD_FPR0
	fld.d	$f31, \tmp, RT_THREAD_FPR31 - RT_THREAD_FPR0
.endm

#endif


#endif /* __ASM_STACKFRAME_H__ */