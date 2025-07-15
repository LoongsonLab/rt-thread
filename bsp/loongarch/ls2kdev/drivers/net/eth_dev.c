#include <eth_dev.h>
#include <eth_platform.h>

// mdio read
int eth_gmac_mdio_read(uint64_t RegBase, uint32_t PhyBase, uint32_t RegOffset, uint16_t *data)
{
    uint32_t addr;
    addr = ((PhyBase << GmiiDevShift) & GmiiDevMask) | ((RegOffset << GmiiRegShift) & GmiiRegMask);
    addr |= GmiiCsrClk4 | GmiiBusy;
    eth_gmac_write_reg(RegBase, GmacGmiiAddr, addr);

    for (uint32_t loop = 0; loop < DEFAULT_LOOP_VARIABLE; loop ++)
    {
        if (!(eth_gmac_read_reg(RegBase, GmacGmiiAddr) & GmiiBusy))
        {
            *data = (uint16_t)(eth_gmac_read_reg(RegBase, GmacGmiiData) & 0xFFFF);
            return 0;
        }
        mdelay(1);
    }

    // mdio timeout
    return -1;
}

// mdio write
int eth_gmac_mdio_write(uint64_t RegBase, uint32_t PhyBase, uint32_t RegOffset, uint16_t data)
{
    uint32_t addr;

    eth_gmac_write_reg(RegBase, GmacGmiiData, data);

    addr = ((PhyBase << GmiiDevShift) & GmiiDevMask) | ((RegOffset << GmiiRegShift) & GmiiRegMask);
    addr |= GmiiWrite | GmiiCsrClk4 | GmiiBusy;
    eth_gmac_write_reg(RegBase, GmacGmiiAddr, addr);

    uint32_t loop;
    for (loop = 0; loop < DEFAULT_LOOP_VARIABLE; loop ++)
    {
        if (!(eth_gmac_read_reg(RegBase, GmacGmiiAddr) & GmiiBusy))
        {
            break;
        }
        mdelay(1);
    }

    if (loop < DEFAULT_LOOP_VARIABLE)
    {
        return 0;
    }
    else
    {
        TR("Error::: PHY not responding Busy bit didnot get cleared !!!!!!\n");
        return -1;
    }

    // mdio timeout
}

void eth_gmac_dma_bus_mode_init(net_device *gmacdev, uint32_t value)
{
    eth_gmac_write_reg(gmacdev->DmaBase, DmaBusMode, value);
}

int eth_gmac_dma_control_init(net_device *gmacdev, uint32_t value)
{
    eth_gmac_write_reg(gmacdev->DmaBase, DmaControl, value);
}

// set back off limit
void eth_gmac_back_off_limit(net_device *gmacdev, uint32_t value)
{
    uint32_t data;
    data = eth_gmac_read_reg(gmacdev->MacBase, GmacConfig);
    data &= (~GmacBackoffLimit);
    data |= value;
    eth_gmac_write_reg(gmacdev->MacBase, GmacConfig, data);
}

// enable reception
void eth_gmac_rx_enable(net_device *gmacdev)
{
    eth_gmac_set_bits(gmacdev->MacBase, GmacConfig, GmacRx);
}

// enable transmission
void eth_gmac_tx_enable(net_device *gmacdev)
{
    eth_gmac_set_bits(gmacdev->MacBase, GmacConfig, GmacTx);
}

// set pass control forwarding
void eth_gmac_set_pass_control(net_device *gmacdev, uint32_t passcontrol)
{
    uint32_t data;
    data = eth_gmac_read_reg(gmacdev->MacBase, GmacFrameFilter);
    data &= (~GmacPassControl);
    data |= passcontrol;
    eth_gmac_write_reg(gmacdev->MacBase, GmacFrameFilter, data);
}

// pause frame generation
void eth_gmac_pause_control(net_device *gmacdev)
{
    uint32_t omr_reg;
    uint32_t mac_flow_control_reg;
    omr_reg = eth_gmac_read_reg(gmacdev->DmaBase, DmaControl);
    omr_reg |= DmaRxFlowCtrlAct4K | DmaRxFlowCtrlDeact5K | DmaEnHwFlowCtrl;
    eth_gmac_write_reg(gmacdev->DmaBase, DmaControl, omr_reg);

    mac_flow_control_reg = eth_gmac_read_reg(gmacdev->MacBase, GmacFlowControl);
    mac_flow_control_reg |= GmacRxFlowControl | GmacTxFlowControl | 0xFFFF0000;
    eth_gmac_write_reg(gmacdev->MacBase, GmacFlowControl, mac_flow_control_reg);
}

