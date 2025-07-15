#ifndef __LS2K_LIBAHCI_H__
#define __LS2K_LIBAHCI_H__

#define AHCI_PCI_BAR 0x24
#define AHCI_MAX_SG 56 /* hardware max is 64K */
#define AHCI_CMD_SLOT_SZ 32
#define AHCI_MAX_CMD_SLOT 32
#define AHCI_RX_FIS_SZ 256
#define AHCI_CMD_TBL_HDR 0x80
#define AHCI_CMD_TBL_CDB 0x40
#define AHCI_CMD_TBL_SZ AHCI_CMD_TBL_HDR + (AHCI_MAX_SG * 16)
#define AHCI_PORT_PRIV_DMA_SZ (AHCI_CMD_SLOT_SZ * AHCI_MAX_CMD_SLOT + \
                               AHCI_CMD_TBL_SZ + AHCI_RX_FIS_SZ)
#define AHCI_CMD_ATAPI (1 << 5)
#define AHCI_CMD_WRITE (1 << 6)
#define AHCI_CMD_PREFETCH (1 << 7)
#define AHCI_CMD_RESET (1 << 8)
#define AHCI_CMD_CLR_BUSY (1 << 10)

#define RX_FIS_D2H_REG 0x40 /* offset of D2H Register FIS data */

/* Global controller registers */
#define HOST_CAP 0x00        /* host capabilities */
#define HOST_CTL 0x04        /* global host control */
#define HOST_IRQ_STAT 0x08   /* interrupt status */
#define HOST_PORTS_IMPL 0x0c /* bitmap of implemented ports */
#define HOST_VERSION 0x10    /* AHCI spec. version compliancy */
#define HOST_CAP2 0x24       /* host capabilities, extended */

/* HOST_CTL bits */
#define HOST_RESET (1 << 0)    /* reset controller; self-clear */
#define HOST_IRQ_EN (1 << 1)   /* global IRQ enable */
#define HOST_AHCI_EN (1 << 31) /* AHCI enabled */

/* Registers for each SATA port */
#define PORT_LST_ADDR 0x00    /* command list DMA addr */
#define PORT_LST_ADDR_HI 0x04 /* command list DMA addr hi */
#define PORT_FIS_ADDR 0x08    /* FIS rx buf addr */
#define PORT_FIS_ADDR_HI 0x0c /* FIS rx buf addr hi */
#define PORT_IRQ_STAT 0x10    /* interrupt status */
#define PORT_IRQ_MASK 0x14    /* interrupt enable/disable mask */
#define PORT_CMD 0x18         /* port command */
#define PORT_TFDATA 0x20      /* taskfile data */
#define PORT_SIG 0x24         /* device TF signature */
#define PORT_CMD_ISSUE 0x38   /* command issue */
#define PORT_SCR 0x28         /* SATA phy register block */
#define PORT_SCR_STAT 0x28    /* SATA phy register: SStatus */
#define PORT_SCR_CTL 0x2c     /* SATA phy register: SControl */
#define PORT_SCR_ERR 0x30     /* SATA phy register: SError */
#define PORT_SCR_ACT 0x34     /* SATA phy register: SActive */

/* PORT_IRQ_{STAT,MASK} bits */
#define PORT_IRQ_COLD_PRES (1 << 31)     /* cold presence detect */
#define PORT_IRQ_TF_ERR (1 << 30)        /* task file error */
#define PORT_IRQ_HBUS_ERR (1 << 29)      /* host bus fatal error */
#define PORT_IRQ_HBUS_DATA_ERR (1 << 28) /* host bus data error */
#define PORT_IRQ_IF_ERR (1 << 27)        /* interface fatal error */
#define PORT_IRQ_IF_NONFATAL (1 << 26)   /* interface non-fatal error */
#define PORT_IRQ_OVERFLOW (1 << 24)      /* xfer exhausted available S/G */
#define PORT_IRQ_BAD_PMP (1 << 23)       /* incorrect port multiplier */

