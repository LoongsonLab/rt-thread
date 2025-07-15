#include <ahci_platform.h>

#include <libahci.h>
#include <libfis.h>
#include <libata.h>
#include <blk_device.h>
#include <drv_ahci.h>

// only for test
uint8_t sector_data[512];

// dump buffer
void dump_buffer(void *p, int len)
{
    uint32_t i;
    uint8_t *q = p;

    plat_printf("----- dump_buffer -----\n");
    plat_printf("buf = %p,len = %d\n\n", p, len);

    for (i = 0; i < len; i++)
    {
        if (!(i & 0xF))
        {
            plat_printf("%p", &q[i]);
        }

        plat_printf(" %02x", q[i]);

        if ((i & 0xF) == 0xF)
        {
            plat_printf("\n");
        }
    }

    plat_printf("\n----------------------\n");
}

int waiting_for_cmd_completed(void *offset, int timeout_msec, uint32_t sign)
{
    int i = 0;
    uint32_t status;

    for (i = 0; ((status = readl(offset)) & sign) && i < timeout_msec; i++)
    {
        mdelay(1);
    }

    return (i < timeout_msec) ? 0 : -1;
}

// get base address of 'port'
static inline void *ahci_port_base(void *base, uint32_t port)
{
    return base + 0x100 + (port * 0x80);
}

// init ahci
int ahci_host_init(struct ahci_uc_priv *uc_priv)
{
    uint32_t tmp;
    int i, j, timeout = 1000;
    struct sata_port_regs *port_mmio = NULL;
    struct sata_host_regs *host_mmio = uc_priv->mmio_base;

    // reset ahci
    tmp = readl(&host_mmio->ghc);

    // ahsata controller reset
    if ((tmp & SATA_HOST_GHC_HR) == 0)
        writel(tmp | SATA_HOST_GHC_HR, &host_mmio->ghc);

    // wait for reset finishing
    do {
        udelay(100);
        tmp = readl(&host_mmio->ghc);
    }
    while ((tmp & SATA_HOST_GHC_HR) && --timeout);

    // reset timeout
    if (timeout <= 0)
    {
        debug("controller reset failed (0x%x)\n", tmp);
        return -1;
    }

    // enable ahci
    writel(SATA_HOST_GHC_AE, &host_mmio->ghc);

    // get cap
    uc_priv->cap = readl(&host_mmio->cap);

    // which ports the sata supports
    uc_priv->port_map = readl(&host_mmio->pi);

    // how many ports the sata supports
    uc_priv->n_ports = (uc_priv->cap & SATA_HOST_CAP_NP_MASK) + 1;

    debug("cap 0x%x  port_map 0x%x  n_ports %d\n",
          uc_priv->cap, uc_priv->port_map, uc_priv->n_ports);
    
    // init each port
    for (i = 0; i < uc_priv->n_ports; i++)
    {
        uc_priv->port[i].port_mmio = ahci_port_base(host_mmio, i);
        port_mmio = uc_priv->port[i].port_mmio;

        // ensure that the sata is in idle state
        tmp = readl(&port_mmio->cmd);

        // check ST FRE FR CR bits
        if (tmp & (SATA_PORT_CMD_CR | SATA_PORT_CMD_FR |
                   SATA_PORT_CMD_FRE | SATA_PORT_CMD_ST))
        {
            debug("Port %d is active, deactivating\n", i);

            // clear ST to stop
            tmp &= ~SATA_PORT_CMD_ST;
            writel(tmp, &port_mmio->cmd);

            // sleep 500 msecs
            mdelay(500);

            // wait for CR
            timeout = 1000;
            while ((readl(&port_mmio->cmd) & SATA_PORT_CMD_CR) && --timeout);

            if (timeout <= 0)
            {
                debug("port reset failed (0x%x)\n", tmp);
                return -1;
            }
        }

        // set Spin-Up Device
        tmp = readl(&port_mmio->cmd);
        writel((tmp | SATA_PORT_CMD_SUD), &port_mmio->cmd);

        // wait for SUD
        timeout = 1000;
        while (!(readl(&port_mmio->cmd) | SATA_PORT_CMD_SUD) && --timeout);

        if (timeout <= 0)
        {
            debug("Spin-Up can't finish!\n");
            return -1;
        }

        // wait for ahci link up
        for (timeout = 50; timeout > 0; -- timeout)
        {
            mdelay(1);
            tmp = readl(&port_mmio->ssts);
            tmp &= SATA_PORT_SSTS_DET_MASK;
            if (tmp == 0x3)
                break;
        }

        if (timeout <= 0)
            debug("STAT link %d timeout\n", i);
        else
            debug("STAT link up\n");

        // clear serr
        tmp = readl(&port_mmio->serr);
        debug("PxSERR 0x%x\n", tmp);
        writel(tmp, &port_mmio->serr);

        // ack any pending irq events for this port
        tmp = readl(&host_mmio->is);
        debug("IS 0x%x\n", tmp);
        if (tmp)
        {
            writel(tmp, &host_mmio->is);
        }

        writel(1 << i, &host_mmio->is);

        // set irq mask (enables interrupts)
        writel(DEF_PORT_IRQ, &port_mmio->ie);

        // register linkup ports
        tmp = readl(&port_mmio->ssts);
        debug("Port %d status: 0x%x\n", i, tmp);
        if ((tmp & SATA_PORT_SSTS_DET_MASK) == 0x03)
        {
            uc_priv->link_port_map |= (0x01 << i);
        }
    }

    tmp = readl(&host_mmio->ghc);
    debug("GHC 0x%x\n", tmp);

    // interrupt enable
    writel(tmp | SATA_HOST_GHC_IE, &host_mmio->ghc);
    tmp = readl(&host_mmio->ghc);
    debug("GHC 0x%x\n", tmp);

    return 0;
}

