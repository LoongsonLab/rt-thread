#include <rtthread.h>
#include <rthw.h>

#include <loongarch.h>
#include <ls2k1000la.h>
#include <drv_clk.h>
#include <io.h>

// node pll
// refclk -> DIVIDER -> PLL -> NODE DIVIDER -> node_clock (~1GHz) -> FREQ_SCALE

// pix pll (2)
// refclk -> DIVIDER -> PLL -> PIX DIVIDER -> pix_clock (100-250MHz)

// ddr pll
// refclk -> DIVIDER -> PLL -> DDR DIVIDER -> ddr_clock (400-700MHz)
//                             GPU DIVIDER -> gpu_clock (300-500MHz)
//                             HDA DIVIDER -> hda_clock (fixed 24MHz)

// dc pll
// refclk -> DIVIDER -> PLL -> DC DIVIDER -> dc_clock (~200MHz)
//                                        -> GMAC DIVIER -> gmac_clock (125MHz)
//                                                       -> FREQ_SCALE -> abp_clock
//                                                       -> FREQ_SCALE -> sata_clock
//                                                       -> FREQ_SCALE -> usb_clock

// refclk 100MHz
#define REF_FREQ 100

// working improperly in qemu-2k1000

struct ls2_pll
{
    rt_uint64_t PLL_NODE_L;
    rt_uint64_t PLL_NODE_H;
    rt_uint64_t PLL_DDR_L;
    rt_uint64_t PLL_DDR_H;
    rt_uint64_t PLL_DC_L;
    rt_uint64_t PLL_DC_H;
    rt_uint64_t PLL_PIX0_L;
    rt_uint64_t PLL_PIX0_H;
    rt_uint64_t PLL_PIX1_L;
    rt_uint64_t PLL_PIX1_H;
    rt_uint64_t FREQ_SCALE;
};

volatile struct ls2_pll *pll = (void *)LS_NODE_PLL_L;

rt_uint64_t clk_get_cpu_rate(void)
{
    rt_uint64_t node_clock, cpu_clock;
    rt_uint64_t ctrl;

    rt_uint64_t l1_div_ref;
    rt_uint64_t l1_div_loopc;
    rt_uint64_t l2_div_out_node;
    rt_uint64_t freqscale;

    node_clock = REF_FREQ * 1000;

    ctrl = readq(LS_NODE_PLL_L);
    l1_div_ref = (ctrl >> NODE_L1DIV_REF_SHIFT) & NODE_L1DIV_REF_MARK;
    l1_div_loopc = (ctrl >> NODE_L1DIV_LOOPC_SHIFT) & NODE_L1DIV_LOOPC_MARK;

    ctrl = readq(LS_NODE_PLL_H);
    l2_div_out_node = (ctrl >> NODE_L2DIV_OUT_SHIFT) & NODE_L2DIV_OUT_MARK;

    node_clock = (node_clock / l1_div_ref * l1_div_loopc / l2_div_out_node) * 1000;

    ctrl = readq(LS_FREQ_SCALE);
    freqscale = (ctrl >> FREQSCALE_NODE_SHIFT) & FREQSCALE_NODE_MARK;

    cpu_clock = node_clock * (freqscale + 1) / 8;
    return cpu_clock;
}

rt_uint64_t clk_get_ddr_rate(void)
{
    rt_uint64_t ddr_clock;
    rt_uint64_t ctrl;

    rt_uint64_t l1_div_ref;
    rt_uint64_t l1_div_loopc;
    rt_uint64_t l2_div_out_ddr;

    ddr_clock = REF_FREQ * 1000;

    ctrl = readq(LS_DDR_PLL_L);
    l1_div_ref = (ctrl >> DDR_L1DIV_REF_SHIFT) & DDR_L1DIV_REF_MARK;
    l1_div_loopc = (ctrl >> DDR_L1DIV_LOOPC_SHIFT) & DDR_L1DIV_LOOPC_MARK;

    ctrl = readq(LS_DDR_PLL_H);
    l2_div_out_ddr = (ctrl >> DDR_L2DIV_OUT_DDR_SHIFT) & DDR_L2DIV_OUT_DDR_MARK;

    ddr_clock = (ddr_clock / l1_div_ref * l1_div_loopc / l2_div_out_ddr) * 1000;
    return ddr_clock;
}

rt_uint64_t clk_get_apb_rate(void)
{
    rt_uint64_t apb_clock;
    rt_uint64_t ctrl;

    rt_uint64_t l1_div_ref;
    rt_uint64_t l1_div_loopc;
    rt_uint64_t l2_div_out_apb;
    rt_uint64_t freqscale;

    apb_clock = REF_FREQ * 1000;

    ctrl = readq(LS_DC_PLL_L);
    l1_div_ref = (ctrl >> DC_L1DIV_REF_SHIFT) & DC_L1DIV_REF_MARK;
    l1_div_loopc = (ctrl >> DC_L1DIV_LOOPC_SHIFT) & DC_L1DIV_LOOPC_MARK;

    ctrl = readq(LS_DC_PLL_H);
    l2_div_out_apb = (ctrl >> DC_L2DIV_OUT_DC_SHIFT) & DC_L2DIV_OUT_DC_MARK;

    apb_clock = (apb_clock / l1_div_ref * l1_div_loopc / l2_div_out_apb) * 1000;

    ctrl = readq(LS_FREQ_SCALE);
    freqscale = (ctrl >> FREQSCALE_APB_SHIFT) & FREQSCALE_APB_MARK;

    apb_clock = apb_clock * (freqscale + 1) / 8;
    return apb_clock;
}
