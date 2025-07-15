#ifndef __LS2K_DRV_AHCI_H__
#define __LS2K_DRV_AHCI_H__

#include <ahci_platform.h>
#include <libahci.h>
#include <blk_device.h>

#define AHCI_GET_CMD_SLOT(c) ((c) ? plat_ffs(c) : 0)

#define MAX_DATA_BYTES_PER_SG (4 * 1024 * 1024)
#define MAX_BYTES_PER_TRANS (AHCI_MAX_SG * MAX_DATA_BYTES_PER_SG)

#define ARCH_DMA_MINALIGN 1024
#define __aligned(x) __attribute__((__aligned__(x)))

#define ALIGN_1(x, a) __ALIGN_MASK((x), (typeof(x))(a)-1)
#define __ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))

#define ROUND(a, b) (((a) + (b)-1) & ~((b)-1))

#define PAD_COUNT(s, pad) (((s)-1) / (pad) + 1)
#define PAD_SIZE(s, pad) (PAD_COUNT(s, pad) * pad)

#define ALLOC_ALIGN_BUFFER_PAD(type, name, size, align, pad)                         \
    char __##name[ROUND(PAD_SIZE((size) * sizeof(type), pad), align) + (align - 1)]; \
                                                                                     \
    type *name = (type *)ALIGN_1((uint64_t)__##name, align)

#define ALLOC_ALIGN_BUFFER(type, name, size, align) \
    ALLOC_ALIGN_BUFFER_PAD(type, name, size, align, 1)

#define ALLOC_CACHE_ALIGN_BUFFER_PAD(type, name, size, pad) \
    ALLOC_ALIGN_BUFFER_PAD(type, name, size, ARCH_DMA_MINALIGN, pad)

#define ALLOC_CACHE_ALIGN_BUFFER(type, name, size) \
    ALLOC_ALIGN_BUFFER(type, name, size, ARCH_DMA_MINALIGN)



int dwc_ahsata_probe(struct rt_device *dev);
int dwc_ahsata_scan(struct rt_device *dev);
int dwc_ahsata_port_status(struct rt_device *dev, int port);
int dwc_ahci_start_ports(struct ahci_uc_priv *uc_priv);\

int ahci_host_init(struct ahci_uc_priv *uc_priv);
void ahci_print_info(struct ahci_uc_priv *uc_priv);

uint64_t sata_read_common(struct ahci_uc_priv *uc_priv,
                          struct blk_device *desc, uint64_t blknr,
                          uint64_t blkcnt, void *buffer);
uint64_t sata_write_common(struct ahci_uc_priv *uc_priv,
                           struct blk_device *desc, uint64_t blknr,
                           uint64_t blkcnt, const void *buffer);

#endif // __LS2K_DRV_AHCI_H__