// dump ahci info
void ahci_print_info(struct ahci_uc_priv *uc_priv)
{
    struct sata_host_regs *host_mmio = uc_priv->mmio_base;
    uint32_t vers, cap, impl, speed;
    const char *speed_s;
    const char *scc_s;

    vers = readl(&host_mmio->vs);
    cap = uc_priv->cap;
    impl = uc_priv->port_map;

    speed = (cap & SATA_HOST_CAP_ISS_MASK) >> SATA_HOST_CAP_ISS_OFFSET;

    if (speed == 1)
    {
        speed_s = "1.5";
    }
    else if (speed == 2)
    {
        speed_s = "3";
    }
    else
    {
        speed_s = "?";
    }

    scc_s = "SATA";

    plat_printf("AHCI %02x%02x.%02x%02x "
               "%u slots %u ports %s Gbps 0x%x impl %s mode\n",
               (vers >> 24) & 0xff,
               (vers >> 16) & 0xff,
               (vers >> 8) & 0xff,
               vers & 0xff,
               ((cap >> 8) & 0x1f) + 1,
               (cap & 0x1f) + 1,
               speed_s,
               impl,
               scc_s);

    plat_printf("flags: "
               "%s%s%s%s%s%s"
               "%s%s%s%s%s%s%s\n",
               cap & (1 << 31) ? "64bit " : "",
               cap & (1 << 30) ? "ncq " : "",
               cap & (1 << 28) ? "ilck " : "",
               cap & (1 << 27) ? "stag " : "",
               cap & (1 << 26) ? "pm " : "",
               cap & (1 << 25) ? "led " : "",
               cap & (1 << 24) ? "clo " : "",
               cap & (1 << 19) ? "nz " : "",
               cap & (1 << 18) ? "only " : "",
               cap & (1 << 17) ? "pmp " : "",
               cap & (1 << 15) ? "pio " : "",
               cap & (1 << 14) ? "slum " : "",
               cap & (1 << 13) ? "part " : "");
}

