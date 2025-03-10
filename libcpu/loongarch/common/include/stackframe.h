#ifndef __ASM_STACKFRAME_H__
#define __ASM_STACKFRAME_H__

#include "regdef.h"
#include "asm-offsets.h"

#ifdef RT_USING_FPU
#include "fpregdef.h"
#endif

#ifdef ARCH_CPU_64BIT
	#define LONG_ADD	add.d
	#define LONG_ADDI	addi.d
	#define LONG_SUB	sub.d
	#define LONG_L		ld.d
	#define LONG_S		st.d
#else
	#error "Now, RT-Thread only suport LoongArch64"
#endif

.macro SAVE_ALL

.endm

.macro RESTORE_ALL

.endm


.macro OPEN_INTERRUPT
    csrsi sstatus, 2
.endm

.macro CLOSE_INTERRUPT
    csrci sstatus, 2
.endm




#endif /* __ASM_STACKFRAME_H__ */