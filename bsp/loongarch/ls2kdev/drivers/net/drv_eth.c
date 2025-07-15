#include <eth_dev.h>
#include <drv_eth.h>
#include <eth_mii.h>

#include <eth_platform.h>

ls_eth_device ls_eth_dev;

// mdio read
uint32_t mdio_read(eth_adapter *adapter, int addr, int reg)
{
    net_device *gmacdev = adapter->dev;
    uint16_t data;
    eth_gmac_mdio_read(gmacdev->MacBase, addr, reg, &data);
    return data;
}

// mdio write
void mdio_write(eth_adapter *adapter, int addr, int reg, int data)
{
    net_device *gmacdev = adapter->dev;
    eth_gmac_mdio_write(gmacdev->MacBase, addr, reg, data);
}

uint32_t mii_nway_result(uint32_t negotiated)
{
    uint32_t ret;

    if (negotiated & LPA_100FULL)
        ret = LPA_100FULL;
    else if (negotiated & LPA_100BASE4)
        ret = LPA_100BASE4;
    else if (negotiated & LPA_100HALF)
        ret = LPA_100HALF;
    else if (negotiated & LPA_10FULL)
        ret = LPA_10FULL;
    else
        ret = LPA_10HALF;

    return ret;
}

int mii_check_gmii_support(mii_if_info *mii)
{
    uint32_t reg;

    reg = mdio_read(mii->dev, mii->phy_id, MII_BMSR);
    if (reg & BMSR_ESTATEN)
    {
        reg = mdio_read(mii->dev, mii->phy_id, MII_ESTATUS);
        if (reg & (ESTATUS_1000_TFULL | ESTATUS_1000_THALF))
            return 1;
    }

    return 0;
}

void mii_ethtool_gset(mii_if_info *mii, ethtool_cmd *ecmd)
{
    eth_adapter *dev = mii->dev;
    uint32_t advert, bmcr, lpa, nego;
    uint32_t advert2 = 0, bmcr2 = 0, lpa2 = 0;

    ecmd->supported =
        (SUPPORTED_10baseT_Half | SUPPORTED_10baseT_Full |
         SUPPORTED_100baseT_Half | SUPPORTED_100baseT_Full |
         SUPPORTED_Autoneg | SUPPORTED_TP | SUPPORTED_MII);
    if (mii->support_gmii)
        ecmd->supported |= SUPPORTED_1000baseT_Half |
                           SUPPORTED_1000baseT_Full;

    /* only supports twisted-pair */
    ecmd->port = PORT_MII;

    /* only supports internal transceiver */
    ecmd->transceiver = XCVR_INTERNAL;

    /* this isn't fully supported at higher layers */
    ecmd->phy_address = mii->phy_id;

    ecmd->advertising = ADVERTISED_TP | ADVERTISED_MII;
    advert = mdio_read(dev, mii->phy_id, MII_ADVERTISE);
    if (mii->support_gmii)
        advert2 = mdio_read(dev, mii->phy_id, MII_CTRL1000);

    if (advert & ADVERTISE_10HALF)
        ecmd->advertising |= ADVERTISED_10baseT_Half;
    if (advert & ADVERTISE_10FULL)
        ecmd->advertising |= ADVERTISED_10baseT_Full;
    if (advert & ADVERTISE_100HALF)
        ecmd->advertising |= ADVERTISED_100baseT_Half;
    if (advert & ADVERTISE_100FULL)
        ecmd->advertising |= ADVERTISED_100baseT_Full;
    if (advert2 & ADVERTISE_1000HALF)
        ecmd->advertising |= ADVERTISED_1000baseT_Half;
    if (advert2 & ADVERTISE_1000FULL)
        ecmd->advertising |= ADVERTISED_1000baseT_Full;

    bmcr = mdio_read(dev, mii->phy_id, MII_BMCR);
    lpa = mdio_read(dev, mii->phy_id, MII_LPA);

    if (mii->support_gmii)
    {
        bmcr2 = mdio_read(dev, mii->phy_id, MII_CTRL1000);
        lpa2 = mdio_read(dev, mii->phy_id, MII_STAT1000);
    }

    if (bmcr & BMCR_ANENABLE)
    {
        ecmd->advertising |= ADVERTISED_Autoneg;
        ecmd->autoneg = AUTONEG_ENABLE;

        nego = mii_nway_result(advert & lpa);
        if ((bmcr2 & (ADVERTISE_1000HALF | ADVERTISE_1000FULL)) &
                (lpa2 >> 2))
            ecmd->speed = SPEED_1000;
        else if (nego == LPA_100FULL || nego == LPA_100HALF)
            ecmd->speed = SPEED_100;
        else
            ecmd->speed = SPEED_10;
        if ((lpa2 & LPA_1000FULL) || nego == LPA_100FULL ||
                nego == LPA_10FULL)
        {
            ecmd->duplex = DUPLEX_FULL;
            mii->full_duplex = 1;
        }
        else
        {
            ecmd->duplex = DUPLEX_HALF;
            mii->full_duplex = 0;
        }
    }
    else
    {
        ecmd->autoneg = AUTONEG_DISABLE;

        ecmd->speed = ((bmcr & BMCR_SPEED1000 &&
                       (bmcr & BMCR_SPEED100) == 0) ? SPEED_1000 :
                       (bmcr & BMCR_SPEED100) ? SPEED_100 : SPEED_10);
        ecmd->duplex = (bmcr & BMCR_FULLDPLX) ? DUPLEX_FULL : DUPLEX_HALF;
    }
}

