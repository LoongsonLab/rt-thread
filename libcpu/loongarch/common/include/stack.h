
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
    rt_ubase_t r_prmd;
    rt_ubase_t r_fcsr;
    rt_ubase_t r_fcc;
} *rt_hw_switch_frame_t;


struct pt_regs
{
    rt_ubase_t r_zero;       /* zero - zero  - not used                            */
    rt_ubase_t r_ra;         /* x1  - ra     - return address for jumps            */
    rt_ubase_t r_tp;         /*              -                                     */
    rt_ubase_t r_sp;         /* x3  - sp     - stack pointer                       */
    rt_ubase_t r_a0;         /* x4  - a0     - return value or function argument 0 */
    rt_ubase_t r_a1;         /* x5  - a1     - return value or function argument 1 */
    rt_ubase_t r_a2;         /* x6  - a2     - function argument 2                 */
    rt_ubase_t r_a3;         /* x7  - a3     - function argument 3                 */
    rt_ubase_t r_a4;         /* x8  - a4     - function argument 4                 */
    rt_ubase_t r_a5;         /* x9  - a5     - function argument 5                 */
    rt_ubase_t r_a6;         /* x10 - a6     - function argument 6                 */
    rt_ubase_t r_a7;         /* x11 - a7     - function argument 7                 */
    rt_ubase_t r_t0;         /* x12 - t0     - temporary register 0                */
    rt_ubase_t r_t1;         /* x13 - t1     - temporary register 1                */
    rt_ubase_t r_t2;         /* x14 - t2     - temporary register 2                */
    rt_ubase_t r_t3;         /* x15 - t3     - temporary register 3                */
    rt_ubase_t r_t4;         /* x16 - t4     - temporary register 4                */
    rt_ubase_t r_t5;         /* x17 - t5     - temporary register 5                */
    rt_ubase_t r_t6;         /* x18 - t6     - temporary register 6                */
    rt_ubase_t r_t7;         /* x19 - t7     - temporary register 7                */
    rt_ubase_t r_t8;         /* x20 - t8     - temporary register 8                */
    rt_ubase_t r_r21;        /* x21 -        - not used                            */
    rt_ubase_t r_fp;         /* x22 - fp     - frame pointer                       */
    rt_ubase_t r_s0;         /* x23 - s0     - saved register 0                    */
    rt_ubase_t r_s1;         /* x24 - s1     - saved register 1                    */
    rt_ubase_t r_s2;         /* x25 - s2     - saved register 2                    */
    rt_ubase_t r_s3;         /* x26 - s3     - saved register 3                    */
    rt_ubase_t r_s4;         /* x27 - s4     - saved register 4                    */
    rt_ubase_t r_s5;         /* x28 - s5     - saved register 5                    */
    rt_ubase_t r_s6;         /* x29 - s6     - saved register 6                    */
    rt_ubase_t r_s7;         /* x30 - s7     - saved register 7                    */
    rt_ubase_t r_s8;         /* x31 - s8     - saved register 8                    */
    rt_ubase_t r_crmd;       /* csr crmd   */
    rt_ubase_t r_prmd;       /* csr prmd   */
    rt_ubase_t r_euen;       /* csr euen   */
    rt_ubase_t r_estat;      /* csr estat  */
    rt_ubase_t r_era;        /* csr era    */
    rt_ubase_t r_bvaddr;     /* csr bvaddr */
    rt_ubase_t r_ecfg;       /* csr ecfg   */
    rt_ubase_t __padding;  /* align to 16bytes */
#ifdef ARCH_LOONGARCH_FPU
    rt_ubase_t f[32];      /* f0~f31 */
#endif /* ARCH_LOONGARCH_FPU */
};




#endif /* __ASM_STACK_H__ */
