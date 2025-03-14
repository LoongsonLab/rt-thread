/*
 * Copyright (C) 2020-2025 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-10     LoongsonLab  the first version
 * 2025-03-14     LoongsonLab  refactor exception routine
 */

#include "loongarch.h"
#include "regdef.h"
#include "asm-generic.h"
#include "stackframe.h"

.macro  build_prep_badv
    csrrd     t0, LOONGARCH_CSR_BADV
    LONG_ST   t0, sp, PT_BVADDR
.endm

.macro  build_prep_fcsr
#ifdef RT_USING_FPU
    movfcsr2gr  a1, fcsr0
#endif
.endm

.macro  build_prep_none
.endm

.macro  BUILD_HANDLER exception handler prep
    .align  5
    START_POINT(handle_\exception)
_666_\exception:
    BACKUP_T0T1
    SAVE_ALL
    build_prep_\prep
    move    a0, sp
    la.abs  t0, do_\handler
    jirl    ra, t0, 0
    la.abs  t0, ret_from_exception
    jirl    ra, t0, 0
    START_POINT_END(handle_\exception)
.endm


/* 
   We judge if execute thread switch when return from
   exception. 
   Note: this codes must be called by instrution 
   "la.abs and jirl" in exception routine.
*/
START_POINT(ret_from_exception)
ret_from_exception:
    la.abs  t0, rt_thread_switch_interrupt_flag
    LONG_LD t1, t0, 0
    beqz    t1, exception_resume /*resume exec*/  
    LONG_ST zero, t0, 0

exception_context_switch:/*context_switch*/
    la.abs  t0, rt_interrupt_from_thread
    LONG_LD a0, t0, 0
    la.abs  t0, rt_interrupt_to_thread
    LONG_LD a1, t0, 0
    /* Disable Interrupt*/
    li.w    t0, CSR_CRMD_IE
    csrxchg  zero, t0, LOONGARCH_CSR_CRMD
    la.abs  t0, rt_hw_context_switch
    jirl    ra, t0, 0

exception_resume:/*resume exec*/
#ifdef RT_USING_SMART
    LONG_LD t0, sp, PT_PRMD
    andi    t0, t0, 0x3 /* extract pplv bit */
    beqz    a0, exception_resume_kernel
exception_resume_user:
    la.abs  t0, arch_ret_to_user
    jirl    ra, t0, 0      
#endif

exception_resume_kernel: /*resume_kernel*/    
    RESTORE_ALL_AND_RET
never_return:
    b never_return
START_POINT_END(ret_from_exception)


/* handle interrupt */
    BUILD_HANDLER vint rt_dispatch_trap none

    BUILD_HANDLER ade ade badv
    BUILD_HANDLER ale ale badv
    BUILD_HANDLER bce bce none
    BUILD_HANDLER bp bp none
    BUILD_HANDLER fpe fpe fcsr
    BUILD_HANDLER fpu fpu none
    BUILD_HANDLER ri ri none

    .align 2
    .global trap_entry
trap_entry:
    ret


.global handle_tlb_load_ptw
handle_tlb_load_ptw:
    jirl     zero, ra, 0

.global handle_tlb_store_ptw
handle_tlb_store_ptw:
    jirl     zero, ra, 0

.global handle_tlb_modify_ptw
handle_tlb_modify_ptw:
    jirl     zero, ra, 0


START_POINT(handle_sys)
    la.abs  t0, handle_syscall
    jr  t0
START_POINT_END(handle_sys)


# Enable and Disable Interrupt
/*
 * void rt_hw_interrupt_enable(rt_base_t level);
 *
 * a0 --> to level
 */
.global rt_hw_interrupt_enable
rt_hw_interrupt_enable: 
    csrwr    a0, LOONGARCH_CSR_CRMD
    jirl     zero, ra, 0

/*
 * rt_base_t rt_hw_interrupt_disable(void);
 *
 * a0 --> to rt_base_t
 */
.global rt_hw_interrupt_disable
rt_hw_interrupt_disable:
    move     a0, zero
    li.w     t0, CSR_CRMD_IE
    csrxchg  a0, t0, LOONGARCH_CSR_CRMD
    jirl	 zero, ra, 0