int mii_link_ok(mii_if_info *mii)
{
    // dummy read
    mdio_read(mii->dev, mii->phy_id, MII_BMSR);
    if (mdio_read(mii->dev, mii->phy_id, MII_BMSR) & BMSR_LSTATUS)
        return 1;
    return 0;
}

// scan phy
void eth_phy_scan(net_device *gmacdev, uint64_t phyBase)
{
    int i, j;
    uint16_t data;
    for (i = phyBase, j = 0; j < 32; i = (i + 1) & 0x1f, j++)
    {
        eth_gmac_mdio_read(gmacdev->MacBase, i, 2, &data);
        if (data != 0 && data != 0xffff) break;

        eth_gmac_mdio_read(gmacdev->MacBase, i, 3, &data);
        if (data != 0 && data != 0xffff) break;
    }

    if (j == 32) {
        plat_printf("can't find phy\n");
    }

    gmacdev->PhyBase = i;
}

// set mac address
void eth_set_mac_addr(net_device *gmacdev, uint8_t *addr)
{
    uint32_t data;
    data = (addr[5] << 8) | addr[4];
    eth_gmac_write_reg(gmacdev->MacBase, GmacAddr0High, data);
    data = (addr[3] << 24) | (addr[2] << 16) | (addr[1] << 8) | addr[0];
    eth_gmac_write_reg(gmacdev->MacBase, GmacAddr0Low, data);
}

// get mac address
void eth_get_mac_addr(net_device *gmacdev, uint8_t *addr)
{
    uint32_t data;
    data = eth_gmac_read_reg(gmacdev->MacBase, GmacAddr0High);
    addr[5] = (data >> 8) & 0xff;
    addr[4] = (data)      & 0xff;
    data = eth_gmac_read_reg(gmacdev->MacBase, GmacAddr0Low);
    addr[3] = (data >> 24) & 0xff;
    addr[2] = (data >> 16) & 0xff;
    addr[1] = (data >> 8)  & 0xff;
    addr[0] = (data)       & 0xff;
}

// set mdc clock
void eth_set_mdc_clk(net_device *gmacdev, uint32_t clk_csr)
{
    uint32_t orig_data;
    orig_data = eth_gmac_read_reg(gmacdev->MacBase, GmacGmiiAddr);
    orig_data &= (~GmiiCsrClkMask);
    orig_data |= clk_csr;
    eth_gmac_write_reg(gmacdev->MacBase, GmacGmiiAddr, orig_data);
}

void eth_gmac_reset(net_device *gmacdev)
{
    uint32_t data = 0;
    eth_gmac_write_reg(gmacdev->DmaBase, DmaBusMode, DmaResetOn);
    do
    {
        mdelay(1);
        data = eth_gmac_read_reg(gmacdev->DmaBase, DmaBusMode);
    }
    while (data & 1);
}

