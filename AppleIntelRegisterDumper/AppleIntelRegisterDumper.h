
/*
 * Copyright © 2006,2009 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *
 *    Eric Anholt <eric@anholt.net>
 *    Pike R. Alpha <pikeralpha@yahoo.com> (OS X port)
 *
 */

#include "intel_reg.h"
#include "intel_chipset.h"

#define DEBUGSTRING(func) void func(char *result, int len, UInt32 reg, UInt32 val)

#define DEFINEREG(reg) { reg, #reg, NULL, 0 }
#define DEFINEREG_16BIT(reg) { reg, #reg, i830_16bit_func, 0 }
#define DEFINEREG2(reg, func) { reg, #reg, func, 0 }

#define DEFINE_FUNC_DUMP(func) void func(struct reg_debug *regs, uint32_t count)
#define DEFINE_FUNC_VOID(func) void func(void)

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))
#define intel_dump_regs(regs) dumpRegisters(regs, ARRAY_SIZE(regs))

static uint32_t devid = 0;

UInt64 gMMIOAddress	= 0;

struct reg_debug
{
	UInt32 reg;
	const char *name;
	void (*debug_output) (char *result, int len, UInt32 reg, UInt32 val);
	UInt32 val;
};

//------------------------------------------------------------------------------

DEBUGSTRING(hsw_debug_pipe_ddi_func_ctl)
{
	const char *enable, *port, *mode, *bpc, *vsync, *hsync, *edp_input;
	const char *width;

	enable = (val & (1<<31)) ? "enabled" : "disabled";
	
	switch ((val >> 28) & 7)
	{
		case 0:
			port = "no port";
			break;
		case 1:
			port = "DDIB";
			break;
		case 2:
			port = "DDIC";
			break;
		case 3:
			port = "DDID";
			break;
		case 4:
			port = "DDIE";
			break;
		default:
			port = "port reserved";
			break;
	}

	switch ((val >> 24) & 7)
	{
		case 0:
			mode = "HDMI";
			break;
		case 1:
			mode = "DVI";
			break;
		case 2:
			mode = "DP SST";
			break;
		case 3:
			mode = "DP MST";
			break;
		case 4:
			mode = "FDI";
			break;
		case 5:
		default:
			mode = "mode reserved";
			break;
	}

	switch ((val >> 20) & 7)
	{
		case 0:
			bpc = "8 bpc";
			break;
		case 1:
			bpc = "10 bpc";
			break;
		case 2:
			bpc = "6 bpc";
			break;
		case 3:
			bpc = "12 bpc";
			break;
		default:
			bpc = "bpc reserved";
			break;
	}

	hsync = (val & (1<<16)) ? "+HSync" : "-HSync";
	vsync = (val & (1<<17)) ? "+VSync" : "-VSync";

	switch ((val >> 12) & 7)
	{
		case 0:
			edp_input = "EDP A ON";
			break;
		case 4:
			edp_input = "EDP A ONOFF";
			break;
		case 5:
			edp_input = "EDP B ONOFF";
			break;
		case 6:
			edp_input = "EDP C ONOFF";
			break;
		default:
			edp_input = "EDP input reserved";
			break;
	}

	switch ((val >> 1) & 7)
	{
		case 0:
			width = "x1";
			break;
		case 1:
			width = "x2";
			break;
		case 3:
			width = "x4";
			break;
		default:
			width = "reserved width";
			break;
	}

	snprintf(result, len, "%s, %s, %s, %s, %s, %s, %s, %s", enable, port, mode, bpc, vsync, hsync, edp_input, width);
}

//------------------------------------------------------------------------------

DEBUGSTRING(hsw_debug_ddi_buf_ctl)
{
	const char *enable, *reversal, *width, *detected;

	enable = (val & (1<<31)) ? "enabled" : "disabled";
	reversal = (val & (1<<16)) ? "reversed" : "not reversed";

	switch ((val >> 1) & 7)
	{
		case 0:
			width = "x1";
			break;
		case 1:
			width = "x2";
			break;
		case 3:
			width = "x4";
			break;
		default:
			width = "reserved";
			break;
	}

	detected = (val & 1) ? "detected" : "not detected";

	snprintf(result, len, "%s %s %s %s", enable, reversal, width, detected);
}

//------------------------------------------------------------------------------

DEBUGSTRING(hsw_debug_port_clk_sel)
{
	const char *clock = NULL;

	switch ((val >> 29 ) & 7)
	{
		case 0:
			clock = "LCPLL 2700";
			break;
		case 1:
			clock = "LCPLL 1350";
			break;
		case 2:
			clock = "LCPLL 810";
			break;
		case 3:
			clock = "SPLL";
			break;
		case 4:
			clock = "WRPLL 1";
			break;
		case 5:
			clock = "WRPLL 2";
			break;
		case 6:
			clock = "Reserved";
			break;
		case 7:
			clock = "None";
			break;
	}

	snprintf(result, len, "%s", clock);
}

//------------------------------------------------------------------------------

DEBUGSTRING(hsw_debug_pipe_clk_sel)
{
	const char *clock;

	switch ((val >> 29) & 7)
	{
		case 0:
			clock = "None";
			break;
		case 2:
			clock = "DDIB";
			break;
		case 3:
			clock = "DDIC";
			break;
		case 4:
			clock = "DDID";
			break;
		case 5:
			clock = "DDIE";
			break;
		default:
			clock = "Reserved";
			break;
	}

	snprintf(result, len, "%s", clock);
}

//------------------------------------------------------------------------------

DEBUGSTRING(hsw_debug_sfuse_strap)
{
	const char *display, *crt, *lane_reversal, *portb, *portc, *portd;

	display = (val & (1<<7)) ? "disabled" : "enabled";
	crt = (val & (1<<6)) ? "yes" : "no";
	lane_reversal = (val & (1<<4)) ? "yes" : "no";
	portb = (val & (1<<2)) ? "yes" : "no";
	portc = (val & (1<<1)) ? "yes" : "no";
	portd = (val & (1<<0)) ? "yes" : "no";

	snprintf(result, len, "display %s, crt %s, lane reversal %s, "
			 "port b %s, port c %s, port d %s", display, crt, lane_reversal,
			 portb, portc, portd);
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_yxminus1)
{
	snprintf(result, len, "%d, %d", ((val & 0xffff0000) >> 16) + 1, (val & 0xffff) + 1);
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_dspcntr)
{
	const char *enabled = val & DISPLAY_PLANE_ENABLE ? "enabled" : "disabled";
	char plane = val & DISPPLANE_SEL_PIPE_B ? 'B' : 'A';

	if (HAS_PCH_SPLIT(devid))
	{
		snprintf(result, len, "%s", enabled);
	}
	else
	{
		snprintf(result, len, "%s, pipe %c", enabled, plane);
	}
}

//------------------------------------------------------------------------------

DEBUGSTRING(ironlake_debug_dspstride)
{
	snprintf(result, len, "%d", val >> 6);
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_xyminus1)
{
	snprintf(result, len, "%d, %d", (val & 0xffff) + 1, ((val & 0xffff0000) >> 16) + 1);
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_xy)
{
	snprintf(result, len, "%d, %d", (val & 0xffff), ((val & 0xffff0000) >> 16));
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_pipeconf)
{
	const char *enabled = val & PIPEACONF_ENABLE ? "enabled" : "disabled";
	const char *bit30 = NULL;
	const char *interlace = NULL;

	if (IS_965(devid))
	{
		bit30 = val & I965_PIPECONF_ACTIVE ? "active" : "inactive";
	}
	else
	{
		bit30 = val & PIPEACONF_DOUBLE_WIDE ? "double-wide" : "single-wide";
	}

	if (HAS_PCH_SPLIT(devid))
	{
		const char *bpc = NULL;
		const char *rotation = NULL;

		switch ((val >> 21) & 7)
		{
			case 0:
				interlace = "pf-pd";
				break;
			case 1:
				interlace = "pf-id";
				break;
			case 3:
				interlace = "if-id";
				break;
			case 4:
				interlace = "if-id-dbl";
				break;
			case 5:
				interlace = "pf-id-dbl";
				break;
			default:
				interlace = "rsvd";
				break;
		}

		switch ((val >> 14) & 3)
		{
			case 0:
				rotation = "rotate 0";
				break;
			case 1:
				rotation = "rotate 90";
				break;
			case 2:
				rotation = "rotate 180";
				break;
			case 3:
				rotation = "rotate 270";
				break;
		}

		switch (val & (7 << 5))
		{
			case PIPECONF_8BPP:
				bpc = "8bpc";
				break;
			case PIPECONF_10BPP:
				bpc = "10bpc";
				break;
			case PIPECONF_6BPP:
				bpc = "6bpc";
				break;
			case PIPECONF_12BPP:
				bpc = "12bpc";
				break;
			default:
				bpc = "invalid bpc";
				break;
		}

		snprintf(result, len, "%s, %s, %s, %s, %s", enabled, bit30, interlace, rotation, bpc);
	}
	else if (IS_GEN4(devid))
	{
		switch ((val >> 21) & 7)
		{
			case 0:
			case 1:
			case 2:
			case 3:
				interlace = "progressive";
				break;
			case 4:
				interlace = "interlaced embedded";
				break;
			case 5:
				interlace = "interlaced";
				break;
			case 6:
				interlace = "interlaced sdvo";
				break;
			case 7:
				interlace = "interlaced legacy";
				break;
		}

		snprintf(result, len, "%s, %s, %s", enabled, bit30, interlace);
	}
	else
	{
		snprintf(result, len, "%s, %s", enabled, bit30);
	}
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_hvtotal)
{
	snprintf(result, len, "%d active, %d total", (val & 0xffff) + 1, ((val & 0xffff0000) >> 16) + 1);
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_hvsyncblank)
{
	snprintf(result, len, "%d start, %d end", (val & 0xffff) + 1, ((val & 0xffff0000) >> 16) + 1);
}

//------------------------------------------------------------------------------

DEBUGSTRING(ironlake_debug_m_tu)
{
	snprintf(result, len, "TU %d, val 0x%x %d", (val >> 25) + 1, val & 0xffffff, val & 0xffffff);
}

//------------------------------------------------------------------------------

DEBUGSTRING(ironlake_debug_n)
{
	snprintf(result, len, "val 0x%x %d", val & 0xffffff, val & 0xffffff);
}

//------------------------------------------------------------------------------

DEBUGSTRING(ironlake_debug_panel_fitting)
{
	const char *vadapt = NULL;
	const char *filter_sel = NULL;

	switch (val & (3 << 25))
	{
		case 0:
			vadapt = "least";
			break;
		case (1 << 25):
			vadapt = "moderate";
			break;
		case (2 << 25):
			vadapt = "reserved";
			break;
		case (3 << 25):
			vadapt = "most";
			break;
	}

	switch (val & (3 << 23))
	{
		case 0:
			filter_sel = "programmed";
			break;
		case (1 << 23):
			filter_sel = "hardcoded";
			break;
		case (2 << 23):
			filter_sel = "edge_enhance";
			break;
		case (3 << 23):
			filter_sel = "edge_soften";
			break;
	}

	snprintf(result, len,
			 "%s, auto_scale %s, auto_scale_cal %s, v_filter %s, vadapt %s, mode %s, filter_sel %s,"
			 "chroma pre-filter %s, vert3tap %s, v_inter_invert %s",
			 val & PF_ENABLE ? "enable" : "disable",
			 val & (1 << 30) ? "no" : "yes",
			 val & (1 << 29) ? "yes" : "no",
			 val & (1 << 28) ? "bypass" : "enable",
			 val & (1 << 27) ? "enable" : "disable",
			 vadapt, filter_sel,
			 val & (1 << 22) ? "enable" : "disable",
			 val & (1 << 21) ? "force" : "auto",
			 val & (1 << 20) ? "field 0" : "field 1");
}

//------------------------------------------------------------------------------

DEBUGSTRING(ironlake_debug_panel_fitting_2)
{
	snprintf(result, len, "vscale %f", val / (float) (1<<15));
}

//------------------------------------------------------------------------------

DEBUGSTRING(ironlake_debug_panel_fitting_3)
{
	snprintf(result, len, "vscale initial phase %f", val / (float) (1<<15));
}

//------------------------------------------------------------------------------

DEBUGSTRING(ironlake_debug_panel_fitting_4)
{
	snprintf(result, len, "hscale %f", val / (float) (1<<15));
}

//------------------------------------------------------------------------------

DEBUGSTRING(ironlake_debug_pf_win)
{
	int a = (val >> 16) & 0x1fff;
	int b = val & 0xfff;

	snprintf(result, len, "%d, %d", a, b);
}

//------------------------------------------------------------------------------

DEBUGSTRING(ironlake_debug_transconf)
{
	const char *enable = val & TRANS_ENABLE ? "enable" : "disable";
	const char *state = val & TRANS_STATE_ENABLE ? "active" : "inactive";
	const char *interlace;
	
	switch ((val >> 21) & 7)
	{
		case 0:
			interlace = "progressive";
			break;
		case 2:
			if (IS_GEN5(devid))
			{
				interlace = "interlaced sdvo";
			}
			else
			{
				interlace = "rsvd";
			}
			break;
		case 3:
			interlace = "interlaced";
			break;
		default:
			interlace = "rsvd";
	}

	snprintf(result, len, "%s, %s, %s", enable, state, interlace);
}

//------------------------------------------------------------------------------

DEBUGSTRING(ironlake_debug_fdi_rx_misc)
{
	snprintf(result, len, "FDI Delay %d", val & ((1 << 13) - 1));
}

//------------------------------------------------------------------------------

DEBUGSTRING(ilk_debug_blc_pwm_cpu_ctl2)
{
	int enable, blinking, granularity;
	const char *pipe = NULL;

	enable = (val >> 31) & 1;

	if (IS_GEN5(devid) || IS_GEN6(devid))
	{
		pipe = ((val >> 29) & 1) ? "B" : "A";
	}
	else
	{
		switch ((val >> 29) & 3)
		{
			case 0:
				pipe = "A";
				break;
			case 1:
				pipe = "B";
				break;
			case 2:
				pipe = "C";
				break;
			case 3:
				if (IS_IVYBRIDGE(devid))
				{
					pipe = "reserved";
				}
				else
				{
					pipe = "EDP";
				}
				break;
		}
	}

	if (IS_GEN5(devid) || IS_GEN6(devid) || IS_IVYBRIDGE(devid))
	{
		snprintf(result, len, "enable %d, pipe %s", enable, pipe);
	}
	else
	{
		blinking = (val >> 28) & 1;
		granularity = ((val >> 27) & 1) ? 8 : 128;

		snprintf(result, len, "enable %d, pipe %s, blinking %d, "
				 "granularity %d", enable, pipe, blinking, granularity);
	}
}

