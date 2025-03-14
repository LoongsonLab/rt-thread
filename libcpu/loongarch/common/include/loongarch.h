/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020-2022 Loongson Technology Corporation Limited
 */
#ifndef _ASM_LOONGARCH_H
#define _ASM_LOONGARCH_H

#ifndef __ASSEMBLY__
#include <larchintrin.h>
/* CPUCFG */
#define read_cpucfg(reg) __cpucfg(reg)

#endif /* !__ASSEMBLY__ */


#ifndef __ASSEMBLY__

/* CSR */
#define csr_read32(reg) __csrrd_w(reg)
#define csr_read64(reg) __csrrd_d(reg)
#define csr_write32(val, reg) __csrwr_w(val, reg)
#define csr_write64(val, reg) __csrwr_d(val, reg)
#define csr_xchg32(val, mask, reg) __csrxchg_w(val, mask, reg)
#define csr_xchg64(val, mask, reg) __csrxchg_d(val, mask, reg)

/* IOCSR */
#define iocsr_read32(reg) __iocsrrd_w(reg)
#define iocsr_read64(reg) __iocsrrd_d(reg)
#define iocsr_write32(val, reg) __iocsrwr_w(val, reg)
#define iocsr_write64(val, reg) __iocsrwr_d(val, reg)

#endif /* !__ASSEMBLY__ */

#ifdef __ASSEMBLY__
#define _ULCAST_ 
#define __CONST64_(X,Y)	(X)
#define _CONST64_(X)	__CONST64_(X, UL)
#define BIT_ULL(x)
#else 
#define _ULCAST_ (unsigned long)
#define __CONST64_(X,Y)	(X##Y)
#define _CONST64_(X)	__CONST64_(X, UL)
#define BIT_ULL(x) (1ULL<<x)
#endif

/* Basic CSR registers */
#define LOONGARCH_CSR_CRMD		0x0	/* Current mode info */
#define  CSR_CRMD_WE_SHIFT		9
#define  CSR_CRMD_WE			(_ULCAST_(0x1) << CSR_CRMD_WE_SHIFT)
#define  CSR_CRMD_DACM_SHIFT		7
#define  CSR_CRMD_DACM_WIDTH		2
#define  CSR_CRMD_DACM			(_ULCAST_(0x3) << CSR_CRMD_DACM_SHIFT)
#define  CSR_CRMD_DACF_SHIFT		5
#define  CSR_CRMD_DACF_WIDTH		2
#define  CSR_CRMD_DACF			(_ULCAST_(0x3) << CSR_CRMD_DACF_SHIFT)
#define  CSR_CRMD_PG_SHIFT		4
#define  CSR_CRMD_PG			(_ULCAST_(0x1) << CSR_CRMD_PG_SHIFT)
#define  CSR_CRMD_DA_SHIFT		3
#define  CSR_CRMD_DA			(_ULCAST_(0x1) << CSR_CRMD_DA_SHIFT)
#define  CSR_CRMD_IE_SHIFT		2
#define  CSR_CRMD_IE			(_ULCAST_(0x1) << CSR_CRMD_IE_SHIFT)
#define  CSR_CRMD_PLV_SHIFT		0
#define  CSR_CRMD_PLV_WIDTH		2
#define  CSR_CRMD_PLV			(_ULCAST_(0x3) << CSR_CRMD_PLV_SHIFT)

#define PLV_KERN			0
#define PLV_USER			3
#define PLV_MASK			0x3

#define LOONGARCH_CSR_PRMD		0x1	/* Prev-exception mode info */
#define  CSR_PRMD_PWE_SHIFT		3
#define  CSR_PRMD_PWE			(_ULCAST_(0x1) << CSR_PRMD_PWE_SHIFT)
#define  CSR_PRMD_PIE_SHIFT		2
#define  CSR_PRMD_PIE			(_ULCAST_(0x1) << CSR_PRMD_PIE_SHIFT)
#define  CSR_PRMD_PPLV_SHIFT		0
#define  CSR_PRMD_PPLV_WIDTH		2
#define  CSR_PRMD_PPLV			(_ULCAST_(0x3) << CSR_PRMD_PPLV_SHIFT)