// init gmac register
void eth_gmac_init(net_device *gmacdev)
{
    if (gmacdev->DuplexMode == FULLDUPLEX) // Full Duplex
    {
        // GMAC Configuration
        eth_gmac_clear_bits(gmacdev->MacBase, GmacConfig, GmacWatchdog);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacConfig, GmacJabber);
        eth_gmac_set_bits(gmacdev->MacBase, GmacConfig, GmacFrameBurst);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacConfig, GmacJumboFrame);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacConfig, GmacRxOwn);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacConfig, GmacLoopback);
        eth_gmac_set_bits(gmacdev->MacBase, GmacConfig, GmacDuplex);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacConfig, GmacRetry);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacConfig, GmacPadCrcStrip);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacConfig, GmacDeferralCheck);
        eth_gmac_back_off_limit(gmacdev, GmacBackoffLimit0);

        eth_gmac_set_bits(gmacdev->DmaBase, DmaControl, DmaStoreAndForward);
        eth_gmac_set_bits(gmacdev->DmaBase, DmaControl, DmaFwdErrorFrames);

        // enable tx and rx
        eth_gmac_tx_enable(gmacdev);
        eth_gmac_rx_enable(gmacdev);

        // set gmac speed
        if (gmacdev->Speed == SPEED1000)
        {
            eth_gmac_clear_bits(gmacdev->MacBase, GmacConfig, GmacMiiGmii);
        }
        else
        {
            eth_gmac_set_bits(gmacdev->MacBase, GmacConfig, GmacMiiGmii);
            if (gmacdev->Speed == SPEED100)
                eth_gmac_set_bits(gmacdev->MacBase, GmacConfig, GmacFESpeed100);
            else
                eth_gmac_clear_bits(gmacdev->MacBase, GmacConfig, GmacFESpeed100);

        }

        // Frame Filter Configuration
        eth_gmac_set_bits(gmacdev->MacBase, GmacFrameFilter, GmacFilter);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacFrameFilter, GmacBroadcast);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacFrameFilter, GmacSrcAddrFilter);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacFrameFilter, GmacMulticastFilter);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacFrameFilter, GmacDestAddrFilterInv);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacFrameFilter, GmacMcastHashFilter);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacFrameFilter, GmacPromiscuousMode);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacFrameFilter, GmacUcastHashFilter);
        eth_gmac_set_pass_control(gmacdev, GmacPassControl0);

        // Flow Control Configuration
        eth_gmac_clear_bits(gmacdev->MacBase, GmacFlowControl, GmacUnicastPauseFrame);
        eth_gmac_set_bits(gmacdev->MacBase, GmacFlowControl, GmacRxFlowControl);
        eth_gmac_set_bits(gmacdev->MacBase, GmacFlowControl, GmacTxFlowControl);
    }
    else // Half Duplex
    {
        // GMAC Configuration
        eth_gmac_clear_bits(gmacdev->MacBase, GmacConfig, GmacWatchdog);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacConfig, GmacJabber);
        eth_gmac_set_bits(gmacdev->MacBase, GmacConfig, GmacFrameBurst);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacConfig, GmacJumboFrame);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacConfig, GmacRxOwn);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacConfig, GmacLoopback);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacConfig, GmacDuplex);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacConfig, GmacRetry);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacConfig, GmacPadCrcStrip);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacConfig, GmacDeferralCheck);
        eth_gmac_back_off_limit(gmacdev, GmacBackoffLimit0);

        eth_gmac_set_bits(gmacdev->DmaBase, DmaControl, DmaStoreAndForward);
        eth_gmac_set_bits(gmacdev->DmaBase, DmaControl, DmaFwdErrorFrames);

        // enable tx and rx
        eth_gmac_tx_enable(gmacdev);
        eth_gmac_rx_enable(gmacdev);

        // set gmac speed
        if (gmacdev->Speed == SPEED1000)
        {
            eth_gmac_clear_bits(gmacdev->MacBase, GmacConfig, GmacMiiGmii);
        }
        else
        {
            eth_gmac_set_bits(gmacdev->MacBase, GmacConfig, GmacMiiGmii);
            if (gmacdev->Speed == SPEED100)
                eth_gmac_set_bits(gmacdev->MacBase, GmacConfig, GmacFESpeed100);
            else
                eth_gmac_clear_bits(gmacdev->MacBase, GmacConfig, GmacFESpeed100);
        }

        // Frame Filter Configuration
        eth_gmac_clear_bits(gmacdev->MacBase, GmacFrameFilter, GmacFilter);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacFrameFilter, GmacBroadcast);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacFrameFilter, GmacSrcAddrFilter);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacFrameFilter, GmacMulticastFilter);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacFrameFilter, GmacDestAddrFilterInv);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacFrameFilter, GmacMcastHashFilter);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacFrameFilter, GmacPromiscuousMode);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacFrameFilter, GmacUcastHashFilter);
        eth_gmac_set_pass_control(gmacdev, GmacPassControl0);

        // Flow Control Configuration
        eth_gmac_clear_bits(gmacdev->MacBase, GmacFlowControl, GmacUnicastPauseFrame);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacFlowControl, GmacRxFlowControl);
        eth_gmac_clear_bits(gmacdev->MacBase, GmacFlowControl, GmacTxFlowControl);
    }

}