//------------------------------------------------------------------------------

DEBUGSTRING(ilk_debug_blc_pwm_cpu_ctl)
{
	int cycle, freq;

	cycle = (val & 0xFFFF);
	
	if (IS_GEN5(devid) || IS_GEN6(devid) || IS_IVYBRIDGE(devid))
	{
		snprintf(result, len, "cycle %d", cycle);
	}
	else
	{
		freq = (val >> 16) & 0xFFFF;

		snprintf(result, len, "cycle %d, freq %d", cycle, freq);
	}
}

//------------------------------------------------------------------------------

DEBUGSTRING(ibx_debug_blc_pwm_ctl1)
{
	int enable, override, inverted_polarity;

	enable = (val >> 31) & 1;
	override = (val >> 30) & 1;
	inverted_polarity = (val >> 29) & 1;

	snprintf(result, len, "enable %d, override %d, inverted polarity %d",
	enable, override, inverted_polarity);
}

//------------------------------------------------------------------------------

DEBUGSTRING(ibx_debug_blc_pwm_ctl2)
{
	int freq, cycle;

	freq = (val >> 16) & 0xFFFF;
	cycle = val & 0xFFFF;

	snprintf(result, len, "freq %d, cycle %d", freq, cycle);
}

//------------------------------------------------------------------------------

DEBUGSTRING(hsw_debug_blc_misc_ctl)
{
	const char *sel;

	sel = (val & 1) ? "PWM1-CPU PWM2-PCH" : "PWM1-PCH PWM2-CPU";

	snprintf(result, len, "%s", sel);
}

//------------------------------------------------------------------------------

DEBUGSTRING(hsw_debug_util_pin_ctl)
{
	int enable, data, inverted_polarity;
	const char *transcoder = NULL;
	const char *mode = NULL;

	enable = (val >> 31) & 1;

	switch ((val >> 29) & 3)
	{
		case 0:
			transcoder = "A";
			break;
		case 1:
			transcoder = "B";
			break;
		case 2:
			transcoder = "C";
			break;
		case 3:
			transcoder = "EDP";
			break;
	}

	switch ((val >> 24) & 0xF)
	{
		case 0:
			mode = "data";
			break;
		case 1:
			mode = "PWM";
			break;
		case 4:
			mode = "Vblank";
			break;
		case 5:
			mode = "Vsync";
			break;
		default:
			mode = "reserved";
			break;
	}

	data = (val >> 23) & 1;
	inverted_polarity = (val >> 22) & 1;

	snprintf(result, len, "enable %d, transcoder %s, mode %s, data %d "
			 "inverted polarity %d", enable, transcoder, mode, data,
			 inverted_polarity);
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_pp_status)
{
	const char *status = val & PP_ON ? "on" : "off";
	const char *ready = val & PP_READY ? "ready" : "not ready";
	const char *seq = "unknown";

	switch (val & PP_SEQUENCE_MASK)
	{
		case PP_SEQUENCE_NONE:
			seq = "idle";
			break;
		case PP_SEQUENCE_ON:
			seq = "on";
			break;
		case PP_SEQUENCE_OFF:
			seq = "off";
			break;
	}

	snprintf(result, len, "%s, %s, sequencing %s", status, ready, seq);
}

//------------------------------------------------------------------------------

DEBUGSTRING(ilk_debug_pp_control)
{
	snprintf(result, len, "blacklight %s, %spower down on reset, panel %s",
			 (val & (1 << 2)) ? "enabled" : "disabled",
			 (val & (1 << 1)) ? "" : "do not ",
			 (val & (1 << 0)) ? "on" : "off");
}

//------------------------------------------------------------------------------

DEBUGSTRING(hsw_debug_sinterrupt)
{
	int portd, portc, portb, crt;
	
	portd = (val >> 23) & 1;
	portc = (val >> 22) & 1;
	portb = (val >> 21) & 1;
	crt = (val >> 19) & 1;
	
	snprintf(result, len, "port d:%d, port c:%d, port b:%d, crt:%d", portd, portc, portb, crt);
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_vgacntrl)
{
	snprintf(result, len, "%s", val & VGA_DISP_DISABLE ? "disabled" : "enabled");
}

//------------------------------------------------------------------------------

DEBUGSTRING(ironlake_debug_rr_hw_ctl)
{
	snprintf(result, len, "low %d, high %d", val & RR_HW_LOW_POWER_FRAMES_MASK, (val & RR_HW_HIGH_POWER_FRAMES_MASK) >> 8);
}

//------------------------------------------------------------------------------

DEBUGSTRING(ironlake_debug_dref_ctl)
{
	const char *cpu_source;
	const char *ssc_source = val & DREF_SSC_SOURCE_ENABLE ? "enable" : "disable";
	const char *nonspread_source =
	val & DREF_NONSPREAD_SOURCE_ENABLE ? "enable" : "disable";
	const char *superspread_source =
	val & DREF_SUPERSPREAD_SOURCE_ENABLE ? "enable" : "disable";
	const char *ssc4_mode =
	val & DREF_SSC4_CENTERSPREAD ? "centerspread" : "downspread";
	const char *ssc1 = val & DREF_SSC1_ENABLE ? "enable" : "disable";
	const char *ssc4 = val & DREF_SSC4_ENABLE ? "enable" : "disable";
	
	switch (val & DREF_CPU_SOURCE_OUTPUT_NONSPREAD)
	{
		case DREF_CPU_SOURCE_OUTPUT_DISABLE:
			cpu_source = "disable";
			break;
		case DREF_CPU_SOURCE_OUTPUT_DOWNSPREAD:
			cpu_source = "downspread";
			break;
		case DREF_CPU_SOURCE_OUTPUT_NONSPREAD:
			cpu_source = "nonspread";
			break;
		default:
			cpu_source = "reserved";
	}
	snprintf(result, len, "cpu source %s, ssc_source %s, nonspread_source %s, "
			 "superspread_source %s, ssc4_mode %s, ssc1 %s, ssc4 %s",
			 cpu_source, ssc_source, nonspread_source,
			 superspread_source, ssc4_mode, ssc1, ssc4);
}

//------------------------------------------------------------------------------

DEBUGSTRING(ironlake_debug_rawclk_freq)
{
	const char *tp1 = NULL, *tp2 = NULL;
	
	switch (val & FDL_TP1_TIMER_MASK)
	{
		case 0:
			tp1 = "0.5us";
			break;
		case (1 << 12):
			tp1 = "1.0us";
			break;
		case (2 << 12):
			tp1 = "2.0us";
			break;
		case (3 << 12):
			tp1 = "4.0us";
			break;
	}

	switch (val & FDL_TP2_TIMER_MASK)
	{
		case 0:
			tp2 = "1.5us";
			break;
		case (1 << 10):
			tp2 = "3.0us";
			break;
		case (2 << 10):
			tp2 = "6.0us";
			break;
		case (3 << 10):
			tp2 = "12.0us";
			break;
	}

	snprintf(result, len, "FDL_TP1 timer %s, FDL_TP2 timer %s, freq %d", tp1, tp2, val & RAWCLK_FREQ_MASK);
}

DEBUGSTRING(snb_debug_dpll_sel)
{
	const char *transa, *transb;
	const char *dplla = NULL, *dpllb = NULL;

	if (HAS_CPT)
	{
		if (val & TRANSA_DPLL_ENABLE)
		{
			transa = "enable";

			if (val & TRANSA_DPLLB_SEL)
			{
				dplla = "B";
			}
			else
			{
				dplla = "A";
			}
		}
		else
		{
			transa = "disable";
		}

		if (val & TRANSB_DPLL_ENABLE)
		{
			transb = "enable";

			if (val & TRANSB_DPLLB_SEL)
			{
				dpllb = "B";
			}
			else
			{
				dpllb = "A";
			}
		}
		else
		{
			transb = "disable";
		}

		snprintf(result, len, "TransA DPLL %s (DPLL %s), TransB DPLL %s (DPLL %s)", transa, dplla, transb, dpllb);
	}
}

//------------------------------------------------------------------------------

DEBUGSTRING(ironlake_debug_pch_dpll)
{
	const char *enable = val & DPLL_VCO_ENABLE ? "enable" : "disable";
	const char *highspeed = val & DPLL_DVO_HIGH_SPEED ? "yes" : "no";
	const char *mode = NULL;
	const char *p2 = NULL;
	int fpa0_p1, fpa1_p1;
	const char *refclk = NULL;
	int sdvo_mul;
		
	if ((val & DPLLB_MODE_LVDS) == DPLLB_MODE_LVDS)
	{
		mode = "LVDS";

		if (val & DPLLB_LVDS_P2_CLOCK_DIV_7)
		{
			p2 = "Div 7";
		}
		else
		{
			p2 = "Div 14";
		}
	}
	else if ((val & DPLLB_MODE_LVDS) == DPLLB_MODE_DAC_SERIAL)
	{
		mode = "Non-LVDS";

		if (val & DPLL_DAC_SERIAL_P2_CLOCK_DIV_5)
		{
			p2 = "Div 5";
		}
		else
		{
			p2 = "Div 10";
		}
	}

	fpa0_p1 = ffs((val & DPLL_FPA01_P1_POST_DIV_MASK) >> 16);
	fpa1_p1 = ffs((val & DPLL_FPA1_P1_POST_DIV_MASK));
		
	switch (val & PLL_REF_INPUT_MASK)
	{
		case PLL_REF_INPUT_DREFCLK:
			refclk = "default 120Mhz";
			break;
		case PLL_REF_INPUT_SUPER_SSC:
			refclk = "SuperSSC 120Mhz";
			break;
		case PLL_REF_INPUT_TVCLKINBC:
			refclk = "SDVO TVClkIn";
			break;
		case PLLB_REF_INPUT_SPREADSPECTRUMIN:
			refclk = "SSC";
			break;
		case PLL_REF_INPUT_DMICLK:
			refclk = "DMI RefCLK";
			break;
	}
		
	sdvo_mul = ((val & PLL_REF_SDVO_HDMI_MULTIPLIER_MASK) >> 9) + 1;
		
	snprintf(result, len, "%s, sdvo high speed %s, mode %s, p2 %s, "
			 "FPA0 P1 %d, FPA1 P1 %d, refclk %s, sdvo/hdmi mul %d",
			 enable, highspeed, mode, p2, fpa0_p1, fpa1_p1, refclk, sdvo_mul);
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_fp)
{
	if (IS_IGD(devid))
	{
		snprintf(result, len, "n = %d, m1 = %d, m2 = %d",
				 ffs((val & FP_N_IGD_DIV_MASK) >> FP_N_DIV_SHIFT) - 1,
				 ((val & FP_M1_DIV_MASK) >> FP_M1_DIV_SHIFT),
				 ((val & FP_M2_IGD_DIV_MASK) >> FP_M2_DIV_SHIFT));
	}

	snprintf(result, len, "n = %d, m1 = %d, m2 = %d",
			 ((val & FP_N_DIV_MASK) >> FP_N_DIV_SHIFT),
			 ((val & FP_M1_DIV_MASK) >> FP_M1_DIV_SHIFT),
			 ((val & FP_M2_DIV_MASK) >> FP_M2_DIV_SHIFT));
}

//------------------------------------------------------------------------------

DEBUGSTRING(ironlake_debug_fdi_tx_ctl)
{
	const char *train = NULL, *voltage = NULL, *pre_emphasis = NULL, *portw =
	NULL;

	switch (val & FDI_LINK_TRAIN_NONE)
	{
		case FDI_LINK_TRAIN_PATTERN_1:
			train = "pattern_1";
			break;
		case FDI_LINK_TRAIN_PATTERN_2:
			train = "pattern_2";
			break;
		case FDI_LINK_TRAIN_PATTERN_IDLE:
			train = "pattern_idle";
			break;
		case FDI_LINK_TRAIN_NONE:
			train = "not train";
			break;
	}

	if (HAS_CPT)
	{
		/* SNB B0 */
		switch (val & (0x3f << 22))
		{
			case FDI_LINK_TRAIN_400MV_0DB_SNB_B:
				voltage = "0.4V";
				pre_emphasis = "0dB";
				break;
			case FDI_LINK_TRAIN_400MV_6DB_SNB_B:
				voltage = "0.4V";
				pre_emphasis = "6dB";
				break;
			case FDI_LINK_TRAIN_600MV_3_5DB_SNB_B:
				voltage = "0.6V";
				pre_emphasis = "3.5dB";
				break;
			case FDI_LINK_TRAIN_800MV_0DB_SNB_B:
				voltage = "0.8V";
				pre_emphasis = "0dB";
				break;
		}
	}
	else
	{
		switch (val & (7 << 25))
		{
			case FDI_LINK_TRAIN_VOLTAGE_0_4V:
				voltage = "0.4V";
				break;
			case FDI_LINK_TRAIN_VOLTAGE_0_6V:
				voltage = "0.6V";
				break;
			case FDI_LINK_TRAIN_VOLTAGE_0_8V:
				voltage = "0.8V";
				break;
			case FDI_LINK_TRAIN_VOLTAGE_1_2V:
				voltage = "1.2V";
				break;
			default:
				voltage = "reserved";
		}
		
		switch (val & (7 << 22))
		{
			case FDI_LINK_TRAIN_PRE_EMPHASIS_NONE:
				pre_emphasis = "none";
				break;
			case FDI_LINK_TRAIN_PRE_EMPHASIS_1_5X:
				pre_emphasis = "1.5x";
				break;
			case FDI_LINK_TRAIN_PRE_EMPHASIS_2X:
				pre_emphasis = "2x";
				break;
			case FDI_LINK_TRAIN_PRE_EMPHASIS_3X:
				pre_emphasis = "3x";
				break;
			default:
				pre_emphasis = "reserved";
		}
		
	}

	switch (val & (7 << 19))
	{
		case FDI_DP_PORT_WIDTH_X1:
			portw = "X1";
			break;
		case FDI_DP_PORT_WIDTH_X2:
			portw = "X2";
			break;
		case FDI_DP_PORT_WIDTH_X3:
			portw = "X3";
			break;
		case FDI_DP_PORT_WIDTH_X4:
			portw = "X4";
			break;
	}

	snprintf(result, len, "%s, train pattern %s, voltage swing %s,"
			 "pre-emphasis %s, port width %s, enhanced framing %s, FDI PLL %s, scrambing %s, master mode %s",
			 val & FDI_TX_ENABLE ? "enable" : "disable",
			 train, voltage, pre_emphasis, portw,
			 val & FDI_TX_ENHANCE_FRAME_ENABLE ? "enable" : "disable",
			 val & FDI_TX_PLL_ENABLE ? "enable" : "disable",
			 val & (1 << 7) ? "disable" : "enable",
			 val & (1 << 0) ? "enable" : "disable");
}

