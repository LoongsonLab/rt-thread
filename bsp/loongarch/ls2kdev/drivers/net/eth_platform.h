#ifndef __LS2K_ETH_PLATFORM_H__
#define __LS2K_ETH_PLATFORM_H__

#include <rtthread.h>
#include <rtdef.h>
#include <rthw.h>

// lwip
#include <netif/ethernetif.h>

#include <ls2k1000la.h>
#include <drv_interrupt.h>
#include <drv_pci.h>



// for delay
#define DEFAULT_LOOP_VARIABLE 1000
#define mdelay      rt_thread_mdelay
#define udelay(...) rt_thread_mdelay(1)


#define plat_printf rt_kprintf
#define plat_assert RT_ASSERT
#define plat_memcpy rt_memcpy


typedef signed   char             int8_t;
typedef signed   short            int16_t;
typedef signed   int              int32_t;
typedef unsigned char             uint8_t;
typedef unsigned short            uint16_t;
typedef unsigned int              uint32_t;
typedef signed   long             int64_t;
typedef unsigned long             uint64_t;



// flush cache
static void flush_cache(void *buf, uint64_t size)
{
}

// alloc memory
static void *plat_malloc(uint32_t bytes)
{
    return rt_malloc(bytes);
}

// free memory
static void plat_free(void *buffer)
{
    rt_free(buffer);
}

// convert virtual address to physical address
static uint64_t gmac_dmamap(uint64_t va, uint32_t size)
{
    return CACHED_TO_PHYS(va);
}

// allocate 16 bytes aligned memory
// addr return physical address
// buf return uncached virtual address
static void *plat_malloc_dmaable(uint32_t size, uint64_t *addr)
{
    void *buf = rt_malloc_align(size, 16);
    *addr = gmac_dmamap(buf, size);
    buf = CACHED_TO_UNCACHED(buf);
    return buf;
}

// convert virtual address to physical address and flush cache
static uint64_t plat_dma_map_single(void *hwdev, void *ptr, uint32_t size)
{
    uint64_t addr = ptr;
    flush_cache(addr, size);
    return gmac_dmamap(addr, size);
}

static uint32_t eth_gmac_read_reg(uint64_t base, uint32_t offset)
{
    uint64_t addr;
    uint32_t data;

    addr = base + (uint32_t)offset;
    data = *(volatile uint32_t *)addr;
    return data;
}

static void eth_gmac_write_reg(uint64_t base, uint32_t offset, uint32_t data)
{
    uint64_t addr;
    addr = base + (uint32_t)offset;
    *(volatile uint32_t *)addr = data;
}

static void eth_gmac_set_bits(uint64_t base, uint32_t offset, uint32_t pos)
{
    uint32_t data;
    data = eth_gmac_read_reg(base, offset);
    data |= pos;
    eth_gmac_write_reg(base, offset, data);
}

static void eth_gmac_clear_bits(uint64_t base, uint32_t offset, uint32_t pos)
{
    uint32_t data;
    data = eth_gmac_read_reg(base, offset);
    data &= (~pos);
    eth_gmac_write_reg(base, offset, data);
}

#endif // __LS2K_ETH_PLATFORM_H__