uint32_t eth_check_phy_init(eth_adapter *adapter)
{
    ethtool_cmd cmd;
    net_device *gmacdev = adapter->dev;

    if (!mii_link_ok(&adapter->mii))
    {
        gmacdev->DuplexMode = FULLDUPLEX;
        gmacdev->Speed = SPEED100;
        return 0;
    }
    else
    {
        mii_ethtool_gset(&adapter->mii, &cmd);
        gmacdev->DuplexMode = (cmd.duplex == DUPLEX_FULL) ? FULLDUPLEX : HALFDUPLEX;
        if (cmd.speed == SPEED_1000)
            gmacdev->Speed = SPEED1000;
        else if (cmd.speed == SPEED_100)
            gmacdev->Speed = SPEED100;
        else
            gmacdev->Speed = SPEED10;
    }

    return gmacdev->Speed | (gmacdev->DuplexMode << 4);
}

void eth_gmac_link_monitor(void *adaptr)
{
    uint32_t data;
    eth_adapter *adapter = adaptr;
    net_device *gmacdev = adapter->dev;

    if (!mii_link_ok(&adapter->mii))
    {
        if (gmacdev->LinkState)
            plat_printf("No Link\n");
        gmacdev->DuplexMode = 0;
        gmacdev->Speed = 0;
        gmacdev->LinkState = 0;
    }
    else
    {
        data = eth_check_phy_init(adapter);

        if (gmacdev->LinkState != data)
        {
            gmacdev->LinkState = data;
            eth_gmac_init(gmacdev);
            plat_printf("Link is up in %s mode\n", (gmacdev->DuplexMode == FULLDUPLEX) ? "FULL DUPLEX" : "HALF DUPLEX");
            if (gmacdev->Speed == SPEED1000)
                plat_printf("Link is with 1000M Speed \n");
            if (gmacdev->Speed == SPEED100)
                plat_printf("Link is with 100M Speed \n");
            if (gmacdev->Speed == SPEED10)
                plat_printf("Link is with 10M Speed \n");
        }
    }
}

// init phy
// no need for now
int eth_phy_init(net_device *gmacdev)
{
    uint32_t phy_id = 0;
    uint16_t data;

    // read phy id
    eth_gmac_mdio_read(gmacdev->MacBase, gmacdev->PhyBase, 2, &data);
    phy_id |= (data << 16);
    eth_gmac_mdio_read(gmacdev->MacBase, gmacdev->PhyBase, 3, &data);
    phy_id |= data;

    switch (phy_id) {
    case 0x001cc915:
        plat_printf("probed phy RTL8211E, id 0x%08x\n", phy_id);
        break;

    case 0x0000010a:
        plat_printf("probed phy YT8511, id 0x%08x\n", phy_id);
        break;

    default:
        plat_printf("unknown ethernet phy id 0x%08x\n", phy_id);
        break;
    }

    return 0;
}

// set up tx descriptor queue in ring mode
int eth_gmac_setup_tx_desc_queue(net_device *gmacdev, uint32_t no_of_desc)
{
    DmaDesc *first_desc = NULL;
    uint64_t dma_addr;
    gmacdev->TxDescCount = 0;

    first_desc = plat_malloc_dmaable(sizeof(DmaDesc) * (no_of_desc), &dma_addr);
    plat_assert(first_desc);

    gmacdev->TxDescCount = no_of_desc;
    gmacdev->TxDesc      = first_desc;
    gmacdev->TxDescDma   = dma_addr;

    for (int i = 0; i < gmacdev->TxDescCount; i++)
    {
        eth_gmac_tx_desc_init_ring(gmacdev->TxDesc + i, i == gmacdev->TxDescCount - 1);
    }

    gmacdev->TxNext = 0;
    gmacdev->TxBusy = 0;
    gmacdev->TxNextDesc = gmacdev->TxDesc;
    gmacdev->TxBusyDesc = gmacdev->TxDesc;
    gmacdev->BusyTxDesc  = 0;

    return 0;
}

