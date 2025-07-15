#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>

#include <ls2k1000la.h>
#include <drv_power.h>
#include <io.h>

void rt_hw_cpu_reset()
{
    writel(0x1, LS_RST_CNT_REG);
    rt_kprintf("reboot system...\n");
    while (1);
}
MSH_CMD_EXPORT_ALIAS(rt_hw_cpu_reset, reboot, reboot cpu);

void rt_hw_cpu_shutdown()
{
    writel(0xffffffff, LS_PM1_STS_REG);
    writel(0x1c00, LS_PM1_CNT_REG);
    writel(0x3c00, LS_PM1_CNT_REG);
    rt_kprintf("shutdown system...\n");
    while (1);
}
MSH_CMD_EXPORT_ALIAS(rt_hw_cpu_shutdown, shutdown, shutdown cpu);