void eth_gmac_rx_desc_init_ring(DmaDesc *desc, bool last_ring_desc)
{
    desc->length = last_ring_desc ? RxDescEndOfRing : 0;
    desc->status = 0;
    desc->buffer1 = 0;
    desc->buffer2 = 0;
    desc->data1 = 0;
    desc->data2 = 0;
}

void eth_gmac_tx_desc_init_ring(DmaDesc *desc, bool last_ring_desc)
{
    desc->status = last_ring_desc ? TxDescEndOfRing : 0;
    desc->length = 0;
    desc->buffer1 = 0;
    desc->buffer2 = 0;
    desc->data1 = 0;
    desc->data2 = 0;
}

void eth_gmac_init_tx_rx_desc_queue(net_device *gmacdev)
{
    int i;

    for (i = 0; i < gmacdev->TxDescCount; i++)
    {
        eth_gmac_tx_desc_init_ring(gmacdev->TxDesc + i, i == gmacdev->TxDescCount - 1);
    }

    for (i = 0; i < gmacdev->RxDescCount; i++)
    {
        eth_gmac_rx_desc_init_ring(gmacdev->RxDesc + i, i == gmacdev->RxDescCount - 1);
    }

    gmacdev->TxNext = 0;
    gmacdev->TxBusy = 0;
    gmacdev->RxNext = 0;
    gmacdev->RxBusy = 0;
}

bool eth_gmac_is_desc_owned_by_dma(DmaDesc *desc)
{
    return ((desc->status & DescOwnByDma) == DescOwnByDma);
}

uint32_t eth_gmac_get_rx_desc_frame_length(uint32_t status)
{
    return ((status & DescFrameLengthMask) >> DescFrameLengthShift);
}

bool eth_gmac_is_desc_valid(uint32_t status)
{
    return ((status & DescError) == 0);
}

bool eth_gmac_is_desc_empty(DmaDesc *desc)
{
    return (((desc->length & DescSize1Mask) == 0) && ((desc->length & DescSize2Mask) == 0));
}

bool eth_gmac_is_rx_desc_valid(uint32_t status)
{
    return ((status & DescError) == 0) && ((status & DescRxFirst) == DescRxFirst) && ((status & DescRxLast) == DescRxLast);
}

bool eth_gmac_is_last_rx_desc(net_device *gmacdev, DmaDesc *desc)
{
    return (((desc->length & RxDescEndOfRing) == RxDescEndOfRing) || ((uint64_t)gmacdev->RxDesc == desc->data2));
}

bool eth_gmac_is_last_tx_desc(net_device *gmacdev, DmaDesc *desc)
{
    return (((desc->status & TxDescEndOfRing) == TxDescEndOfRing) || ((uint64_t)gmacdev->TxDesc == desc->data2));
}

// get tx desc queue ptr
int eth_gmac_get_tx_qptr(net_device *gmacdev, uint32_t *Status, uint32_t *Buffer1, uint32_t *Length1, uint64_t *Data1, uint32_t *Buffer2, uint32_t *Length2, uint64_t *Data2)
{
    uint32_t txover = gmacdev->TxBusy;
    DmaDesc *txdesc = gmacdev->TxBusyDesc;
    int i;

    if (eth_gmac_is_desc_empty(txdesc))
    {
        return -1;
    }

    if (eth_gmac_is_desc_owned_by_dma(txdesc))
    {
        return -1;
    }

    (gmacdev->BusyTxDesc)--;

    if (Status != 0)
        *Status = txdesc->status;
    if (Buffer1 != 0)
        *Buffer1 = txdesc->buffer1;
    if (Length1 != 0)
        *Length1 = (txdesc->length & DescSize1Mask) >> DescSize1Shift;
    if (Data1 != 0)
        *Data1 = txdesc->data1;

    if (Buffer2 != 0)
        *Buffer2 = txdesc->buffer2;
    if (Length2 != 0)
        *Length2 = (txdesc->length & DescSize2Mask) >> DescSize2Shift;
    if (Data1 != 0)
        *Data2 = txdesc->data2;

    gmacdev->TxBusy     = eth_gmac_is_last_tx_desc(gmacdev, txdesc) ? 0 : txover + 1;
    gmacdev->TxBusyDesc = eth_gmac_is_last_tx_desc(gmacdev, txdesc) ? gmacdev->TxDesc : (txdesc + 1);
    eth_gmac_tx_desc_init_ring(txdesc, eth_gmac_is_last_tx_desc(gmacdev, txdesc));

    return txover;
}