// configure sgdma
uint32_t ahci_fill_sg(struct ahci_uc_priv *uc_priv, uint8_t port,
                     uint8_t *buf, uint32_t buf_len)
{
    struct ahci_ioports *pp = &uc_priv->port[port];
    struct ahci_sg *ahci_sg = pp->cmd_tbl_sg;
    uint32_t sg_count, max_bytes;
    uint32_t i;

    max_bytes = MAX_DATA_BYTES_PER_SG; // 4MiB
    sg_count = ((buf_len - 1) / max_bytes) + 1;

    if (sg_count > AHCI_MAX_SG) // 56 * 4MiB
    {
        plat_printf("[ahci_fill_sg] Too much sg\n");
        return -1;
    }

    for (i = 0; i < sg_count; i++)
    {
        ahci_sg->addr_lo = (uint32_t)(CACHED_TO_PHYS(buf + i * max_bytes) & 0xffffffff);
        ahci_sg->addr_hi = (uint32_t)(CACHED_TO_PHYS(buf + i * max_bytes) >> 32);
        ahci_sg->flags_size = (0x3fffff & (buf_len < max_bytes ? (buf_len - 1) : (max_bytes - 1)));
        ahci_sg++;
        buf_len -= max_bytes;
    }

    return sg_count;
}

// fill cmd slot
void ahci_fill_cmd_slot(struct ahci_ioports *pp, uint32_t cmd_slot, uint32_t opts)
{
    struct ahci_cmd_hdr *cmd_hdr = (struct ahci_cmd_hdr *)(pp->cmd_slot +
                                                           AHCI_CMD_SLOT_SZ * cmd_slot);
    plat_memset(cmd_hdr, 0, AHCI_CMD_SLOT_SZ);
    cmd_hdr->opts = opts;
    cmd_hdr->status = 0;
    cmd_hdr->tbl_addr_lo = (uint32_t)(CACHED_TO_PHYS(pp->cmd_tbl) & 0xffffffff);
    cmd_hdr->tbl_addr_hi = (uint32_t)(CACHED_TO_PHYS(pp->cmd_tbl) >> 32);
}

// send ahci cmd
int ahci_exec_ata_cmd(struct ahci_uc_priv *uc_priv, uint8_t port,
                      struct sata_fis_h2d *cfis, uint8_t *buf, uint32_t buf_len,
                      int is_write)
{
    struct ahci_ioports *pp = &uc_priv->port[port];
    struct sata_port_regs *port_mmio = pp->port_mmio;
    uint32_t opts;
    uint32_t sg_count = 0, cmd_slot = 0;

    // get available slot
    cmd_slot = AHCI_GET_CMD_SLOT(readl(&port_mmio->ci));

    if (32 == cmd_slot)
    {
        plat_printf("Can't find empty command slot!\n");
        return 0;
    }

    // check xfer length
    // 65536 * 512
    if (buf_len > MAX_BYTES_PER_TRANS)
    {
        plat_printf("Max transfer length is %dB\n\r",
                   MAX_BYTES_PER_TRANS);
        return 0;
    }

    plat_memcpy(pp->cmd_tbl, cfis, sizeof(struct sata_fis_h2d));

    if (buf && buf_len)
    {
        sg_count = ahci_fill_sg(uc_priv, port, buf, buf_len);
    }
    opts = (sizeof(struct sata_fis_h2d) >> 2) | (sg_count << 16) | (is_write << 6);

    if (is_write)
    {
        ahci_flush_cache(buf, buf_len);
    }

    ahci_fill_cmd_slot(pp, cmd_slot, opts);

    ahci_flush_cache(pp->cmd_slot, AHCI_PORT_PRIV_DMA_SZ);
    
    // start transfer
    writel(1 << cmd_slot, &port_mmio->ci);

    // 10000
    if (waiting_for_cmd_completed((uint8_t *)&port_mmio->ci, 1000,
                                  1 << cmd_slot))
    {
        plat_printf("timeout exit!\n");
        return -1;
    }

    invalidate_dcache_range(pp->cmd_slot,
                            pp->cmd_slot + AHCI_PORT_PRIV_DMA_SZ);

    debug("ahci_exec_ata_cmd: %d byte transferred.\n",
          pp->cmd_slot->status);

    if (!is_write)
    {
        invalidate_dcache_range(buf, buf + buf_len);
    }

    return buf_len;
}

