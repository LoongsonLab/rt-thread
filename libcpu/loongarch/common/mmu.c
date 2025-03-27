/*
 * Copyright (C) 2020-2025 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-10     LoongsonLab  the first version
 * 2025-03-22     LoongsonLab  fix tlb refill and page fault
 * 2025-03-23     LoongsonLab  fix map_one_page_4K, not page ref
 * 2025-03-27     LoongsonLab  fix unmap_one_page_4K, free not used pages
 */


#include <rtthread.h>
#include <rtconfig.h>
#include <stddef.h>

#define DBG_TAG "hw.mmu"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#ifdef RT_USING_SMART
#include <ioremap.h>
#include <lwp_user_mm.h>
#endif

#include <board.h>
#include <mm_aspace.h>
#include <mm_page.h>
#include <mm_private.h>

#include "mmu.h"
#include "loongarch.h"

#define SZ_4K		0x00001000
#define PTE_SIZE    512

static void *current_mmu_table = RT_NULL;

rt_align(SZ_4K) rt_ubase_t MMUTable[PTE_SIZE];


void rt_hw_aspace_switch(rt_aspace_t aspace)
{
	uintptr_t page_table = (uintptr_t)rt_kmem_v2p(aspace->page_table);
	current_mmu_table = aspace->page_table;

	// update CSR PGDL(user)
	__asm__ __volatile__(
		"csrwr %[pgdl_val], %[pgdl_reg] \n\t"
		: [pgdl_val] "+r" (current_mmu_table)
		: [pgdl_reg] "i"  (LOONGARCH_CSR_PGDL)
		: "memory"
	);

	rt_hw_tlb_invalidate_all_local();
}

/* get current page table. */
void *rt_hw_mmu_tbl_get()
{
	return current_mmu_table;
}


/**
 * @brief Unmap a page table use vaddr and physic address.
 *
 * This function umap 4K page and set three level page table.
 *
 * @param aspace Pointer to the address space structure containing the page table information.
 * @param v_addr The starting virtual address to be mapped.
 * 
 * @return On success, returns 0,
 *         On failure, returns error code.
 * 
 */
static int unmap_one_page_4K(struct rt_aspace *aspace, void *v_addr) {
	int ret = 0;
	int i = 0;
	unsigned long map_level_vaddr = v_addr;
	unsigned long page = 0;
	unsigned int level_off = 0;
	unsigned int map_level = MMU_PG_LEVEL;
	unsigned int mmu_pg_shift = 39 - ARCH_PAGE_LEVEL_SHIFT;
	rt_ubase_t *map_level_tbl = ((rt_ubase_t *)aspace->page_table);
	unsigned long *need_free_pages[MMU_PG_LEVEL] = {0};

	while(map_level > 0) 
	{
		level_off = map_level_vaddr >> mmu_pg_shift;
		level_off &= ARCH_PAGE_LEVEL_MASK;
		page = map_level_tbl[level_off];
		if (page)
		{
			page &= ARCH_PAGE_ADDRESS_MASK;

            if (!page) // TODO: if page table occurs error?
                return 0;
            page = ARCH_PAGE_VA_MASK(page);

			ret = rt_page_ref_get(page, 0);
			if (ret == 1)
			{
				rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, map_level_tbl + level_off, sizeof(void *));
                if (map_level > 1)
                    need_free_pages[map_level] = ARCH_PAGE_PA2VA(page);
			}
		} else {
            // when level page is null, we stop walk and return.
            return 0;
        }
		// update tbl
		map_level--;
		map_level_tbl = page;
		mmu_pg_shift -= ARCH_PAGE_LEVEL_SHIFT;
	}

	// final free all unused pages.
	for (i = 0; i < MMU_PG_LEVEL; ++i)
	{
		if (need_free_pages[i])
		{
			rt_pages_free(need_free_pages[i], 0);
		}
	}
	return 0;
}

/**
 * @brief Map a page table use vaddr and physic address.
 *
 * This function map 4K page and set three level page table.
 *
 * @param aspace Pointer to the address space structure containing the page table information.
 * @param v_addr The starting virtual address to be mapped.
 * @param p_addr The starting physical address to be mapped.
 * 
 * @return On success, returns 0,
 *         On failure, returns error code.
 * 
 * @note Current configuration only support 3 Level Page Table Map and Max width of VA
 *       is 39. The Page Size is only 4KB, in the future we will support 2M and 1G mappings.
 * 
 */
