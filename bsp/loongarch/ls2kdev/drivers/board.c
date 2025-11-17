#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include <loongarch.h>
#include <ls2k1000la.h>
#include <board.h>
#include <io.h>
#include <drv_pci.h>
#include <drv_timer.h>
#include <drv_power.h>

/*
void show_cpuinfo(void *v)
{
    unsigned long n = 0;
    unsigned int version = read_cpucfg(LOONGARCH_CPUCFG0) & 0xff;
    unsigned int fp_version = (read_cpucfg(LOONGARCH_CPUCFG2) & CPUCFG2_FPVERS) >> 3;

    unsigned int config = read_cpucfg(LOONGARCH_CPUCFG1);

    rt_kprintf("system type\t\t: %s\n\n", "Generic Loongson64 System");
    rt_kprintf("processor\t\t: %ld\n", n);
    rt_kprintf("package\t\t\t: %d\n", 0);
    rt_kprintf("core\t\t\t: %d\n", 0);
    rt_kprintf("global_id\t\t: %d\n", 0);
    rt_kprintf("CPU Family\t\t: %s\n", __cpu_family[n]);
    rt_kprintf("Model Name\t\t: %s\n", __cpu_full_name[n]);
    rt_kprintf("CPU Revision\t\t: 0x%02x\n", version);
    rt_kprintf("FPU Revision\t\t: 0x%02x\n", fp_version);
    rt_kprintf("CPU MHz\t\t\t: %llu.%02llu\n",
        cpu_clock_freq / 1000000, (cpu_clock_freq / 10000) % 100);
    rt_kprintf("BogoMIPS\t\t: %llu.%02llu\n",
        (lpj_fine * cpu_clock_freq / const_clock_freq) / (500000/1000),
        ((lpj_fine * cpu_clock_freq / const_clock_freq) / (5000/1000)) % 100);
    rt_kprintf("TLB Entries\t\t: %d\n", cpu_data[n].tlbsize);
    rt_kprintf("Address Sizes\t\t: %d bits physical, %d bits virtual\n",
        cpu_pabits + 1, cpu_vabits + 1);

    rt_kprintf("ISA\t\t\t:");
    switch (config & CPUCFG1_ISA)
    {
    case 0:
        rt_kprintf(" loongarch32r");
        break;
    case 0:
        rt_kprintf(" loongarch32s");
        break;
    case 0:
        rt_kprintf(" loongarch64");
        break;
    default:
        rt_kprintf(" unknown ISA level");
    }
    rt_kprintf("\n");

    rt_kprintf("Features\t\t:");
    if (cpu_has_cpucfg) rt_kprintf(" cpucfg");
    if (cpu_has_lam) rt_kprintf(" lam");
    if (cpu_has_ual) rt_kprintf(" ual");
    if (cpu_has_fpu) rt_kprintf(" fpu");
    if (cpu_has_lsx) rt_kprintf(" lsx");
    if (cpu_has_lasx) rt_kprintf(" lasx");
    if (cpu_has_crc32) rt_kprintf(" crc32");
    if (cpu_has_complex) rt_kprintf(" complex");
    if (cpu_has_crypto) rt_kprintf(" crypto");
    if (cpu_has_ptw) rt_kprintf(" ptw");
    if (cpu_has_lspw) rt_kprintf(" lspw");
    if (cpu_has_lvz) rt_kprintf(" lvz");
    if (cpu_has_lbt_x86) rt_kprintf(" lbt_x86");
    if (cpu_has_lbt_arm) rt_kprintf(" lbt_arm");
    if (cpu_has_lbt_mips) rt_kprintf(" lbt_mips");
    rt_kprintf("\n");

    rt_kprintf("Hardware Watchpoint\t: %s", str_yes_no(cpu_has_watch));
    if (cpu_has_watch) {
        rt_kprintf(", iwatch count: %d, dwatch count: %d",
            cpu_data[n].watch_ireg_count, cpu_data[n].watch_dreg_count);
    }

    rt_kprintf("\n\n");
}
*/

void rt_hw_board_info_dump()
{
#define PRID_COMP_MASK      0xff0000
#define PRID_SERIES_MASK    0x00f000
#define PRID_PRODUCT_MASK   0x000fff

    rt_uint32_t prid = read_cpucfg(LOONGARCH_CPUCFG0);
    rt_kprintf("prid: %u\n", prid);

    rt_uint32_t cpuid = csr_read32(LOONGARCH_IOCSR_CPUNAME);
    rt_kprintf("cpuid: %u\n", cpuid & PRID_PRODUCT_MASK);

    rt_kprintf("const_clock_freq: %u\n", read_const_freq());
}

void rt_hw_board_init(void)
{
    // init heap memory system
    rt_system_heap_init(RT_HW_HEAP_BEGIN, RT_HW_HEAP_END);

    // init hardware interrupt
    rt_hw_interrupt_init();

    // init hardware uart device
    rt_hw_uart_init();
    // set console device
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);

    // init operating system timer
    rt_hw_timer_init();
    // set a tick hook
    rt_tick_sethook(RT_NULL);

    // set a idle hook
    rt_thread_idle_sethook(rt_hw_cpu_idle);
    // rt_thread_idle_sethook(RT_NULL);

    // init components
    rt_components_board_init();

    rt_kprintf("heap: [0x%08x - 0x%08x]\n", (rt_uint64_t)RT_HW_HEAP_BEGIN, (rt_uint64_t)RT_HW_HEAP_END);

    // dump board info
    rt_hw_board_info_dump();
}