#define LOONGARCH_CSR_EUEN		0x2	/* Extended unit enable */
#define  CSR_EUEN_LBTEN_SHIFT		3
#define  CSR_EUEN_LBTEN			(_ULCAST_(0x1) << CSR_EUEN_LBTEN_SHIFT)
#define  CSR_EUEN_LASXEN_SHIFT		2
#define  CSR_EUEN_LASXEN		(_ULCAST_(0x1) << CSR_EUEN_LASXEN_SHIFT)
#define  CSR_EUEN_LSXEN_SHIFT		1
#define  CSR_EUEN_LSXEN			(_ULCAST_(0x1) << CSR_EUEN_LSXEN_SHIFT)
#define  CSR_EUEN_FPEN_SHIFT		0
#define  CSR_EUEN_FPEN			(_ULCAST_(0x1) << CSR_EUEN_FPEN_SHIFT)

#define LOONGARCH_CSR_MISC		0x3	/* Misc config */

#define LOONGARCH_CSR_ECFG		0x4	/* Exception config */
#define  CSR_ECFG_VS_SHIFT		16
#define  CSR_ECFG_VS_WIDTH		3
#define  CSR_ECFG_VS_SHIFT_END		(CSR_ECFG_VS_SHIFT + CSR_ECFG_VS_WIDTH - 1)
#define  CSR_ECFG_VS			(_ULCAST_(0x7) << CSR_ECFG_VS_SHIFT)
#define  CSR_ECFG_IM_SHIFT		0
#define  CSR_ECFG_IM_WIDTH		14
#define  CSR_ECFG_IM			(_ULCAST_(0x3fff) << CSR_ECFG_IM_SHIFT)

#define LOONGARCH_CSR_ESTAT		0x5	/* Exception status */
#define  CSR_ESTAT_ESUBCODE_SHIFT	22
#define  CSR_ESTAT_ESUBCODE_WIDTH	9
#define  CSR_ESTAT_ESUBCODE		(_ULCAST_(0x1ff) << CSR_ESTAT_ESUBCODE_SHIFT)
#define  CSR_ESTAT_EXC_SHIFT		16
#define  CSR_ESTAT_EXC_WIDTH		6
#define  CSR_ESTAT_EXC			(_ULCAST_(0x3f) << CSR_ESTAT_EXC_SHIFT)
#define  CSR_ESTAT_IS_SHIFT		0
#define  CSR_ESTAT_IS_WIDTH		15
#define  CSR_ESTAT_IS			(_ULCAST_(0x7fff) << CSR_ESTAT_IS_SHIFT)

#define  CSR_ESTAT_IS_IPI		(_ULCAST_(0x1000) << CSR_ESTAT_IS_SHIFT)
#define  CSR_ESTAT_IS_TI		(_ULCAST_(0x800) << CSR_ESTAT_IS_SHIFT)
#define  CSR_ESTAT_IS_PMC		(_ULCAST_(0x400) << CSR_ESTAT_IS_SHIFT)
#define  CSR_ESTAT_IS_HW		(_ULCAST_(0x3fc) << CSR_ESTAT_IS_SHIFT)
#define  CSR_ESTAT_IS_SW		(_ULCAST_(0x3) << CSR_ESTAT_IS_SHIFT)

