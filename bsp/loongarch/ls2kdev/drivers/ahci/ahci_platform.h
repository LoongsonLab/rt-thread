#ifndef __LS2K_AHCI_PLATFORM_H__
#define __LS2K_AHCI_PLATFORM_H__

#include <rtthread.h>
#include <rthw.h>
#include <rtconfig.h>
#include <drivers/blk.h>

#include <loongarch.h>
#include <ls2k1000la.h>
#include <drv_pci.h>

typedef signed   char             int8_t;
typedef signed   short            int16_t;
typedef signed   int              int32_t;
typedef unsigned char             uint8_t;
typedef unsigned short            uint16_t;
typedef unsigned int              uint32_t;
typedef signed   long             int64_t;
typedef unsigned long             uint64_t;



#define mdelay rt_thread_mdelay
#define udelay(...) rt_thread_mdelay(1)


#define plat_printf rt_kprintf
#define plat_assert RT_ASSERT
#define plat_malloc rt_malloc
#define plat_memset rt_memset
#define plat_memcpy rt_memcpy


// for debug
#define AHCI_DEBUG 0
#if(AHCI_DEBUG)
#define debug plat_printf
#else
#define debug(...)
#endif


static void ahci_flush_cache(void *buf, uint64_t size)
{
    ;
}

static void invalidate_dcache_range(void *start, void *end)
{
    ;
}


static inline uint32_t readl(void *addr)
{
    return *((volatile uint32_t *)addr);
}

static inline void writel(uint32_t data, void *addr)
{
    *((volatile uint32_t *)addr) = data;
}

static int plat_ffs(int i)
{
    int bit;

    if (0 == i)
        return 0;

    for (bit = 1; !(i & 1); ++bit)
        i >>= 1;

    return bit;
}

#endif // __LS2K_AHCI_PLATFORM_H__
