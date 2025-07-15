#ifndef __LS2K_DRV_ETH_H__
#define __LS2K_DRV_ETH_H__

#include <eth_dev.h>
#include <eth_platform.h>

typedef struct ls_eth_device
{
    struct eth_device parent;   // rtthread
    uint8_t dev_addr[6];        // mac address
    char *name;                 // device name
    void *iobase;               // register address
    struct rt_timer link_timer; // monitor link state
    void *priv;                 // struct eth_adapter
} ls_eth_device;

extern ls_eth_device ls_eth_dev;

void eth_gmac_link_monitor(void *adaptr);

int eth_init(rt_device_t device);
void eth_rx_irq(int irq, void *param);

int eth_gmac_tx(rt_device_t device, struct pbuf *p);
struct pbuf *eth_gmac_rx(rt_device_t device);

#endif // __LS2K_DRV_ETH_H__
