
/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020-2022 Loongson Technology Corporation Limited
 */
#ifndef _ASM_ASM_OFFSETS_H
#define _ASM_ASM_OFFSETS_H


#ifdef ARCH_CPU_64BIT
	#define LONGSIZE	8
    #define LONGMASK	7
    #define LONGLOG		3
#endif

/* LoongArch pt_regs offsets. */
#define PT_R0      (0  * LONGSIZE)     
#define PT_R1      (1  * LONGSIZE)     
#define PT_R2      (2  * LONGSIZE)    
#define PT_R3      (3  * LONGSIZE)    
#define PT_R4      (4  * LONGSIZE)    
#define PT_R5      (5  * LONGSIZE)    
#define PT_R6      (6  * LONGSIZE)    
#define PT_R7      (7  * LONGSIZE)    
#define PT_R8      (8  * LONGSIZE)    
#define PT_R9      (9  * LONGSIZE)    
#define PT_R10     (10 * LONGSIZE)  
#define PT_R11     (11 * LONGSIZE)  
#define PT_R12     (12 * LONGSIZE)  
#define PT_R13     (13 * LONGSIZE) 
#define PT_R14     (14 * LONGSIZE) 
#define PT_R15     (15 * LONGSIZE) 
#define PT_R16     (16 * LONGSIZE) 
#define PT_R17     (17 * LONGSIZE) 
#define PT_R18     (18 * LONGSIZE) 
#define PT_R19     (19 * LONGSIZE) 
#define PT_R20     (20 * LONGSIZE) 
#define PT_R21     (21 * LONGSIZE) 
#define PT_R22     (22 * LONGSIZE) 
#define PT_R23     (23 * LONGSIZE) 
#define PT_R24     (24 * LONGSIZE) 
#define PT_R25     (25 * LONGSIZE) 
#define PT_R26     (26 * LONGSIZE) 
#define PT_R27     (27 * LONGSIZE) 
#define PT_R28     (28 * LONGSIZE) 
#define PT_R29     (29 * LONGSIZE) 
#define PT_R30     (30 * LONGSIZE) 
#define PT_R31     (31 * LONGSIZE) 

#define PT_CRMD     (32 * LONGSIZE)
#define PT_PRMD     (33 * LONGSIZE)
#define PT_EUEN     (34 * LONGSIZE)
#define PT_ESTAT    (35 * LONGSIZE)
#define PT_ERA      (36 * LONGSIZE)
#define PT_BVADDR   (37 * LONGSIZE)

#define PT_REG_END	(PT_BVADDR + LONGSIZE)










#endif /* _ASM_ASM_OFFSETS_H */