// set up rx descriptor queue in ring mode
int eth_gmac_setup_rx_desc_queue(net_device *gmacdev, uint32_t no_of_desc)
{
    DmaDesc *first_desc = NULL;
    uint64_t dma_addr;

    first_desc = plat_malloc_dmaable(sizeof(DmaDesc) * no_of_desc, &dma_addr);
    plat_assert(first_desc);

    gmacdev->RxDescCount = no_of_desc;
    gmacdev->RxDesc      = first_desc;
    gmacdev->RxDescDma   = dma_addr;

    for (int i = 0; i < gmacdev->RxDescCount; i++)
    {
        eth_gmac_rx_desc_init_ring(gmacdev->RxDesc + i, i == gmacdev->RxDescCount - 1);
    }

    gmacdev->RxNext = 0;
    gmacdev->RxBusy = 0;
    gmacdev->RxNextDesc = gmacdev->RxDesc;
    gmacdev->RxBusyDesc = gmacdev->RxDesc;
    gmacdev->BusyRxDesc = 0;

    return 0;
}

// transmit
int eth_gmac_tx(rt_device_t device, struct pbuf *p)
{
    ls_eth_device *dev = device;
    eth_adapter *adapter = dev->priv;
    net_device *gmacdev = adapter->dev;
    plat_assert(adapter);
    plat_assert(gmacdev);

    uint64_t pbuf;
    uint64_t dma_addr;
    uint32_t index;
    DmaDesc *dpr;

    pbuf = (uint64_t)plat_malloc(p->tot_len);
    plat_assert(pbuf);

    pbuf_copy_partial(p, (void *)pbuf, p->tot_len, 0);
    dma_addr = plat_dma_map_single(gmacdev, (void *)pbuf, p->tot_len);

    int ret = eth_gmac_set_tx_qptr(gmacdev, dma_addr, p->tot_len, pbuf, 0, 0, 0, &index, dpr);
    if (ret < 0)
    {
        plat_free(pbuf);
        return -1;
    }
    
    eth_gmac_resume_dma_tx(gmacdev);

    int desc_index;
    uint64_t data1, data2;
    uint32_t dma_addr1, dma_addr2;
    uint32_t length1, length2;
    do
    {
        uint32_t status;
        desc_index = eth_gmac_get_tx_qptr(gmacdev, &status, &dma_addr1, &length1, &data1, &dma_addr2, &length2, &data2);
        if (desc_index >= 0 && data1 != 0)
        {
            plat_free((void *)(data1));

            if (eth_gmac_is_desc_valid(status))
            {
                adapter->stats.tx_bytes += length1;
                adapter->stats.tx_packets ++;
            }
            else
            {
                adapter->stats.tx_errors ++;
            }
        }
    }
    while (desc_index >= 0);

    return 0;
}

// receive
struct pbuf *eth_gmac_rx(rt_device_t device)
{
    ls_eth_device *dev = device;
    eth_adapter *adapter = dev->priv;
    net_device *gmacdev = adapter->dev;
    plat_assert(adapter);
    plat_assert(gmacdev);

    uint64_t data1, data2;
    uint32_t dma_addr1, dma_addr2;
    uint32_t len;
    uint32_t status;

    struct pbuf *pbuf = NULL;

    // handle the receive descriptors
    int desc_index = eth_gmac_get_rx_qptr(gmacdev, &status, &dma_addr1, NULL, &data1, &dma_addr2, NULL, &data2);

    if (((uint32_t)desc_index >= RECEIVE_DESC_SIZE) && (desc_index != -1))
    {
        plat_printf("desc_index error\n");
        return -1;
    }

    if (desc_index >= 0 && data1 != 0)
    {
        if (eth_gmac_is_rx_desc_valid(status))
        {
            len = eth_gmac_get_rx_desc_frame_length(status) - 4; // dont care crc
            adapter->stats.rx_bytes += len;
            adapter->stats.rx_packets ++;

            // lwip require rx to return struct pbuf
            dma_addr1 =  plat_dma_map_single(gmacdev, (void *)data1, RX_BUF_SIZE);
            pbuf = pbuf_alloc(PBUF_LINK, len, PBUF_RAM);
            plat_assert(pbuf);
            plat_memcpy(pbuf->payload, data1, len);
        }
        else
        {
            adapter->stats.rx_errors ++;
        }

        desc_index = eth_gmac_set_rx_qptr(gmacdev, dma_addr1, RX_BUF_SIZE, data1, 0, 0, 0);
        if (desc_index < 0)
        {
            plat_free((void *)data1);
        }
    }
    return pbuf;
}