static int map_one_page_4K(struct rt_aspace *aspace, void *v_addr, void *p_addr, unsigned long attr) {
    unsigned long page = 0;
    unsigned long map_level_vaddr = v_addr;
    unsigned int level_off = 0;
    unsigned int map_level = MMU_PG_LEVEL;
    unsigned int mmu_pg_shift = 39 - ARCH_PAGE_LEVEL_SHIFT;
	rt_ubase_t *map_level_tbl = ((rt_ubase_t *)aspace->page_table);

	// 1. judge va if illegal 
	if (v_addr == RT_NULL)
	{
		return MMU_MAP_ERROR_VANOTALIGN;
	}
	
	if (((unsigned long)v_addr) & ARCH_PAGE_MASK)
	{
		return MMU_MAP_ERROR_PANOTALIGN;
	}

	// 2. walk page table
	do {
		level_off = map_level_vaddr >> mmu_pg_shift;
		level_off &= ARCH_PAGE_LEVEL_MASK;
		page = map_level_tbl[level_off];
		if (page)
		{
			// 2.1 find a valid level
			if (page & ARCH_PAGE_MASK) // align page size
				return MMU_MAP_ERROR_PANOTALIGN;

			// 2.2 increase ref
            page = ARCH_PAGE_VA_MASK(page); // virtual address
			page &= ARCH_PAGE_ADDRESS_MASK;
			rt_page_ref_inc((void *)page, 0);
			
			// 2.3 update
			map_level--;
			map_level_tbl = page;
			mmu_pg_shift -= ARCH_PAGE_LEVEL_SHIFT;
			
			// next level table
			continue;
		} 
		// need alloc a new table 
		else 
		{
			// 3.1 alloca a page 
			page = (unsigned long)rt_pages_alloc_ext(0, PAGE_ANY_AVAILABLE);

			if (!page)
			{
				unmap_one_page_4K(aspace, v_addr);
				return MMU_MAP_ERROR_NOPAGE;
			}

			// 3.2 clear and notify dcache
			rt_memset((void *)page, 0, ARCH_PAGE_SIZE);
			rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, (void *)page, ARCH_PAGE_SIZE);

			map_level_tbl[level_off] = (rt_ubase_t)ARCH_PAGE_PA_MASK(page);
			rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, map_level_tbl + level_off, sizeof(void *));

			// 3.3 update
			map_level--;
			map_level_tbl = page;
			mmu_pg_shift -= ARCH_PAGE_LEVEL_SHIFT;
		}
	} while (map_level > 1);

	// here, handle PTE.
handle_pte:
	attr |= 0x0; // TODO: if handle NO_EXEC flags?
	p_addr = ((unsigned long)p_addr) | attr;

    level_off = map_level_vaddr >> mmu_pg_shift;
	level_off &= ARCH_PAGE_LEVEL_MASK;
	map_level_tbl[level_off] = p_addr;

    rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, map_level_tbl + level_off, sizeof(void *));

	return 0;
}

/**
 * @brief Judge vaddr address which is a Kernel address.
 *
 * This function judges a virtual addresses if in rang of 0x9000xxxx_xxxxxxxx
 *
 * @param vaddr The starting virtual address to be mapped.
 * @return On kernel address, returns 1,
 *         On user space address, returns 0.
 */