//------------------------------------------------------------------------------

DEBUGSTRING(ironlake_debug_fdi_rx_ctl)
{
	const char *train = NULL, *portw = NULL, *bpc = NULL;
	
	if (HAS_CPT)
	{
		switch (val & FDI_LINK_TRAIN_PATTERN_MASK_CPT)
		{
			case FDI_LINK_TRAIN_PATTERN_1_CPT:
				train = "pattern_1";
				break;
			case FDI_LINK_TRAIN_PATTERN_2_CPT:
				train = "pattern_2";
				break;
			case FDI_LINK_TRAIN_PATTERN_IDLE_CPT:
				train = "pattern_idle";
				break;
			case FDI_LINK_TRAIN_NORMAL_CPT:
				train = "not train";
				break;
		}
	}
	else
	{
		switch (val & FDI_LINK_TRAIN_NONE)
		{
			case FDI_LINK_TRAIN_PATTERN_1:
				train = "pattern_1";
				break;
			case FDI_LINK_TRAIN_PATTERN_2:
				train = "pattern_2";
				break;
			case FDI_LINK_TRAIN_PATTERN_IDLE:
				train = "pattern_idle";
				break;
			case FDI_LINK_TRAIN_NONE:
				train = "not train";
				break;
		}
	}

	switch (val & (7 << 19))
	{
		case FDI_DP_PORT_WIDTH_X1:
			portw = "X1";
			break;
		case FDI_DP_PORT_WIDTH_X2:
			portw = "X2";
			break;
		case FDI_DP_PORT_WIDTH_X3:
			portw = "X3";
			break;
		case FDI_DP_PORT_WIDTH_X4:
			portw = "X4";
			break;
	}
	
	switch (val & (7 << 16))
	{
		case FDI_8BPC:
			bpc = "8bpc";
			break;
		case FDI_10BPC:
			bpc = "10bpc";
			break;
		case FDI_6BPC:
			bpc = "6bpc";
			break;
		case FDI_12BPC:
			bpc = "12bpc";
			break;
	}
	
	snprintf(result, len, "%s, train pattern %s, port width %s, %s,"
			 "link_reverse_strap_overwrite %s, dmi_link_reverse %s, FDI PLL %s,"
			 "FS ecc %s, FE ecc %s, FS err report %s, FE err report %s,"
			 "scrambing %s, enhanced framing %s, %s",
			 val & FDI_RX_ENABLE ? "enable" : "disable",
			 train, portw, bpc,
			 val & FDI_LINK_REVERSE_OVERWRITE ? "yes" : "no",
			 val & FDI_DMI_LINK_REVERSE_MASK ? "yes" : "no",
			 val & FDI_RX_PLL_ENABLE ? "enable" : "disable",
			 val & FDI_FS_ERR_CORRECT_ENABLE ? "enable" : "disable",
			 val & FDI_FE_ERR_CORRECT_ENABLE ? "enable" : "disable",
			 val & FDI_FS_ERR_REPORT_ENABLE ? "enable" : "disable",
			 val & FDI_FE_ERR_REPORT_ENABLE ? "enable" : "disable",
			 val & (1 << 7) ? "disable" : "enable",
			 val & FDI_RX_ENHANCE_FRAME_ENABLE ? "enable" :
			 "disable", val & FDI_SEL_PCDCLK ? "PCDClk" : "RawClk");
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_adpa)
{
	char disp_pipe = (val & ADPA_PIPE_B_SELECT) ? 'B' : 'A';
	const char *enable = (val & ADPA_DAC_ENABLE) ? "enabled" : "disabled";
	char hsync = (val & ADPA_HSYNC_ACTIVE_HIGH) ? '+' : '-';
	char vsync = (val & ADPA_VSYNC_ACTIVE_HIGH) ? '+' : '-';

	if (HAS_CPT)
	{
		disp_pipe = val & (1<<29) ? 'B' : 'A';
	}
	
	if (HAS_PCH_SPLIT(devid))
	{
		snprintf(result, len, "%s, transcoder %c, %chsync, %cvsync", enable, disp_pipe, hsync, vsync);
	}
	else
	{
		snprintf(result, len, "%s, pipe %c, %chsync, %cvsync", enable, disp_pipe, hsync, vsync);
	}
}

//------------------------------------------------------------------------------

DEBUGSTRING(ironlake_debug_hdmi)
{
	int disp_pipe;
	const char *enable, *bpc = NULL, *encoding;
	const char *mode, *audio, *vsync, *hsync, *detect;
	
	if (val & PORT_ENABLE)
	{
		enable = "enabled";
	}
	else
	{
		enable = "disabled";
	}
	
	if (HAS_CPT)
	{
		disp_pipe = (val & (3<<29)) >> 29;
	}
	else
	{
		disp_pipe = (val & TRANSCODER_B) >> 29;
	}

	switch (val & (7 << 26))
	{
		case COLOR_FORMAT_8bpc:
			bpc = "8bpc";
			break;
		case COLOR_FORMAT_12bpc:
			bpc = "12bpc";
			break;
	}

	if ((val & (3 << 10)) == TMDS_ENCODING)
	{
		encoding = "TMDS";
	}
	else
	{
		encoding = "SDVO";
	}

	if (val & (1 << 9))
	{
		mode = "HDMI";
	}
	else
	{
		mode = "DVI";
	}

	if (val & AUDIO_ENABLE)
	{
		audio = "enabled";
	}
	else
	{
		audio = "disabled";
	}

	if (val & VSYNC_ACTIVE_HIGH)
	{
		vsync = "+vsync";
	}
	else
	{
		vsync = "-vsync";
	}

	if (val & HSYNC_ACTIVE_HIGH)
	{
		hsync = "+hsync";
	}
	else
	{
		hsync = "-hsync";
	}
	
	if (val & PORT_DETECTED)
	{
		detect = "detected";
	}
	else
	{
		detect = "non-detected";
	}
	
	snprintf(result, len, "%s pipe %c %s %s %s audio %s %s %s %s", enable, disp_pipe + 'A', bpc, encoding, mode, audio, vsync, hsync, detect);
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_lvds)
{
	char disp_pipe = val & LVDS_PIPEB_SELECT ? 'B' : 'A';
	const char *enable = val & LVDS_PORT_EN ? "enabled" : "disabled";
	int depth;
	const char *channels;
	
	if ((val & LVDS_A3_POWER_MASK) == LVDS_A3_POWER_UP)
	{
		depth = 24;
	}
	else
	{
		depth = 18;
	}

	if ((val & LVDS_B0B3_POWER_MASK) == LVDS_B0B3_POWER_UP)
	{
		channels = "2 channels";
	}
	else
	{
		channels = "1 channel";
	}
	
	if (HAS_CPT)
	{
		disp_pipe = val & (1<<29) ? 'B' : 'A';
	}

	snprintf(result, len, "%s, pipe %c, %d bit, %s", enable, disp_pipe, depth, channels);
}

//------------------------------------------------------------------------------

DEBUGSTRING(snb_debug_trans_dp_ctl)
{
	const char *enable, *port = NULL, *bpc = NULL, *vsync, *hsync;
	
	if (HAS_CPT)
	{
		if (val & TRANS_DP_OUTPUT_ENABLE)
		{
			enable = "enable";
		}
		else
		{
			enable = "disable";
		}
	
		switch (val & TRANS_DP_PORT_SEL_MASK)
		{
			case TRANS_DP_PORT_SEL_B:
				port = "B";
				break;
			case TRANS_DP_PORT_SEL_C:
				port = "C";
				break;
			case TRANS_DP_PORT_SEL_D:
				port = "D";
				break;
			default:
				port = "none";
				break;
		}

		switch (val & (7<<9))
		{
			case TRANS_DP_8BPC:
				bpc = "8bpc";
				break;
			case TRANS_DP_10BPC:
				bpc = "10bpc";
				break;
			case TRANS_DP_6BPC:
				bpc = "6bpc";
				break;
			case TRANS_DP_12BPC:
				bpc = "12bpc";
				break;
		}

		if (val & TRANS_DP_VSYNC_ACTIVE_HIGH)
		{
			vsync = "+vsync";
		}
		else
		{
			vsync = "-vsync";
		}

		if (val & TRANS_DP_HSYNC_ACTIVE_HIGH)
		{
			hsync = "+hsync";
		}
		else
		{
			hsync = "-hsync";
		}

		snprintf(result, len, "%s port %s %s %s %s", enable, port, bpc, vsync, hsync);
	}
}

//------------------------------------------------------------------------------