// CSR ECFG
#define ECFG0_IM		0x00005fff
#define ECFGB_SIP0		0
#define ECFGF_SIP0		(_ULCAST_(1) << ECFGB_SIP0)
#define ECFGB_SIP1		1
#define ECFGF_SIP1		(_ULCAST_(1) << ECFGB_SIP1)
#define ECFGB_IP0		2
#define ECFGF_IP0		(_ULCAST_(1) << ECFGB_IP0)
#define ECFGB_IP1		3
#define ECFGF_IP1		(_ULCAST_(1) << ECFGB_IP1)
#define ECFGB_IP2		4
#define ECFGF_IP2		(_ULCAST_(1) << ECFGB_IP2)
#define ECFGB_IP3		5
#define ECFGF_IP3		(_ULCAST_(1) << ECFGB_IP3)
#define ECFGB_IP4		6
#define ECFGF_IP4		(_ULCAST_(1) << ECFGB_IP4)
#define ECFGB_IP5		7
#define ECFGF_IP5		(_ULCAST_(1) << ECFGB_IP5)
#define ECFGB_IP6		8
#define ECFGF_IP6		(_ULCAST_(1) << ECFGB_IP6)
#define ECFGB_IP7		9
#define ECFGF_IP7		(_ULCAST_(1) << ECFGB_IP7)
#define ECFGB_PMC		10
#define ECFGF_PMC		(_ULCAST_(1) << ECFGB_PMC)
#define ECFGB_TIMER		11
#define ECFGF_TIMER		(_ULCAST_(1) << ECFGB_TIMER)
#define ECFGB_IPI		12
#define ECFGF_IPI		(_ULCAST_(1) << ECFGB_IPI)
#define ECFGF(hwirq)		(_ULCAST_(1) << hwirq)

#define ESTATF_IP		0x00003fff


#define set_csr_estat(val)	\
	csr_xchg32(val, val, LOONGARCH_CSR_ESTAT)
#define clear_csr_estat(val)	\
	csr_xchg32(~(val), val, LOONGARCH_CSR_ESTAT)


#define LOONGARCH_CSR_ERA		0x6	/* Exception return address */

#define LOONGARCH_CSR_BADV		0x7	/* Bad virtual address */

#define LOONGARCH_CSR_BADI		0x8	/* Bad instruction */

#define LOONGARCH_CSR_EENTRY		0xc	/* Exception entry */

#define LOONGARCH_CSR_ASID		0x18	/* ASID */
#define  CSR_ASID_BIT_SHIFT		16	/* ASIDBits */
#define  CSR_ASID_BIT_WIDTH		8
#define  CSR_ASID_BIT			(_ULCAST_(0xff) << CSR_ASID_BIT_SHIFT)
#define  CSR_ASID_ASID_SHIFT		0
#define  CSR_ASID_ASID_WIDTH		10
#define  CSR_ASID_ASID			(_ULCAST_(0x3ff) << CSR_ASID_ASID_SHIFT)

#define LOONGARCH_CSR_PGDL		0x19	/* Page table base address when VA[VALEN-1] = 0 */

#define LOONGARCH_CSR_PGDH		0x1a	/* Page table base address when VA[VALEN-1] = 1 */

#define LOONGARCH_CSR_PGD		0x1b	/* Page table base */

#define LOONGARCH_CSR_PWCTL0		0x1c	/* PWCtl0 */
#define  CSR_PWCTL0_PTEW_SHIFT		30
#define  CSR_PWCTL0_PTEW_WIDTH		2
#define  CSR_PWCTL0_PTEW		(_ULCAST_(0x3) << CSR_PWCTL0_PTEW_SHIFT)
#define  CSR_PWCTL0_DIR1WIDTH_SHIFT	25
#define  CSR_PWCTL0_DIR1WIDTH_WIDTH	5
#define  CSR_PWCTL0_DIR1WIDTH		(_ULCAST_(0x1f) << CSR_PWCTL0_DIR1WIDTH_SHIFT)
#define  CSR_PWCTL0_DIR1BASE_SHIFT	20
#define  CSR_PWCTL0_DIR1BASE_WIDTH	5
#define  CSR_PWCTL0_DIR1BASE		(_ULCAST_(0x1f) << CSR_PWCTL0_DIR1BASE_SHIFT)
#define  CSR_PWCTL0_DIR0WIDTH_SHIFT	15
#define  CSR_PWCTL0_DIR0WIDTH_WIDTH	5
#define  CSR_PWCTL0_DIR0WIDTH		(_ULCAST_(0x1f) << CSR_PWCTL0_DIR0WIDTH_SHIFT)
#define  CSR_PWCTL0_DIR0BASE_SHIFT	10
#define  CSR_PWCTL0_DIR0BASE_WIDTH	5
#define  CSR_PWCTL0_DIR0BASE		(_ULCAST_(0x1f) << CSR_PWCTL0_DIR0BASE_SHIFT)
#define  CSR_PWCTL0_PTWIDTH_SHIFT	5
#define  CSR_PWCTL0_PTWIDTH_WIDTH	5
#define  CSR_PWCTL0_PTWIDTH		(_ULCAST_(0x1f) << CSR_PWCTL0_PTWIDTH_SHIFT)
#define  CSR_PWCTL0_PTBASE_SHIFT	0
#define  CSR_PWCTL0_PTBASE_WIDTH	5
#define  CSR_PWCTL0_PTBASE		(_ULCAST_(0x1f) << CSR_PWCTL0_PTBASE_SHIFT)

