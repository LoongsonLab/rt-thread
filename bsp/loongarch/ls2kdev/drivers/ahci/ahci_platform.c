#include <ahci_platform.h>

#include <libahci.h>
#include <blk_device.h>

// disk read
rt_ssize_t dwc_ahsata_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_uint64_t size)
{
    struct blk_device *blk = (struct blk_device *)dev;
    return sata_read_common(blk->ahci_device, blk, pos, size, buffer);
}

// disk write
rt_ssize_t dwc_ahsata_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_uint64_t size)
{
    struct blk_device *blk = (struct blk_device *)dev;
    return sata_write_common(blk->ahci_device, blk, pos, size, buffer);
}

rt_err_t dwc_ahsata_control(rt_device_t dev, int cmd, void *args)
{
    struct blk_device *blk = (struct blk_device *)dev;

    switch (cmd)
    {
    case RT_DEVICE_CTRL_BLK_GETGEOME:
        if (args != NULL)
        {
            struct rt_device_blk_geometry *info = (struct rt_device_blk_geometry *)args;
            info->sector_count = blk->lba;
            info->bytes_per_sector = blk->blksz;
            info->block_size = blk->blksz;
        }
        break;

    default:
        rt_kprintf("[dwc_ahsata_control] Unimplemented cmd\n");
        return -RT_ERROR;
    }

    return RT_EOK;
}

static struct rt_device_ops ahsata_scan_ops = {
    .init = RT_NULL,
    .open = RT_NULL,
    .close = RT_NULL,
    .control = dwc_ahsata_control,
    .read = dwc_ahsata_read,
    .write = dwc_ahsata_write
};

// scan ahci and register device for block device
int dwc_ahsata_scan(struct rt_device *dev)
{
    struct ahci_uc_priv *uc_priv = (struct ahci_uc_priv *)dev;
    struct blk_device *blk;
    rt_err_t ret;

    blk = (struct blk_device *)rt_device_create(RT_Device_Class_Block, sizeof(struct blk_device) - sizeof(struct rt_device));
    blk->parent.ops = &ahsata_scan_ops;
    blk->ahci_device = uc_priv;
    blk->blksz = 512;
    blk->log2blksz = 9;
    blk->lba = 0;
    ret = rt_device_register((rt_device_t)blk, "ls_ahci_blk", RT_DEVICE_FLAG_RDWR);

    if (ret != RT_EOK)
    {
        debug("Can't create device\n");
        return ret;
    }

    ret = dwc_ahsata_scan_common(uc_priv, blk);

    if (ret)
    {
        debug("%s: Failed to scan bus\n", __func__);
        return ret;
    }

    return 0;
}

int dwc_ahsata_probe(struct rt_device *dev)
{
    struct ahci_uc_priv *uc_priv = (struct ahci_uc_priv *)dev;
    int ret;

    uc_priv->host_flags = ATA_FLAG_SATA | ATA_FLAG_NO_LEGACY |
                         ATA_FLAG_MMIO | ATA_FLAG_PIO_DMA | ATA_FLAG_NO_ATAPI;

    // init ahci host
    ret = ahci_host_init(uc_priv);
    if (ret)
        return ret;

    ahci_print_info(uc_priv);

    return dwc_ahci_start_ports(uc_priv);
}

static struct rt_device_ops ahci_host_ops =
{
    .init = RT_NULL,
    .open = RT_NULL,
    .close = RT_NULL,
    .read = RT_NULL,
    .write = RT_NULL,
    .control = RT_NULL
};

// 初始化时调用
// 初始化ahci硬盘控制器，并注册相关的设备
// ls_ahci，ahci控制器
// ls_ahci_blk，硬盘块设备本体
int rt_hw_ahci_host_init()
{
    struct pci_header *p = (struct pci_header *)(LS_PCIE_SATA_ADDR);
    rt_uint64_t ahci_base = PHYS_TO_UNCACHED((p->BaseAddressRegister[0]) & 0xfffffff0);

    struct ahci_uc_priv *ahci_device;
    ahci_device = (struct ahci_uc_priv *)rt_device_create(RT_Device_Class_Miscellaneous, sizeof(struct ahci_uc_priv) - sizeof(struct rt_device));

    ahci_device->mmio_base = ahci_base;
    ahci_device->parent.ops = &ahci_host_ops;

    if (rt_device_register((rt_device_t)ahci_device, "ls_ahci", 0) != RT_EOK)
    {
        rt_kprintf("ahci device register failed\n");
        return -RT_ERROR;
    }

    if (dwc_ahsata_probe((rt_device_t)ahci_device) != 0)
    {
        rt_kprintf("ahci probe failed\n");
        return -RT_ERROR;
    }

    if (dwc_ahsata_scan((rt_device_t)ahci_device) != 0)
    {
        rt_kprintf("ahci host sata device scan failed\n");
        return -RT_ERROR;
    }

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_ahci_host_init);
