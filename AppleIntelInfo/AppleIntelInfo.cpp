/*
 * Copyright (c) 2012-2016 Pike R. Alpha. All rights reserved.
 *
 * Original idea and initial development of MSRDumper.kext (c) 2011 by RevoGirl.
 *
 * Thanks to George for his help and continuation of Sam's work, but it is
 * time for us to push the envelope and add some really interesting stuff.
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial
 * 4.0 Unported License => http://creativecommons.org/licenses/by-nc/4.0
 */

#include "AppleIntelInfo.h"

#if WRITE_LOG_REPORT
//==============================================================================

int AppleIntelInfo::writeReport(void)
{
	int error = 0;
	int length = (int)strlen(logBuffer);

	struct vnode * vp;
	
	if (mCtx)
	{
		if ((error = vnode_open(FILE_PATH, (O_TRUNC | O_CREAT | FWRITE | O_NOFOLLOW), S_IRUSR | S_IWUSR, VNODE_LOOKUP_NOFOLLOW, &vp, mCtx)))
		{
			IOLOG("AppleIntelInfo.kext: Error, vnode_open(%s) failed with error %d!\n", FILE_PATH, error);
		}
		else
		{
			if ((error = vnode_isreg(vp)) == VREG)
			{
				if ((error = vn_rdwr(UIO_WRITE, vp, logBuffer, length, reportFileOffset, UIO_SYSSPACE, IO_NOCACHE|IO_NODELOCKED|IO_UNIT, vfs_context_ucred(mCtx), (int *) 0, vfs_context_proc(mCtx))))
				{
					IOLOG("AppleIntelInfo.kext: Error, vn_rdwr(%s) failed with error %d!\n", FILE_PATH, error);
				}
				else
				{
					reportFileOffset += length;
				}
			}
			else
			{
				IOLOG("AppleIntelInfo.kext: Error, vnode_isreg(%s) failed with error %d!\n", FILE_PATH, error);
			}
		
			if ((error = vnode_close(vp, FWASWRITTEN, mCtx)))
			{
				IOLOG("AppleIntelInfo.kext: Error, vnode_close() failed with error %d!\n", error);
			}
		}
	}
	else
	{
		IOLOG("AppleIntelInfo.kext: mCtx == NULL!\n");
		error = 0xFFFF;
	}
	
	return error;
}
#endif


//==============================================================================

void AppleIntelInfo::reportHWP(void)
{
	uint32_t cpuid_reg[4];
	unsigned long long msr;
	unsigned long long hwp_enabled;

	do_cpuid(0x00000006, cpuid_reg);

	if ((cpuid_reg[eax] & 0x80) == 0x80)
	{
		gHwpEnabled = (rdmsr64(IA32_PM_ENABLE) & 1);

		if (gHwpEnabled)
		{
			short mantissa	= 0;
			short exponent	= 0;

			if (gCpuModel == CPU_MODEL_SKYLAKE || gCpuModel == CPU_MODEL_SKYLAKE_DT)
			{
				UInt64 pPerf = rdmsr64(IA32_PPERF);
				UInt64 aPerf = rdmsr64(IA32_APERF);
				float busy = ((pPerf * 100) / aPerf);
				UInt8 multiplier = (UInt8)(((gClockRatio + 0.5) * busy) / 100);

				IOLOG("MSR_PPERF........................(0x63E) : 0x%llX (%d)\n", msr, multiplier);
			}

			IOLOG("\nIA32_PM_ENABLE...................(0x770) : 0x%llX ", hwp_enabled);
			IOLOG("(HWP Enabled)\n");
			
			msr = rdmsr64(IA32_HWP_CAPABILITIES);

			IOLOG("\nIA32_HWP_CAPABILITIES............(0x771) : 0x%llX\n", msr);
			IOLOG("-----------------------------------------\n");
			IOLOG(" - Highest Performance.................. : %llu\n", bitfield32(msr, 7, 0));
			IOLOG(" - Guaranteed Performance............... : %llu\n", bitfield32(msr, 15, 8));
			IOLOG(" - Most Efficient Performance........... : %llu\n", bitfield32(msr, 23, 16));
			IOLOG(" - Lowest Performance................... : %llu\n", bitfield32(msr, 31, 24));

			if ((cpuid_reg[eax] & 0x800) == 0x800)
			{
				msr = rdmsr64(IA32_HWP_REQUEST_PKG);
				
				IOLOG("\nIA32_HWP_REQUEST_PKG.............(0x772) : 0x%llX\n", msr);
				IOLOG("-----------------------------------------\n");
				IOLOG(" - Minimum Performance.................. : %llu\n", bitfield32(msr, 7, 0));
				IOLOG(" - Maximum Performance.................. : %llu\n", bitfield32(msr, 15, 8));
				IOLOG(" - Desired Performance.................. : %llu\n", bitfield32(msr, 23, 16));
				IOLOG(" - Energy Efficient Performance......... : %llu\n", bitfield32(msr, 31, 24));

				mantissa = bitfield32(msr, 38, 32);
				exponent = bitfield32(msr, 41, 39);

				IOLOG(" - Activity Window...................... : %d, %d\n", mantissa, exponent);
			}
			
			if ((cpuid_reg[eax] & 0x100) == 0x100)
			{
				msr = rdmsr64(IA32_HWP_INTERRUPT);

				IOLOG("\nIA32_HWP_INTERRUPT...............(0x773) : 0x%llX\n", msr);
				IOLOG("------------------------------------------\n");
				IOLOG(" - Guaranteed Performance Change........ : %s\n", (msr & 1) ? "1 (Interrupt generated on change of)": "0 (Interrupt generation disabled)");
				IOLOG(" - Excursion Minimum.................... : %s\n", (msr & 2) ? "1 (Interrupt generated when unable to meet)": "0 (Interrupt generation disabled)");
			}
			
			msr = rdmsr64(IA32_HWP_REQUEST);
			
			IOLOG("\nIA32_HWP_REQUEST................(0x774) : 0x%llX\n", msr);
			IOLOG("-----------------------------------------\n");
			IOLOG(" - Minimum Performance................. : %llu\n", bitfield32(msr, 7, 0));
			IOLOG(" - Maximum Performance................. : %llu\n", bitfield32(msr, 15, 8));
			IOLOG(" - Desired Performance................. : %llu\n", bitfield32(msr, 23, 16));
			IOLOG(" - Energy Efficient Performance........ : %llu\n", bitfield32(msr, 31, 24));
			
			mantissa = bitfield32(msr, 38, 32);
			exponent = bitfield32(msr, 41, 39);

			IOLOG(" - Activity Window..................... : %d, %d\n", mantissa, exponent);
			IOLOG(" - Package Control..................... : %s\n", (msr & 0x40000000000) ? "1 (control inputs to be derived from IA32_HWP_REQUEST_PKG)": "0");
			
			msr = rdmsr64(IA32_HWP_STATUS);

			IOLOG("\nIA32_HWP_STATUS..................(0x777) : 0x%llX\n", msr);
			IOLOG("-----------------------------------------\n");
			IOLOG(" - Guaranteed Performance Change....... : %s\n", (msr & 1) ? "1 (has occured)" : "0 (has not occured)");
			IOLOG(" - Excursion To Minimum................ : %s\n", (msr & 4) ? "1 (has occured)" : "0 (has not occured)");
		}
		else
		{
			IOLOG("(HWP Disabled)\n");
		}
	}
}


//==============================================================================