#define LOONGARCH_CSR_PWCTL1		0x1d	/* PWCtl1 */
#define  CSR_PWCTL1_PTW_SHIFT		24
#define  CSR_PWCTL1_PTW_WIDTH		1
#define  CSR_PWCTL1_PTW			(_ULCAST_(0x1) << CSR_PWCTL1_PTW_SHIFT)
#define  CSR_PWCTL1_DIR3WIDTH_SHIFT	18
#define  CSR_PWCTL1_DIR3WIDTH_WIDTH	5
#define  CSR_PWCTL1_DIR3WIDTH		(_ULCAST_(0x1f) << CSR_PWCTL1_DIR3WIDTH_SHIFT)
#define  CSR_PWCTL1_DIR3BASE_SHIFT	12
#define  CSR_PWCTL1_DIR3BASE_WIDTH	5
#define  CSR_PWCTL1_DIR3BASE		(_ULCAST_(0x1f) << CSR_PWCTL0_DIR3BASE_SHIFT)
#define  CSR_PWCTL1_DIR2WIDTH_SHIFT	6
#define  CSR_PWCTL1_DIR2WIDTH_WIDTH	5
#define  CSR_PWCTL1_DIR2WIDTH		(_ULCAST_(0x1f) << CSR_PWCTL1_DIR2WIDTH_SHIFT)
#define  CSR_PWCTL1_DIR2BASE_SHIFT	0
#define  CSR_PWCTL1_DIR2BASE_WIDTH	5
#define  CSR_PWCTL1_DIR2BASE		(_ULCAST_(0x1f) << CSR_PWCTL0_DIR2BASE_SHIFT)

#define LOONGARCH_CSR_STLBPGSIZE	0x1e
#define  CSR_STLBPGSIZE_PS_WIDTH	6
#define  CSR_STLBPGSIZE_PS		(_ULCAST_(0x3f))


/* KSave registers */
#define LOONGARCH_CSR_KS0		0x30
#define LOONGARCH_CSR_KS1		0x31
#define LOONGARCH_CSR_KS2		0x32
#define LOONGARCH_CSR_KS3		0x33
#define LOONGARCH_CSR_KS4		0x34
#define LOONGARCH_CSR_KS5		0x35
#define LOONGARCH_CSR_KS6		0x36
#define LOONGARCH_CSR_KS7		0x37
#define LOONGARCH_CSR_KS8		0x38

/* Exception allocated KS0, KS1 and KS2 statically */
#define EXCEPTION_KS0			LOONGARCH_CSR_KS0
#define EXCEPTION_KS1			LOONGARCH_CSR_KS1
#define EXCEPTION_KS2			LOONGARCH_CSR_KS2
#define EXC_KSAVE_MASK			(1 << 0 | 1 << 1 | 1 << 2)

/* Percpu-data base allocated KS3 statically */
#define PERCPU_BASE_KS			LOONGARCH_CSR_KS3
#define PERCPU_KSAVE_MASK		(1 << 3)


/* Timer registers */
#define LOONGARCH_CSR_TMID		0x40	/* Timer ID */

#define LOONGARCH_CSR_TCFG		0x41	/* Timer config */
#define  CSR_TCFG_VAL_SHIFT		2
#define	 CSR_TCFG_VAL_WIDTH		48
#define  CSR_TCFG_VAL			(_ULCAST_(0x3fffffffffff) << CSR_TCFG_VAL_SHIFT)
#define  CSR_TCFG_PERIOD_SHIFT		1
#define  CSR_TCFG_PERIOD		(_ULCAST_(0x1) << CSR_TCFG_PERIOD_SHIFT)
#define  CSR_TCFG_EN			(_ULCAST_(0x1))

