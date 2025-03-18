/*
 * Copyright (C) 2020-2025 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-10     LoongsonLab  the first version
 */

#ifndef _ASM_MMU_H
#define _ASM_MMU_H


#ifdef RT_USING_SMART
#include <mm_aspace.h>
#include <mm_page.h>

struct mem_desc
{
    rt_size_t vaddr_start;
    rt_size_t vaddr_end;
    rt_ubase_t paddr_start;
    rt_size_t attr;
    struct rt_varea varea;
};

#endif

/* RAM, Flash, or ROM */
#define NORMAL_MEM 0
/* normal nocache memory mapping type */
#define NORMAL_NOCACHE_MEM 1
/* MMIO region */
#define DEVICE_MEM 2

#define ARCH_MAP_FAILED ((void *)-1)


#define PAGE_OFFSET_BIT  12
#define ARCH_PAGE_SHIFT  PAGE_OFFSET_BIT
#define ARCH_PAGE_SIZE   (1ULL << ARCH_PAGE_SHIFT)
#define ARCH_PAGE_MASK   (ARCH_PAGE_SIZE - 1)

#define MMU_PG_LEVEL 3
#define ARCH_PAGE_LEVEL_SHIFT (ARCH_PAGE_SHIFT - 3)
#define ARCH_PAGE_LEVEL_MASK  ((1 << ARCH_PAGE_LEVEL_SHIFT) - 1)
#define ARCH_PAGE_ADDRESS_MASK 0xfffffffffffff000UL

#define MMU_MAP_K_DEVICE   0x0
#define MMU_MAP_K_RWCB   0x0
#define MMU_MAP_K_RW   0x0
#define MMU_MAP_U_RWCB   0x0
#define MMU_MAP_U_RWCB_XN   0x0
#define MMU_MAP_U_RW   0x0
#define MMU_MAP_EARLY   0x0

#define RT_HW_MMU_PROT_READ    1
#define RT_HW_MMU_PROT_WRITE   2
#define RT_HW_MMU_PROT_EXECUTE 4
#define RT_HW_MMU_PROT_KERNEL  8
#define RT_HW_MMU_PROT_USER    16
#define RT_HW_MMU_PROT_CACHE   32


#define ARCH_ADDRESS_WIDTH_BITS 64

#define PHYSICAL_ADDRESS_WIDTH_BITS 56

#define ARCH_VADDR_WIDTH 39


rt_inline void rt_hw_tlb_invalidate_all_local(void)
{
    __asm__ volatile("invtlb 0x0, $zero, $zero" ::: "memory");
}

rt_inline void rt_hw_tlb_invalidate_range(rt_aspace_t aspace, void *start,
                                              size_t size, size_t stride)
{
    __asm__ volatile("invtlb 0x0, $zero, $zero" ::: "memory");
}

// MMU related functions.
void *rt_hw_mmu_pgtbl_create(void);
void rt_hw_mmu_pgtbl_delete(void *pgtbl);

int rt_hw_mmu_map_init(rt_aspace_t aspace, void *v_address, rt_ubase_t size,
                       rt_ubase_t *vtable, rt_ubase_t pv_off);
void rt_hw_mmu_kernel_map_init(rt_aspace_t aspace, rt_ubase_t vaddr_start,
                               rt_ubase_t size);

void rt_hw_mmu_setup(rt_aspace_t aspace, struct mem_desc *mdesc, int desc_nr);
int rt_hw_mmu_control(struct rt_aspace *aspace, void *vaddr, size_t size,
                      enum rt_mmu_cntl cmd);

void *rt_hw_mmu_map(rt_aspace_t aspace, void *v_addr, void *p_addr, size_t size,
                    size_t attr);
void rt_hw_mmu_unmap(rt_aspace_t aspace, void *v_addr, size_t size);

void rt_hw_aspace_switch(rt_aspace_t aspace);

void *rt_hw_mmu_v2p(rt_aspace_t aspace, void *vaddr);

void *rt_hw_mmu_tbl_get(void);


/**
 * @brief Remove permission from attribution
 *
 * @param attr architecture specified mmu attribution
 * @param prot protect that will be removed
 * @return size_t returned attribution
 */
rt_inline size_t rt_hw_mmu_attr_rm_perm(size_t attr, rt_base_t prot)
{
    // switch (prot)
    // {
    //     /* remove write permission for user */
    //     case RT_HW_MMU_PROT_WRITE | RT_HW_MMU_PROT_USER:
    //         attr &= ~PTE_W;
    //         break;
    //     /* remove write permission for kernel */
    //     case RT_HW_MMU_PROT_WRITE | RT_HW_MMU_PROT_KERNEL:
    //         attr &= ~PTE_W;
    //         break;
    //     default:
    //         RT_ASSERT(0);
    // }
    return 0;
}

/**
 * @brief Add permission from attribution
 *
 * @param attr architecture specified mmu attribution
 * @param prot protect that will be added
 * @return size_t returned attribution
 */
rt_inline size_t rt_hw_mmu_attr_add_perm(size_t attr, rt_base_t prot)
{
    // switch (prot)
    // {
    //     /* add write permission for user */
    //     case RT_HW_MMU_PROT_WRITE | RT_HW_MMU_PROT_USER:
    //         attr |= (PTE_R | PTE_W | PTE_U);
    //         break;
    //     default:
    //         RT_ASSERT(0);
    // }
    return 0;
}


#define MMU_MAP_ERROR_VANOTALIGN -1
#define MMU_MAP_ERROR_PANOTALIGN -2
#define MMU_MAP_ERROR_NOPAGE     -3
#define MMU_MAP_ERROR_CONFLICT   -4


// #define KERNEL_VADDR_START ((void *)0x9000000000000000)
#define KERNEL_VADDR_SIZE  (0x1000000000000000)
#define KERNEL_VADDR_END   ((void *)0x9000000000000000 + KERNEL_VADDR_SIZE - 1)


#endif /* _ASM_MMU_H */