// isr
void eth_rx_irq(int irq, void *param)
{
    ls_eth_device *dev = param;
    eth_adapter *adapter = dev->priv;
    net_device *gmacdev = adapter->dev;

    uint32_t interrupt, dma_status;

    dma_status = eth_gmac_read_reg(gmacdev->DmaBase, DmaStatus);
    if (dma_status == 0)
    {
        // nothing to handle
        return;
    }

    // disable interrupt
    eth_gmac_disable_interrupt_all(gmacdev);

    // check gmac interrupt

    // 28 gmac pmt interrupt
    if (dma_status & GmacPmtIntr)
    {
        plat_printf("gmac pmt interrupt\n");
        eth_gmac_rx_enable(gmacdev);
        eth_gmac_tx_enable(gmacdev);
        eth_gmac_enable_dma_rx(gmacdev);
        eth_gmac_enable_dma_tx(gmacdev);
    }

    // 27 gmac mmc interrupt
    if (dma_status & GmacMmcIntr)
    {
        plat_printf("gmac mmc interrupt\n");
    }

    // 26 gmac line interface interrupt
    if (dma_status & GmacLineIntfIntr)
    {
        plat_printf("gmac line interrupt\n");
    }

    // check dma interrupt

    // clear interrupt
    eth_gmac_write_reg(gmacdev->DmaBase, DmaStatus, dma_status);

    // 13 fatal bus error interrupt
    if (dma_status & DmaIntErrorMask)
    {
        plat_printf("gmac dma error interrupt\n");

        // restart all subsys
        eth_gmac_disable_dma_tx(gmacdev);
        eth_gmac_disable_dma_rx(gmacdev);

        eth_gmac_take_desc_ownership_tx(gmacdev);
        eth_gmac_take_desc_ownership_rx(gmacdev);

        eth_gmac_init_tx_rx_desc_queue(gmacdev);

        eth_gmac_reset(gmacdev);

        eth_set_mac_addr(gmacdev, dev->dev_addr);
        eth_gmac_dma_bus_mode_init(gmacdev, DmaFixedBurstEnable | DmaBurstLength8 | DmaDescriptorSkip1);
        eth_gmac_dma_control_init(gmacdev, DmaStoreAndForward);
        eth_gmac_write_reg(gmacdev->DmaBase, DmaRxBaseAddr, (uint32_t)gmacdev->RxDescDma);
        eth_gmac_write_reg(gmacdev->DmaBase, DmaTxBaseAddr, (uint32_t)gmacdev->TxDescDma);
        eth_gmac_init(gmacdev);
        eth_gmac_enable_dma_rx(gmacdev);
        eth_gmac_enable_dma_tx(gmacdev);
    }

    // 6 receive interrupt
    if (dma_status & DmaIntRxNormMask)
    {
        // plat_printf("gmac dma rx normal\n");
        eth_device_ready(param);
    }

    // 7 receive buffer unavailable
    if (dma_status & DmaIntRxAbnMask)
    {
        plat_printf("gmac dma rx abnormal\n");
        eth_gmac_write_reg(gmacdev->DmaBase, DmaStatus, 0x80);
        eth_gmac_resume_dma_rx(gmacdev);
        eth_device_ready(param);
    }

    // 8 receive process stopped
    if (dma_status & DmaIntRxStoppedMask)
    {
        plat_printf("Receiver stopped seeing Rx interrupts\n");
        eth_gmac_enable_dma_rx(gmacdev);
    }

    // 0 transmit interrupt (transmit completed)
    if (dma_status & DmaIntTxNormMask)
    {
        // plat_printf("gmac dma tx normal\n");
    }

    // 5 transmit underflow
    if (dma_status & DmaIntTxAbnMask)
    {
        plat_printf("Abnormal Tx Interrupt Seen\n");
    }

    // 1 transmit process stopped
    if (dma_status & DmaIntTxStoppedMask)
    {
        plat_printf("Transmitter stopped sending the packets\n");
        eth_gmac_disable_dma_tx(gmacdev);
        eth_gmac_take_desc_ownership_tx(gmacdev);
        eth_gmac_enable_dma_tx(gmacdev);
    }

    // enable interrupt
    eth_gmac_enable_interrupt(gmacdev, DmaIntEnable);
}