// get rx desc queue ptr
int eth_gmac_get_rx_qptr(net_device *gmacdev, uint32_t *Status, uint32_t *Buffer1, uint32_t *Length1, uint64_t *Data1, uint32_t *Buffer2, uint32_t *Length2, uint64_t *Data2)
{
    uint32_t rxnext = gmacdev->RxBusy;
    DmaDesc *rxdesc = gmacdev->RxBusyDesc;

    if (eth_gmac_is_desc_owned_by_dma(rxdesc))
    {
        return -1;
    }

    if (eth_gmac_is_desc_empty(rxdesc))
    {
        return -1;
    }

    if (Status != 0)
        *Status = rxdesc->status;

    if (Length1 != 0)
        *Length1 = (rxdesc->length & DescSize1Mask) >> DescSize1Shift;
    if (Buffer1 != 0)
        *Buffer1 = rxdesc->buffer1;
    if (Data1 != 0)
        *Data1 = rxdesc->data1;
    if (Length2 != 0)
        *Length2 = (rxdesc->length & DescSize2Mask) >> DescSize2Shift;
    if (Buffer2 != 0)
        *Buffer2 = rxdesc->buffer2;
    if (Data1 != 0)
        *Data2 = rxdesc->data2;

    uint64_t len =  eth_gmac_get_rx_desc_frame_length(*Status);
    gmacdev->RxBusy = eth_gmac_is_last_rx_desc(gmacdev, rxdesc) ? 0 : rxnext + 1;
    gmacdev->RxBusyDesc = eth_gmac_is_last_rx_desc(gmacdev, rxdesc) ? gmacdev->RxDesc : (rxdesc + 1);
    eth_gmac_rx_desc_init_ring(rxdesc, eth_gmac_is_last_rx_desc(gmacdev, rxdesc));

    (gmacdev->BusyRxDesc)--;
    return (rxnext);
}

// set tx desc queue
int eth_gmac_set_tx_qptr(net_device *gmacdev, uint32_t Buffer1, uint32_t Length1, uint64_t Data1, uint32_t Buffer2, uint32_t Length2, uint64_t Data2, uint32_t *index, DmaDesc *Dpr)
{
    uint32_t txnext = gmacdev->TxNext;
    DmaDesc *txdesc = gmacdev->TxNextDesc;

    *index = txnext;
    Dpr = txdesc;

    if (!eth_gmac_is_desc_empty(txdesc))
    {
        return -1;
    }

    (gmacdev->BusyTxDesc)++;

    txdesc->length |= (((Length1 << DescSize1Shift) & DescSize1Mask) | ((Length2 << DescSize2Shift) & DescSize2Mask));
    txdesc->status |= (DescTxFirst | DescTxLast | DescTxIntEnable);

    txdesc->buffer1 = Buffer1;
    txdesc->data1 = Data1;

    txdesc->buffer2 = Buffer2;
    txdesc->data2 = Data2;

    txdesc->status |= DescOwnByDma;

    gmacdev->TxNext = eth_gmac_is_last_tx_desc(gmacdev, txdesc) ? 0 : txnext + 1;
    gmacdev->TxNextDesc = eth_gmac_is_last_tx_desc(gmacdev, txdesc) ? gmacdev->TxDesc : (txdesc + 1);

    return txnext;
}

