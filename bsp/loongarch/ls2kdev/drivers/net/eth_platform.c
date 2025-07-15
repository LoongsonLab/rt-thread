#include "rtthread.h"
#include <drv_eth.h>
#include <eth_dev.h>
#include <eth_platform.h>

// default mac address
#define DEFAULT_MAC_ADDRESS {0x00, 0x55, 0x7B, 0xB5, 0x7D, 0xF7}
// eth device name
#define ETH_DEV_NAME "e0"

// gmac0 寄存器配置地址
uint64_t gmac_base = 0;

uint64_t gmac0_register_addr_init()
{
    // get gmac0 register address through PCIe header
    struct pci_header *p = (struct pci_header *)(LS_PCIE_GMAC0_ADDR);
    return PHYS_TO_UNCACHED((p->BaseAddressRegister[0]) & 0xfffffff0);
}

// use timer to monitor link state
void eth_timer_init(ls_eth_device *device)
{
    eth_adapter *adapter = device->priv;

    rt_timer_init(&device->link_timer, "link_timer",
                  eth_gmac_link_monitor,
                  (void *)adapter,
                  RT_TICK_PER_SECOND,
                  RT_TIMER_FLAG_PERIODIC);
    rt_timer_start(&device->link_timer);
}

// install eth isr
void eth_isr_install()
{
    rt_hw_interrupt_install(LS2K_GMAC0_SBD_IRQ, eth_rx_irq, (void *)&ls_eth_dev, "e0_isr");
    rt_hw_interrupt_umask(LS2K_GMAC0_SBD_IRQ);
}

// only to get mac address
rt_err_t eth_control(rt_device_t device, int cmd, void *args)
{
    ls_eth_device *ls_dev = device;
    switch (cmd)
    {
    case NIOCTL_GADDR:
        if (args) rt_memcpy(args, ls_dev->dev_addr, 6);
        else return -RT_ERROR;
        break;

    default :
        break;
    }
    return RT_EOK;
}

static struct rt_device_ops eth_ops = {
    .init = eth_init,
    .open = RT_NULL,
    .close = RT_NULL,
    .read = RT_NULL,
    .write = RT_NULL,
    .control = eth_control
};

// 最开始的初始化
// rtthread还会调用eth_init
int rt_hw_eth_init(void)
{
    // 获取gmac地址
    gmac_base = gmac0_register_addr_init();

    // 获取配置mac地址
    uint8_t mac_addr[6] = DEFAULT_MAC_ADDRESS;

    rt_memset(&ls_eth_dev, 0, sizeof(ls_eth_dev));

    // 初始化adapter并分配内存
    eth_adapter *adapter;
    adapter = (eth_adapter *)plat_malloc(sizeof(eth_adapter));
    RT_ASSERT(adapter)
    rt_memset(adapter, 0, sizeof(eth_adapter));

    // 初始化adapter->dev并分配内存
    adapter->dev = (net_device *)plat_malloc(sizeof(net_device));
    RT_ASSERT(adapter->dev)
    rt_memset(adapter->dev, 0, sizeof(net_device));

    // set ls_eth_device
    ls_eth_dev.name = ETH_DEV_NAME;
    ls_eth_dev.iobase = gmac_base;
    ls_eth_dev.priv = adapter;

    // set mac address
    for (int i = 0; i < 6; ++ i)
        ls_eth_dev.dev_addr[i] = mac_addr[i];

    // set tx/rx handler for eth_device
    struct eth_device *eth_dev  = &ls_eth_dev.parent;
    eth_dev->eth_tx             = eth_gmac_tx;
    eth_dev->eth_rx             = eth_gmac_rx;

    // set handler for rt_device
    struct rt_device *rt_dev    = &eth_dev->parent;
    rt_dev->type                = RT_Device_Class_NetIf;
    rt_dev->ops                 = &eth_ops;
    rt_dev->user_data           = RT_NULL;

    // use ls_eth_dev.parent to init eth device
    // init device
    eth_device_init(eth_dev, ETH_DEV_NAME);
    // set link up
    eth_device_linkchange(eth_dev, RT_TRUE);

    return 0;
}
INIT_COMPONENT_EXPORT(rt_hw_eth_init);