void AppleIntelInfo::reportHDC(void)
{
	uint8_t index = 0;
	unsigned long long msr;

	IOLOG("HDC Supported\n");

	msr = rdmsr64(IA32_PKG_HDC_CTL);

	IOLOG("\nIA32_PKG_HDC_CTL.................(0xDB0) : 0x%llX\n", msr);

	if (msr)
	{
		IOLOG("------------------------------------------\n");
		IOLOG("HDC Pkg Enable...................(0x652) : %s\n", bitfield32(msr, 0, 0) ? "1 (HDC allowed)" : "0 (HDC not allowed)");
	}

	msr = rdmsr64(IA32_PM_CTL1);

	IOLOG("\nIA32_PM_CTL1.....................(0xDB1) : 0x%llX\n", msr);
	
	if (msr)
	{
		IOLOG("------------------------------------------\n");
		IOLOG("HDC Allow Block..................(0xDB1) : %s\n", bitfield32(msr, 0, 0) ? "1 (HDC blocked)" : "0 (HDC not blocked/allowed)");
	}

	msr = rdmsr64(IA32_THREAD_STALL);
	
	IOLOG("\nIA32_THREAD_STALL................(0xDB2) : 0x%llX\n", msr);
	
	if (msr)
	{

		IOLOG("------------------------------------------\n");
		IOLOG("Stall Cycle Counter...............(0xDB2) : %llu, %s\n", msr, msr ? "1 (forced-idle supported)" : "0 (forced-idle not supported)");
	}

	msr = rdmsr64(MSR_PKG_HDC_CONFIG);
	index = bitfield32(msr, 2, 0);

	IOLOG("\nMSR_PKG_HDC_CONFIG...............(0x652) : 0x%llX\n", msr);

	if (msr)
	{
		const char * cxCountText[5] = { "no-counting", "count package C2 only", "count package C3 and deeper", "count package C6 and deeper", "count package C7 and deeper" };

		IOLOG("------------------------------------------\n");
		IOLOG("Pkg Cx Monitor ..................(0x652) : %d (%s)", index, cxCountText[index]);
	}
	
	msr = rdmsr64(MSR_CORE_HDC_RESIDENCY);

	IOLOG("\nMSR_CORE_HDC_RESIDENCY...........(0x653) : 0x%llX\n", msr);
	
	if (msr)
	{
		IOLOG("------------------------------------------\n");
		IOLOG("Core Cx Duty Cycle Count................ : %llu %s\n", msr, msr ? "(forced-idle cycle count)": "(not supported/no forced-idle serviced)");
		
	}
	
	msr = rdmsr64(MSR_PKG_HDC_SHALLOW_RESIDENCY);

	IOLOG("\nMSR_PKG_HDC_SHALLOW_RESIDENCY....(0x655) : 0x%llX\n", msr);
	
	if (msr)
	{

		IOLOG("------------------------------------------\n");
		IOLOG("Pkg C2 Duty Cycle Count................. : %llu %s\n", msr, msr ? "(forced-idle cycle count)": "(not supported/no forced-idle serviced)");
		
	}
	
	msr = rdmsr64(MSR_PKG_HDC_DEEP_RESIDENCY);

	IOLOG("\nMSR_PKG_HDC_DEEP_RESIDENCY.......(0x656) : 0x%llX\n", msr);
	
	if (msr)
	{
		const char * cxText[5] = { "x", "2", "3", "6", "7" };

		IOLOG("------------------------------------------\n");
		IOLOG("Pkg C%s Duty Cycle Count................ : %llu %s\n", cxText[index], msr, msr ? "(forced-idle cycle count)": "(not supported/no forced-idle serviced)");
		
	}
}

//==============================================================================

uint32_t AppleIntelInfo::getBusFrequency(void)
{
	size_t size = 4;
	uint32_t frequency = 0;

	switch (gCpuModel)
	{
		case CPU_MODEL_NEHALEM:
		case CPU_MODEL_FIELDS:
		case CPU_MODEL_DALES_32NM:
		case CPU_MODEL_WESTMERE:
		case CPU_MODEL_NEHALEM_EX:
		case CPU_MODEL_WESTMERE_EX:
			return (133 * 1000000);
			break;

		default:
			// Check sysctl hw.busfrequency to see if the setup is right or wrong.
			if (sysctlbyname("hw.busfrequency", &frequency, &size, NULL, 0) == 0)
			{
				// Is it 100000000?
				if ((frequency / 1000000) > 100)
				{
					// No. Log warning.
					IOLOG("\nWarning: Clover hw.busfrequency error detected : %x\n", frequency);
				}
			}

			return (100 * 1000000);
			break;
	}
	return 0;
}


//==============================================================================

const char * AppleIntelInfo::getUnitText(uint8_t unit)
{
	const char * milliText[10] = { "1 ", "500 milli-", "250 milli-", "125 milli-", "62.5 milli-", "31.2 milli-", "15.6 milli-", "7.8 milli-", "3.9 milli-", "2 milli-" };
	const char * microText[7] = { "976.6 micro-", "488.3 micro-", "244.1 micro-", "122.1 micro-", "61 micro-", "30.5 micro-", "15.3 micro-" };

	if (unit <= 9)
	{
		return milliText[unit];
	}
	else
	{
		return microText[unit-10];
	}

	return NULL;
}

//==============================================================================