// set rx desc queue
int eth_gmac_set_rx_qptr(net_device *gmacdev, uint32_t Buffer1, uint32_t Length1, uint64_t Data1, uint32_t Buffer2, uint32_t Length2, uint64_t Data2)
{
    uint32_t rxnext = gmacdev->RxNext;
    DmaDesc *rxdesc = gmacdev->RxNextDesc;

    if (!eth_gmac_is_desc_empty(rxdesc))
    {
        return -1;
    }

    rxdesc->length |= (((Length1 << DescSize1Shift) & DescSize1Mask) | ((Length2 << DescSize2Shift) & DescSize2Mask));
    rxdesc->buffer1 = Buffer1;
    rxdesc->data1 = Data1;
    rxdesc->buffer2 = Buffer2;
    rxdesc->data2 = Data2;
    if ((rxnext % 1) != 0)
        rxdesc->length |= RxDisIntCompl;
    rxdesc->status = DescOwnByDma;
    gmacdev->RxNext     = eth_gmac_is_last_rx_desc(gmacdev, rxdesc) ? 0 : rxnext + 1;
    gmacdev->RxNextDesc = eth_gmac_is_last_rx_desc(gmacdev, rxdesc) ? gmacdev->RxDesc : (rxdesc + 1);

    (gmacdev->BusyRxDesc)++;
    return rxnext;
}

// clear all interrupts
void eth_gmac_clear_interrupt(net_device *gmacdev)
{
    uint32_t data;
    data = eth_gmac_read_reg(gmacdev->DmaBase, DmaStatus);
    eth_gmac_write_reg(gmacdev->DmaBase, DmaStatus, data);
}

// enable interrupt
void eth_gmac_enable_interrupt(net_device *gmacdev, uint32_t interrupts)
{
    eth_gmac_write_reg(gmacdev->DmaBase, DmaInterrupt, interrupts);
}

// disable all interrupts
void eth_gmac_disable_interrupt_all(net_device *gmacdev)
{
    eth_gmac_write_reg(gmacdev->DmaBase, DmaInterrupt, DmaIntDisable);
}

// disable interrupt
void eth_gmac_disable_interrupt(net_device *gmacdev, uint32_t interrupts)
{
    eth_gmac_clear_bits(gmacdev->DmaBase, DmaInterrupt, interrupts);
}

// enable dma rx
void eth_gmac_enable_dma_rx(net_device *gmacdev)
{
    uint32_t data;
    data = eth_gmac_read_reg(gmacdev->DmaBase, DmaControl);
    data |= DmaRxStart;
    eth_gmac_write_reg(gmacdev->DmaBase, DmaControl, data);
}

// enable dma tx
void eth_gmac_enable_dma_tx(net_device *gmacdev)
{
    uint32_t data;
    data = eth_gmac_read_reg(gmacdev->DmaBase, DmaControl);
    data |= DmaTxStart;
    eth_gmac_write_reg(gmacdev->DmaBase, DmaControl, data);
}

// resume dma tx
void eth_gmac_resume_dma_tx(net_device *gmacdev)
{
    eth_gmac_write_reg(gmacdev->DmaBase, DmaTxPollDemand, 1);

}

// resume dma rx
void eth_gmac_resume_dma_rx(net_device *gmacdev)
{
    eth_gmac_write_reg(gmacdev->DmaBase, DmaRxPollDemand, 0);
}

// take ownership of desc
void eth_gmac_take_desc_ownership(DmaDesc *desc)
{
    if (desc)
    {
        desc->status &= ~DescOwnByDma;
    }
}

// take ownership of all tx desc
void eth_gmac_take_desc_ownership_rx(net_device *gmacdev)
{
    uint32_t i;
    DmaDesc *desc;
    desc = gmacdev->RxDesc;
    for (i = 0; i < gmacdev->RxDescCount; i++)
    {
        eth_gmac_take_desc_ownership(desc + i);
    }
}

// take ownership of all rx desc
void eth_gmac_take_desc_ownership_tx(net_device *gmacdev)
{
    uint32_t i;
    DmaDesc *desc;
    desc = gmacdev->TxDesc;
    for (i = 0; i < gmacdev->TxDescCount; i++)
    {
        eth_gmac_take_desc_ownership(desc + i);
    }
}

// disable dma tx
void eth_gmac_disable_dma_tx(net_device *gmacdev)
{
    uint32_t data;
    data = eth_gmac_read_reg(gmacdev->DmaBase, DmaControl);
    data &= (~DmaTxStart);
    eth_gmac_write_reg(gmacdev->DmaBase, DmaControl, data);
}

// disable dma rx
void eth_gmac_disable_dma_rx(net_device *gmacdev)
{
    uint32_t data;
    data = eth_gmac_read_reg(gmacdev->DmaBase, DmaControl);
    data &= (~DmaRxStart);
    eth_gmac_write_reg(gmacdev->DmaBase, DmaControl, data);
}