DEBUGSTRING(ivb_debug_port)
{
	const char *drrs = NULL;

	switch (val & (2 << 30))
	{
		case PORT_DBG_DRRS_HW_STATE_OFF:
			drrs = "off";
			break;
		case PORT_DBG_DRRS_HW_STATE_LOW:
			drrs = "low";
			break;
		/* case PORT_DBG_DRRS_HW_STATE_HIGH:
			drrs = "high";
			break; */
	}

	snprintf(result, len, "HW DRRS %s", drrs);
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_16bit_func)
{
	snprintf(result, len, "0x%04x", (uint16_t) val);
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_dcc)
{
	const char *addressing = NULL;
	
	if (IS_MOBILE(devid))
	{
		if (IS_965(devid))
		{
			if (val & (1 << 1))
			{
				addressing = "dual channel interleaved";
			}
			else
			{
				addressing = "single or dual channel asymmetric";
			}
		}
		else
		{
			switch (val & 3)
			{
				case 0:
					addressing = "single channel";
					break;
				case 1:
					addressing = "dual channel asymmetric";
					break;
				case 2:
					addressing = "dual channel interleaved";
					break;
				case 3:
					addressing = "unknown channel layout";
					break;
			}
		}
	
		snprintf(result, len, "%s, XOR randomization: %sabled, XOR bit: %d",
				 addressing, (val & (1 << 10)) ? "dis" : "en", (val & (1 << 9)) ? 17 : 11);
	}
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_chdecmisc)
{
	const char *enhmodesel = NULL;
	
	switch ((val >> 5) & 3)
	{
		case 1:
			enhmodesel = "XOR bank/rank";
			break;
		case 2:
			enhmodesel = "swap bank";
			break;
		case 3:
			enhmodesel = "XOR bank";
			break;
		case 0:
			enhmodesel = "none";
			break;
	}
	
	snprintf(result, len,
			 "%s, ch2 enh %sabled, ch1 enh %sabled, "
			 "ch0 enh %sabled, "
			 "flex %sabled, ep %spresent", enhmodesel,
			 (val & (1 << 4)) ? "en" : "dis",
			 (val & (1 << 3)) ? "en" : "dis",
			 (val & (1 << 2)) ? "en" : "dis",
			 (val & (1 << 1)) ? "en" : "dis",
			 (val & (1 << 0)) ? "" : "not ");
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_vga_pd)
{
	int vga0_p1, vga0_p2, vga1_p1, vga1_p2;
	
	/* XXX: i9xx version */
	
	if (val & VGA0_PD_P1_DIV_2)
	{
		vga0_p1 = 2;
	}
	else
	{
		vga0_p1 = ((val & VGA0_PD_P1_MASK) >> VGA0_PD_P1_SHIFT) + 2;
	}

	vga0_p2 = (val & VGA0_PD_P2_DIV_4) ? 4 : 2;
	
	if (val & VGA1_PD_P1_DIV_2)
	{
		vga1_p1 = 2;
	}
	else
	{
		vga1_p1 = ((val & VGA1_PD_P1_MASK) >> VGA1_PD_P1_SHIFT) + 2;
	}

	vga1_p2 = (val & VGA1_PD_P2_DIV_4) ? 4 : 2;
	
	snprintf(result, len, "vga0 p1 = %d, p2 = %d, vga1 p1 = %d, p2 = %d", vga0_p1, vga0_p2, vga1_p1, vga1_p2);
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_dpll_test)
{
	const char *dpllandiv = val & DPLLA_TEST_N_BYPASS ? ", DPLLA N bypassed" : "";
	const char *dpllamdiv = val & DPLLA_TEST_M_BYPASS ? ", DPLLA M bypassed" : "";
	const char *dpllainput = val & DPLLA_INPUT_BUFFER_ENABLE ? "" : ", DPLLA input buffer disabled";
	const char *dpllbndiv = val & DPLLB_TEST_N_BYPASS ? ", DPLLB N bypassed" : "";
	const char *dpllbmdiv = val & DPLLB_TEST_M_BYPASS ? ", DPLLB M bypassed" : "";
	const char *dpllbinput = val & DPLLB_INPUT_BUFFER_ENABLE ? "" : ", DPLLB input buffer disabled";
	
	snprintf(result, len, "%s%s%s%s%s%s", dpllandiv, dpllamdiv, dpllainput, dpllbndiv, dpllbmdiv, dpllbinput);
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_dspclk_gate_d)
{
	const char *DPUNIT_B = val & DPUNIT_B_CLOCK_GATE_DISABLE ? " DPUNIT_B" : "";
	const char *VSUNIT = val & VSUNIT_CLOCK_GATE_DISABLE ? " VSUNIT" : "";
	const char *VRHUNIT = val & VRHUNIT_CLOCK_GATE_DISABLE ? " VRHUNIT" : "";
	const char *VRDUNIT = val & VRDUNIT_CLOCK_GATE_DISABLE ? " VRDUNIT" : "";
	const char *AUDUNIT = val & AUDUNIT_CLOCK_GATE_DISABLE ? " AUDUNIT" : "";
	const char *DPUNIT_A = val & DPUNIT_A_CLOCK_GATE_DISABLE ? " DPUNIT_A" : "";
	const char *DPCUNIT = val & DPCUNIT_CLOCK_GATE_DISABLE ? " DPCUNIT" : "";
	const char *TVRUNIT = val & TVRUNIT_CLOCK_GATE_DISABLE ? " TVRUNIT" : "";
	const char *TVCUNIT = val & TVCUNIT_CLOCK_GATE_DISABLE ? " TVCUNIT" : "";
	const char *TVFUNIT = val & TVFUNIT_CLOCK_GATE_DISABLE ? " TVFUNIT" : "";
	const char *TVEUNIT = val & TVEUNIT_CLOCK_GATE_DISABLE ? " TVEUNIT" : "";
	const char *DVSUNIT = val & DVSUNIT_CLOCK_GATE_DISABLE ? " DVSUNIT" : "";
	const char *DSSUNIT = val & DSSUNIT_CLOCK_GATE_DISABLE ? " DSSUNIT" : "";
	const char *DDBUNIT = val & DDBUNIT_CLOCK_GATE_DISABLE ? " DDBUNIT" : "";
	const char *DPRUNIT = val & DPRUNIT_CLOCK_GATE_DISABLE ? " DPRUNIT" : "";
	const char *DPFUNIT = val & DPFUNIT_CLOCK_GATE_DISABLE ? " DPFUNIT" : "";
	const char *DPBMUNIT = val & DPBMUNIT_CLOCK_GATE_DISABLE ? " DPBMUNIT" : "";
	const char *DPLSUNIT = val & DPLSUNIT_CLOCK_GATE_DISABLE ? " DPLSUNIT" : "";
	const char *DPLUNIT = val & DPLUNIT_CLOCK_GATE_DISABLE ? " DPLUNIT" : "";
	const char *DPOUNIT = val & DPOUNIT_CLOCK_GATE_DISABLE ? " DPOUNIT" : "";
	const char *DPBUNIT = val & DPBUNIT_CLOCK_GATE_DISABLE ? " DPBUNIT" : "";
	const char *DCUNIT = val & DCUNIT_CLOCK_GATE_DISABLE ? " DCUNIT" : "";
	const char *DPUNIT = val & DPUNIT_CLOCK_GATE_DISABLE ? " DPUNIT" : "";
	const char *VRUNIT = val & VRUNIT_CLOCK_GATE_DISABLE ? " VRUNIT" : "";
	const char *OVHUNIT = val & OVHUNIT_CLOCK_GATE_DISABLE ? " OVHUNIT" : "";
	const char *DPIOUNIT = val & DPIOUNIT_CLOCK_GATE_DISABLE ? " DPIOUNIT" : "";
	const char *OVFUNIT = val & OVFUNIT_CLOCK_GATE_DISABLE ? " OVFUNIT" : "";
	const char *OVBUNIT = val & OVBUNIT_CLOCK_GATE_DISABLE ? " OVBUNIT" : "";
	const char *OVRUNIT = val & OVRUNIT_CLOCK_GATE_DISABLE ? " OVRUNIT" : "";
	const char *OVCUNIT = val & OVCUNIT_CLOCK_GATE_DISABLE ? " OVCUNIT" : "";
	const char *OVUUNIT = val & OVUUNIT_CLOCK_GATE_DISABLE ? " OVUUNIT" : "";
	const char *OVLUNIT = val & OVLUNIT_CLOCK_GATE_DISABLE ? " OVLUNIT" : "";
	
	snprintf(result, len,
			 "clock gates disabled:%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
			 DPUNIT_B, VSUNIT, VRHUNIT, VRDUNIT, AUDUNIT, DPUNIT_A, DPCUNIT,
			 TVRUNIT, TVCUNIT, TVFUNIT, TVEUNIT, DVSUNIT, DSSUNIT, DDBUNIT,
			 DPRUNIT, DPFUNIT, DPBMUNIT, DPLSUNIT, DPLUNIT, DPOUNIT, DPBUNIT,
			 DCUNIT, DPUNIT, VRUNIT, OVHUNIT, DPIOUNIT, OVFUNIT, OVBUNIT,
			 OVRUNIT, OVCUNIT, OVUUNIT, OVLUNIT);
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_sdvo)
{
	const char *enable = val & SDVO_ENABLE ? "enabled" : "disabled";
	char disp_pipe = val & SDVO_PIPE_B_SELECT ? 'B' : 'A';
	const char *stall = val & SDVO_STALL_SELECT ? "enabled" : "disabled";
	const char *detected = val & SDVO_DETECTED ? "" : "not ";
	const char *gang = val & SDVOC_GANG_MODE ? ", gang mode" : "";
	char sdvoextra[20];
	
	if (IS_915(devid))
	{
		sprintf(sdvoextra, ", SDVO mult %d", (int)((val & SDVO_PORT_MULTIPLY_MASK) >> SDVO_PORT_MULTIPLY_SHIFT) + 1);
	}
	else
	{
		sdvoextra[0] = '\0';
	}
	
	snprintf(result, len, "%s, pipe %c, stall %s, %sdetected%s%s", enable, disp_pipe, stall, detected, sdvoextra, gang);
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_dvo)
{
	const char *enable = val & DVO_ENABLE ? "enabled" : "disabled";
	char disp_pipe = val & DVO_PIPE_B_SELECT ? 'B' : 'A';
	const char *stall;
	char hsync = val & DVO_HSYNC_ACTIVE_HIGH ? '+' : '-';
	char vsync = val & DVO_VSYNC_ACTIVE_HIGH ? '+' : '-';
	
	switch (val & DVO_PIPE_STALL_MASK)
	{
		case DVO_PIPE_STALL_UNUSED:
			stall = "no stall";
			break;
		case DVO_PIPE_STALL:
			stall = "stall";
			break;
		case DVO_PIPE_STALL_TV:
			stall = "TV stall";
			break;
		default:
			stall = "unknown stall";
			break;
	}
	
	snprintf(result, len, "%s, pipe %c, %s, %chsync, %cvsync", enable, disp_pipe, stall, hsync, vsync);
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_pp_control)
{
	snprintf(result, len, "power target: %s", val & POWER_TARGET_ON ? "on" : "off");
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_dspstride)
{
	snprintf(result, len, "%d bytes", val);
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_pipestat)
{
	const char *_FIFO_UNDERRUN = val & FIFO_UNDERRUN ? " FIFO_UNDERRUN" : "";
	const char *_CRC_ERROR_ENABLE =
	val & CRC_ERROR_ENABLE ? " CRC_ERROR_ENABLE" : "";
	const char *_CRC_DONE_ENABLE =
	val & CRC_DONE_ENABLE ? " CRC_DONE_ENABLE" : "";
	const char *_GMBUS_EVENT_ENABLE =
	val & GMBUS_EVENT_ENABLE ? " GMBUS_EVENT_ENABLE" : "";
	const char *_VSYNC_INT_ENABLE =
	val & VSYNC_INT_ENABLE ? " VSYNC_INT_ENABLE" : "";
	const char *_DLINE_COMPARE_ENABLE =
	val & DLINE_COMPARE_ENABLE ? " DLINE_COMPARE_ENABLE" : "";
	const char *_DPST_EVENT_ENABLE =
	val & DPST_EVENT_ENABLE ? " DPST_EVENT_ENABLE" : "";
	const char *_LBLC_EVENT_ENABLE =
	val & LBLC_EVENT_ENABLE ? " LBLC_EVENT_ENABLE" : "";
	const char *_OFIELD_INT_ENABLE =
	val & OFIELD_INT_ENABLE ? " OFIELD_INT_ENABLE" : "";
	const char *_EFIELD_INT_ENABLE =
	val & EFIELD_INT_ENABLE ? " EFIELD_INT_ENABLE" : "";
	const char *_SVBLANK_INT_ENABLE =
	val & SVBLANK_INT_ENABLE ? " SVBLANK_INT_ENABLE" : "";
	const char *_VBLANK_INT_ENABLE =
	val & VBLANK_INT_ENABLE ? " VBLANK_INT_ENABLE" : "";
	const char *_OREG_UPDATE_ENABLE =
	val & OREG_UPDATE_ENABLE ? " OREG_UPDATE_ENABLE" : "";
	const char *_CRC_ERROR_INT_STATUS =
	val & CRC_ERROR_INT_STATUS ? " CRC_ERROR_INT_STATUS" : "";
	const char *_CRC_DONE_INT_STATUS =
	val & CRC_DONE_INT_STATUS ? " CRC_DONE_INT_STATUS" : "";
	const char *_GMBUS_INT_STATUS =
	val & GMBUS_INT_STATUS ? " GMBUS_INT_STATUS" : "";
	const char *_VSYNC_INT_STATUS =
	val & VSYNC_INT_STATUS ? " VSYNC_INT_STATUS" : "";
	const char *_DLINE_COMPARE_STATUS =
	val & DLINE_COMPARE_STATUS ? " DLINE_COMPARE_STATUS" : "";
	const char *_DPST_EVENT_STATUS =
	val & DPST_EVENT_STATUS ? " DPST_EVENT_STATUS" : "";
	const char *_LBLC_EVENT_STATUS =
	val & LBLC_EVENT_STATUS ? " LBLC_EVENT_STATUS" : "";
	const char *_OFIELD_INT_STATUS =
	val & OFIELD_INT_STATUS ? " OFIELD_INT_STATUS" : "";
	const char *_EFIELD_INT_STATUS =
	val & EFIELD_INT_STATUS ? " EFIELD_INT_STATUS" : "";
	const char *_SVBLANK_INT_STATUS =
	val & SVBLANK_INT_STATUS ? " SVBLANK_INT_STATUS" : "";
	const char *_VBLANK_INT_STATUS =
	val & VBLANK_INT_STATUS ? " VBLANK_INT_STATUS" : "";
	const char *_OREG_UPDATE_STATUS =
	val & OREG_UPDATE_STATUS ? " OREG_UPDATE_STATUS" : "";
	snprintf(result, len,
			 "status:%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
			 _FIFO_UNDERRUN,
			 _CRC_ERROR_ENABLE,
			 _CRC_DONE_ENABLE,
			 _GMBUS_EVENT_ENABLE,
			 _VSYNC_INT_ENABLE,
			 _DLINE_COMPARE_ENABLE,
			 _DPST_EVENT_ENABLE,
			 _LBLC_EVENT_ENABLE,
			 _OFIELD_INT_ENABLE,
			 _EFIELD_INT_ENABLE,
			 _SVBLANK_INT_ENABLE,
			 _VBLANK_INT_ENABLE,
			 _OREG_UPDATE_ENABLE,
			 _CRC_ERROR_INT_STATUS,
			 _CRC_DONE_INT_STATUS,
			 _GMBUS_INT_STATUS,
			 _VSYNC_INT_STATUS,
			 _DLINE_COMPARE_STATUS,
			 _DPST_EVENT_STATUS,
			 _LBLC_EVENT_STATUS,
			 _OFIELD_INT_STATUS,
			 _EFIELD_INT_STATUS,
			 _SVBLANK_INT_STATUS,
			 _VBLANK_INT_STATUS,
			 _OREG_UPDATE_STATUS);
}

//------------------------------------------------------------------------------

DEBUGSTRING(i830_debug_dpll)
{
	const char *enabled = val & DPLL_VCO_ENABLE ? "enabled" : "disabled";
	const char *dvomode = val & DPLL_DVO_HIGH_SPEED ? "dvo" : "non-dvo";
	const char *vgamode = val & DPLL_VGA_MODE_DIS ? "" : ", VGA";
	const char *mode = "unknown";
	const char *clock = "unknown";
	const char *fpextra = val & DISPLAY_RATE_SELECT_FPA1 ? ", using FPx1!" : "";
	char sdvoextra[20];
	int p1, p2 = 0;
	
	if (IS_GEN2(devid))
	{
		// char is_lvds = (INREG(LVDS) & LVDS_PORT_EN) && (reg == DPLL_B);
		char is_lvds = (MMIO_READ32(LVDS) & LVDS_PORT_EN) && (reg == DPLL_B);
		
		if (is_lvds)
		{
			mode = "LVDS";
			p1 = ffs((val & DPLL_FPA01_P1_POST_DIV_MASK_I830_LVDS) >> DPLL_FPA01_P1_POST_DIV_SHIFT);

			// if ((INREG(LVDS) & LVDS_CLKB_POWER_MASK) == LVDS_CLKB_POWER_UP)
			if ((MMIO_READ32(LVDS) & LVDS_CLKB_POWER_MASK) == LVDS_CLKB_POWER_UP)
			{
				p2 = 7;
			}
			else
			{
				p2 = 14;
			}
		}
		else
		{
			mode = "DAC/serial";

			if (val & PLL_P1_DIVIDE_BY_TWO)
			{
				p1 = 2;
			}
			else
			{
				/* Map the number in the field to (3, 33) */
				p1 = ((val & DPLL_FPA01_P1_POST_DIV_MASK_I830) >> DPLL_FPA01_P1_POST_DIV_SHIFT) + 2;
			}

			if (val & PLL_P2_DIVIDE_BY_4)
			{
				p2 = 4;
			}
			else
			{
				p2 = 2;
			}
		}
	}
	else
	{
		if (IS_IGD(devid))
		{
			p1 = ffs((val & DPLL_FPA01_P1_POST_DIV_MASK_IGD) >> DPLL_FPA01_P1_POST_DIV_SHIFT_IGD);
		}
		else
		{
			p1 = ffs((val & DPLL_FPA01_P1_POST_DIV_MASK) >> DPLL_FPA01_P1_POST_DIV_SHIFT);
		}
		switch (val & DPLL_MODE_MASK)
		{
			case DPLLB_MODE_DAC_SERIAL:
				mode = "DAC/serial";
				p2 = val & DPLL_DAC_SERIAL_P2_CLOCK_DIV_5 ? 5 : 10;
				break;
			case DPLLB_MODE_LVDS:
				mode = "LVDS";
				p2 = val & DPLLB_LVDS_P2_CLOCK_DIV_7 ? 7 : 14;
				break;
		}
	}
	
	switch (val & PLL_REF_INPUT_MASK)
	{
		case PLL_REF_INPUT_DREFCLK:
			clock = "default";
			break;
		case PLL_REF_INPUT_TVCLKINA:
			clock = "TV A";
			break;
		case PLL_REF_INPUT_TVCLKINBC:
			clock = "TV B/C";
			break;
		case PLLB_REF_INPUT_SPREADSPECTRUMIN:
			if (reg == DPLL_B)
				clock = "spread spectrum";
			break;
	}
	
	if (IS_945(devid))
	{
		sprintf(sdvoextra, ", SDVO mult %d", (int)((val & SDVO_MULTIPLIER_MASK) >> SDVO_MULTIPLIER_SHIFT_HIRES) + 1);
	}
	else
	{
		sdvoextra[0] = '\0';
	}
	
	snprintf(result, len, "%s, %s%s, %s clock, %s mode, p1 = %d, p2 = %d%s%s",  enabled, dvomode, vgamode, clock, mode, p1, p2,  fpextra, sdvoextra);
}

//------------------------------------------------------------------------------