void AppleIntelInfo::reportMSRs(void)
{
	uint8_t core_limit;
	uint32_t performanceState;
	uint32_t cpuid_reg[4];
	uint64_t msr;

	char brandstring[48];

	do_cpuid(0x80000002, cpuid_reg);	// First 16 bytes.
	bcopy((char *)cpuid_reg, &brandstring[0], 16);
 
	do_cpuid(0x80000003, cpuid_reg);	// Next 16 bytes.
	bcopy((char *)cpuid_reg, &brandstring[16], 16);
	
	do_cpuid(0x80000004, cpuid_reg);	// Last 16 bytes.
	bcopy((char *)cpuid_reg, &brandstring[32], 16);
	
	IOLOG("\nProcessor Brandstring....................: %s\n", brandstring);

	do_cpuid(0x00000001, cpuid_reg);
	uint8_t model = (bitfield32(cpuid_reg[eax], 19, 16) << 4) + bitfield32(cpuid_reg[eax],  7,  4);

	IOLOG("\nProcessor Signature..................... : 0x%X\n", cpuid_reg[eax]);
	IOLOG("------------------------------------------\n");
	IOLOG(" - Family............................... : %lu\n", bitfield32(cpuid_reg[eax], 11,  8));
	IOLOG(" - Stepping............................. : %lu\n", bitfield32(cpuid_reg[eax],  3,  0));
	IOLOG(" - Model................................ : 0x%X (%d)\n", model, model);

	do_cpuid(0x00000006, cpuid_reg);

	IOLOG("\nModel Specific Registers (MSRs)\n------------------------------------------\n");

	IOLOG("\nMSR_CORE_THREAD_COUNT............(0x35)  : 0x%llX\n", msr);
	IOLOG("------------------------------------------\n");
	IOLOG(" - Core Count........................... : %d\n", gCoreCount);
	IOLOG(" - Thread Count......................... : %d\n", gThreadCount);

	msr = rdmsr64(MSR_PLATFORM_INFO);
	performanceState = bitfield32(msr, 15, 8);

	IOLOG("\nMSR_PLATFORM_INFO................(0xCE)  : 0x%llX\n", msr);
	IOLOG("------------------------------------------\n");
	IOLOG(" - Maximum Non-Turbo Ratio.............. : 0x%X (%u MHz)\n", performanceState, (performanceState * gBclk));
	IOLOG(" - Ratio Limit for Turbo Mode........... : %s\n", (msr & (1 << 28)) ? "1 (programmable)" : "0 (not programmable)");
	IOLOG(" - TDP Limit for Turbo Mode............. : %s\n", (msr & (1 << 29)) ? "1 (programmable)" : "0 (not programmable)");
	IOLOG(" - Low Power Mode Support............... : %s\n", (msr & (1UL << 32)) ? "1 (LPM supported)": "0 (LMP not supported)");

	if (bitfield32(msr, 34, 33) == 0)
	{
		IOLOG(" - Number of ConfigTDP Levels........... : 0 (only base TDP level available)\n");
	}
	else
	{
		IOLOG(" - Number of ConfigTDP Levels........... : %llu (additional TDP level(s) available)\n", bitfield32(msr, 34, 33));
	}

	IOLOG(" - Maximum Efficiency Ratio............. : %llu\n", bitfield32(msr, 47, 40));
	
	if (bitfield32(msr, 55, 48) > 0)
	{
		IOLOG(" - Minimum Operating Ratio.............. : %llu\n", bitfield32(msr, 55, 48));
	}

	UInt64 msr_pmg_cst_config_control = rdmsr64(MSR_PKG_CST_CONFIG_CONTROL);

	IOLOG("\nMSR_PMG_CST_CONFIG_CONTROL.......(0xE2)  : 0x%llX\n", msr_pmg_cst_config_control);
	IOLOG("------------------------------------------\n");
	IOLOG(" - I/O MWAIT Redirection Enable......... : %s\n", (msr_pmg_cst_config_control & (1 << 10)) ? "1 (enabled, IO read of MSR(0xE4) mapped to MWAIT)" : "0 (not enabled)");
	IOLOG(" - CFG Lock............................. : %s\n", (msr_pmg_cst_config_control & (1 << 15)) ? "1 (MSR locked until next reset)" : "0 (MSR not locked)");

	IOLOG(" - C3 State Auto Demotion............... : %s\n", (msr_pmg_cst_config_control & (1 << 25)) ? "1 (enabled)" : "0 (disabled/unsupported)");
	IOLOG(" - C1 State Auto Demotion............... : %s\n", (msr_pmg_cst_config_control & (1 << 26)) ? "1 (enabled)" : "0 (disabled/unsupported)");

	IOLOG(" - C3 State Undemotion.................. : %s\n", (msr_pmg_cst_config_control & (1 << 27)) ? "1 (enabled)" : "0 (disabled/unsupported)");
	IOLOG(" - C1 State Undemotion.................. : %s\n", (msr_pmg_cst_config_control & (1 << 28)) ? "1 (enabled)" : "0 (disabled/unsupported)");

	// Intel® CoreTM M Processors and 5th Generation Intel® CoreTM Processors
	// Intel® Xeon® Processor D and Intel Xeon Processors E5 v4 Family Based on the Broadwell Microarchitecture
	IOLOG(" - Package C-State Auto Demotion........ : %s\n", (msr_pmg_cst_config_control & (1 << 29)) ? "1 (enabled)" : "0 (disabled/unsupported)");
	IOLOG(" - Package C-State Undemotion........... : %s\n", (msr_pmg_cst_config_control & (1 << 30)) ? "1 (enabled)" : "0 (disabled/unsupported)");

	msr = rdmsr64(MSR_PMG_IO_CAPTURE_BASE);

	IOLOG("\nMSR_PMG_IO_CAPTURE_BASE..........(0xE4)  : 0x%llX\n", msr);

	if (msr)
	{
		IOLOG("------------------------------------------\n");
		IOLOG(" - LVL_2 Base Address................... : 0x%llx\n", bitfield32(msr, 15, 0));
	}

	if (msr_pmg_cst_config_control & (1 << 10))
	{
		switch(bitfield32(msr, 18, 16))
		{
			case 0: IOLOG(" - C-state Range........................ : %llu (%s)\n", bitfield32(msr, 18, 16), "C3 is the max C-State to include");
				break;

			case 1: IOLOG(" - C-state Range........................ : %llu (%s)\n", bitfield32(msr, 18, 16), "C6 is the max C-State to include");
				break;

			case 2: IOLOG(" - C-state Range........................ : %llu (%s)\n", bitfield32(msr, 18, 16), "C7 is the max C-State to include");
				break;
		}
	}
	else
	{
		IOLOG(" - C-state Range........................ : %llu (%s)\n", bitfield32(msr, 18, 16), "C-States not included, I/O MWAIT redirection not enabled");
	}

	IOLOG("\nIA32_MPERF.......................(0xE7)  : 0x%llX\n", (unsigned long long)rdmsr64(IA32_MPERF));
	
	UInt64 aPerf = rdmsr64(IA32_APERF);

	IOLOG("IA32_APERF.......................(0xE8)  : 0x%llX\n", aPerf);

	if (gCpuModel == CPU_MODEL_BROADWELL_E)
	{
		IOLOG("MSR_0x150........................(0x150) : 0x%llX\n", (unsigned long long)rdmsr64(0x150));
	}

	IOLOG("\nMSR_FLEX_RATIO...................(0x194) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_FLEX_RATIO));

	if (msr)
	{
		IOLOG("------------------------------------------\n");
	}

	msr = rdmsr64(MSR_IA32_PERF_STATUS);
	performanceState = bitfield32(msr, 15, 0);

	IOLOG("\nMSR_IA32_PERF_STATUS.............(0x198) : 0x%llX\n", msr);

	if (msr)
	{
		IOLOG("------------------------------------------\n");
		IOLOG(" - Current Performance State Value...... : 0x%X (%u MHz)\n", performanceState, ((performanceState >> 8) * gBclk));
	}

	msr = rdmsr64(MSR_IA32_PERF_CONTROL);
	performanceState = bitfield32(msr, 15, 0);

	IOLOG("\nMSR_IA32_PERF_CONTROL............(0x199) : 0x%llX\n", msr);

	if (msr)
	{
		IOLOG("------------------------------------------\n");
		IOLOG(" - Target performance State Value....... : 0x%X (%u MHz)\n", performanceState, ((performanceState >> 8) * gBclk));
	}

	if (cpuid_reg[eax] & (1 << 0))
	{
		IOLOG(" - Intel Dynamic Acceleration........... : %s\n", (msr & (1UL << 32)) ? "1 (IDA disengaged)" : "0 (IDA engaged)");
	}

	IOLOG("\nIA32_CLOCK_MODULATION............(0x19A) : 0x%llX\n", (unsigned long long)rdmsr64(IA32_CLOCK_MODULATION));
	IOLOG("IA32_THERM_STATUS................(0x19C) : 0x%llX\n", (unsigned long long)rdmsr64(IA32_THERM_STATUS));

	msr = rdmsr64(IA32_MISC_ENABLES);

	IOLOG("\nIA32_MISC_ENABLES................(0x1A0) : 0x%llX\n", msr);

	if (msr)
	{
		IOLOG("------------------------------------------\n");
		IOLOG(" - Fast-Strings......................... : %s\n", (msr & (1 <<  0)) ? "1 (enabled)" : "0 (disabled)");
		IOLOG(" - Automatic Thermal Control Circuit.... : %s\n", (msr & (1 <<  3)) ? "1 (enabled)" : "0 (disabled)");
		IOLOG(" - Performance Monitoring............... : %s\n", (msr & (1 <<  7)) ? "1 (available)" : "not available");
		IOLOG(" - Processor Event Based Sampling....... : %s\n", (msr & (1 << 12)) ? "1 (PEBS not supported)" : "0 (PEBS supported)");
		IOLOG(" - Enhanced Intel SpeedStep Technology.. : %s\n", (msr & (1 << 16)) ? "1 (enabled)" : "0 (disabled)");
		IOLOG(" - MONITOR FSM.......................... : %s\n", (msr & (1 << 18)) ? "1 (MONITOR/MWAIT supported)" : "0 (MONITOR/MWAIT not supported)");
		IOLOG(" - CFG Lock............................. : %s\n", (msr & (1 << 20)) ? "1 (MSR locked until next reset)" : "0 (MSR not locked)");
	}

	msr = rdmsr64(MSR_TEMPERATURE_TARGET);
	uint8_t time_unit = bitfield32(msr, 6, 0);

	IOLOG("\nMSR_TEMPERATURE_TARGET...........(0x1A2) : 0x%llX\n", msr);
	
	if (msr)
	{
		char timeString[25];
		memset(timeString, 0, 25);
		IOLOG("------------------------------------------\n");

		if (time_unit)
		{
			snprintf(timeString, 25, "(%sSeconds)", getUnitText(time_unit));
		}

		IOLOG(" - Turbo Attenuation Units.............. : %u %s\n", time_unit, timeString);
		IOLOG(" - Temperature Target................... : %llu\n", bitfield32(msr, 23, 16));
		IOLOG(" - TCC Activation Offset................ : %llu\n", bitfield32(msr, 29, 24));
	}

	msr = rdmsr64(MSR_MISC_PWR_MGMT);

	IOLOG("\nMSR_MISC_PWR_MGMT................(0x1AA) : 0x%llX\n", msr);

	if (msr)
	{
		IOLOG("------------------------------------------\n");
		IOLOG(" - EIST Hardware Coordination........... : %s\n", (msr & (1 <<  0)) ? "1 (hardware coordination disabled)" : "0 (hardware coordination enabled)");

		IOLOG(" - Energy/Performance Bias support...... : %lu\n", bitfield32(cpuid_reg[ecx],  3,  3) );
		IOLOG(" - Energy/Performance Bias.............. : %s\n", (msr & (1 <<  1)) ? "1 (enabled/MSR visible to software)" : "0 (disabled/MSR not visible to software)");

		IOLOG(" - Thermal Interrupt Coordination Enable : %s\n", (msr & (1 << 22)) ? "1 (thermal interrupt routed to all cores)" : "0 (thermal interrupt not rerouted)");
	}

	msr = rdmsr64(MSR_TURBO_RATIO_LIMIT);

	IOLOG("\nMSR_TURBO_RATIO_LIMIT............(0x1AD) : 0x%llX\n", msr);
	IOLOG("------------------------------------------\n");

	for (int i = 1; (i < 9) && (i <= gCoreCount); i++)
	{
		core_limit = bitfield32(msr, 7, 0);
		
		if (core_limit)
		{
			IOLOG(" - Maximum Ratio Limit for C%02d.......... : %X (%u MHz) %s\n", i, core_limit, (core_limit * gBclk), ((i > gCoreCount) && core_limit) ? "(garbage / unused)" : "");

			msr = (msr >> 8);
		}
	}
	//
	// Intel® Xeon® Processor E5 v3 Family
	//
	if (gCoreCount > 8)
	{
		msr = rdmsr64(MSR_TURBO_RATIO_LIMIT1);
	
		IOLOG("\nMSR_TURBO_RATIO_LIMIT1...........(0x1AE) : 0x%llX\n", msr);
		IOLOG("------------------------------------------\n");
	
		for (int i = 9; (i < 17) && (i <= gCoreCount); i++)
		{
			core_limit = bitfield32(msr, 7, 0);
		
			if (core_limit)
			{
				IOLOG(" - Maximum Ratio Limit for C%02d.......... : %X (%u MHz) %s\n", i, core_limit, (core_limit * gBclk), ((i > gCoreCount) && core_limit) ? "(garbage / unused)" : "");
		
				msr = (msr >> 8);
			}
		}
	}
	//
	// Intel® Xeon® Processor E5 v3 Family
	//
	if (gCoreCount > 16)
	{
		msr = rdmsr64(MSR_TURBO_RATIO_LIMIT2);
	
		IOLOG("\nMSR_TURBO_RATIO_LIMIT2...........(0x1AF) : 0x%llX\n", msr);
		IOLOG("------------------------------------------\n");
	
		for (int i = 17; (i < 33) && (i <= gCoreCount); i++)
		{
			core_limit = bitfield32(msr, 7, 0);
		
			if (core_limit)
			{
				IOLOG(" - Maximum Ratio Limit for C%02d.......... : %X (%u MHz) %s\n", i, core_limit, (core_limit * gBclk), ((i > gCoreCount) && core_limit) ? "(garbage / unused)" : "");
		
				msr = (msr >> 8);
			}
		}
	}
	
	if (bitfield32(cpuid_reg[ecx], 3, 3) == 1)
	{
		msr = rdmsr64(IA32_ENERGY_PERF_BIAS);

		IOLOG("\nIA32_ENERGY_PERF_BIAS............(0x1B0) : 0x%llX\n", msr);
		
		if (msr)
		{
			IOLOG("------------------------------------------\n");
		
			switch(bitfield32(msr, 3, 0))
			{
				case 0:
				case 1:
					IOLOG(" - Power Policy Preference...............: %llu (%s)\n", bitfield32(msr, 3, 0), "highest performance");
					break;

				case 5:
					IOLOG(" - Power Policy Preference...............: %llu (%s)\n", bitfield32(msr, 3, 0), "balanced performance and energy saving");
					break;

				case 15:
					IOLOG(" - Power Policy Preference...............: %llu (%s)\n", bitfield32(msr, 3, 0), "maximize energy saving");
					break;
			}
		}
	}

	msr = rdmsr64(MSR_POWER_CTL);

	IOLOG("\nMSR_POWER_CTL....................(0x1FC) : 0x%llX\n", msr);
	
	if (msr)
	{
		IOLOG("------------------------------------------\n");
		IOLOG(" - C1E Enable............................: %llu %s\n", bitfield32(msr, 1, 1), bitfield32(msr, 1, 1) ? "(enabled)": "(disabled)");
	}

	msr = rdmsr64(MSR_RAPL_POWER_UNIT);

	uint8_t power_unit = bitfield32(msr, 3, 0);
	uint8_t energy_status_unit = bitfield32(msr, 12, 8);
	time_unit = bitfield32(msr, 19, 16);

	float joulesPerEnergyUnit = 1. / float(1ULL << energy_status_unit);

	unsigned int Y = 0;
	unsigned int Z = 0;

	IOLOG("\nMSR_RAPL_POWER_UNIT..............(0x606) : 0x%llX\n", msr);

	if (msr)
	{
		IOLOG("------------------------------------------\n");
		IOLOG(" - Power Units.......................... : %u (1/%d Watt)\n", power_unit, (1 << power_unit));
		IOLOG(" - Energy Status Units.................. : %u (%sJoules)\n", energy_status_unit, getUnitText(energy_status_unit));
		IOLOG(" - Time Units .......................... : %u (%sSeconds)\n", time_unit, getUnitText(time_unit));
	}

	msr = rdmsr64(MSR_PKG_POWER_LIMIT);

	IOLOG("\nMSR_PKG_POWER_LIMIT..............(0x610) : 0x%llX\n", msr);

	if (msr)
	{
		IOLOG("------------------------------------------\n");
		IOLOG(" - Package Power Limit #1............... : %llu Watt\n", (bitfield32(msr, 14, 0) >> power_unit));
		IOLOG(" - Enable Power Limit #1................ : %s\n", bitfield32(msr, 15, 15) ? "1 (enabled)": "0 (disabled)");
		IOLOG(" - Package Clamping Limitation #1....... : %s\n", bitfield32(msr, 16, 16) ? "1 (allow going below OS-requested P/T state during Time Window for Power Limit #1)": "0 (disabled)");

		Y = bitfield32(msr, 21, 17);
		Z = bitfield32(msr, 23, 22);

		IOLOG(" - Time Window for Power Limit #1....... : %llu (%u milli-Seconds)\n", bitfield32(msr, 23, 17), (unsigned int)(((1 << Y) * (1.0 + Z) / 4.0) * time_unit));
		IOLOG(" - Package Power Limit #2............... : %llu Watt\n", (bitfield32(msr, 46, 32) >> power_unit));
		IOLOG(" - Enable Power Limit #2................ : %s\n", (msr & (1UL << 47)) ? "1 (enabled)": "0 (disabled)");
		IOLOG(" - Package Clamping Limitation #2....... : %s\n", (msr & (1UL << 48)) ? "1 (allow going below OS-requested P/T state setting Time Window for Power Limit #2)": "0 (disabled)");

		Y = bitfield32(msr, 53, 49);
		Z = bitfield32(msr, 55, 54);
	
		IOLOG(" - Time Window for Power Limit #2....... : %llu (%u milli-Seconds)\n", bitfield32(msr, 55, 49), (unsigned int)(((1 << Y) * (1.0 + Z) / 4.0) * time_unit));
		IOLOG(" - Lock................................. : %s\n", bitfield32(msr, 63, 63) ? "1 (MSR locked until next reset)": "0 (MSR not locked)");
	}

	msr = rdmsr64(MSR_PKG_ENERGY_STATUS);

	IOLOG("\nMSR_PKG_ENERGY_STATUS............(0x611) : 0x%llX\n", msr);

	if (msr)
	{
		IOLOG("------------------------------------------\n");
		IOLOG(" - Total Energy Consumed................ : %llu Joules (Watt = Joules / seconds)\n", (long long unsigned)(bitfield32(msr, 31, 0) * joulesPerEnergyUnit));
	}

	msr = rdmsr64(MSR_PKG_POWER_INFO);

	IOLOG("\nMSR_PKG_POWER_INFO...............(0x614) : 0x%llX\n", msr);

	if (msr)
	{
		IOLOG("------------------------------------------\n");
		IOLOG(" - Thermal Spec Power................... : %llu Watt\n", (bitfield32(msr, 14, 0) >> power_unit));
		IOLOG(" - Minimum Power........................ : %llu\n", (bitfield32(msr, 16, 30) >> power_unit));
		IOLOG(" - Maximum Power........................ : %llu\n", (bitfield32(msr, 46, 32) >> power_unit));
		IOLOG(" - Maximum Time Window.................. : %llu\n", (bitfield32(msr, 58, 48) >> time_unit));
	}

	if (gCpuModel == CPU_MODEL_SB_CORE) // 0x2A - Intel 325462.pdf Vol.3C 35-120
	{
		IOLOG("\nMSR_PP0_CURRENT_CONFIG...........(0x601) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PP0_CURRENT_CONFIG));
	}

	msr = rdmsr64(MSR_PP0_POWER_LIMIT);

	IOLOG("\nMSR_PP0_POWER_LIMIT..............(0x638) : 0x%llX\n", msr);

	if (msr)
	{
		IOLOG("------------------------------------------\n");
		IOLOG(" - Power Limit.......................... : %llu Watt\n", (bitfield32(msr, 14, 0) >> power_unit));
		IOLOG(" - Enable Power Limit................... : %s\n", (msr & (1UL << 15)) ? "1 (enabled)": "0 (disabled)");
		IOLOG(" - Clamping Limitation.................. : %s\n", (msr & (1UL << 16)) ? "1 (allow going below OS-requested P/T state setting Time Window for Power Limit #2)": "0 (disabled)");
	
		Y = bitfield32(msr, 21, 17);
		Z = bitfield32(msr, 23, 22);

		IOLOG(" - Time Window for Power Limit.......... : %llu (%u milli-Seconds)\n", bitfield32(msr, 23, 17), (unsigned int)(((1 << Y) * (1.0 + Z)) * time_unit));
		IOLOG(" - Lock................................. : %s\n", bitfield32(msr, 31, 31) ? "1 (MSR locked until next reset)": "0 (MSR not locked)");
	}

	msr = rdmsr64(MSR_PP0_ENERGY_STATUS);

	IOLOG("\nMSR_PP0_ENERGY_STATUS............(0x639) : 0x%llX\n", msr);

	if (msr)
	{
		IOLOG("------------------------------------------\n");
		IOLOG(" - Total Energy Consumed................ : %llu Joules (Watt = Joules / seconds)\n", (long long unsigned)(bitfield32(msr, 31, 0) * joulesPerEnergyUnit));
	}

	if (gCpuModel == CPU_MODEL_SB_CORE) // 0x2A - Intel 325462.pdf Vol.3C 35-120
	{
		msr = rdmsr64(MSR_PP0_POLICY);

		IOLOG("\nMSR_PP0_POLICY...................(0x63a) : 0x%llX\n", msr);

		if (msr)
		{
			IOLOG("------------------------------------------\n");
			IOLOG(" - Priority Level....................... : %llu\n", bitfield32(msr, 4, 0));
		}
	}

	IOLOG("\nMSR_TURBO_ACTIVATION_RATIO.......(0x64C) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_TURBO_ACTIVATION_RATIO));

#if REPORT_IGPU_P_STATES
	if (igpuEnabled)
	{
//		IOLOG("MSR_PP1_CURRENT_CONFIG...........(0x602) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PP1_CURRENT_CONFIG));

		switch (gCpuModel)
		{
			case CPU_MODEL_SB_CORE:				// 0x2A - Intel 325462.pdf Vol.3C 35-120
			case CPU_MODEL_IB_CORE:				// 0x3A - Intel 325462.pdf Vol.3C 35-125 (Referring to Table 35-13)
				IOLOG("MSR_PP1_CURRENT_CONFIG...........(0x602) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PP1_CURRENT_CONFIG));

			case CPU_MODEL_HASWELL:				// 0x3C - Intel 325462.pdf Vol.3C 35-140
			case CPU_MODEL_HASWELL_ULT:			// 0x45 - Intel 325462.pdf Vol.3C 35-140
			case CPU_MODEL_CRYSTALWELL:			// 0x46 - Intel 325462.pdf Vol.3C 35-140
			case CPU_MODEL_SKYLAKE:				// 0x4E -
			case CPU_MODEL_SKYLAKE_DT:			// 0x5E -

				msr = rdmsr64(MSR_PP1_POWER_LIMIT);

				IOLOG("\nMSR_PP1_POWER_LIMIT..............(0x640) : 0x%llX\n", msr);

				if (msr)
				{
					IOLOG("------------------------------------------\n");
					IOLOG(" - Power Limit.......................... : %llu Watt\n", (bitfield32(msr, 14, 0) >> power_unit));
					IOLOG(" - Enable Power Limit................... : %s\n", (msr & (1UL << 15)) ? "1 (enabled)": "0 (disabled)");
					IOLOG(" - Clamping Limitation.................. : %s\n", (msr & (1UL << 16)) ? "1 (allow going below OS-requested P/T state setting Time Window for Power Limit #2)": "0 (disabled)");
				
					Y = bitfield32(msr, 21, 17);
					Z = bitfield32(msr, 23, 22);
				
					IOLOG(" - Time Window for Power Limit.......... : %llu (%u milli-Seconds)\n", bitfield32(msr, 23, 17), (unsigned int)(((1 << Y) * (1.0 + Z)) * time_unit));
					IOLOG(" - Lock................................. : %s\n", bitfield32(msr, 31, 31) ? "1 (MSR locked until next reset)": "0 (MSR not locked)");
				}

				msr = rdmsr64(MSR_PP1_ENERGY_STATUS);

				IOLOG("\nMSR_PP1_ENERGY_STATUS............(0x641) : 0x%llX\n", msr);

				if (msr)
				{
					IOLOG("------------------------------------------\n");
					IOLOG(" - Total Energy Consumed................ : %llu Joules (Watt = Joules / seconds)\n", (long long unsigned)(bitfield32(msr, 31, 0) * joulesPerEnergyUnit));
				}

				msr = rdmsr64(MSR_PP1_POLICY);

				IOLOG("\nMSR_PP1_POLICY...................(0x642) : 0x%llX\n", msr);

				if (msr)
				{
					IOLOG("------------------------------------------\n");
					IOLOG(" - Priority Level....................... : %llu\n", bitfield32(msr, 4, 0));
				}

				break;
		}
	}
#endif
	
	IOLOG("\n");

	switch (gCpuModel)
	{
		case CPU_MODEL_IB_CORE:				// 0x3A - Intel 325462.pdf Vol.3C 35-126
		case CPU_MODEL_IB_CORE_EX:			// 0x3B - Intel 325462.pdf Vol.3C 35-126
		// case CPU_MODEL_IB_CORE_XEON:		// 0x3E - Intel 325462.pdf Vol.3C 35-126
		case CPU_MODEL_HASWELL:				// 0x3C - Intel 325462.pdf Vol.3C 35-133
		case CPU_MODEL_HASWELL_ULT:			// 0x45 - Intel 325462.pdf Vol.3C 35-133
		case CPU_MODEL_CRYSTALWELL:			// 0x46 - Intel 325462.pdf Vol.3C 35-133
		case CPU_MODEL_HASWELL_SVR:

			IOLOG("MSR_CONFIG_TDP_NOMINAL...........(0x648) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_CONFIG_TDP_NOMINAL));
			IOLOG("MSR_CONFIG_TDP_LEVEL1............(0x649) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_CONFIG_TDP_LEVEL1));
			IOLOG("MSR_CONFIG_TDP_LEVEL2............(0x64a) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_CONFIG_TDP_LEVEL2));
			IOLOG("MSR_CONFIG_TDP_CONTROL...........(0x64b) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_CONFIG_TDP_CONTROL));
			IOLOG("MSR_TURBO_ACTIVATION_RATIO.......(0x64c) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_TURBO_ACTIVATION_RATIO));
			break;
	}

	switch (gCpuModel)
	{
		case CPU_MODEL_SB_CORE:	
		case CPU_MODEL_IB_CORE:
		case CPU_MODEL_IB_CORE_EX:
		case CPU_MODEL_IB_CORE_XEON:
		case CPU_MODEL_HASWELL:
		case CPU_MODEL_HASWELL_ULT:
		case CPU_MODEL_CRYSTALWELL:
		case CPU_MODEL_BROADWELL_E:

			IOLOG("MSR_PKGC3_IRTL...................(0x60a) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKGC3_IRTL));
			break;

		case CPU_MODEL_NEHALEM:
		case CPU_MODEL_NEHALEM_EX:
			break;
	}

	IOLOG("MSR_PKGC6_IRTL...................(0x60b) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKGC6_IRTL));

#if REPORT_C_STATES
	if (gCheckC7)
	{
		IOLOG("MSR_PKGC7_IRTL...................(0x60c) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKGC7_IRTL));
	}
#endif

	switch (gCpuModel)
	{
		case CPU_MODEL_NEHALEM:
		case CPU_MODEL_NEHALEM_EX:
		case CPU_MODEL_BROADWELL_E:

			IOLOG("MSR_PKG_C2_RESIDENCY.............(0x60d) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C2_RESIDENCY));
			IOLOG("MSR_PKG_C3_RESIDENCY.............(0x3f8) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C3_RESIDENCY));
			break;

		default:

			IOLOG("MSR_PKG_C2_RESIDENCY.............(0x60d) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C2_RESIDENCY));
			/*
			 * Is package C3 auto-demotion/undemotion enabled i.e. is bit-25 or bit-27 set?
			 */
			if ((msr_pmg_cst_config_control & 0x2000000) || (msr_pmg_cst_config_control & 0x8000000))
			{
				IOLOG("MSR_PKG_C3_RESIDENCY.............(0x3f8) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C3_RESIDENCY));
			}
	}
	
	IOLOG("MSR_PKG_C6_RESIDENCY.............(0x3f9) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C6_RESIDENCY));

#if REPORT_C_STATES
	if (gCheckC7)
	{
		IOLOG("MSR_PKG_C7_RESIDENCY.............(0x3fa) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C7_RESIDENCY));
	}
#endif

	if (gCpuModel == CPU_MODEL_HASWELL_ULT) // 0x45 - Intel 325462.pdf Vol.3C 35-136
	{
		IOLOG("MSR_PKG_C8_RESIDENCY............(0x630) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C8_RESIDENCY));
		IOLOG("MSR_PKG_C9_RESIDENCY............(0x631) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C9_RESIDENCY));
		IOLOG("MSR_PKG_C10_RESIDENCY...........(0x632) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C10_RESIDENCY));
		
		IOLOG("MSR_PKG_C8_LATENCY..............(0x633) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C8_RESIDENCY));
		IOLOG("MSR_PKG_C9_LATENCY..............(0x634) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C9_RESIDENCY));
		IOLOG("MSR_PKG_C10_LATENCY.............(0x635) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C10_RESIDENCY));
	}

	if (gCpuModel == CPU_MODEL_SKYLAKE || gCpuModel == CPU_MODEL_SKYLAKE_DT)
	{
		msr = rdmsr64(MSR_PLATFORM_ENERGY_COUNTER);

		IOLOG("\nMSR_PLATFORM_ENERGY_COUNTER......(0x64D) : 0x%llX %s\n", bitfield32(msr, 31, 0), (bitfield32(msr, 31, 0) == 0) ? "(not supported by hardware/BIOS)" : "");

		if (msr)
		{
			IOLOG("------------------------------------------\n");
		}
		
		msr = rdmsr64(MSR_PPERF);

		IOLOG("\nMSR_PPERF........................(0x64E) : 0x%llX\n", msr);
		
		if (msr)
		{
			IOLOG("------------------------------------------\n");
			
			// busy = ((aPerf * 100) / msr);
			IOLOG(" - Hardware workload scalability........ : %llu\n", bitfield32(msr, 63, 0));
		}

		msr = rdmsr64(MSR_CORE_PERF_LIMIT_REASONS);

		IOLOG("\nMSR_CORE_PERF_LIMIT_REASONS......(0x64F) : 0x%llX\n", msr);
		
		if (msr)
		{
			IOLOG("------------------------------------------\n");
			IOLOG(" - PROCHOT Status....................... : %s\n", bitfield32(msr,  1,  1) ? "1 (frequency reduced below OS request due to assertion of external PROCHOT)": "0");
			IOLOG(" - Thermal Status....................... : %s\n", bitfield32(msr,  2,  2) ? "1 (frequency reduced below OS request due to a thermal event)": "0");
			// bit  3 Reserved.
			IOLOG(" - Residency State Regulation Status.... : %s\n", bitfield32(msr,  4,  4) ? "1 (frequency reduced below OS request due to residency state regulation limit)": "0");
			IOLOG(" - Running Average Thermal Limit Status. : %s\n", bitfield32(msr,  5,  5) ? "1 (frequency reduced below OS request due to Running Average Thermal Limit)": "0");
			IOLOG(" - VR Therm Alert Status................ : %s\n", bitfield32(msr,  6,  6) ? "1 (frequency reduced below OS request due to a thermal alert from a processor Voltage Regulator)" : "0");
			IOLOG(" - VR Therm Design Current Status....... : %s\n", bitfield32(msr,  7,  7) ? "1 (frequency reduced below OS request due to VR thermal design current limit)" : "0");
			IOLOG(" - Other Status......................... : %s\n", bitfield32(msr,  8,  8) ? "1 (frequency reduced below OS request due to electrical or other constraints)" : "0");
			// bit  9 Reserved.
			IOLOG(" - Package/Platform-Level #1 Power Limit : %s\n", bitfield32(msr, 10, 10) ? "1 (frequency reduced below OS request due to power limit)" : "0");
			IOLOG(" - Package/Platform-Level #2 Power Limit : %s\n", bitfield32(msr, 11, 11) ? "1 (frequency reduced below OS request due to power limit)" : "0");
			IOLOG(" - Max Turbo Limit Status............... : %s\n", bitfield32(msr, 12, 12) ? "1 (frequency reduced below OS request due to multi-core turbo limits)" : "0");
			IOLOG(" - Turbo Transition Attenuation Status.. : %s\n", bitfield32(msr, 13, 13) ? "1 (frequency reduced below OS request due to turbo transition attenuation)": "0");
			// bit 15-14 Reserved.
			IOLOG(" - PROCHOT Log.......................... : %s\n", bitfield32(msr, 16, 16) ? "1 (status bit has asserted)" : "0");
			IOLOG(" - Thermal Log.......................... : %s\n", bitfield32(msr, 17, 17) ? "1 (status bit has asserted)" : "0");
			// bit 19-18 Reserved.
			IOLOG(" - Residency State Regulation Log....... : %s\n", bitfield32(msr, 20, 20) ? "1 (status bit has asserted)" : "0");
			IOLOG(" - Running Average Thermal Limit Log.... : %s\n", bitfield32(msr, 21, 21) ? "1 (status bit has asserted)" : "0");
			IOLOG(" - VR Therm Alert Log................... : %s\n", bitfield32(msr, 22, 22) ? "1 (status bit has asserted)" : "0");
			IOLOG(" - VR Thermal Design Current Log........ : %s\n", bitfield32(msr, 23, 23) ? "1 (status bit has asserted)" : "0");
			IOLOG(" - Other Status Log..................... : %s\n", bitfield32(msr, 24, 24) ? "1 (status bit has asserted)" : "0");
			// bit 25 Reserved.
			IOLOG(" - Package/Platform-Level #1 Power Limit : %s\n", bitfield32(msr, 26, 26) ? "1 (status bit has asserted)" : "0");
			IOLOG(" - Package/Platform-Level #2 Power Limit : %s\n", bitfield32(msr, 27, 27) ? "1 (status bit has asserted)" : "0");
			IOLOG(" - Max Turbo Limit Log.................. : %s\n", bitfield32(msr, 28, 28) ? "1 (status bit has asserted)" : "0");
			IOLOG(" - Turbo Transition Attenuation Log..... : %s\n", bitfield32(msr, 29, 29) ? "1 (status bit has asserted)" : "0");
			// bit 63-30 Reserved.
		}
		
		if ((cpuid_reg[eax] & 0x2000) == 0x2000) // bit-13 HDC base registers IA32_PKG_HDC_CTL, IA32_PM_CTL1, IA32_THREAD_STALL MSRs are supported if set.
		{
			reportHDC();
		}
	}

	IOLOG("\nIA32_TSC_DEADLINE................(0x6E0) : 0x%llX\n", (unsigned long long)rdmsr64(0x6E0));

	reportHWP();
}


#if REPORT_C_STATES
//==============================================================================

inline void getCStates(void *magic)
{
	UInt32 logicalCoreNumber = cpu_number();

	if (gCheckC3 && rdmsr64(MSR_CORE_C3_RESIDENCY) > 0)
	{
		gC3Cores |= (1 << logicalCoreNumber);
	}

	if (gCheckC6 && rdmsr64(MSR_CORE_C6_RESIDENCY) > 0)
	{
		gC6Cores |= (1 << logicalCoreNumber);
	}

	if (gCheckC7 && rdmsr64(MSR_CORE_C7_RESIDENCY) > 0)
	{
		gC7Cores |= (1 << logicalCoreNumber);
	}

	/* if (logicalCoreNumber < 12)
	{
		uint64_t stateValue = rdmsr64(MSR_IA32_PERF_CONTROL);
		stateValue = 0x2D00; // 0x2800;
		wrmsr64(MSR_IA32_PERF_CONTROL, stateValue);
	} */

	uint64_t msr = rdmsr64(0x10);
	gTSC = rdtsc64();
	
	// IOLOG("AICPUI: TSC of logical core %d is: msr(0x10) = 0x%llx, rdtsc = 0x%llx\n", logicalCoreNumber, msr, gTSC);

	if (msr > (gTSC + 4096))
	{
		IOLog("Error: TSC of logical core %d is out of sync (0x%llx)!\n", logicalCoreNumber, msr);
	}
}
#endif

//==============================================================================

IOReturn AppleIntelInfo::loopTimerEvent(void)
{
	UInt8 currentMultiplier = (rdmsr64(MSR_IA32_PERF_STS) >> 8);
	gCoreMultipliers |= (1ULL << currentMultiplier);

#if REPORT_IGPU_P_STATES
	UInt8 currentIgpuMultiplier = 0;

	if (igpuEnabled)
	{
		if (gCpuModel == CPU_MODEL_SKYLAKE)
		{
			currentIgpuMultiplier = (UInt8)(((gMchbar[1] * 16.666) + 0.5) / 50);
		}
		else
		{
			currentIgpuMultiplier = (UInt8)gMchbar[1];
		}

		gIGPUMultipliers |= (1ULL << currentIgpuMultiplier);
	}
#endif
	
	timerEventSource->setTimeoutTicks(Interval);

	if (loopLock)
	{
		return kIOReturnTimeout;
	}

	loopLock = true;

	UInt8 pState = 0;

#if REPORT_IPG_STYLE
	if (logIPGStyle)
	{
		UInt64 aPerf = 0;
		float busy = 0;

		aPerf = (rdmsr64(IA32_APERF));
		wrmsr64(IA32_APERF, 0ULL);

		if (gHwpEnabled)
		{
			UInt64 pPerf = (rdmsr64(IA32_MPERF));
			busy = ((aPerf * 100) / pPerf);
		}
		else
		{
			UInt64 mPerf = (rdmsr64(IA32_MPERF));
			wrmsr64(IA32_MPERF, 0ULL);
			busy = ((aPerf * 100) / mPerf);
		}
		
		pState = (UInt8)(((gClockRatio + 0.5) * busy) / 100);

/*		if (pState != currentMultiplier)
		{ */
			gCoreMultipliers |= (1ULL << pState);

			if ((pState < currentMultiplier) && (pState < gMinRatio))
			{
				pState = gMinRatio;
			}
			/*
			 * Commented out after fabio67 (fabiosun) confirmed that
			 * the wrmsr() below triggered a KP on his configuration
			 * wrmsr64(199, (pState << 8));
			 */
		// }
	}
#endif

#if REPORT_C_STATES
	if (logCStates)
	{
		UInt32 magic = 0;
		mp_rendezvous_no_intrs(getCStates, &magic);
		IOSleep(1);
	}
#endif

	int currentBit = 0;
	UInt64 value = 0ULL;

#if REPORT_IGPU_P_STATES
	if ((gCoreMultipliers != gTriggeredPStates) || (gIGPUMultipliers != gTriggeredIGPUPStates))
#else
	#if REPORT_IPG_STYLE
		if ((gCoreMultipliers != gTriggeredPStates) || (currentMultiplier != pState))
	#else
		if (gCoreMultipliers != gTriggeredPStates)
	#endif
#endif
		{
			gTriggeredPStates = gCoreMultipliers;
			IOLOG("CPU P-States [ ");

			for (currentBit = gMinRatio; currentBit <= gMaxRatio; currentBit++)
			{
				value = (1ULL << currentBit);

				if ((gTriggeredPStates & value) == value)
				{
					if (currentBit == currentMultiplier)
					{
						IOLOG("(%d) ", currentBit);
					}
					else
					{
						IOLOG("%d ", currentBit);
					}
				}
			}

#if REPORT_IGPU_P_STATES
			if (igpuEnabled)
			{
				gTriggeredIGPUPStates = gIGPUMultipliers;
				IOLOG("] iGPU P-States [ ");

				for (currentBit = 1; currentBit <= 32; currentBit++)
				{
					value = (1ULL << currentBit);

					if ((gTriggeredIGPUPStates & value) == value)
					{
						if (currentBit == currentIgpuMultiplier)
						{
							IOLOG("(%d) ", currentBit);
						}
						else
						{
							IOLOG("%d ", currentBit);
						}
					}
				}
			}
#endif
			IOLOG("]\n");
		}

#if REPORT_C_STATES
	if (gCheckC3 && (gTriggeredC3Cores != gC3Cores))
	{
		gTriggeredC3Cores = gC3Cores;
		IOLOG("CPU C3-Cores [ ");

		for (currentBit = 0; currentBit < gThreadCount; currentBit++)
		{
			value = (1ULL << currentBit);

			if ((gTriggeredC3Cores & value) == value)
			{
				IOLOG("%d ", currentBit);
			}
		}

		IOLOG("]\n");
	}

	if (gCheckC6 && (gTriggeredC6Cores != gC6Cores))
	{
		gTriggeredC6Cores = gC6Cores;
		IOLOG("CPU C6-Cores [ ");

		for (currentBit = 0; currentBit < gThreadCount; currentBit++)
		{
			value = (1ULL << currentBit);

			if ((gTriggeredC6Cores & value) == value)
			{
				IOLOG("%d ", currentBit);
			}
		}

		IOLOG("]\n");
	}

	if (gCheckC7 && (gTriggeredC7Cores != gC7Cores))
	{
		gTriggeredC7Cores = gC7Cores;
		IOLOG("CPU C7-Cores [ ");

		for (currentBit = 0; currentBit < gThreadCount; currentBit++)
		{
			value = (1ULL << currentBit);

			if ((gTriggeredC7Cores & value) == value)
			{
				IOLOG("%d ", currentBit);
			}
		}

		IOLOG("]\n");
	}
#endif

	loopLock = false;

	return kIOReturnSuccess;
}

//==============================================================================

IOService* AppleIntelInfo::probe(IOService *provider, SInt32 *score)
{
	IOService *ret = super::probe(provider, score);

	if (ret != this)
	{
		return 0;
	}

	return ret;
}

//==============================================================================

bool AppleIntelInfo::start(IOService *provider)
{
	if (IOService::start(provider))
	{
		simpleLock = IOSimpleLockAlloc();

		if (simpleLock)
		{
			mCtx = vfs_context_current();

			IOLOG("\nAppleIntelInfo.kext v%s Copyright © 2012-2016 Pike R. Alpha. All rights reserved\n", VERSION);
			// os_log_info(OS_LOG_DEFAULT, "v%s Copyright © 2012-2016 Pike R. Alpha. All rights reserved", VERSION);
#if REPORT_MSRS
			OSBoolean * key_logMSRs = OSDynamicCast(OSBoolean, getProperty("logMSRs"));

			if (key_logMSRs)
			{
				logMSRs = (bool)key_logMSRs->getValue();
			}

			IOLOG("\nSettings:\n------------------------------------------\nlogMSRs..................................: %d\n", logMSRs);
#endif

			// wrmsr64(MSR_PP0_POWER_LIMIT, 0);
			// wrmsr64(MSR_PP0_CURRENT_CONFIG, 0x10141400001F40);

#if REPORT_IGPU_P_STATES
			OSBoolean * key_logIGPU = OSDynamicCast(OSBoolean, getProperty("logIGPU"));

			if (key_logIGPU)
			{
				igpuEnabled = (bool)key_logIGPU->getValue();
			}

			if (igpuEnabled)
			{
				if ((READ_PCI8_NB(DEVEN) & DEVEN_D2EN_MASK) == 0) // Is the IGPU enabled and visible?
				{
					igpuEnabled = false;
				}
			}

			IOLOG("logIGPU..................................: %d\n", igpuEnabled);
#endif

#if REPORT_INTEL_REGS
			OSBoolean * key_logIntelRegs = OSDynamicCast(OSBoolean, getProperty("logIntelRegs"));

			if (key_logIntelRegs)
			{
				logIntelRegs = (bool)key_logIntelRegs->getValue();
			}

			IOLOG("logIntelRegs............................: %d\n", logIntelRegs);
#endif

#if REPORT_C_STATES
			OSBoolean * key_logCStates = OSDynamicCast(OSBoolean, getProperty("logCStates"));

			if (key_logCStates)
			{
				logCStates = (bool)key_logCStates->getValue();
			}

			IOLOG("logCStates...............................: %d\n", logCStates);
#endif

#if REPORT_IPG_STYLE
			OSBoolean * key_logIPGStyle = OSDynamicCast(OSBoolean, getProperty("logIPGStyle"));
			
			if (key_logIPGStyle)
			{
				logIPGStyle = (bool)key_logIPGStyle->getValue();
			}

			IOLOG("logIPGStyle..............................: %d\n", logIPGStyle);
#endif
			
			UInt64 msr = rdmsr64(MSR_PLATFORM_INFO);
			gClockRatio = (UInt8)((msr >> 8) & 0xff);

			msr = rdmsr64(MSR_IA32_PERF_STS);
			gCoreMultipliers |= (1ULL << (msr >> 8));
			
			uint32_t cpuid_reg[4];
			do_cpuid(0x00000001, cpuid_reg);
			
			gCpuModel = bitfield32(cpuid_reg[eax], 7,  4) + (bitfield32(cpuid_reg[eax], 19, 16) << 4);

			gBclk = (getBusFrequency() / 1000000);

#if REPORT_C_STATES
			switch (gCpuModel) // TODO: Verify me!
			{
				case CPU_MODEL_SB_CORE:			// 0x2A - Intel 325462.pdf Vol.3C 35-111
				case CPU_MODEL_SB_JAKETOWN:		// 0x2D - Intel 325462.pdf Vol.3C 35-111
				case CPU_MODEL_IB_CORE:			// 0x3A - Intel 325462.pdf Vol.3C 35-125 (Refering to Table 35-12)
				case CPU_MODEL_IB_CORE_EX:		// 0x3B - Intel 325462.pdf Vol.3C 35-125 (Refering to Table 35-12)
					// No C7 support for Intel® Xeon® Processor E5-1600 v2/E5-2600 v2 (Product Families Datasheet Volume One of Two page 19)
					// case CPU_MODEL_IB_CORE_XEON:	// 0x3E - Intel 325462.pdf Vol.3C 35-125 (Refering to Table 35-12)
				case CPU_MODEL_HASWELL:			// 0x3C - Intel 325462.pdf Vol.3C 35-136
				case CPU_MODEL_BROADWELL:		// 0x3D
				case CPU_MODEL_HASWELL_ULT:		// 0x45 - Intel 325462.pdf Vol.3C 35-136
				case CPU_MODEL_CRYSTALWELL:		// 0x46
				case CPU_MODEL_BRYSTALWELL:		// 0x47
				case CPU_MODEL_SKYLAKE:			// 0x4E
				case CPU_MODEL_SKYLAKE_DT:		// 0x5E
					gCheckC7 = true;
					break;
			}
#endif
			
			msr = rdmsr64(MSR_PLATFORM_INFO);
			gMinRatio = (UInt8)((msr >> 40) & 0xff);
			gClockRatio = (UInt8)((msr >> 8) & 0xff);
			msr = rdmsr64(MSR_CORE_THREAD_COUNT);
			gCoreCount = bitfield32(msr, 31, 16);
			gThreadCount = bitfield32(msr, 15, 0);

#if REPORT_MSRS
			gTSC = rdtsc64();
			IOLOG("InitialTSC...............................: 0x%llx (%llu MHz)\n", gTSC, ((gTSC / gClockRatio) / 1000000000));

			// MWAIT information
			do_cpuid(0x00000005, cpuid_reg);
			uint32_t supportedMwaitCStates = bitfield32(cpuid_reg[edx], 31,  0);

			IOLOG("MWAIT C-States...........................: %d\n", supportedMwaitCStates);

			if (logMSRs)
			{
				reportMSRs();
			}
#endif

#if REPORT_INTEL_REGS
			if (logIntelRegs)
			{
				outl(0xcf8, 0x80001000);
				uint32_t value = inl(0xcfc);
				
				if ((value & 0x0000ffff) == 0x8086)
				{
					devid = ((value >> 16) & 0x0000ffff);
					
					reportIntelRegs();
				}
			}
#endif

			IOLOG("\nCPU Ratio Info:\n------------------------------------------\nBase Clock Frequency (BLCK)............. : %d MHz\n", gBclk);
			IOLOG("Maximum Efficiency Ratio/Frequency.......: %2d (%4d MHz)\n", gMinRatio, (gMinRatio * gBclk));
			IOLOG("Maximum non-Turbo Ratio/Frequency........: %2d (%4d MHz)\n", gClockRatio, (gClockRatio * gBclk));
			
			if (!((rdmsr64(IA32_MISC_ENABLES) >> 32) & 0x40))	// Turbo Mode Enabled?
			{
				msr = rdmsr64(MSR_TURBO_RATIO_LIMIT);
				gMaxRatio = (UInt8)(msr & 0xff);
				IOLOG("Maximum Turbo Ratio/Frequency............: %2d (%4d MHz)\n", gMaxRatio, (gMaxRatio * gBclk));
			}
			else
			{
				gMaxRatio = gClockRatio;
				IOLOG("Maximum Ratio/Frequency..................: %2d (%4d MHz)\n", gMaxRatio, (gMaxRatio * gBclk));
			}

#if REPORT_IGPU_P_STATES
			if (igpuEnabled)
			{
				IOPhysicalAddress address = (IOPhysicalAddress)(0xFED10000 + 0x5948);
				memDescriptor = IOMemoryDescriptor::withPhysicalAddress(address, 0x53, kIODirectionInOut);

				if (memDescriptor != NULL)
				{
					if ((result = memDescriptor->prepare()) == kIOReturnSuccess)
					{
						memoryMap = memDescriptor->map();

						if (memoryMap != NULL)
						{
							gMchbar = (UInt8 *)memoryMap->getVirtualAddress();

							// Preventing a stupid (UEFI) BIOS limit.
							if (gMchbar[0x4C] < gMchbar[0x50])
							{
								gMchbar[0x4C] = gMchbar[0x50];
							}

							//
							// Examples IGPU multiplier:	17 (multiplier) * 50 (frequency in MHz) =  850 MHz
							//								22 (multiplier) * 50 (frequency in MHz) = 1100 MHz
							//								6 P-States: 850, 900, 950, 1000, 1050 and 1100 MHz
							//
							// Current RP-State, when the graphics engine is in RC6, this reflects the last used ratio.
							IOLOG("\nIGPU Info:\n------------------------------------------\n");
							IOLOG("IGPU Current Frequency...................: %4d MHz\n", IGPU_RATIO_TO_FREQUENCY((UInt8)gMchbar[0x01])); // RP_STATE_RATIO (CURRENT_FREQUENCY)
							// Maximum RPN base frequency capability for the Integrated GFX Engine (GT).
							IOLOG("IGPU Minimum Frequency...................: %4d MHz\n", IGPU_RATIO_TO_FREQUENCY((UInt8)gMchbar[0x52])); // RPN_CAP (MIN_FREQUENCY)
							// Maximum RP1 base frequency capability for the Integrated GFX Engine (GT).
							IOLOG("IGPU Maximum Non-Turbo Frequency.........: %4d MHz\n", IGPU_RATIO_TO_FREQUENCY((UInt8)gMchbar[0x51])); // RP1_CAP (MAX_NON_TURBO)
							// Maximum RP0 base frequency capability for the Integrated GFX Engine (GT).
							IOLOG("IGPU Maximum Turbo Frequency.............: %4d MHz\n", IGPU_RATIO_TO_FREQUENCY((UInt8)gMchbar[0x50])); // RP0_CAP (MAX_TURBO))

							// Maximum base frequency limit for the Integrated GFX Engine (GT) allowed during run-time.
							if (gMchbar[0x4C] == 255)
							{
								IOLOG("IGPU Maximum limit.......................: No Limit\n\n"); // RPSTT_LIM
							}
							else
							{
								IOLOG("IGPU Maximum limit.......................: %4d MHz\n\n", IGPU_RATIO_TO_FREQUENCY((UInt8)gMchbar[0x4C])); // RPSTT_LIM
							}
						}
						else
						{
							IOLOG("Error: memoryMap == NULL\n");
						}
					}
					else
					{
						IOLOG("Error: memDescriptor->prepare() failed!\n");
					}
				}
				else
				{
					IOLOG("Error: memDescriptor == NULL\n");
				}
			}
#endif
			IOLOG("P-State ratio * %d = Frequency in MHz\n------------------------------------------\n", gBclk);

			timerEventSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &AppleIntelInfo::loopTimerEvent));
			workLoop = getWorkLoop();

			if (timerEventSource && workLoop && (kIOReturnSuccess == workLoop->addEventSource(timerEventSource)))
			{
				this->registerService(0);
				timerEventSource->setTimeoutMS(1000);

				return true;
			}
		}
	}

	return false;
}

//==============================================================================

void AppleIntelInfo::stop(IOService *provider)
{
	if (simpleLock)
	{
		IOSimpleLockFree(simpleLock);
	}

	if (timerEventSource)
	{
		if (workLoop)
		{
			timerEventSource->cancelTimeout();
			workLoop->removeEventSource(timerEventSource);
		}

		timerEventSource->release();
		timerEventSource = NULL;
	}

	super::stop(provider);
}

//==============================================================================

void AppleIntelInfo::free()
{
#if REPORT_IGPU_P_STATES
	if (igpuEnabled)
	{
		if (memoryMap)
		{
			memoryMap->release();
			memoryMap = NULL;
		}

		if (memDescriptor)
		{
			memDescriptor->release();
			memDescriptor = NULL;
		}
	}
#endif

	super::free();
}