int eth_init(rt_device_t device)
{
    ls_eth_device *dev = device;
    eth_adapter *adapter = dev->priv;
    net_device *gmacdev = adapter->dev;

    int status = 0;

    // set mac address
    uint8_t mac_addr[6];
    for (int i = 0; i < 6; ++ i)
        mac_addr[i] = dev->dev_addr[i];

    // set gmac reg address
    gmacdev->MacBase = (dev->iobase + MACBASE);
    gmacdev->DmaBase = (dev->iobase + DMABASE);
    gmacdev->PhyBase = DEFAULT_PHY_BASE;

    // scan all phy
    eth_phy_scan(gmacdev, DEFAULT_PHY_BASE);

    // init mac address
    eth_set_mac_addr(gmacdev, mac_addr);

    // init mii
    adapter->mii.dev = adapter;
    adapter->mii.phy_id = adapter->dev->PhyBase;
    adapter->mii.support_gmii = mii_check_gmii_support(&adapter->mii);

    // init all mac subsys
    eth_gmac_reset(gmacdev);

    // read version
    gmacdev->Version = eth_gmac_read_reg(gmacdev->MacBase, GmacVersion);

    // set mdc clock to 150-250MHz
    eth_set_mdc_clk(gmacdev, GmiiCsrClk4);

    eth_phy_init(adapter->dev);
    eth_check_phy_init(adapter);

    // tx desc_queue
    eth_gmac_setup_tx_desc_queue(gmacdev, TRANSMIT_DESC_SIZE);
    eth_gmac_write_reg(gmacdev->DmaBase, DmaTxBaseAddr, (uint32_t)gmacdev->TxDescDma);

    // rx desc_queue
    eth_gmac_setup_rx_desc_queue(gmacdev, RECEIVE_DESC_SIZE);
    eth_gmac_write_reg(gmacdev->DmaBase, DmaRxBaseAddr, (uint32_t)gmacdev->RxDescDma);

    // dma interface
    eth_gmac_dma_bus_mode_init(gmacdev, DmaBurstLength4 | DmaDescriptorSkip1);
    eth_gmac_dma_control_init(gmacdev, DmaStoreAndForward | DmaTxSecondFrame | DmaRxThreshCtrl128);

    // init gmac in fullduplex or halfduplex
    eth_gmac_init(gmacdev);

    // pause frame generation
    eth_gmac_pause_control(gmacdev);

    // init all rx buffer
    uint64_t buffer;
    do
    {
        buffer = (uint64_t)plat_malloc(RX_BUF_SIZE);
        plat_assert(buffer);

        uint64_t dma_addr = plat_dma_map_single(gmacdev, (void *)buffer, RX_BUF_SIZE);
        status = eth_gmac_set_rx_qptr(gmacdev, dma_addr, RX_BUF_SIZE, (uint64_t)buffer, 0, 0, 0);
        if (status < 0)
        {
            plat_printf("set rx qptr failed\n");
            plat_free(buffer);
        }
    }
    while (status >= 0 && (status < (RECEIVE_DESC_SIZE - 1)));

    eth_gmac_clear_interrupt(gmacdev);

    // disable mmc tx interrupt
    eth_gmac_set_bits(gmacdev->MacBase, GmacMmcIntrMaskTx, 0xFFFFFFFF);
    // disable mmc rx interrupt
    eth_gmac_set_bits(gmacdev->MacBase, GmacMmcIntrMaskRx, 0xFFFFFFFF);
    // disable mmc rx ipc interrupt
    eth_gmac_set_bits(gmacdev->MacBase, GmacMmcRxIpcIntrMask, 0xFFFFFFFF);

    // enable interrupt
    eth_gmac_enable_interrupt(gmacdev, DmaIntEnable);
    eth_gmac_enable_dma_rx(gmacdev);
    eth_gmac_enable_dma_tx(gmacdev);

    // wait
    mdelay(1);

    // eth_check_phy_init(adapter);
    // eth_gmac_init(gmacdev);

    // init timer to monitor link
    eth_timer_init(dev);

    // install isr
    eth_isr_install();

    return 0;
}