#define PORT_IRQ_PHYRDY (1 << 22)     /* PhyRdy changed */
#define PORT_IRQ_DEV_ILCK (1 << 7)    /* device interlock */
#define PORT_IRQ_CONNECT (1 << 6)     /* port connect change status */
#define PORT_IRQ_SG_DONE (1 << 5)     /* descriptor processed */
#define PORT_IRQ_UNK_FIS (1 << 4)     /* unknown FIS rx'd */
#define PORT_IRQ_SDB_FIS (1 << 3)     /* Set Device Bits FIS rx'd */
#define PORT_IRQ_DMAS_FIS (1 << 2)    /* DMA Setup FIS rx'd */
#define PORT_IRQ_PIOS_FIS (1 << 1)    /* PIO Setup FIS rx'd */
#define PORT_IRQ_D2H_REG_FIS (1 << 0) /* D2H Register FIS rx'd */

#define PORT_IRQ_FATAL PORT_IRQ_TF_ERR | PORT_IRQ_HBUS_ERR | PORT_IRQ_HBUS_DATA_ERR | PORT_IRQ_IF_ERR

#define DEF_PORT_IRQ PORT_IRQ_FATAL | PORT_IRQ_PHYRDY | PORT_IRQ_CONNECT | PORT_IRQ_SG_DONE | PORT_IRQ_UNK_FIS | PORT_IRQ_SDB_FIS | PORT_IRQ_DMAS_FIS | PORT_IRQ_PIOS_FIS | PORT_IRQ_D2H_REG_FIS

/* PORT_SCR_STAT bits */
#define PORT_SCR_STAT_DET_MASK 0x3
#define PORT_SCR_STAT_DET_COMINIT 0x1
#define PORT_SCR_STAT_DET_PHYRDY 0x3

/* PORT_CMD bits */
#define PORT_CMD_ATAPI (1 << 24)   /* Device is ATAPI */
#define PORT_CMD_LIST_ON (1 << 15) /* cmd list DMA engine running */
#define PORT_CMD_FIS_ON (1 << 14)  /* FIS DMA engine running */
#define PORT_CMD_FIS_RX (1 << 4)   /* Enable FIS receive DMA engine */
#define PORT_CMD_CLO (1 << 3)      /* Command list override */
#define PORT_CMD_POWER_ON (1 << 2) /* Power up device */
#define PORT_CMD_SPIN_UP (1 << 1)  /* Spin up device */
#define PORT_CMD_START (1 << 0)    /* Enable port DMA engine */

#define PORT_CMD_ICC_ACTIVE (0x1 << 28)  /* Put i/f in active state */
#define PORT_CMD_ICC_PARTIAL (0x2 << 28) /* Put i/f in partial state */
#define PORT_CMD_ICC_SLUMBER (0x6 << 28) /* Put i/f in slumber state */

#define AHCI_MAX_PORTS 32

#define ATA_FLAG_SATA (1 << 3)
#define ATA_FLAG_NO_LEGACY (1 << 4)  /* no legacy mode check */
#define ATA_FLAG_MMIO (1 << 6)       /* use MMIO, not PIO */
#define ATA_FLAG_SATA_RESET (1 << 7) /* (obsolete) use COMRESET */
#define ATA_FLAG_PIO_DMA (1 << 8)    /* PIO cmds via DMA */
#define ATA_FLAG_NO_ATAPI (1 << 11)  /* No ATAPI support */

#define DWC_AHSATA_MAX_CMD_SLOTS 32