static int rt_hw_mmu_kernel(void *vaddr)
{
    if (((unsigned long )vaddr >= (unsigned long )KERNEL_VADDR_START) &&
        ((unsigned long )vaddr < (unsigned long )KERNEL_VADDR_END))
        return 1;
    else
        return 0; // user space
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

    // if we are in kernel space, and return virtual
    if (rt_hw_mmu_kernel(v_addr))
        return unmap_va;

	/* TODO trying with HUGEPAGE here */
	while (npages--)
	{
		MM_PGTBL_LOCK(aspace);
		ret = map_one_page_4K(aspace, v_addr, p_addr, attr);
		MM_PGTBL_UNLOCK(aspace);
		if (ret != 0)
		{
			/* error, undo map */
			while (unmap_va != v_addr)
			{
				MM_PGTBL_LOCK(aspace);
				unmap_one_page_4K(aspace, unmap_va);
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
	size_t npages = size >> ARCH_PAGE_SHIFT;

	/* caller guarantee that v_addr & size are page aligned */
	if (!aspace->page_table)
	{
		return;
	}

	while (npages > 0)
	{
		MM_PGTBL_LOCK(aspace);
		unmap_one_page_4K(aspace, v_addr);
		MM_PGTBL_UNLOCK(aspace);

		npages--;
		v_addr = (char *)v_addr + ARCH_PAGE_SIZE;
	}
}

#ifdef RT_USING_SMART
static void _init_region(void *vaddr, size_t size)
{
    rt_ioremap_start = vaddr;
    rt_ioremap_size = size;
    rt_mpr_start = rt_ioremap_start - rt_mpr_size;
    LOG_D("rt_ioremap_start: %p, rt_mpr_start: %p", rt_ioremap_start, rt_mpr_start);
}
#else
static inline void _init_region(void *vaddr, size_t size)
{
    rt_mpr_start = vaddr - rt_mpr_size;
}
#endif

int rt_hw_mmu_map_init(rt_aspace_t aspace, void *v_address, rt_ubase_t size,
                       rt_ubase_t *vtable, rt_ubase_t pv_off)
{
	size_t va_s, va_e;

	if (!aspace || !vtable)
	{
		return -1;
	}

	va_s = (size_t)v_address;
	va_e = (size_t)v_address + size - 1;

	if (va_e < va_s)
	{
	    return -1;
	}

	va_s >>= MMU_PG_LEVEL_3_SHIFT;
	va_e >>= MMU_PG_LEVEL_3_SHIFT;

	if (va_s == 0)
	{
		return -1;
	}

	rt_aspace_init(aspace, (void *)KERNEL_VADDR_START, KERNEL_VADDR_END - KERNEL_VADDR_START,
	               vtable);

	_init_region(v_address, size);

	return 0;
}


static rt_ubase_t *_query(struct rt_aspace *aspace, void *vaddr, int *level_shift)
{
	unsigned long map_level_vaddr = vaddr;
	unsigned long page = 0;
	unsigned long page_ppn = 0;
	unsigned long *page_pte = 0;
	unsigned int level_off = 0;
	unsigned int map_level = MMU_PG_LEVEL;
	unsigned int mmu_pg_shift = 39 - ARCH_PAGE_LEVEL_SHIFT;
	rt_ubase_t *map_level_tbl = ((rt_ubase_t *)aspace->page_table);

	while(map_level > 0) 
	{
		level_off = map_level_vaddr >> mmu_pg_shift;
		level_off &= ARCH_PAGE_LEVEL_MASK;
		page = map_level_tbl[level_off];

		page_pte = map_level_tbl + level_off;

		page_ppn = page & ~ARCH_PAGE_MASK; // clear bit[11:0]

		if (!page_ppn && (map_level != 1))
		{
			*level_shift = map_level;
			return (void *)0;
		}

		if (map_level == 1)
		{
			if (!(page & ARCH_PAGE_BIT_V))
			{
				*level_shift = map_level;
				return (void *)0;
			}
            goto find_entry;;
		}
		map_level--;
		map_level_tbl = ARCH_PAGE_VA_MASK(page);
		mmu_pg_shift -= ARCH_PAGE_LEVEL_SHIFT;
	}

find_entry:
	*level_shift = mmu_pg_shift;
	return page_pte;
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
	int level_shift;
	unsigned long paddr;

	if (aspace == &rt_kernel_space)
	{
		paddr = (unsigned long)(vaddr) & 0x000fffffffffffff;
	}
	else
	{
		unsigned long *pte = _query(aspace, vaddr, &level_shift);

		if (pte)
		{
			paddr = *pte & ARCH_PAGE_ADDRESS_MASK;
			paddr |= (rt_ubase_t)vaddr & ((1ul << level_shift) - 1);
		}
		else
		{
			paddr = (unsigned long)ARCH_MAP_FAILED;
		}
	}

	return (void *)paddr;
}


/**
 * @brief Modify Page Table Attributes.
 *
 * In LoongArch, if instruction store write a page, but underlying tlb attributes
 * of this page is NOT DIRTY (in bit 1), this will raise a PME exception.
 *  
 * This function set page table attribute is dirty.
 *
 * @param aspace Pointer to the address space structure containing the page table information.
 * @param v_addr The starting virtual address to be mapped.
 * 
 * @return On success, returns 0,
 *         On has been modified, returns 1.
 */
int rt_hw_mmu_update_modify_page(rt_aspace_t aspace, rt_ubase_t *fault_vaddr)
{
	int level_shift;
	unsigned long paddr;
	unsigned long pte_entry;
	unsigned long new_pte_entry;
	int ret = 0;
	
    if (aspace)
    {
        rt_varea_t varea;

        RD_LOCK(aspace);
        varea = _aspace_bst_search(aspace, fault_vaddr);
        if (varea)
        {
            unsigned long *pte = _query(aspace, fault_vaddr, &level_shift);
            pte_entry = *pte;
            if (!(pte_entry & MMU_PAGE_DIRTY))
            {
                new_pte_entry = pte_entry | MMU_PAGE_DIRTY;
                *pte = new_pte_entry;

                ret = 0;

                rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, pte, sizeof(unsigned long));
                // update tlb
                rt_hw_tlb_invalidate_all_local();
            } 
            else
            	ret = 1;
        }

        RD_UNLOCK(aspace);
    }
    return ret;
}


static int _noncache(rt_base_t *pte)
{
    return 0;
}

static int _cache(rt_base_t *pte)
{
    return 0;
}


static int (*control_handler[MMU_CNTL_DUMMY_END])(rt_base_t *pte)=
{
	[MMU_CNTL_CACHE] = _cache,
	[MMU_CNTL_NONCACHE] = _noncache,
};


int rt_hw_mmu_control(struct rt_aspace *aspace, void *vaddr, size_t size,
                      enum rt_mmu_cntl cmd)
{
	int level_shift;
	int err = -RT_EINVAL;
	rt_ubase_t vstart = (rt_ubase_t)vaddr;
	rt_ubase_t vend = vstart + size;

	int (*handler)(rt_ubase_t * pte);
	if (cmd >= 0 && cmd < MMU_CNTL_DUMMY_END)
	{
		handler = control_handler[cmd];

		while (vstart < vend)
		{
			rt_ubase_t *pte = _query(aspace, (void *)vstart, &level_shift);
			rt_ubase_t range_end = vstart + (1ul << level_shift);
			RT_ASSERT(range_end <= vend);

			if (pte)
			{
				err = handler(pte);
				RT_ASSERT(err == RT_EOK);
			}
			vstart = range_end;
		}
	}
	else
	{
		err = -RT_ENOSYS;
	}

	return err;
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
	void *err;
	for (size_t i = 0; i < desc_nr; i++)
	{
		size_t attr;
		switch (mdesc->attr)
		{
			case NORMAL_MEM:
				attr = MMU_MAP_K_RWCB;
				break;
			case NORMAL_NOCACHE_MEM:
				attr = MMU_MAP_K_RWCB;
				break;
			case DEVICE_MEM:
				attr = MMU_MAP_K_DEVICE;
				break;
			default:
				attr = MMU_MAP_K_DEVICE;
		}

		struct rt_mm_va_hint hint = {
			.flags = MMF_MAP_FIXED,
			.limit_start = aspace->start,
			.limit_range_size = aspace->size,
			.map_size = mdesc->vaddr_end - mdesc->vaddr_start + 1,
			.prefer = (void *)mdesc->vaddr_start};

		if (mdesc->paddr_start == (rt_uintptr_t)ARCH_MAP_FAILED)
			mdesc->paddr_start = mdesc->vaddr_start + PV_OFFSET;

		rt_aspace_map_phy_static(aspace, &mdesc->varea, &hint, attr,
								mdesc->paddr_start >> MM_PAGE_SHIFT, &err);
		mdesc++;
    }

	rt_hw_aspace_switch(&rt_kernel_space);
	rt_page_cleanup();
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
	size_t *mmu_table;
	mmu_table = (size_t *)rt_pages_alloc_ext(0, PAGE_ANY_AVAILABLE);
	if (!mmu_table)
	{
		return RT_NULL;
	}

	memset(mmu_table, 0, ARCH_PAGE_SIZE);
	rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, mmu_table, ARCH_PAGE_SIZE);
	return mmu_table;
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