void ahci_set_feature(struct ahci_uc_priv *uc_priv, uint8_t port)
{
    struct sata_fis_h2d h2d __aligned(ARCH_DMA_MINALIGN);
    struct sata_fis_h2d *cfis = &h2d;

    plat_memset(cfis, 0, sizeof(struct sata_fis_h2d));
    cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
    cfis->pm_port_c = 0x80;
    cfis->command = ATA_CMD_SET_FEATURES;
    cfis->features = SETFEATURES_XFER;
    cfis->sector_count = plat_ffs(uc_priv->udma_mask + 1) + 0x3e;

    ahci_exec_ata_cmd(uc_priv, port, cfis, NULL, 0, READ_CMD);
}

int ahci_port_start(struct ahci_uc_priv *uc_priv, uint8_t port)
{
    struct ahci_ioports *pp = &uc_priv->port[port];
    struct sata_port_regs *port_mmio = pp->port_mmio;
    uint32_t port_status;
    uint64_t mem;
    int timeout = 10000000;

    debug("Enter start port: %d\n", port);
    port_status = readl(&port_mmio->ssts);
    debug("Port %d status: %x\n", port, port_status);

    if ((port_status & 0xf) != 0x03)
    {
        plat_printf("No Link on this port!\n");
        return -1;
    }

    mem = plat_malloc(AHCI_PORT_PRIV_DMA_SZ + 1024);
    plat_assert(mem);

    // align to 1024 bytes
    mem = (mem + 0x400) & (~0x3ff);
    plat_memset(mem, 0, AHCI_PORT_PRIV_DMA_SZ);

    // First item in chunk of DMA memory: 32-slot command table,
    // 32 bytes each in size
    pp->cmd_slot = (struct ahci_cmd_hdr *)mem;
    debug("cmd_slot = 0x%016lx\n", pp->cmd_slot);
    mem += (AHCI_CMD_SLOT_SZ * DWC_AHSATA_MAX_CMD_SLOTS);

    // Second item: Received-FIS area, 256-Byte aligned
    pp->rx_fis = mem;
    mem += AHCI_RX_FIS_SZ;

    // Third item: data area for storing a single command
    // and its scatter-gather table
    pp->cmd_tbl = mem;
    debug("cmd_tbl_dma = 0x%lx\n", pp->cmd_tbl);

    mem += AHCI_CMD_TBL_HDR;

    pp->cmd_tbl_sg = mem;
    writel((CACHED_TO_PHYS(pp->cmd_slot) & 0xffffffff), &port_mmio->clb);
    writel((CACHED_TO_PHYS(pp->cmd_slot) >> 32), &port_mmio->clbu);
    writel((CACHED_TO_PHYS(pp->rx_fis) & 0xffffffff), &port_mmio->fb);
    writel((CACHED_TO_PHYS(pp->rx_fis) >> 32), &port_mmio->fbu);

    writel(PORT_CMD_ICC_ACTIVE | PORT_CMD_FIS_RX |
            PORT_CMD_POWER_ON | PORT_CMD_SPIN_UP |
            PORT_CMD_START, &port_mmio->cmd);

    debug("Exit start port %d\n", port);

    // Wait device ready
    while ((readl(&port_mmio->tfd) & (SATA_PORT_TFD_STS_ERR |
                                      SATA_PORT_TFD_STS_DRQ | SATA_PORT_TFD_STS_BSY)) &&
           --timeout);

    if (timeout <= 0)
    {
        debug("Device not ready for BSY, DRQ and"
              "ERR in TFD!\n");
        return -1;
    }

    return 0;
}

void dwc_ahsata_print_info(struct blk_device *pdev)
{
    plat_printf("SATA Device Info:\n\r");
    plat_printf("S/N: %s\n\rProduct model number: %s\n\r"
        "Firmware version: %s\n\rCapacity: %lu sectors\n\r",
        pdev->product, pdev->vendor, pdev->revision, pdev->lba);
}

void dwc_ahsata_identify(struct ahci_uc_priv *uc_priv, uint16_t *id)
{
    struct sata_fis_h2d h2d __aligned(ARCH_DMA_MINALIGN);
    struct sata_fis_h2d *cfis = &h2d;
    uint8_t port = uc_priv->hard_port_no;

    plat_memset(cfis, 0, sizeof(struct sata_fis_h2d));

    cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
    cfis->pm_port_c = 0x80;
    cfis->command = ATA_CMD_ID_ATA;

    ahci_exec_ata_cmd(uc_priv, port, cfis, (uint8_t *)id, ATA_ID_WORDS * 2,
                      READ_CMD);
}

