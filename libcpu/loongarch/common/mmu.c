/*
 * Copyright (C) 2020-2025 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-10     LoongsonLab  the first version
 */


#include <rtthread.h>
#include <stddef.h>

#define DBG_TAG "hw.mmu"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#ifdef RT_USING_SMART
#include <board.h>
#include <ioremap.h>
#include <lwp_user_mm.h>
#endif


#define SZ_4K		0x00001000
#define PTE_SIZE    512

static void *current_mmu_table = RT_NULL;

rt_align(SZ_4K) rt_ubase_t MMUTable[PTE_SIZE];

void rt_hw_aspace_switch(rt_aspace_t aspace)
{
    uintptr_t page_table = (uintptr_t)rt_kmem_v2p(aspace->page_table);
    current_mmu_table = aspace->page_table;

    rt_hw_tlb_invalidate_all_local();
}

/* get current page table. */
void *rt_hw_mmu_tbl_get()
{
    return current_mmu_table;
}


/**
 * @brief Maps a virtual address space to a physical address space.
 *
 * This function maps a specified range of virtual addresses to a range of physical addresses
 * and sets the attributes of the page table entries (PTEs). If an error occurs during the
 * mapping process, the function will automatically roll back any partially completed mappings.
 *
 * @param aspace Pointer to the address space structure containing the page table information.
 * @param v_addr The starting virtual address to be mapped.
 * @param p_addr The starting physical address to be mapped.
 * @param size The size of the memory to be mapped (in bytes).
 * @param attr The attributes of the page table entries (e.g., read/write permissions, cache policies).
 *
 * @return On success, returns the starting virtual address `v_addr`;
 *         On failure, returns `NULL`.
 *
 * @note This function will not override existing page table entries.
 * @warning The caller must ensure that `v_addr` and `p_addr` are page-aligned,
 *          and `size` is a multiple of the page size.
 *
 */
void *rt_hw_mmu_map(struct rt_aspace *aspace, void *v_addr, void *p_addr,
                    size_t size, size_t attr)
{

	int ret = -1;
    void *unmap_va = v_addr;
    size_t npages = size >> ARCH_PAGE_SHIFT;

    /* TODO trying with HUGEPAGE here */
    while (npages--)
    {
        MM_PGTBL_LOCK(aspace);
        ret = _map_one_page(aspace, v_addr, p_addr, attr);
        MM_PGTBL_UNLOCK(aspace);
        if (ret != 0)
        {
            /* error, undo map */
            while (unmap_va != v_addr)
            {
                MM_PGTBL_LOCK(aspace);
                _unmap_area(aspace, unmap_va, ARCH_PAGE_SIZE);
                MM_PGTBL_UNLOCK(aspace);
                unmap_va += ARCH_PAGE_SIZE;
            }
            break;
        }
        v_addr += ARCH_PAGE_SIZE;
        p_addr += ARCH_PAGE_SIZE;
    }

    if (ret == 0)
    {
        return unmap_va;
    }

    return NULL;
}

/**
 * @brief Unmaps a range of virtual memory addresses from the specified address space.
 *
 * This function is responsible for unmapping a contiguous region of virtual memory
 * from the given address space. It handles multiple pages and ensures thread safety
 * by locking the page table during the unmapping operation.
 *
 * @param aspace Pointer to the address space structure from which the memory will be unmapped.
 * @param v_addr Starting virtual address to unmap. Must be page-aligned.
 * @param size Size of the memory region to unmap. Must be page-aligned.
 *
 * @note The caller must ensure that both `v_addr` and `size` are page-aligned.
 *
 * @details The function operates in a loop, unmapping memory in chunks. It uses the
 * `_unmap_area` function to perform the actual unmapping, which is called within a
 * locked section to ensure thread safety. The loop continues until the entire region
 * is unmapped.
 *
 * @see _unmap_area
 * @note unmap is different from map that it can handle multiple pages
 */
void rt_hw_mmu_unmap(struct rt_aspace *aspace, void *v_addr, size_t size)
{
    /* caller guarantee that v_addr & size are page aligned */
    if (!aspace->page_table)
    {
        return;
    }
    size_t unmapped = 0;

    while (size > 0)
    {
        MM_PGTBL_LOCK(aspace);
        // unmapped = _unmap_area(aspace, v_addr, size);
        MM_PGTBL_UNLOCK(aspace);

        /* when unmapped == 0, region not exist in pgtbl */
        if (!unmapped || unmapped > size) break;

        size -= unmapped;
        v_addr += unmapped;
    }
}

int rt_hw_mmu_map_init(rt_aspace_t aspace, void *v_address, rt_ubase_t size,
                       rt_ubase_t *vtable, rt_ubase_t pv_off)
{
    return 0;
}


/**
 * @brief Translate a virtual address to a physical address.
 *
 * This function translates a given virtual address (`vaddr`) to its corresponding
 * physical address (`paddr`) using the page table in the specified address space (`aspace`).
 *
 * @param aspace Pointer to the address space structure containing the page table.
 * @param vaddr The virtual address to be translated.
 *
 * @return The translated physical address. If the translation fails, `ARCH_MAP_FAILED` is returned.
 *
 * @note The function queries the page table entry (PTE) for the virtual address using `_query`.
 *       If a valid PTE is found, the physical address is extracted and combined with the offset
 *       from the virtual address. If no valid PTE is found, a debug log is recorded, and
 *       `ARCH_MAP_FAILED` is returned.
 */
void *rt_hw_mmu_v2p(struct rt_aspace *aspace, void *vaddr)
{
    return (void *)0;
}

int rt_hw_mmu_control(struct rt_aspace *aspace, void *vaddr, size_t size,
                      enum rt_mmu_cntl cmd)
{
    return 0;
}

/**
 * @brief setup Page Table for kernel space. It's a fixed map
 * and all mappings cannot be changed after initialization.
 *
 * Memory region in struct mem_desc must be page aligned,
 * otherwise is a failure and no report will be
 * returned.
 *
 * @param aspace Pointer to the address space structure.
 * @param mdesc Pointer to the array of memory descriptors.
 * @param desc_nr Number of memory descriptors in the array.
 */
void rt_hw_mmu_setup(rt_aspace_t aspace, struct mem_desc *mdesc, int desc_nr)
{
}


/**
 * @brief Creates and initializes a new MMU page table.
 *
 * This function allocates a new MMU page table, copies the kernel space
 * page table into it, and flushes the data cache to ensure consistency.
 *
 * @return
 * - A pointer to the newly allocated MMU page table on success.
 * - RT_NULL if the allocation fails.
 */
void *rt_hw_mmu_pgtbl_create(void)
{
    return (void *)0;
}


/**
 * @brief Deletes an MMU page table.
 *
 * This function frees the memory allocated for the given MMU page table.
 *
 * @param pgtbl Pointer to the MMU page table to be deleted.
 */
void rt_hw_mmu_pgtbl_delete(void *pgtbl)
{
    rt_pages_free(pgtbl, 0);
}