DEBUGSTRING(i810_debug_915_fence)
{
	char format = (val & 1 << 12) ? 'Y' : 'X';
	int pitch = 128 << ((val & 0x70) >> 4);
	unsigned int offset = val & 0x0ff00000;
	int size = (1024 * 1024) << ((val & 0x700) >> 8);
	
	if (IS_965(devid) || (IS_915(devid) && reg >= FENCE_NEW))
	{
		return;
	}
	
	if (format == 'X')
	{
		pitch *= 4;
	}

	if (val & 1)
	{
		snprintf(result, len, "enabled, %c tiled, %4d pitch, 0x%08x - 0x%08x (%dkb)", format, pitch, offset, offset + size, size / 1024);
	}
	else
	{
		snprintf(result, len, "disabled");
	}
}

//------------------------------------------------------------------------------

DEBUGSTRING(i810_debug_965_fence_start)
{
	const char *enable = (val & FENCE_VALID) ? " enabled" : "disabled";
	char format = (val & I965_FENCE_Y_MAJOR) ? 'Y' : 'X';
	int pitch = ((val & 0xffc) >> 2) * 128 + 128;
	unsigned int offset = val & 0xfffff000;
	
	if (IS_965(devid))
	{
		snprintf(result, len, "%s, %c tile walk, %4d pitch, 0x%08x start", enable, format, pitch, offset);
	}
}

//------------------------------------------------------------------------------

DEBUGSTRING(i810_debug_965_fence_end)
{
	unsigned int end = val & 0xfffff000;
	
	if (IS_965(devid))
	{
		snprintf(result, len, "                                   0x%08x end", end);
	}
}

//------------------------------------------------------------------------------

DEBUGSTRING(gen6_rp_control)
{
	snprintf(result, len, "%s", (val & (1 << 7)) ? "enabled" : "disabled");
}

//------------------------------------------------------------------------------