/* HBA Capabilities Register */
#define SATA_HOST_CAP_S64A 0x80000000
#define SATA_HOST_CAP_SNCQ 0x40000000
#define SATA_HOST_CAP_SSNTF 0x20000000
#define SATA_HOST_CAP_SMPS 0x10000000
#define SATA_HOST_CAP_SSS 0x08000000
#define SATA_HOST_CAP_SALP 0x04000000
#define SATA_HOST_CAP_SAL 0x02000000
#define SATA_HOST_CAP_SCLO 0x01000000
#define SATA_HOST_CAP_ISS_MASK 0x00f00000
#define SATA_HOST_CAP_ISS_OFFSET 20
#define SATA_HOST_CAP_SNZO 0x00080000
#define SATA_HOST_CAP_SAM 0x00040000
#define SATA_HOST_CAP_SPM 0x00020000
#define SATA_HOST_CAP_PMD 0x00008000
#define SATA_HOST_CAP_SSC 0x00004000
#define SATA_HOST_CAP_PSC 0x00002000
#define SATA_HOST_CAP_NCS 0x00001f00
#define SATA_HOST_CAP_CCCS 0x00000080
#define SATA_HOST_CAP_EMS 0x00000040
#define SATA_HOST_CAP_SXS 0x00000020
#define SATA_HOST_CAP_NP_MASK 0x0000001f

/* Global HBA Control Register */
#define SATA_HOST_GHC_AE 0x80000000
#define SATA_HOST_GHC_IE 0x00000002
#define SATA_HOST_GHC_HR 0x00000001

/* AHCI Version Register */
#define SATA_HOST_VS_MJR_MASK 0xffff0000
#define SATA_HOST_VS_MJR_OFFSET 16
#define SATA_HOST_VS_MJR_MNR 0x0000ffff

/* HBA Capabilities Extended Register */
#define SATA_HOST_CAP2_APST 0x00000004

/* BIST Activate FIS Register */
#define SATA_HOST_BISTAFR_NCP_MASK 0x0000ff00
#define SATA_HOST_BISTAFR_NCP_OFFSET 8
#define SATA_HOST_BISTAFR_PD_MASK 0x000000ff
#define SATA_HOST_BISTAFR_PD_OFFSET 0

/* BIST Control Register */
#define SATA_HOST_BISTCR_FERLB 0x00100000
#define SATA_HOST_BISTCR_TXO 0x00040000
#define SATA_HOST_BISTCR_CNTCLR 0x00020000
#define SATA_HOST_BISTCR_NEALB 0x00010000
#define SATA_HOST_BISTCR_LLC_MASK 0x00000700
#define SATA_HOST_BISTCR_LLC_OFFSET 8
#define SATA_HOST_BISTCR_ERREN 0x00000040
#define SATA_HOST_BISTCR_FLIP 0x00000020
#define SATA_HOST_BISTCR_PV 0x00000010
#define SATA_HOST_BISTCR_PATTERN_MASK 0x0000000f
#define SATA_HOST_BISTCR_PATTERN_OFFSET 0

/* Port# Command Register */
#define SATA_PORT_CMD_ICC_MASK 0xf0000000
#define SATA_PORT_CMD_ASP 0x08000000
#define SATA_PORT_CMD_ALPE 0x04000000
#define SATA_PORT_CMD_DLAE 0x02000000
#define SATA_PORT_CMD_ATAPI 0x01000000
#define SATA_PORT_CMD_APSTE 0x00800000
#define SATA_PORT_CMD_ESP 0x00200000
#define SATA_PORT_CMD_CPD 0x00100000
#define SATA_PORT_CMD_MPSP 0x00080000
#define SATA_PORT_CMD_HPCP 0x00040000
#define SATA_PORT_CMD_PMA 0x00020000
#define SATA_PORT_CMD_CPS 0x00010000
#define SATA_PORT_CMD_CR 0x00008000
#define SATA_PORT_CMD_FR 0x00004000
#define SATA_PORT_CMD_MPSS 0x00002000
#define SATA_PORT_CMD_CCS_MASK 0x00001f00
#define SATA_PORT_CMD_FRE 0x00000010
#define SATA_PORT_CMD_CLO 0x00000008
#define SATA_PORT_CMD_POD 0x00000004
#define SATA_PORT_CMD_SUD 0x00000002
#define SATA_PORT_CMD_ST 0x00000001