void dwc_ahsata_xfer_mode(struct ahci_uc_priv *uc_priv, uint16_t *id)
{
    uc_priv->pio_mask = id[ATA_ID_PIO_MODES];
    uc_priv->udma_mask = id[ATA_ID_UDMA_MODES];
    debug("pio %04x, udma %04x\n\r", uc_priv->pio_mask, uc_priv->udma_mask);
}

// set cmd for lba28
uint32_t dwc_ahsata_rw_cmd(struct ahci_uc_priv *uc_priv, uint32_t start,
                           uint32_t blkcnt, uint8_t *buffer, int is_write)
{
    struct sata_fis_h2d h2d __aligned(ARCH_DMA_MINALIGN);
    struct sata_fis_h2d *cfis = &h2d;
    uint8_t port = uc_priv->hard_port_no;
    uint32_t block;

    block = start;

    plat_memset(cfis, 0, sizeof(struct sata_fis_h2d));

    cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
    cfis->pm_port_c = 0x80;
    cfis->command = (is_write) ? ATA_CMD_WRITE : ATA_CMD_READ;
    cfis->device = ATA_LBA;

    cfis->device |= (block >> 24) & 0xf;
    cfis->lba_high = (block >> 16) & 0xff;
    cfis->lba_mid = (block >> 8) & 0xff;
    cfis->lba_low = block & 0xff;
    cfis->sector_count = blkcnt & 0xff; // 12

    if (ahci_exec_ata_cmd(uc_priv, port, cfis, buffer,
                          ATA_SECT_SIZE * blkcnt, is_write) > 0)
        return blkcnt;
    else
        return 0;
}

// flush cache for lba28
void dwc_ahsata_flush_cache(struct ahci_uc_priv *uc_priv)
{
    struct sata_fis_h2d h2d __aligned(ARCH_DMA_MINALIGN);
    struct sata_fis_h2d *cfis = &h2d;
    uint8_t port = uc_priv->hard_port_no;

    plat_memset(cfis, 0, sizeof(struct sata_fis_h2d));

    cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
    cfis->pm_port_c = 0x80; /* is command */
    cfis->command = ATA_CMD_FLUSH;

    ahci_exec_ata_cmd(uc_priv, port, cfis, NULL, 0, 0);
}

// read/write for lba28
uint32_t ata_low_level_rw_lba28(struct ahci_uc_priv *uc_priv, uint32_t blknr,
                                uint64_t blkcnt, const void *buffer, int is_write)
{
    uint32_t start, blks;
    uint8_t *addr;
    int max_blks;

    start = blknr;
    blks = blkcnt;
    addr = (uint8_t *)buffer;

    max_blks = ATA_MAX_SECTORS;
    do
    {
        if (blks > max_blks)
        {
            if (max_blks != dwc_ahsata_rw_cmd(uc_priv, start,
                                              max_blks, addr,
                                              is_write))
                return 0;
            start += max_blks;
            blks -= max_blks;
            addr += ATA_SECT_SIZE * max_blks;
        }
        else
        {
            if (blks != dwc_ahsata_rw_cmd(uc_priv, start, blks,
                                          addr, is_write))
                return 0;
            start += blks;
            blks = 0;
            addr += ATA_SECT_SIZE * blks;
        }
    } while (blks != 0);

    return blkcnt;
}

