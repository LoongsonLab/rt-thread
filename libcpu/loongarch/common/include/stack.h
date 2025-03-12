
#ifndef __STACK_H__
#define __STACK_H__

#include "asm-offsets.h"
#include "stackframe.h"

#include <rtthread.h>

typedef struct rt_hw_switch_frame
{
    rt_ubase_t r_ra;
    rt_ubase_t r_s0;
    rt_ubase_t r_s1;
    rt_ubase_t r_s2;
    rt_ubase_t r_s3;
    rt_ubase_t r_s4;
    rt_ubase_t r_s5;
    rt_ubase_t r_s6;
    rt_ubase_t r_s7;
    rt_ubase_t r_s8;
    rt_ubase_t r_fp;
    rt_ubase_t r_tp;
    rt_ubase_t r_sp;
    rt_ubase_t r_crmd;
    rt_ubase_t r_fcsr;
    rt_ubase_t r_fcc;
} *rt_hw_switch_frame_t;


struct pt_regs
{
    rt_ubase_t r_zero;       /* zero - zero  - not used                            */
    rt_ubase_t r_ra;         /* r1  - ra     - return address for jumps            */
    rt_ubase_t r_tp;         /* r2  - tp     - thread pointer                      */
    rt_ubase_t r_sp;         /* r3  - sp     - stack pointer                       */
    rt_ubase_t r_a0;         /* r4  - a0     - return value or function argument 0 */
    rt_ubase_t r_a1;         /* r5  - a1     - return value or function argument 1 */
    rt_ubase_t r_a2;         /* r6  - a2     - function argument 2                 */
    rt_ubase_t r_a3;         /* r7  - a3     - function argument 3                 */
    rt_ubase_t r_a4;         /* r8  - a4     - function argument 4                 */
    rt_ubase_t r_a5;         /* r9  - a5     - function argument 5                 */
    rt_ubase_t r_a6;         /* r10 - a6     - function argument 6                 */
    rt_ubase_t r_a7;         /* r11 - a7     - function argument 7                 */
    rt_ubase_t r_t0;         /* r12 - t0     - temporary register 0                */
    rt_ubase_t r_t1;         /* r13 - t1     - temporary register 1                */
    rt_ubase_t r_t2;         /* r14 - t2     - temporary register 2                */
    rt_ubase_t r_t3;         /* r15 - t3     - temporary register 3                */
    rt_ubase_t r_t4;         /* r16 - t4     - temporary register 4                */
    rt_ubase_t r_t5;         /* r17 - t5     - temporary register 5                */
    rt_ubase_t r_t6;         /* r18 - t6     - temporary register 6                */
    rt_ubase_t r_t7;         /* r19 - t7     - temporary register 7                */
    rt_ubase_t r_t8;         /* r20 - t8     - temporary register 8                */
    rt_ubase_t r_r21;        /* r21 -        - not used                            */
    rt_ubase_t r_fp;         /* r22 - fp     - frame pointer                       */
    rt_ubase_t r_s0;         /* r23 - s0     - saved register 0                    */
    rt_ubase_t r_s1;         /* r24 - s1     - saved register 1                    */
    rt_ubase_t r_s2;         /* r25 - s2     - saved register 2                    */
    rt_ubase_t r_s3;         /* r26 - s3     - saved register 3                    */
    rt_ubase_t r_s4;         /* r27 - s4     - saved register 4                    */
    rt_ubase_t r_s5;         /* r28 - s5     - saved register 5                    */
    rt_ubase_t r_s6;         /* r29 - s6     - saved register 6                    */
    rt_ubase_t r_s7;         /* r30 - s7     - saved register 7                    */
    rt_ubase_t r_s8;         /* r31 - s8     - saved register 8                    */
    rt_ubase_t r_crmd;       /* csr crmd   (r32) */
    rt_ubase_t r_prmd;       /* csr prmd   (r33) */
    rt_ubase_t r_euen;       /* csr euen   (r34) */
    rt_ubase_t r_estat;      /* csr estat  (r35) */
    rt_ubase_t r_era;        /* csr era    (r36) */
    rt_ubase_t r_bvaddr;     /* csr bvaddr (r37) */
    rt_ubase_t r_ecfg;       /* csr ecfg   (r38) */
    rt_ubase_t __padding;    /* align to 16bytes */
#ifdef ARCH_LOONGARCH_FPU
    rt_ubase_t f[32];      /* f0~f31 */
#endif /* ARCH_LOONGARCH_FPU */
};


#define DEFAULT_THREAD_CRMD 0xb4

#endif /* __ASM_STACK_H__ */