#define LOONGARCH_CSR_TVAL		0x42	/* Timer value */

#define LOONGARCH_CSR_CNTC		0x43	/* Timer offset */

#define LOONGARCH_CSR_TINTCLR		0x44	/* Timer interrupt clear */
#define  CSR_TINTCLR_TI_SHIFT		0
#define  CSR_TINTCLR_TI			(1 << CSR_TINTCLR_TI_SHIFT)

#define DMW_PABITS	48


/* Direct Map windows registers */
#define LOONGARCH_CSR_DMWIN0		0x180	/* 64 direct map win0: MEM & IF */
#define LOONGARCH_CSR_DMWIN1		0x181	/* 64 direct map win1: MEM & IF */
#define LOONGARCH_CSR_DMWIN2		0x182	/* 64 direct map win2: MEM */
#define LOONGARCH_CSR_DMWIN3		0x183	/* 64 direct map win3: MEM */

/* Direct Map window 0/1/2/3 */
#define CSR_DMW0_PLV0		_CONST64_(1 << 0)
#define CSR_DMW0_VSEG		_CONST64_(0x8000)
#define CSR_DMW0_BASE		(CSR_DMW0_VSEG << DMW_PABITS)
#define CSR_DMW0_INIT		(CSR_DMW0_BASE | CSR_DMW0_PLV0)

#define CSR_DMW1_PLV0		_CONST64_(1 << 0)
#define CSR_DMW1_MAT		_CONST64_(1 << 4)
#define CSR_DMW1_VSEG		_CONST64_(0x9000)
#define CSR_DMW1_BASE		(CSR_DMW1_VSEG << DMW_PABITS)
#define CSR_DMW1_INIT		(CSR_DMW1_BASE | CSR_DMW1_MAT | CSR_DMW1_PLV0)

#define CSR_DMW2_PLV0		_CONST64_(1 << 0)
#define CSR_DMW2_MAT		_CONST64_(2 << 4)
#define CSR_DMW2_VSEG		_CONST64_(0xa000)
#define CSR_DMW2_BASE		(CSR_DMW2_VSEG << DMW_PABITS)
#define CSR_DMW2_INIT		(CSR_DMW2_BASE | CSR_DMW2_MAT | CSR_DMW2_PLV0)

#define CSR_DMW3_INIT		0x0


/* ExStatus.ExcCode */
#define EXCCODE_RSV		0	/* Reserved */
#define EXCCODE_TLBL		1	/* TLB miss on a load */
#define EXCCODE_TLBS		2	/* TLB miss on a store */
#define EXCCODE_TLBI		3	/* TLB miss on a ifetch */
#define EXCCODE_TLBM		4	/* TLB modified fault */
#define EXCCODE_TLBNR		5	/* TLB Read-Inhibit exception */
#define EXCCODE_TLBNX		6	/* TLB Execution-Inhibit exception */
#define EXCCODE_TLBPE		7	/* TLB Privilege Error */
#define EXCCODE_ADE		8	/* Address Error */
	#define EXSUBCODE_ADEF		0	/* Fetch Instruction */
	#define EXSUBCODE_ADEM		1	/* Access Memory*/
#define EXCCODE_ALE		9	/* Unalign Access */
#define EXCCODE_BCE		10	/* Bounds Check Error */
#define EXCCODE_SYS		11	/* System call */
#define EXCCODE_BP		12	/* Breakpoint */
#define EXCCODE_INE		13	/* Inst. Not Exist */
#define EXCCODE_IPE		14	/* Inst. Privileged Error */
#define EXCCODE_FPDIS		15	/* FPU Disabled */
#define EXCCODE_LSXDIS		16	/* LSX Disabled */
#define EXCCODE_LASXDIS		17	/* LASX Disabled */
#define EXCCODE_FPE		18	/* Floating Point Exception */
	#define EXCSUBCODE_FPE		0	/* Floating Point Exception */
	#define EXCSUBCODE_VFPE		1	/* Vector Exception */