uint32_t dwc_ahsata_rw_cmd_ext(struct ahci_uc_priv *uc_priv, uint64_t start,
                               uint64_t blkcnt, uint8_t *buffer, int is_write)
{
    struct sata_fis_h2d h2d __aligned(ARCH_DMA_MINALIGN);
    struct sata_fis_h2d *cfis = &h2d;
    uint8_t port = uc_priv->hard_port_no;
    uint64_t block;

    block = start;

    plat_memset(cfis, 0, sizeof(struct sata_fis_h2d));

    cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D; // 0
    cfis->pm_port_c = 0x80; // 1

    cfis->command = (is_write) ? ATA_CMD_WRITE_EXT : ATA_CMD_READ_EXT; // 2

    cfis->lba_high_exp = (block >> 40) & 0xff;
    cfis->lba_mid_exp = (block >> 32) & 0xff;
    cfis->lba_low_exp = (block >> 24) & 0xff;
    cfis->lba_high = (block >> 16) & 0xff;
    cfis->lba_mid = (block >> 8) & 0xff;
    cfis->lba_low = block & 0xff;
    cfis->device = ATA_LBA; // 7
    cfis->sector_count_exp = (blkcnt >> 8) & 0xff; // 13
    cfis->sector_count = blkcnt & 0xff; // 12

    // 512 bytes * blkcnt
    if (ahci_exec_ata_cmd(uc_priv, port, cfis, buffer,
                          ATA_SECT_SIZE * blkcnt, is_write) > 0)
        return blkcnt;
    else
        return 0;
}

void dwc_ahsata_flush_cache_ext(struct ahci_uc_priv *uc_priv)
{
    struct sata_fis_h2d h2d __aligned(ARCH_DMA_MINALIGN);
    struct sata_fis_h2d *cfis = &h2d;
    uint8_t port = uc_priv->hard_port_no;

    plat_memset(cfis, 0, sizeof(struct sata_fis_h2d));

    cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
    cfis->pm_port_c = 0x80; /* is command */
    cfis->command = ATA_CMD_FLUSH_EXT;

    ahci_exec_ata_cmd(uc_priv, port, cfis, NULL, 0, 0);
}

void dwc_ahsata_init_wcache(struct ahci_uc_priv *uc_priv, uint16_t *id)
{
    if (ata_id_has_wcache(id) && ata_id_wcache_enabled(id))
        uc_priv->flags |= SATA_FLAG_WCACHE;
    if (ata_id_has_flush(id))
        uc_priv->flags |= SATA_FLAG_FLUSH;
    if (ata_id_has_flush_ext(id))
        uc_priv->flags |= SATA_FLAG_FLUSH_EXT;
}

// read/write for lba48
uint32_t ata_low_level_rw_lba48(struct ahci_uc_priv *uc_priv, uint64_t blknr,
                                uint64_t blkcnt, const void *buffer,
                                int is_write)
{
    uint64_t start, blks;
    uint8_t *addr;
    int max_blks;

    start = blknr;
    blks = blkcnt;
    addr = (uint8_t *)buffer;

    max_blks = ATA_MAX_SECTORS_LBA48;

    do
    {
        if (blks > max_blks)
        {
            if (max_blks != dwc_ahsata_rw_cmd_ext(uc_priv, start, max_blks, addr, is_write))
                return 0;
            start += max_blks;
            blks -= max_blks;
            addr += ATA_SECT_SIZE * max_blks;
        }
        else
        {
            if (blks != dwc_ahsata_rw_cmd_ext(uc_priv, start, blks, addr, is_write))
                return 0;
            start += blks;
            blks = 0;
            addr += ATA_SECT_SIZE * blks;
        }
    } while (blks != 0);

    return blkcnt;
}

int dwc_ahci_start_ports(struct ahci_uc_priv *uc_priv)
{
    uint32_t linkmap;
    int i;

    linkmap = uc_priv->link_port_map;

    if (0 == linkmap)
    {
        plat_printf("No port device detected!\n");
        return -1;
    }

    for (i = 0; i < uc_priv->n_ports; i++)
    {
        if ((linkmap >> i) && ((linkmap >> i) & 0x01))
        {
            if (ahci_port_start(uc_priv, (uint8_t)i))
            {
                plat_printf("Can not start port %d\n", i);
                return 1;
            }

            uc_priv->hard_port_no = i;
            break;
        }
    }

    return 0;
}