/* Port# Task File Data Register */
#define SATA_PORT_TFD_ERR_MASK 0x0000ff00
#define SATA_PORT_TFD_STS_MASK 0x000000ff
#define SATA_PORT_TFD_STS_ERR 0x00000001
#define SATA_PORT_TFD_STS_DRQ 0x00000008
#define SATA_PORT_TFD_STS_BSY 0x00000080

/* Port# Serial ATA Status {SStatus} Register */
#define SATA_PORT_SSTS_IPM_MASK 0x00000f00
#define SATA_PORT_SSTS_SPD_MASK 0x000000f0
#define SATA_PORT_SSTS_DET_MASK 0x0000000f

#define SATA_FLAG_Q_DEP_MASK 0x0000000f
#define SATA_FLAG_WCACHE 0x00000100
#define SATA_FLAG_FLUSH 0x00000200
#define SATA_FLAG_FLUSH_EXT 0x00000400

#define READ_CMD 0
#define WRITE_CMD 1

struct ahci_cmd_hdr
{
    uint32_t opts;
    uint32_t status;
    uint32_t tbl_addr_lo;
    uint32_t tbl_addr_hi;
    uint32_t reserved[4];
};

struct ahci_sg
{
    uint32_t addr_lo;
    uint32_t addr_hi;
    uint32_t reserved;
    uint32_t flags_size;
};

struct ahci_ioports
{
    void *port_mmio;
    struct ahci_cmd_hdr *cmd_slot;
    struct ahci_sg *cmd_tbl_sg;
    uint64_t cmd_tbl;
    uint32_t rx_fis;
};

struct ahci_uc_priv
{
    struct rt_device parent;
    struct ahci_ioports port[AHCI_MAX_PORTS];
    uint16_t *ataid[AHCI_MAX_PORTS];
    uint32_t n_ports;
    uint32_t hard_port_no;
    uint32_t host_flags;
    uint32_t host_set_flags;
    void *mmio_base;
    uint32_t pio_mask;
    uint32_t udma_mask;
    uint32_t flags;
    uint32_t cap;           // cache of HOST_CAP register
    uint32_t port_map;      // cache of HOST_PORTS_IMPL reg
    uint32_t link_port_map; // linkup port map
};

struct sata_port_regs
{
    uint32_t clb;
    uint32_t clbu;
    uint32_t fb;
    uint32_t fbu;
    uint32_t is;
    uint32_t ie;
    uint32_t cmd;
    uint32_t res1[1];
    uint32_t tfd;
    uint32_t sig;
    uint32_t ssts;
    uint32_t sctl;
    uint32_t serr;
    uint32_t sact;
    uint32_t ci;
    uint32_t sntf;
    uint32_t res2[1];
    uint32_t dmacr;
    uint32_t res3[1];
    uint32_t phycr;
    uint32_t physr;
};

struct sata_host_regs
{
    uint32_t cap;
    uint32_t ghc;
    uint32_t is;
    uint32_t pi;
    uint32_t vs;
    uint32_t ccc_ctl;
    uint32_t ccc_ports;
    uint32_t res1[2];
    uint32_t cap2;
    uint32_t res2[30];
    uint32_t bistafr;
    uint32_t bistcr;
    uint32_t bistfctr;
    uint32_t bistsr;
    uint32_t bistdecr;
    uint32_t res3[2];
    uint32_t oobr;
    uint32_t res4[8];
    uint32_t timer1ms;
    uint32_t res5[1];
    uint32_t gparam1r;
    uint32_t gparam2r;
    uint32_t pparamr;
    uint32_t testr;
    uint32_t versionr;
    uint32_t idr;
};

#endif // __LS2K_LIBAHCI_H__