#define EXCCODE_WATCH		19	/* WatchPoint Exception */
	#define EXCSUBCODE_WPEF		0	/* ... on Instruction Fetch */
	#define EXCSUBCODE_WPEM		1	/* ... on Memory Accesses */
#define EXCCODE_BTDIS		20	/* Binary Trans. Disabled */
#define EXCCODE_BTE		21	/* Binary Trans. Exception */
#define EXCCODE_GSPR		22	/* Guest Privileged Error */
#define EXCCODE_HVC		23	/* Hypercall */
#define EXCCODE_GCM		24	/* Guest CSR modified */
	#define EXCSUBCODE_GCSC		0	/* Software caused */
	#define EXCSUBCODE_GCHC		1	/* Hardware caused */
#define EXCCODE_SE		25	/* Security */

/* Interrupt numbers */
#define INT_SWI0	0	/* Software Interrupts */
#define INT_SWI1	1
#define INT_HWI0	2	/* Hardware Interrupts */
#define INT_HWI1	3
#define INT_HWI2	4
#define INT_HWI3	5
#define INT_HWI4	6
#define INT_HWI5	7
#define INT_HWI6	8
#define INT_HWI7	9
#define INT_PCOV	10	/* Performance Counter Overflow */
#define INT_TI		11	/* Timer */
#define INT_IPI		12
#define INT_NMI		13
#define INT_AVEC	14

/* ExcCodes corresponding to interrupts */
#define EXCCODE_INT_NUM		(INT_AVEC + 1)
#define EXCCODE_INT_START	64
#define EXCCODE_INT_END		(EXCCODE_INT_START + EXCCODE_INT_NUM - 1)





// IOCSR
#define LOONGARCH_IOCSR_VENDOR		0x10

#define LOONGARCH_IOCSR_CPUNAME		0x20

#define LOONGARCH_IOCSR_NODECNT		0x408

#define LOONGARCH_IOCSR_MISC_FUNC	0x420
#define  IOCSR_MISC_FUNC_SOFT_INT	BIT_ULL(10)
#define  IOCSR_MISC_FUNC_TIMER_RESET	BIT_ULL(21)
#define  IOCSR_MISC_FUNC_EXT_IOI_EN	BIT_ULL(48)
#define  IOCSR_MISC_FUNC_AVEC_EN	BIT_ULL(51)



#ifndef __ASSEMBLY__

#define read_csr_asid()				csr_read32(LOONGARCH_CSR_ASID)
#define write_csr_asid(val)			csr_write32(val, LOONGARCH_CSR_ASID)
#define read_csr_ecfg()				csr_read32(LOONGARCH_CSR_ECFG)
#define write_csr_ecfg(val)			csr_write32(val, LOONGARCH_CSR_ECFG)
#define read_csr_estat()			csr_read32(LOONGARCH_CSR_ESTAT)
#define write_csr_estat(val)		csr_write32(val, LOONGARCH_CSR_ESTAT)
#define write_csr_tintclear(val)	csr_write32(val, LOONGARCH_CSR_TINTCLR)

/* IOCSR */
#define iocsr_read32(reg) __iocsrrd_w(reg)
#define iocsr_read64(reg) __iocsrrd_d(reg)
#define iocsr_write32(val, reg) __iocsrwr_w(val, reg)
#define iocsr_write64(val, reg) __iocsrwr_d(val, reg)


static inline unsigned long set_csr_ecfg(unsigned long set_val)
{
	unsigned long res, new_val;
	res = read_csr_ecfg();
	new_val = res | set_val;
	write_csr_ecfg(new_val);
	return res;
}

static inline unsigned long clear_csr_ecfg(unsigned long clear)
{
	unsigned long res, new_val;
	res = read_csr_ecfg();
	new_val = res & ~clear;
	write_csr_ecfg(new_val);
	return res;
}

#endif




#endif /* _ASM_LOONGARCH_H */