int dwc_ahsata_scan_common(struct ahci_uc_priv *uc_priv, struct blk_device *pdev)
{
    uint8_t serial[ATA_ID_SERNO_LEN + 1] = {0};
    uint8_t firmware[ATA_ID_FW_REV_LEN + 1] = {0};
    uint8_t product[ATA_ID_PROD_LEN + 1] = {0};
    uint8_t port = uc_priv->hard_port_no;
    ALLOC_CACHE_ALIGN_BUFFER(uint16_t, id, ATA_ID_WORDS);

    // identify device
    dwc_ahsata_identify(uc_priv, id);

    // serial number
    ata_id_c_string(id, serial, ATA_ID_SERNO, sizeof(serial));
    plat_memcpy(pdev->product, serial, sizeof(serial));

    // firmware version
    ata_id_c_string(id, firmware, ATA_ID_FW_REV, sizeof(firmware));
    plat_memcpy(pdev->revision, firmware, sizeof(firmware));

    // product model
    ata_id_c_string(id, product, ATA_ID_PROD, sizeof(product));
    plat_memcpy(pdev->vendor, product, sizeof(product));

    // ata_dump_id(id);

    // total sectors
    pdev->lba = ata_id_n_sectors(id);

    pdev->type = DEV_TYPE_HARDDISK;
    pdev->blksz = ATA_SECT_SIZE;
    pdev->lun = 0;

    // check if support LBA48
    if (ata_id_has_lba48(id))
    {
        pdev->lba48 = 1;
        debug("Device support LBA48\n\r");
    }

    // get the NCQ queue depth from device
    uc_priv->flags &= (~SATA_FLAG_Q_DEP_MASK);
    uc_priv->flags |= ata_id_queue_depth(id);

    // get the xfer mode from device
    dwc_ahsata_xfer_mode(uc_priv, id);

    // get the write cache status from device
    dwc_ahsata_init_wcache(uc_priv, id);

    // set the xfer mode to highest speed
    ahci_set_feature(uc_priv, port);
    dwc_ahsata_print_info(pdev);

    // read the first sector
    // dwc_ahsata_read((rt_device_t)pdev, 0, sector_data, 1);
    // dump_buffer(sector_data, 512);

    return 0;
}

// read function between ahci driver and os
uint64_t sata_read_common(struct ahci_uc_priv *uc_priv,
                          struct blk_device *desc, uint64_t blknr,
                          uint64_t blkcnt, void *buffer)
{
    uint64_t rc;

    if (desc->lba48)
        rc = ata_low_level_rw_lba48(uc_priv, blknr, blkcnt, buffer,
                                    READ_CMD);
    else
        rc = ata_low_level_rw_lba28(uc_priv, blknr, blkcnt, buffer,
                                    READ_CMD);

    return rc;
}

// write function between ahci driver and os
uint64_t sata_write_common(struct ahci_uc_priv *uc_priv,
                           struct blk_device *desc, uint64_t blknr,
                           uint64_t blkcnt, const void *buffer)
{
    uint64_t rc;
    uint32_t flags = uc_priv->flags;

    if (desc->lba48)
    {
        rc = ata_low_level_rw_lba48(uc_priv, blknr, blkcnt, buffer,
                                    WRITE_CMD);
        if ((flags & SATA_FLAG_WCACHE) && (flags & SATA_FLAG_FLUSH_EXT))
            dwc_ahsata_flush_cache_ext(uc_priv);
    }
    else
    {
        rc = ata_low_level_rw_lba28(uc_priv, blknr, blkcnt, buffer,
                                    WRITE_CMD);
        if ((flags & SATA_FLAG_WCACHE) && (flags & SATA_FLAG_FLUSH))
            dwc_ahsata_flush_cache(uc_priv);
    }

    return rc;
}

int dwc_ahsata_port_status(struct rt_device *dev, int port)
{
    struct ahci_uc_priv *uc_priv = (struct ahci_uc_priv *)dev;
    struct sata_port_regs *port_mmio;

    port_mmio = uc_priv->port[port].port_mmio;
    return (readl(&port_mmio->ssts) & SATA_PORT_SSTS_DET_MASK) ? 0 : -1;
}

int dwc_ahsata_bus_reset(struct rt_device *dev)
{
    struct ahci_uc_priv *uc_priv = (struct ahci_uc_priv *)dev;
    struct sata_host_regs *host_mmio = uc_priv->mmio_base;

    writel(&host_mmio->ghc, SATA_HOST_GHC_HR);

    while (readl(&host_mmio->ghc) & SATA_HOST_GHC_HR)
    {
        udelay(100);
    }

    return 0;
}
