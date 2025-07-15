#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include <loongarch.h>
#include <ls2k1000la.h>
#include <drv_timer.h>

rt_uint64_t const_clock_freq = 0;

rt_uint64_t read_const_freq(void)
{
    rt_uint64_t res;
    rt_uint64_t base_freq;
    rt_uint64_t cfm, cfd;

    // read CPUCFG2
    res = read_cpucfg(LOONGARCH_CPUCFG2);
    // constant timer LLFTP
    if (!(res & CPUCFG2_LLFTP))
        return 0;

    // read frequency of oscillator
    base_freq = read_cpucfg(LOONGARCH_CPUCFG4);
    // low ccmul，high ccdiv
    res = read_cpucfg(LOONGARCH_CPUCFG5);
    cfm = res & 0xffff;
    cfd = (res >> 16) & 0xffff;

    if (!base_freq || !cfm || !cfd)
        return 0;

    return (base_freq * cfm / cfd);
}

void rt_hw_timer_handler(void)
{
    rt_uint64_t timer_config;
    rt_uint64_t int_val;

    if (const_clock_freq == 0)
        const_clock_freq = 100000000;

    int_val = const_clock_freq / RT_TICK_PER_SECOND;
    timer_config = int_val & CSR_TCFG_VAL;
    // turn off periodic
    timer_config &= ~CSR_TCFG_PERIOD;
    // turn on timer
    timer_config |= CSR_TCFG_EN;

    // increase a OS tick
    rt_tick_increase();
    csr_write64(timer_config, LOONGARCH_CSR_TCFG);
}

void rt_hw_timer_init(void)
{
    rt_uint64_t timer_config;
    rt_uint64_t int_val;

    const_clock_freq = read_const_freq();

    int_val = const_clock_freq / RT_TICK_PER_SECOND;
    timer_config = int_val & CSR_TCFG_VAL;
    // turn off periodic
    timer_config &= ~CSR_TCFG_PERIOD;
    // turn on timer
    timer_config |= CSR_TCFG_EN;

    write_csr_tintclear(CSR_TINTCLR_TI);
    csr_write64(timer_config, LOONGARCH_CSR_TCFG);
    set_csr_ecfg(ECFGF_TIMER);
}