static struct reg_debug gen6_fences[] = {
#define DEFINEFENCE_SNB(i) \
{ FENCE_REG_SANDYBRIDGE_0 + (i) * 8, "FENCE START "#i, NULL, 0 }, \
{ FENCE_REG_SANDYBRIDGE_0 + (i) * 8 + 4, "FENCE END "#i, NULL, 0 }
	DEFINEFENCE_SNB(0),
	DEFINEFENCE_SNB(1),
	DEFINEFENCE_SNB(2),
	DEFINEFENCE_SNB(3),
	DEFINEFENCE_SNB(4),
	DEFINEFENCE_SNB(5),
	DEFINEFENCE_SNB(6),
	DEFINEFENCE_SNB(7),
	DEFINEFENCE_SNB(8),
	DEFINEFENCE_SNB(9),
	DEFINEFENCE_SNB(10),
	DEFINEFENCE_SNB(11),
	DEFINEFENCE_SNB(12),
	DEFINEFENCE_SNB(13),
	DEFINEFENCE_SNB(14),
	DEFINEFENCE_SNB(15),
	DEFINEFENCE_SNB(16),
	DEFINEFENCE_SNB(17),
	DEFINEFENCE_SNB(18),
	DEFINEFENCE_SNB(19),
	DEFINEFENCE_SNB(20),
	DEFINEFENCE_SNB(20),
	DEFINEFENCE_SNB(21),
	DEFINEFENCE_SNB(22),
	DEFINEFENCE_SNB(23),
	DEFINEFENCE_SNB(24),
	DEFINEFENCE_SNB(25),
	DEFINEFENCE_SNB(26),
	DEFINEFENCE_SNB(27),
	DEFINEFENCE_SNB(28),
	DEFINEFENCE_SNB(29),
	DEFINEFENCE_SNB(30),
	DEFINEFENCE_SNB(31),
};

//------------------------------------------------------------------------------

static struct reg_debug gen6_rp_debug_regs[] = {
	DEFINEREG2(GEN6_RP_CONTROL, gen6_rp_control),
	DEFINEREG(GEN6_RPNSWREQ),
	DEFINEREG(GEN6_RP_DOWN_TIMEOUT),
	DEFINEREG(GEN6_RP_INTERRUPT_LIMITS),
	DEFINEREG(GEN6_RP_UP_THRESHOLD),
	DEFINEREG(GEN6_RP_UP_EI),
	DEFINEREG(GEN6_RP_DOWN_EI),
	DEFINEREG(GEN6_RP_IDLE_HYSTERSIS),
	DEFINEREG(GEN6_RC_STATE),
	DEFINEREG(GEN6_RC_CONTROL),
	DEFINEREG(GEN6_RC1_WAKE_RATE_LIMIT),
	DEFINEREG(GEN6_RC6_WAKE_RATE_LIMIT),
	DEFINEREG(GEN6_RC_EVALUATION_INTERVAL),
	DEFINEREG(GEN6_RC_IDLE_HYSTERSIS),
	DEFINEREG(GEN6_RC_SLEEP),
	DEFINEREG(GEN6_RC1e_THRESHOLD),
	DEFINEREG(GEN6_RC6_THRESHOLD),
	DEFINEREG(GEN6_RC_VIDEO_FREQ),
	DEFINEREG(GEN6_PMIER),
	DEFINEREG(GEN6_PMIMR),
	DEFINEREG(GEN6_PMINTRMSK),
};

//------------------------------------------------------------------------------

static struct reg_debug intel_debug_regs[] = {
	DEFINEREG2(DCC, i830_debug_dcc),
	DEFINEREG2(CHDECMISC, i830_debug_chdecmisc),
	DEFINEREG_16BIT(C0DRB0),
	DEFINEREG_16BIT(C0DRB1),
	DEFINEREG_16BIT(C0DRB2),
	DEFINEREG_16BIT(C0DRB3),
	DEFINEREG_16BIT(C1DRB0),
	DEFINEREG_16BIT(C1DRB1),
	DEFINEREG_16BIT(C1DRB2),
	DEFINEREG_16BIT(C1DRB3),
	DEFINEREG_16BIT(C0DRA01),
	DEFINEREG_16BIT(C0DRA23),
	DEFINEREG_16BIT(C1DRA01),
	DEFINEREG_16BIT(C1DRA23),
	
	DEFINEREG(PGETBL_CTL),
	
	DEFINEREG2(VCLK_DIVISOR_VGA0, i830_debug_fp),
	DEFINEREG2(VCLK_DIVISOR_VGA1, i830_debug_fp),
	DEFINEREG2(VCLK_POST_DIV, i830_debug_vga_pd),
	DEFINEREG2(DPLL_TEST, i830_debug_dpll_test),
	DEFINEREG(CACHE_MODE_0),
	DEFINEREG(D_STATE),
	DEFINEREG2(DSPCLK_GATE_D, i830_debug_dspclk_gate_d),
	DEFINEREG(RENCLK_GATE_D1),
	DEFINEREG(RENCLK_GATE_D2),
	/*  DEFINEREG(RAMCLK_GATE_D),	CRL only */
	DEFINEREG2(SDVOB, i830_debug_sdvo),
	DEFINEREG2(SDVOC, i830_debug_sdvo),
	/*    DEFINEREG(UDIB_SVB_SHB_CODES), CRL only */
	/*    DEFINEREG(UDIB_SHA_BLANK_CODES), CRL only */
	DEFINEREG(SDVOUDI),
	DEFINEREG(DSPARB),
	DEFINEREG(FW_BLC),
	DEFINEREG(FW_BLC2),
	DEFINEREG(FW_BLC_SELF),
	DEFINEREG(DSPFW1),
	DEFINEREG(DSPFW2),
	DEFINEREG(DSPFW3),
	
	DEFINEREG2(ADPA, i830_debug_adpa),
	DEFINEREG2(LVDS, i830_debug_lvds),
	DEFINEREG2(DVOA, i830_debug_dvo),
	DEFINEREG2(DVOB, i830_debug_dvo),
	DEFINEREG2(DVOC, i830_debug_dvo),
	DEFINEREG(DVOA_SRCDIM),
	DEFINEREG(DVOB_SRCDIM),
	DEFINEREG(DVOC_SRCDIM),
	
	DEFINEREG(BLC_PWM_CTL),
	DEFINEREG(BLC_PWM_CTL2),
	
	DEFINEREG2(PP_CONTROL, i830_debug_pp_control),
	DEFINEREG2(PP_STATUS, i830_debug_pp_status),
	DEFINEREG(PP_ON_DELAYS),
	DEFINEREG(PP_OFF_DELAYS),
	DEFINEREG(PP_DIVISOR),
	DEFINEREG(PFIT_CONTROL),
	DEFINEREG(PFIT_PGM_RATIOS),
	DEFINEREG(PORT_HOTPLUG_EN),
	DEFINEREG(PORT_HOTPLUG_STAT),
	
	DEFINEREG2(DSPACNTR, i830_debug_dspcntr),
	DEFINEREG2(DSPASTRIDE, i830_debug_dspstride),
	DEFINEREG2(DSPAPOS, i830_debug_xy),
	DEFINEREG2(DSPASIZE, i830_debug_xyminus1),
	DEFINEREG(DSPABASE),
	DEFINEREG(DSPASURF),
	DEFINEREG(DSPATILEOFF),
	DEFINEREG2(PIPEACONF, i830_debug_pipeconf),
	DEFINEREG2(PIPEASRC, i830_debug_yxminus1),
	DEFINEREG2(PIPEASTAT, i830_debug_pipestat),
	DEFINEREG(PIPEA_GMCH_DATA_M),
	DEFINEREG(PIPEA_GMCH_DATA_N),
	DEFINEREG(PIPEA_DP_LINK_M),
	DEFINEREG(PIPEA_DP_LINK_N),
	DEFINEREG(CURSOR_A_BASE),
	DEFINEREG(CURSOR_A_CONTROL),
	DEFINEREG(CURSOR_A_POSITION),
	
	DEFINEREG2(FPA0, i830_debug_fp),
	DEFINEREG2(FPA1, i830_debug_fp),
	DEFINEREG2(DPLL_A, i830_debug_dpll),
	DEFINEREG(DPLL_A_MD),
	DEFINEREG2(HTOTAL_A, i830_debug_hvtotal),
	DEFINEREG2(HBLANK_A, i830_debug_hvsyncblank),
	DEFINEREG2(HSYNC_A, i830_debug_hvsyncblank),
	DEFINEREG2(VTOTAL_A, i830_debug_hvtotal),
	DEFINEREG2(VBLANK_A, i830_debug_hvsyncblank),
	DEFINEREG2(VSYNC_A, i830_debug_hvsyncblank),
	DEFINEREG(BCLRPAT_A),
	DEFINEREG(VSYNCSHIFT_A),
	
	DEFINEREG2(DSPBCNTR, i830_debug_dspcntr),
	DEFINEREG2(DSPBSTRIDE, i830_debug_dspstride),
	DEFINEREG2(DSPBPOS, i830_debug_xy),
	DEFINEREG2(DSPBSIZE, i830_debug_xyminus1),
	DEFINEREG(DSPBBASE),
	DEFINEREG(DSPBSURF),
	DEFINEREG(DSPBTILEOFF),
	DEFINEREG2(PIPEBCONF, i830_debug_pipeconf),
	DEFINEREG2(PIPEBSRC, i830_debug_yxminus1),
	DEFINEREG2(PIPEBSTAT, i830_debug_pipestat),
	DEFINEREG(PIPEB_GMCH_DATA_M),
	DEFINEREG(PIPEB_GMCH_DATA_N),
	DEFINEREG(PIPEB_DP_LINK_M),
	DEFINEREG(PIPEB_DP_LINK_N),
	DEFINEREG(CURSOR_B_BASE),
	DEFINEREG(CURSOR_B_CONTROL),
	DEFINEREG(CURSOR_B_POSITION),
	
	DEFINEREG2(FPB0, i830_debug_fp),
	DEFINEREG2(FPB1, i830_debug_fp),
	DEFINEREG2(DPLL_B, i830_debug_dpll),
	DEFINEREG(DPLL_B_MD),
	DEFINEREG2(HTOTAL_B, i830_debug_hvtotal),
	DEFINEREG2(HBLANK_B, i830_debug_hvsyncblank),
	DEFINEREG2(HSYNC_B, i830_debug_hvsyncblank),
	DEFINEREG2(VTOTAL_B, i830_debug_hvtotal),
	DEFINEREG2(VBLANK_B, i830_debug_hvsyncblank),
	DEFINEREG2(VSYNC_B, i830_debug_hvsyncblank),
	DEFINEREG(BCLRPAT_B),
	DEFINEREG(VSYNCSHIFT_B),
	
	DEFINEREG(VCLK_DIVISOR_VGA0),
	DEFINEREG(VCLK_DIVISOR_VGA1),
	DEFINEREG(VCLK_POST_DIV),
	DEFINEREG2(VGACNTRL, i830_debug_vgacntrl),
	
	DEFINEREG(TV_CTL),
	DEFINEREG(TV_DAC),
	DEFINEREG(TV_CSC_Y),
	DEFINEREG(TV_CSC_Y2),
	DEFINEREG(TV_CSC_U),
	DEFINEREG(TV_CSC_U2),
	DEFINEREG(TV_CSC_V),
	DEFINEREG(TV_CSC_V2),
	DEFINEREG(TV_CLR_KNOBS),
	DEFINEREG(TV_CLR_LEVEL),
	DEFINEREG(TV_H_CTL_1),
	DEFINEREG(TV_H_CTL_2),
	DEFINEREG(TV_H_CTL_3),
	DEFINEREG(TV_V_CTL_1),
	DEFINEREG(TV_V_CTL_2),
	DEFINEREG(TV_V_CTL_3),
	DEFINEREG(TV_V_CTL_4),
	DEFINEREG(TV_V_CTL_5),
	DEFINEREG(TV_V_CTL_6),
	DEFINEREG(TV_V_CTL_7),
	DEFINEREG(TV_SC_CTL_1),
	DEFINEREG(TV_SC_CTL_2),
	DEFINEREG(TV_SC_CTL_3),
	DEFINEREG(TV_WIN_POS),
	DEFINEREG(TV_WIN_SIZE),
	DEFINEREG(TV_FILTER_CTL_1),
	DEFINEREG(TV_FILTER_CTL_2),
	DEFINEREG(TV_FILTER_CTL_3),
	DEFINEREG(TV_CC_CONTROL),
	DEFINEREG(TV_CC_DATA),
	DEFINEREG(TV_H_LUMA_0),
	DEFINEREG(TV_H_LUMA_59),
	DEFINEREG(TV_H_CHROMA_0),
	DEFINEREG(TV_H_CHROMA_59),
	
	DEFINEREG(FBC_CFB_BASE),
	DEFINEREG(FBC_LL_BASE),
	DEFINEREG(FBC_CONTROL),
	DEFINEREG(FBC_COMMAND),
	DEFINEREG(FBC_STATUS),
	DEFINEREG(FBC_CONTROL2),
	DEFINEREG(FBC_FENCE_OFF),
	DEFINEREG(FBC_MOD_NUM),
	
	DEFINEREG(MI_MODE),
	/* DEFINEREG(MI_DISPLAY_POWER_DOWN), CRL only */
	DEFINEREG(MI_ARB_STATE),
	DEFINEREG(MI_RDRET_STATE),
	DEFINEREG(ECOSKPD),
	
	DEFINEREG(DP_B),
	DEFINEREG(DPB_AUX_CH_CTL),
	DEFINEREG(DPB_AUX_CH_DATA1),
	DEFINEREG(DPB_AUX_CH_DATA2),
	DEFINEREG(DPB_AUX_CH_DATA3),
	DEFINEREG(DPB_AUX_CH_DATA4),
	DEFINEREG(DPB_AUX_CH_DATA5),
	
	DEFINEREG(DP_C),
	DEFINEREG(DPC_AUX_CH_CTL),
	DEFINEREG(DPC_AUX_CH_DATA1),
	DEFINEREG(DPC_AUX_CH_DATA2),
	DEFINEREG(DPC_AUX_CH_DATA3),
	DEFINEREG(DPC_AUX_CH_DATA4),
	DEFINEREG(DPC_AUX_CH_DATA5),
	
	DEFINEREG(DP_D),
	DEFINEREG(DPD_AUX_CH_CTL),
	DEFINEREG(DPD_AUX_CH_DATA1),
	DEFINEREG(DPD_AUX_CH_DATA2),
	DEFINEREG(DPD_AUX_CH_DATA3),
	DEFINEREG(DPD_AUX_CH_DATA4),
	DEFINEREG(DPD_AUX_CH_DATA5),
	
	DEFINEREG(AUD_CONFIG),
	DEFINEREG(AUD_HDMIW_STATUS),
	DEFINEREG(AUD_CONV_CHCNT),
	DEFINEREG(VIDEO_DIP_CTL),
	DEFINEREG(AUD_PINW_CNTR),
	DEFINEREG(AUD_CNTL_ST),
	DEFINEREG(AUD_PIN_CAP),
	DEFINEREG(AUD_PINW_CAP),
	DEFINEREG(AUD_PINW_UNSOLRESP),
	DEFINEREG(AUD_OUT_DIG_CNVT),
	DEFINEREG(AUD_OUT_CWCAP),
	DEFINEREG(AUD_GRP_CAP),
	
#define DEFINEFENCE_915(i) \
{ FENCE+i*4, "FENCE  " #i, i810_debug_915_fence, 0 }
#define DEFINEFENCE_945(i)						\
{ FENCE_NEW+(i - 8) * 4, "FENCE  " #i, i810_debug_915_fence, 0 }
	
	DEFINEFENCE_915(0),
	DEFINEFENCE_915(1),
	DEFINEFENCE_915(2),
	DEFINEFENCE_915(3),
	DEFINEFENCE_915(4),
	DEFINEFENCE_915(5),
	DEFINEFENCE_915(6),
	DEFINEFENCE_915(7),
	DEFINEFENCE_945(8),
	DEFINEFENCE_945(9),
	DEFINEFENCE_945(10),
	DEFINEFENCE_945(11),
	DEFINEFENCE_945(12),
	DEFINEFENCE_945(13),
	DEFINEFENCE_945(14),
	DEFINEFENCE_945(15),
	
#define DEFINEFENCE_965(i) \
{ FENCE_NEW+i*8, "FENCE START " #i, i810_debug_965_fence_start, 0 }, \
{ FENCE_NEW+i*8+4, "FENCE END " #i, i810_debug_965_fence_end, 0 }
	
	DEFINEFENCE_965(0),
	DEFINEFENCE_965(1),
	DEFINEFENCE_965(2),
	DEFINEFENCE_965(3),
	DEFINEFENCE_965(4),
	DEFINEFENCE_965(5),
	DEFINEFENCE_965(6),
	DEFINEFENCE_965(7),
	DEFINEFENCE_965(8),
	DEFINEFENCE_965(9),
	DEFINEFENCE_965(10),
	DEFINEFENCE_965(11),
	DEFINEFENCE_965(12),
	DEFINEFENCE_965(13),
	DEFINEFENCE_965(14),
	DEFINEFENCE_965(15),
	
	DEFINEREG(INST_PM),
};

//------------------------------------------------------------------------------

static struct reg_debug ironlake_debug_regs[] = {
	DEFINEREG(PGETBL_CTL),
	DEFINEREG(INSTDONE_I965),
	DEFINEREG(INSTDONE_1),
	DEFINEREG2(CPU_VGACNTRL, i830_debug_vgacntrl),
	DEFINEREG(DIGITAL_PORT_HOTPLUG_CNTRL),
	
	DEFINEREG2(RR_HW_CTL, ironlake_debug_rr_hw_ctl),
	
	DEFINEREG(FDI_PLL_BIOS_0),
	DEFINEREG(FDI_PLL_BIOS_1),
	DEFINEREG(FDI_PLL_BIOS_2),
	
	DEFINEREG(DISPLAY_PORT_PLL_BIOS_0),
	DEFINEREG(DISPLAY_PORT_PLL_BIOS_1),
	DEFINEREG(DISPLAY_PORT_PLL_BIOS_2),
	
	DEFINEREG(FDI_PLL_FREQ_CTL),
	
	/* pipe B */
	
	DEFINEREG2(PIPEACONF, i830_debug_pipeconf),
	
	DEFINEREG2(HTOTAL_A, i830_debug_hvtotal),
	DEFINEREG2(HBLANK_A, i830_debug_hvsyncblank),
	DEFINEREG2(HSYNC_A, i830_debug_hvsyncblank),
	DEFINEREG2(VTOTAL_A, i830_debug_hvtotal),
	DEFINEREG2(VBLANK_A, i830_debug_hvsyncblank),
	DEFINEREG2(VSYNC_A, i830_debug_hvsyncblank),
	DEFINEREG(VSYNCSHIFT_A),
	DEFINEREG2(PIPEASRC, i830_debug_yxminus1),
	
	DEFINEREG2(PIPEA_DATA_M1, ironlake_debug_m_tu),
	DEFINEREG2(PIPEA_DATA_N1, ironlake_debug_n),
	DEFINEREG2(PIPEA_DATA_M2, ironlake_debug_m_tu),
	DEFINEREG2(PIPEA_DATA_N2, ironlake_debug_n),
	
	DEFINEREG2(PIPEA_LINK_M1, ironlake_debug_n),
	DEFINEREG2(PIPEA_LINK_N1, ironlake_debug_n),
	DEFINEREG2(PIPEA_LINK_M2, ironlake_debug_n),
	DEFINEREG2(PIPEA_LINK_N2, ironlake_debug_n),
	
	DEFINEREG2(DSPACNTR, i830_debug_dspcntr),
	DEFINEREG(DSPABASE),
	DEFINEREG2(DSPASTRIDE, ironlake_debug_dspstride),
	DEFINEREG(DSPASURF),
	DEFINEREG2(DSPATILEOFF, i830_debug_xy),
	
	/* pipe B */
	
	DEFINEREG2(PIPEBCONF, i830_debug_pipeconf),
	
	DEFINEREG2(HTOTAL_B, i830_debug_hvtotal),
	DEFINEREG2(HBLANK_B, i830_debug_hvsyncblank),
	DEFINEREG2(HSYNC_B, i830_debug_hvsyncblank),
	DEFINEREG2(VTOTAL_B, i830_debug_hvtotal),
	DEFINEREG2(VBLANK_B, i830_debug_hvsyncblank),
	DEFINEREG2(VSYNC_B, i830_debug_hvsyncblank),
	DEFINEREG(VSYNCSHIFT_B),
	DEFINEREG2(PIPEBSRC, i830_debug_yxminus1),
	
	DEFINEREG2(PIPEB_DATA_M1, ironlake_debug_m_tu),
	DEFINEREG2(PIPEB_DATA_N1, ironlake_debug_n),
	DEFINEREG2(PIPEB_DATA_M2, ironlake_debug_m_tu),
	DEFINEREG2(PIPEB_DATA_N2, ironlake_debug_n),
	
	DEFINEREG2(PIPEB_LINK_M1, ironlake_debug_n),
	DEFINEREG2(PIPEB_LINK_N1, ironlake_debug_n),
	DEFINEREG2(PIPEB_LINK_M2, ironlake_debug_n),
	DEFINEREG2(PIPEB_LINK_N2, ironlake_debug_n),
	
	DEFINEREG2(DSPBCNTR, i830_debug_dspcntr),
	DEFINEREG(DSPBBASE),
	DEFINEREG2(DSPBSTRIDE, ironlake_debug_dspstride),
	DEFINEREG(DSPBSURF),
	DEFINEREG2(DSPBTILEOFF, i830_debug_xy),
	
	/* pipe C */
	
	DEFINEREG2(PIPECCONF, i830_debug_pipeconf),
	
	DEFINEREG2(HTOTAL_C, i830_debug_hvtotal),
	DEFINEREG2(HBLANK_C, i830_debug_hvsyncblank),
	DEFINEREG2(HSYNC_C, i830_debug_hvsyncblank),
	DEFINEREG2(VTOTAL_C, i830_debug_hvtotal),
	DEFINEREG2(VBLANK_C, i830_debug_hvsyncblank),
	DEFINEREG2(VSYNC_C, i830_debug_hvsyncblank),
	DEFINEREG(VSYNCSHIFT_C),
	DEFINEREG2(PIPECSRC, i830_debug_yxminus1),
	
	DEFINEREG2(PIPEC_DATA_M1, ironlake_debug_m_tu),
	DEFINEREG2(PIPEC_DATA_N1, ironlake_debug_n),
	DEFINEREG2(PIPEC_DATA_M2, ironlake_debug_m_tu),
	DEFINEREG2(PIPEC_DATA_N2, ironlake_debug_n),
	
	DEFINEREG2(PIPEC_LINK_M1, ironlake_debug_n),
	DEFINEREG2(PIPEC_LINK_N1, ironlake_debug_n),
	DEFINEREG2(PIPEC_LINK_M2, ironlake_debug_n),
	DEFINEREG2(PIPEC_LINK_N2, ironlake_debug_n),
	
	DEFINEREG2(DSPCCNTR, i830_debug_dspcntr),
	DEFINEREG(DSPCBASE),
	DEFINEREG2(DSPCSTRIDE, ironlake_debug_dspstride),
	DEFINEREG(DSPCSURF),
	DEFINEREG2(DSPCTILEOFF, i830_debug_xy),
	
	/* Panel fitter */
	
	DEFINEREG2(PFA_CTL_1, ironlake_debug_panel_fitting),
	DEFINEREG2(PFA_CTL_2, ironlake_debug_panel_fitting_2),
	DEFINEREG2(PFA_CTL_3, ironlake_debug_panel_fitting_3),
	DEFINEREG2(PFA_CTL_4, ironlake_debug_panel_fitting_4),
	DEFINEREG2(PFA_WIN_POS, ironlake_debug_pf_win),
	DEFINEREG2(PFA_WIN_SIZE, ironlake_debug_pf_win),
	DEFINEREG2(PFB_CTL_1, ironlake_debug_panel_fitting),
	DEFINEREG2(PFB_CTL_2, ironlake_debug_panel_fitting_2),
	DEFINEREG2(PFB_CTL_3, ironlake_debug_panel_fitting_3),
	DEFINEREG2(PFB_CTL_4, ironlake_debug_panel_fitting_4),
	DEFINEREG2(PFB_WIN_POS, ironlake_debug_pf_win),
	DEFINEREG2(PFB_WIN_SIZE, ironlake_debug_pf_win),
	DEFINEREG2(PFC_CTL_1, ironlake_debug_panel_fitting),
	DEFINEREG2(PFC_CTL_2, ironlake_debug_panel_fitting_2),
	DEFINEREG2(PFC_CTL_3, ironlake_debug_panel_fitting_3),
	DEFINEREG2(PFC_CTL_4, ironlake_debug_panel_fitting_4),
	DEFINEREG2(PFC_WIN_POS, ironlake_debug_pf_win),
	DEFINEREG2(PFC_WIN_SIZE, ironlake_debug_pf_win),
	
	/* PCH */
	
	DEFINEREG2(PCH_DREF_CONTROL, ironlake_debug_dref_ctl),
	DEFINEREG2(PCH_RAWCLK_FREQ, ironlake_debug_rawclk_freq),
	DEFINEREG(PCH_DPLL_TMR_CFG),
	DEFINEREG(PCH_SSC4_PARMS),
	DEFINEREG(PCH_SSC4_AUX_PARMS),
	DEFINEREG2(PCH_DPLL_SEL, snb_debug_dpll_sel),
	DEFINEREG(PCH_DPLL_ANALOG_CTL),
	
	DEFINEREG2(PCH_DPLL_A, ironlake_debug_pch_dpll),
	DEFINEREG2(PCH_DPLL_B, ironlake_debug_pch_dpll),
	DEFINEREG2(PCH_FPA0, i830_debug_fp),
	DEFINEREG2(PCH_FPA1, i830_debug_fp),
	DEFINEREG2(PCH_FPB0, i830_debug_fp),
	DEFINEREG2(PCH_FPB1, i830_debug_fp),
	
	DEFINEREG2(TRANS_HTOTAL_A, i830_debug_hvtotal),
	DEFINEREG2(TRANS_HBLANK_A, i830_debug_hvsyncblank),
	DEFINEREG2(TRANS_HSYNC_A, i830_debug_hvsyncblank),
	DEFINEREG2(TRANS_VTOTAL_A, i830_debug_hvtotal),
	DEFINEREG2(TRANS_VBLANK_A, i830_debug_hvsyncblank),
	DEFINEREG2(TRANS_VSYNC_A, i830_debug_hvsyncblank),
	DEFINEREG(TRANS_VSYNCSHIFT_A),
	
	DEFINEREG2(TRANSA_DATA_M1, ironlake_debug_m_tu),
	DEFINEREG2(TRANSA_DATA_N1, ironlake_debug_n),
	DEFINEREG2(TRANSA_DATA_M2, ironlake_debug_m_tu),
	DEFINEREG2(TRANSA_DATA_N2, ironlake_debug_n),
	DEFINEREG2(TRANSA_DP_LINK_M1, ironlake_debug_n),
	DEFINEREG2(TRANSA_DP_LINK_N1, ironlake_debug_n),
	DEFINEREG2(TRANSA_DP_LINK_M2, ironlake_debug_n),
	DEFINEREG2(TRANSA_DP_LINK_N2, ironlake_debug_n),
	
	DEFINEREG2(TRANS_HTOTAL_B, i830_debug_hvtotal),
	DEFINEREG2(TRANS_HBLANK_B, i830_debug_hvsyncblank),
	DEFINEREG2(TRANS_HSYNC_B, i830_debug_hvsyncblank),
	DEFINEREG2(TRANS_VTOTAL_B, i830_debug_hvtotal),
	DEFINEREG2(TRANS_VBLANK_B, i830_debug_hvsyncblank),
	DEFINEREG2(TRANS_VSYNC_B, i830_debug_hvsyncblank),
	DEFINEREG(TRANS_VSYNCSHIFT_B),
	
	DEFINEREG2(TRANSB_DATA_M1, ironlake_debug_m_tu),
	DEFINEREG2(TRANSB_DATA_N1, ironlake_debug_n),
	DEFINEREG2(TRANSB_DATA_M2, ironlake_debug_m_tu),
	DEFINEREG2(TRANSB_DATA_N2, ironlake_debug_n),
	DEFINEREG2(TRANSB_DP_LINK_M1, ironlake_debug_n),
	DEFINEREG2(TRANSB_DP_LINK_N1, ironlake_debug_n),
	DEFINEREG2(TRANSB_DP_LINK_M2, ironlake_debug_n),
	DEFINEREG2(TRANSB_DP_LINK_N2, ironlake_debug_n),
	
	DEFINEREG2(TRANS_HTOTAL_C, i830_debug_hvtotal),
	DEFINEREG2(TRANS_HBLANK_C, i830_debug_hvsyncblank),
	DEFINEREG2(TRANS_HSYNC_C, i830_debug_hvsyncblank),
	DEFINEREG2(TRANS_VTOTAL_C, i830_debug_hvtotal),
	DEFINEREG2(TRANS_VBLANK_C, i830_debug_hvsyncblank),
	DEFINEREG2(TRANS_VSYNC_C, i830_debug_hvsyncblank),
	DEFINEREG(TRANS_VSYNCSHIFT_C),
	
	DEFINEREG2(TRANSC_DATA_M1, ironlake_debug_m_tu),
	DEFINEREG2(TRANSC_DATA_N1, ironlake_debug_n),
	DEFINEREG2(TRANSC_DATA_M2, ironlake_debug_m_tu),
	DEFINEREG2(TRANSC_DATA_N2, ironlake_debug_n),
	DEFINEREG2(TRANSC_DP_LINK_M1, ironlake_debug_n),
	DEFINEREG2(TRANSC_DP_LINK_N1, ironlake_debug_n),
	DEFINEREG2(TRANSC_DP_LINK_M2, ironlake_debug_n),
	DEFINEREG2(TRANSC_DP_LINK_N2, ironlake_debug_n),
	
	DEFINEREG2(TRANSACONF, ironlake_debug_transconf),
	DEFINEREG2(TRANSBCONF, ironlake_debug_transconf),
	DEFINEREG2(TRANSCCONF, ironlake_debug_transconf),
	
	DEFINEREG2(FDI_TXA_CTL, ironlake_debug_fdi_tx_ctl),
	DEFINEREG2(FDI_TXB_CTL, ironlake_debug_fdi_tx_ctl),
	DEFINEREG2(FDI_TXC_CTL, ironlake_debug_fdi_tx_ctl),
	DEFINEREG2(FDI_RXA_CTL, ironlake_debug_fdi_rx_ctl),
	DEFINEREG2(FDI_RXB_CTL, ironlake_debug_fdi_rx_ctl),
	DEFINEREG2(FDI_RXC_CTL, ironlake_debug_fdi_rx_ctl),
	
	DEFINEREG(DPAFE_BMFUNC),
	DEFINEREG(DPAFE_DL_IREFCAL0),
	DEFINEREG(DPAFE_DL_IREFCAL1),
	DEFINEREG(DPAFE_DP_IREFCAL),
	
	DEFINEREG(PCH_DSPCLK_GATE_D),
	DEFINEREG(PCH_DSP_CHICKEN1),
	DEFINEREG(PCH_DSP_CHICKEN2),
	DEFINEREG(PCH_DSP_CHICKEN3),
	
	DEFINEREG2(FDI_RXA_MISC, ironlake_debug_fdi_rx_misc),
	DEFINEREG2(FDI_RXB_MISC, ironlake_debug_fdi_rx_misc),
	DEFINEREG2(FDI_RXC_MISC, ironlake_debug_fdi_rx_misc),
	DEFINEREG(FDI_RXA_TUSIZE1),
	DEFINEREG(FDI_RXA_TUSIZE2),
	DEFINEREG(FDI_RXB_TUSIZE1),
	DEFINEREG(FDI_RXB_TUSIZE2),
	DEFINEREG(FDI_RXC_TUSIZE1),
	DEFINEREG(FDI_RXC_TUSIZE2),
	
	DEFINEREG(FDI_PLL_CTL_1),
	DEFINEREG(FDI_PLL_CTL_2),
	
	DEFINEREG(FDI_RXA_IIR),
	DEFINEREG(FDI_RXA_IMR),
	DEFINEREG(FDI_RXB_IIR),
	DEFINEREG(FDI_RXB_IMR),
	
	DEFINEREG2(PCH_ADPA, i830_debug_adpa),
	DEFINEREG2(HDMIB, ironlake_debug_hdmi),
	DEFINEREG2(HDMIC, ironlake_debug_hdmi),
	DEFINEREG2(HDMID, ironlake_debug_hdmi),
	DEFINEREG2(PCH_LVDS, i830_debug_lvds),
	DEFINEREG(CPU_eDP_A),
	DEFINEREG(PCH_DP_B),
	DEFINEREG(PCH_DP_C),
	DEFINEREG(PCH_DP_D),
	DEFINEREG2(TRANS_DP_CTL_A, snb_debug_trans_dp_ctl),
	DEFINEREG2(TRANS_DP_CTL_B, snb_debug_trans_dp_ctl),
	DEFINEREG2(TRANS_DP_CTL_C, snb_debug_trans_dp_ctl),
	
	DEFINEREG2(BLC_PWM_CPU_CTL2, ilk_debug_blc_pwm_cpu_ctl2),
	DEFINEREG2(BLC_PWM_CPU_CTL, ilk_debug_blc_pwm_cpu_ctl),
	DEFINEREG2(BLC_PWM_PCH_CTL1, ibx_debug_blc_pwm_ctl1),
	DEFINEREG2(BLC_PWM_PCH_CTL2, ibx_debug_blc_pwm_ctl2),
	
	DEFINEREG2(PCH_PP_STATUS, i830_debug_pp_status),
	DEFINEREG2(PCH_PP_CONTROL, ilk_debug_pp_control),
	DEFINEREG(PCH_PP_ON_DELAYS),
	DEFINEREG(PCH_PP_OFF_DELAYS),
	DEFINEREG(PCH_PP_DIVISOR),
	
	DEFINEREG2(PORT_DBG, ivb_debug_port),
	
	DEFINEREG(RC6_RESIDENCY_TIME),
	DEFINEREG(RC6p_RESIDENCY_TIME),
	DEFINEREG(RC6pp_RESIDENCY_TIME),
};

//------------------------------------------------------------------------------

static struct reg_debug haswell_debug_regs[] = {
	/* Power wells */
	DEFINEREG(HSW_PWR_WELL_CTL1),
	DEFINEREG(HSW_PWR_WELL_CTL2),
	DEFINEREG(HSW_PWR_WELL_CTL3),
	DEFINEREG(HSW_PWR_WELL_CTL4),
	DEFINEREG(HSW_PWR_WELL_CTL5),
	DEFINEREG(HSW_PWR_WELL_CTL6),
	
	/* DDI pipe function */
	DEFINEREG2(PIPE_DDI_FUNC_CTL_A, hsw_debug_pipe_ddi_func_ctl),
	DEFINEREG2(PIPE_DDI_FUNC_CTL_B, hsw_debug_pipe_ddi_func_ctl),
	DEFINEREG2(PIPE_DDI_FUNC_CTL_C, hsw_debug_pipe_ddi_func_ctl),
	DEFINEREG2(PIPE_DDI_FUNC_CTL_EDP, hsw_debug_pipe_ddi_func_ctl),
	
	/* DP transport control */
	DEFINEREG(DP_TP_CTL_A),
	DEFINEREG(DP_TP_CTL_B),
	DEFINEREG(DP_TP_CTL_C),
	DEFINEREG(DP_TP_CTL_D),
	DEFINEREG(DP_TP_CTL_E),
	
	/* DP status */
	DEFINEREG(DP_TP_STATUS_A),
	DEFINEREG(DP_TP_STATUS_B),
	DEFINEREG(DP_TP_STATUS_C),
	DEFINEREG(DP_TP_STATUS_D),
	DEFINEREG(DP_TP_STATUS_E),
	
	/* DDI buffer control */
	DEFINEREG2(DDI_BUF_CTL_A, hsw_debug_ddi_buf_ctl),
	DEFINEREG2(DDI_BUF_CTL_B, hsw_debug_ddi_buf_ctl),
	DEFINEREG2(DDI_BUF_CTL_C, hsw_debug_ddi_buf_ctl),
	DEFINEREG2(DDI_BUF_CTL_D, hsw_debug_ddi_buf_ctl),
	DEFINEREG2(DDI_BUF_CTL_E, hsw_debug_ddi_buf_ctl),
	
	/* Clocks */
	DEFINEREG(SPLL_CTL),
	DEFINEREG(LCPLL_CTL),
	DEFINEREG(WRPLL_CTL1),
	DEFINEREG(WRPLL_CTL2),
	
	/* DDI port clock control */
	DEFINEREG2(PORT_CLK_SEL_A, hsw_debug_port_clk_sel),
	DEFINEREG2(PORT_CLK_SEL_B, hsw_debug_port_clk_sel),
	DEFINEREG2(PORT_CLK_SEL_C, hsw_debug_port_clk_sel),
	DEFINEREG2(PORT_CLK_SEL_D, hsw_debug_port_clk_sel),
	DEFINEREG2(PORT_CLK_SEL_E, hsw_debug_port_clk_sel),
	
	/* Pipe clock control */
	DEFINEREG2(PIPE_CLK_SEL_A, hsw_debug_pipe_clk_sel),
	DEFINEREG2(PIPE_CLK_SEL_B, hsw_debug_pipe_clk_sel),
	DEFINEREG2(PIPE_CLK_SEL_C, hsw_debug_pipe_clk_sel),
	
	/* Fuses */
	DEFINEREG2(SFUSE_STRAP, hsw_debug_sfuse_strap),
	
	/* Pipe A */
	DEFINEREG2(PIPEASRC, i830_debug_yxminus1),
	DEFINEREG2(DSPACNTR, i830_debug_dspcntr),
	DEFINEREG2(DSPASTRIDE, ironlake_debug_dspstride),
	DEFINEREG(DSPASURF),
	DEFINEREG2(DSPATILEOFF, i830_debug_xy),
	
	/* Pipe B */
	DEFINEREG2(PIPEBSRC, i830_debug_yxminus1),
	DEFINEREG2(DSPBCNTR, i830_debug_dspcntr),
	DEFINEREG2(DSPBSTRIDE, ironlake_debug_dspstride),
	DEFINEREG(DSPBSURF),
	DEFINEREG2(DSPBTILEOFF, i830_debug_xy),
	
	/* Pipe C */
	DEFINEREG2(PIPECSRC, i830_debug_yxminus1),
	DEFINEREG2(DSPCCNTR, i830_debug_dspcntr),
	DEFINEREG2(DSPCSTRIDE, ironlake_debug_dspstride),
	DEFINEREG(DSPCSURF),
	DEFINEREG2(DSPCTILEOFF, i830_debug_xy),
	
	/* Transcoder A */
	DEFINEREG2(PIPEACONF, i830_debug_pipeconf),
	DEFINEREG2(HTOTAL_A, i830_debug_hvtotal),
	DEFINEREG2(HBLANK_A, i830_debug_hvsyncblank),
	DEFINEREG2(HSYNC_A, i830_debug_hvsyncblank),
	DEFINEREG2(VTOTAL_A, i830_debug_hvtotal),
	DEFINEREG2(VBLANK_A, i830_debug_hvsyncblank),
	DEFINEREG2(VSYNC_A, i830_debug_hvsyncblank),
	DEFINEREG(VSYNCSHIFT_A),
	DEFINEREG2(PIPEA_DATA_M1, ironlake_debug_m_tu),
	DEFINEREG2(PIPEA_DATA_N1, ironlake_debug_n),
	DEFINEREG2(PIPEA_LINK_M1, ironlake_debug_n),
	DEFINEREG2(PIPEA_LINK_N1, ironlake_debug_n),
	
	/* Transcoder B */
	DEFINEREG2(PIPEBCONF, i830_debug_pipeconf),
	DEFINEREG2(HTOTAL_B, i830_debug_hvtotal),
	DEFINEREG2(HBLANK_B, i830_debug_hvsyncblank),
	DEFINEREG2(HSYNC_B, i830_debug_hvsyncblank),
	DEFINEREG2(VTOTAL_B, i830_debug_hvtotal),
	DEFINEREG2(VBLANK_B, i830_debug_hvsyncblank),
	DEFINEREG2(VSYNC_B, i830_debug_hvsyncblank),
	DEFINEREG(VSYNCSHIFT_B),
	DEFINEREG2(PIPEB_DATA_M1, ironlake_debug_m_tu),
	DEFINEREG2(PIPEB_DATA_N1, ironlake_debug_n),
	DEFINEREG2(PIPEB_LINK_M1, ironlake_debug_n),
	DEFINEREG2(PIPEB_LINK_N1, ironlake_debug_n),
	
	/* Transcoder C */
	DEFINEREG2(PIPECCONF, i830_debug_pipeconf),
	DEFINEREG2(HTOTAL_C, i830_debug_hvtotal),
	DEFINEREG2(HBLANK_C, i830_debug_hvsyncblank),
	DEFINEREG2(HSYNC_C, i830_debug_hvsyncblank),
	DEFINEREG2(VTOTAL_C, i830_debug_hvtotal),
	DEFINEREG2(VBLANK_C, i830_debug_hvsyncblank),
	DEFINEREG2(VSYNC_C, i830_debug_hvsyncblank),
	DEFINEREG(VSYNCSHIFT_C),
	DEFINEREG2(PIPEC_DATA_M1, ironlake_debug_m_tu),
	DEFINEREG2(PIPEC_DATA_N1, ironlake_debug_n),
	DEFINEREG2(PIPEC_LINK_M1, ironlake_debug_n),
	DEFINEREG2(PIPEC_LINK_N1, ironlake_debug_n),
	
	/* Transcoder EDP */
	DEFINEREG2(PIPEEDPCONF, i830_debug_pipeconf),
	DEFINEREG2(HTOTAL_EDP, i830_debug_hvtotal),
	DEFINEREG2(HBLANK_EDP, i830_debug_hvsyncblank),
	DEFINEREG2(HSYNC_EDP, i830_debug_hvsyncblank),
	DEFINEREG2(VTOTAL_EDP, i830_debug_hvtotal),
	DEFINEREG2(VBLANK_EDP, i830_debug_hvsyncblank),
	DEFINEREG2(VSYNC_EDP, i830_debug_hvsyncblank),
	DEFINEREG(VSYNCSHIFT_EDP),
	DEFINEREG2(PIPEEDP_DATA_M1, ironlake_debug_m_tu),
	DEFINEREG2(PIPEEDP_DATA_N1, ironlake_debug_n),
	DEFINEREG2(PIPEEDP_LINK_M1, ironlake_debug_n),
	DEFINEREG2(PIPEEDP_LINK_N1, ironlake_debug_n),
	
	/* CPU Panel fitter */
	DEFINEREG2(PFA_CTL_1, ironlake_debug_panel_fitting),
	DEFINEREG2(PFA_WIN_POS, ironlake_debug_pf_win),
	DEFINEREG2(PFA_WIN_SIZE, ironlake_debug_pf_win),
	
	DEFINEREG2(PFB_CTL_1, ironlake_debug_panel_fitting),
	DEFINEREG2(PFB_WIN_POS, ironlake_debug_pf_win),
	DEFINEREG2(PFB_WIN_SIZE, ironlake_debug_pf_win),
	
	DEFINEREG2(PFC_CTL_1, ironlake_debug_panel_fitting),
	DEFINEREG2(PFC_WIN_POS, ironlake_debug_pf_win),
	DEFINEREG2(PFC_WIN_SIZE, ironlake_debug_pf_win),
	
	/* LPT */
	DEFINEREG2(TRANS_HTOTAL_A, i830_debug_hvtotal),
	DEFINEREG2(TRANS_HBLANK_A, i830_debug_hvsyncblank),
	DEFINEREG2(TRANS_HSYNC_A, i830_debug_hvsyncblank),
	DEFINEREG2(TRANS_VTOTAL_A, i830_debug_hvtotal),
	DEFINEREG2(TRANS_VBLANK_A, i830_debug_hvsyncblank),
	DEFINEREG2(TRANS_VSYNC_A, i830_debug_hvsyncblank),
	DEFINEREG(TRANS_VSYNCSHIFT_A),
	
	DEFINEREG2(TRANSACONF, ironlake_debug_transconf),
	
	DEFINEREG2(FDI_RXA_MISC, ironlake_debug_fdi_rx_misc),
	DEFINEREG(FDI_RXA_TUSIZE1),
	DEFINEREG(FDI_RXA_IIR),
	DEFINEREG(FDI_RXA_IMR),
	
	DEFINEREG2(BLC_PWM_CPU_CTL2, ilk_debug_blc_pwm_cpu_ctl2),
	DEFINEREG2(BLC_PWM_CPU_CTL, ilk_debug_blc_pwm_cpu_ctl),
	DEFINEREG2(BLC_PWM2_CPU_CTL2, ilk_debug_blc_pwm_cpu_ctl2),
	DEFINEREG2(BLC_PWM2_CPU_CTL, ilk_debug_blc_pwm_cpu_ctl),
	DEFINEREG2(BLC_MISC_CTL, hsw_debug_blc_misc_ctl),
	DEFINEREG2(BLC_PWM_PCH_CTL1, ibx_debug_blc_pwm_ctl1),
	DEFINEREG2(BLC_PWM_PCH_CTL2, ibx_debug_blc_pwm_ctl2),
	
	DEFINEREG2(UTIL_PIN_CTL, hsw_debug_util_pin_ctl),
	
	DEFINEREG2(PCH_PP_STATUS, i830_debug_pp_status),
	DEFINEREG2(PCH_PP_CONTROL, ilk_debug_pp_control),
	DEFINEREG(PCH_PP_ON_DELAYS),
	DEFINEREG(PCH_PP_OFF_DELAYS),
	DEFINEREG(PCH_PP_DIVISOR),
	
	DEFINEREG(PIXCLK_GATE),
	
	DEFINEREG2(SDEISR, hsw_debug_sinterrupt),
	
	DEFINEREG(RC6_RESIDENCY_TIME)
};

//------------------------------------------------------------------------------

static struct reg_debug i945gm_mi_regs[] = {
	DEFINEREG(PGETBL_CTL),
	DEFINEREG(PGTBL_ER),
	DEFINEREG(EXCC),
	DEFINEREG(HWS_PGA),
	DEFINEREG(IPEIR),
	DEFINEREG(IPEHR),
	DEFINEREG(INSTDONE),
	DEFINEREG(NOP_ID),
	DEFINEREG(HWSTAM),
	DEFINEREG(SCPD0),
	DEFINEREG(IER),
	DEFINEREG(IIR),
	DEFINEREG(IMR),
	DEFINEREG(ISR),
	DEFINEREG(EIR),
	DEFINEREG(EMR),
	DEFINEREG(ESR),
	DEFINEREG(INST_PM),
	DEFINEREG(ECOSKPD),
};

//------------------------------------------------------------------------------

void intel_dump_other_regs(void)
{
	int i;
	int fp, dpll;
	int disp_pipe;
	int n, m1, m2, m, p1, p2;
	int ref;
	int dot;
	int phase;
	
	for (disp_pipe = 0; disp_pipe <= 1; disp_pipe++)
	{
		if (disp_pipe == 0)
		{
			fp = MMIO_READ32(FPA0);
			dpll = MMIO_READ32(DPLL_A);
		}
		else
		{
			fp = MMIO_READ32(FPB0);
			dpll = MMIO_READ32(DPLL_B);
		}

		if (IS_GEN2(devid))
		{
			uint32_t lvds = MMIO_READ32(LVDS);

			if (devid == PCI_CHIP_I855_GM && (lvds & LVDS_PORT_EN) && (lvds & LVDS_PIPEB_SELECT) == (disp_pipe << 30))
			{
				if ((lvds & LVDS_CLKB_POWER_MASK) == LVDS_CLKB_POWER_UP)
				{
					p2 = 7;
				}
				else
				{
					p2 = 14;
				}

				switch ((dpll >> 16) & 0x3f)
				{
					case 0x01:
						p1 = 1;
						break;
					case 0x02:
						p1 = 2;
						break;
					case 0x04:
						p1 = 3;
						break;
					case 0x08:
						p1 = 4;
						break;
					case 0x10:
						p1 = 5;
						break;
					case 0x20:
						p1 = 6;
						break;
					default:
						p1 = 1;
						printf("LVDS P1 0x%x invalid encoding\n", (dpll >> 16) & 0x3f);
						break;
				}
			}
			else
			{
				if (dpll & (1 << 23))
				{
					p2 = 4;
				}
				else
				{
					p2 = 2;
				}

				if (dpll & PLL_P1_DIVIDE_BY_TWO)
				{
					p1 = 2;
				}
				else
				{
					p1 = ((dpll >> 16) & 0x3f) + 2;
				}
			}

			switch ((dpll >> 13) & 0x3)
			{
				case 0:
					ref = 48000;
					break;
				case 3:
					ref = 66000;
					break;
				default:
					ref = 0;
					printf("ref out of range\n");
					break;
			}
		}
		else
		{
			uint32_t lvds = MMIO_READ32(LVDS);

			if ((lvds & LVDS_PORT_EN) && (lvds & LVDS_PIPEB_SELECT) == (disp_pipe << 30))
			{
				if ((lvds & LVDS_CLKB_POWER_MASK) == LVDS_CLKB_POWER_UP)
				{
					p2 = 7;
				}
				else
				{
					p2 = 14;
				}
			}
			else
			{
				switch ((dpll >> 24) & 0x3)
				{
					case 0:
						p2 = 10;
						break;
					case 1:
						p2 = 5;
						break;
					default:
						p2 = 1;
						printf("p2 out of range\n");
						break;
				}
			}

			if (IS_IGD(devid))
			{
				i = (dpll >> DPLL_FPA01_P1_POST_DIV_SHIFT_IGD) & 0x1ff;
			}
			else
			{
				i = (dpll >> DPLL_FPA01_P1_POST_DIV_SHIFT) & 0xff;
			}

			switch (i)
			{
				case 1:
					p1 = 1;
					break;
				case 2:
					p1 = 2;
					break;
				case 4:
					p1 = 3;
					break;
				case 8:
					p1 = 4;
					break;
				case 16:
					p1 = 5;
					break;
				case 32:
					p1 = 6;
					break;
				case 64:
					p1 = 7;
					break;
				case 128:
					p1 = 8;
					break;
				case 256:
					if (IS_IGD(devid))
					{
						p1 = 9;
						break;
					}	// fallback
				default:
					p1 = 1;
					printf("p1 out of range\n");
					break;
			}

			switch ((dpll >> 13) & 0x3)
			{
				case 0:
					ref = 96000;
					break;
				case 3:
					ref = 100000;
					break;
				default:
					ref = 0;
					printf("ref out of range\n");
					break;
			}
		}

		if (IS_965(devid))
		{
			phase = (dpll >> 9) & 0xf;

			switch (phase)
			{
				case 6:
					break;
				default:
					printf("SDVO phase shift %d out of range -- probably not an issue.\n", phase);
					break;
			}
		}

		switch ((dpll >> 8) & 1)
		{
			case 0:
				break;
			default:
				printf("fp select out of range\n");
				break;
		}

		m1 = ((fp >> 8) & 0x3f);

		if (IS_IGD(devid))
		{
			n = ffs((fp & FP_N_IGD_DIV_MASK) >> FP_N_DIV_SHIFT) - 1;
			m2 = (fp & FP_M2_IGD_DIV_MASK) >> FP_M2_DIV_SHIFT;
			m = m2 + 2;
			dot = (ref * m) / n / (p1 * p2);
		}
		else
		{
			n = ((fp >> 16) & 0x3f);
			m2 = ((fp >> 0) & 0x3f);
			//m = 5 * (m1 + 2) + (m2 + 2);
			dot = (ref * (5 * (m1 + 2) + (m2 + 2)) / (n + 2)) / (p1 * p2);
		}
		
		printf("pipe %s dot %d n %d m1 %d m2 %d p1 %d p2 %d\n", disp_pipe == 0 ? "A" : "B", dot, n, m1, m2, p1, p2);
	}
}

//------------------------------------------------------------------------------
// void AppleIntelInfo::dumpRegisters(struct reg_debug *regs, uint32_t count)

DEFINE_FUNC_DUMP(dumpRegisters)
{
	char name[30];
	char debug[1024];

	for (int i = 0; i < count; i++)
	{
		UInt32 val = MMIO_READ32((UInt64)regs[i].reg);

		memset(name, 0, 30);
		memset(debug, 0, 1024);
		memcpy(name, regs[i].name, strlen(regs[i].name));
		//			  "123456789 123456789 1234567"
		strncat(name, "...........................", (27 - strlen(regs[i].name)));

		if (regs[i].debug_output != NULL)
		{
			regs[i].debug_output(debug, sizeof(debug), regs[i].reg, val);
			printf("%s: 0x%08x (%s)\n", name, val, debug);
		}
		else
		{
			printf("%s: 0x%08x\n", name, val);
		}
	}
}

//------------------------------------------------------------------------------
// void getPCHDeviceID(void)

DEFINE_FUNC_VOID(getPCHDeviceID)
{
	outl(0xcf8, 0x8000F800);
	UInt64 pch_device = inl(0xcfc);

	IOLog("PCH device.................: 0x%llX\n", pch_device);

	if ((pch_device & 0x0000ffff) == 0x8086)
	{
		switch ((pch_device & 0xff000000))
		{
			case 0x3b000000:
				intel_pch = PCH_IBX;
				break;
			case 0x1c000000:
			case 0x1e000000:
				intel_pch = PCH_CPT;
				break;
			case 0x8c000000:
			case 0x9c000000:
				intel_pch = PCH_LPT;
				break;
			default:
				intel_pch = PCH_NONE;
		}
	}
}

//------------------------------------------------------------------------------
// void AppleIntelInfo::reportIntelRegs(void)

DEFINE_FUNC_VOID(reportIntelRegs)
{
	getPCHDeviceID();

	outl(0xcf8, 0x80001010);
	UInt64 mmio = (inl(0xcfc) & 0x7FFFC00000); // mask bits 38-22

	IOPhysicalAddress address = (IOPhysicalAddress)(mmio);

	// 4 MB combined for MMIO (2 MB) and Global GTT table aperture (2 MB).
	IOMemoryDescriptor * memDescriptor = IOMemoryDescriptor::withPhysicalAddress(address, 0x400000, kIODirectionInOut);

	if (memDescriptor != NULL)
	{
		IOReturn result = memDescriptor->prepare();

		if (result == kIOReturnSuccess)
		{
			IOMemoryMap * memoryMap = memDescriptor->map();

			if (memoryMap != NULL)
			{
				int64_t mmio = memoryMap->getVirtualAddress();
				gMMIOAddress = mmio;

				IOLog("CPU_VGACNTRL...............: 0x%X\n", MMIO_READ32(CPU_VGACNTRL));

				if (IS_HASWELL(devid) || IS_BROADWELL(devid))
				{
					IOLog("IS_HASWELL(devid) || IS_BROADWELL(devid)\n");
					intel_dump_regs(haswell_debug_regs);
				}
				else if (IS_GEN5(devid) || IS_GEN6(devid) || IS_IVYBRIDGE(devid))
				{
					IOLog("IS_GEN5(devid) || IS_GEN6(devid) || IS_IVYBRIDGE(devid)\n");
					intel_dump_regs(ironlake_debug_regs);
				}
				else if (IS_945GM(devid))
				{
					IOLog("IS_945GM(devid)\n");
					intel_dump_regs(i945gm_mi_regs);
					intel_dump_regs(intel_debug_regs);
					intel_dump_other_regs();
				}
				else
				{
					IOLog("IS_ELSE(devid)\n");
					intel_dump_regs(intel_debug_regs);
					intel_dump_other_regs();
				}
				
				if (IS_GEN6(devid) || IS_GEN7(devid))
				{
					IOLog("IS_GEN6(devid) || IS_GEN7(devid)\n");
					intel_dump_regs(gen6_fences);
					intel_dump_regs(gen6_rp_debug_regs);
				}

				memoryMap->release();
				memoryMap = NULL;
			}
		}

		memDescriptor->release();
		memDescriptor = NULL;
	}
}

