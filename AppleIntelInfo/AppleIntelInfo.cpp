/*
 * Copyright (c) 2012-2015 Pike R. Alpha. All rights reserved.
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

//==============================================================================

void AppleIntelInfo::reportMSRs(UInt8 aCPUModel)
{
	// 123456789 123456789 123456789 123456789 123456789 123456789
	// MSR_PLATFORM_INFO..........(0xCE)  : 0x80838F3012200
	
	IOLOG("\nModel Specific Regiters\n------------------------------------\nMSR_CORE_THREAD_COUNT......(0x35)  : 0x%llX\n", (unsigned long long)rdmsr64(MSR_CORE_THREAD_COUNT));

	IOLOG("MSR_PLATFORM_INFO..........(0xCE)  : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PLATFORM_INFO));

	UInt64 msr_pmg_cst_config_control = rdmsr64(MSR_PKG_CST_CONFIG_CONTROL);

	IOLOG("MSR_PMG_CST_CONFIG_CONTROL.(0xE2)  : 0x%llX\n", msr_pmg_cst_config_control);
	IOLOG("MSR_PMG_IO_CAPTURE_BASE....(0xE4)  : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PMG_IO_CAPTURE_BASE));
	IOLOG("IA32_MPERF.................(0xE7)  : 0x%llX\n", (unsigned long long)rdmsr64(IA32_MPERF));
	IOLOG("IA32_APERF.................(0xE8)  : 0x%llX\n", (unsigned long long)rdmsr64(IA32_APERF));

	IOLOG("MSR_FLEX_RATIO.............(0x194) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_FLEX_RATIO));
	IOLOG("MSR_IA32_PERF_STATUS.......(0x198) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_IA32_PERF_STATUS));
	IOLOG("MSR_IA32_PERF_CONTROL......(0x199) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_IA32_PERF_CONTROL));
	IOLOG("IA32_CLOCK_MODULATION......(0x19A) : 0x%llX\n", (unsigned long long)rdmsr64(IA32_CLOCK_MODULATION));
	IOLOG("IA32_THERM_STATUS..........(0x19C) : 0x%llX\n", (unsigned long long)rdmsr64(IA32_THERM_STATUS));

	IOLOG("IA32_MISC_ENABLES..........(0x1A0) : 0x%llX\n", (unsigned long long)rdmsr64(IA32_MISC_ENABLES));
	IOLOG("MSR_MISC_PWR_MGMT..........(0x1AA) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_MISC_PWR_MGMT));
	IOLOG("MSR_TURBO_RATIO_LIMIT......(0x1AD) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_TURBO_RATIO_LIMIT));

	IOLOG("IA32_ENERGY_PERF_BIAS......(0x1B0) : 0x%llX\n", (unsigned long long)rdmsr64(IA32_ENERGY_PERF_BIAS));

	IOLOG("MSR_POWER_CTL..............(0x1FC) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_POWER_CTL));

	IOLOG("MSR_RAPL_POWER_UNIT........(0x606) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_RAPL_POWER_UNIT));
	IOLOG("MSR_PKG_POWER_LIMIT........(0x610) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_POWER_LIMIT));
	IOLOG("MSR_PKG_ENERGY_STATUS......(0x611) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_ENERGY_STATUS));
	IOLOG("MSR_PKG_POWER_INFO.........(0x614) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_POWER_INFO));

	IOLOG("MSR_PP0_CURRENT_CONFIG.....(0x601) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PP0_CURRENT_CONFIG));
	IOLOG("MSR_PP0_POWER_LIMIT........(0x638) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PP0_POWER_LIMIT));
	IOLOG("MSR_PP0_ENERGY_STATUS......(0x639) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PP0_ENERGY_STATUS));
	IOLOG("MSR_PP0_POLICY.............(0x63a) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PP0_POLICY));

#if REPORT_IGPU_P_STATES
	if (igpuEnabled)
	{
//		IOLOG("MSR_PP1_CURRENT_CONFIG.....(0x602) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PP1_CURRENT_CONFIG));

		switch (aCPUModel)
		{
			case CPU_MODEL_SB_CORE:				// 0x2A - Intel 325462.pdf Vol.3C 35-120
			case CPU_MODEL_IB_CORE:				// 0x3A - Intel 325462.pdf Vol.3C 35-125 (Referring to Table 35-13)
				IOLOG("MSR_PP1_CURRENT_CONFIG.....(0x602) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PP1_CURRENT_CONFIG));

			case CPU_MODEL_HASWELL:				// 0x3C - Intel 325462.pdf Vol.3C 35-140
			case CPU_MODEL_HASWELL_ULT:			// 0x45 - Intel 325462.pdf Vol.3C 35-140
			case CPU_MODEL_CRYSTALWELL:			// 0x46 - Intel 325462.pdf Vol.3C 35-140

				IOLOG("MSR_PP1_POWER_LIMIT........(0x640) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PP1_POWER_LIMIT));
				IOLOG("MSR_PP1_ENERGY_STATUS......(0x641) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PP1_ENERGY_STATUS));
				IOLOG("MSR_PP1_POLICY.............(0x642) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PP1_POLICY));
				break;
		}
	}
#endif

	switch (aCPUModel)
	{
		case CPU_MODEL_IB_CORE:				// 0x3A - Intel 325462.pdf Vol.3C 35-126
		case CPU_MODEL_IB_CORE_EX:			// 0x3B - Intel 325462.pdf Vol.3C 35-126
		// case CPU_MODEL_IB_CORE_XEON:		// 0x3E - Intel 325462.pdf Vol.3C 35-126
		case CPU_MODEL_HASWELL:				// 0x3C - Intel 325462.pdf Vol.3C 35-133
		case CPU_MODEL_HASWELL_ULT:			// 0x45 - Intel 325462.pdf Vol.3C 35-133
		case CPU_MODEL_CRYSTALWELL:			// 0x46 - Intel 325462.pdf Vol.3C 35-133

			IOLOG("MSR_CONFIG_TDP_NOMINAL.....(0x648) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_CONFIG_TDP_NOMINAL));
			IOLOG("MSR_CONFIG_TDP_LEVEL1......(0x649) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_CONFIG_TDP_LEVEL1));
			IOLOG("MSR_CONFIG_TDP_LEVEL2......(0x64a) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_CONFIG_TDP_LEVEL2));
			IOLOG("MSR_CONFIG_TDP_CONTROL.....(0x64b) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_CONFIG_TDP_CONTROL));
			IOLOG("MSR_TURBO_ACTIVATION_RATIO.(0x64c) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_TURBO_ACTIVATION_RATIO));
			break;
	}

	switch (aCPUModel)
	{
		case CPU_MODEL_SB_CORE:	
		case CPU_MODEL_IB_CORE:
		case CPU_MODEL_IB_CORE_EX:
		case CPU_MODEL_IB_CORE_XEON:
		case CPU_MODEL_HASWELL:
		case CPU_MODEL_HASWELL_ULT:
		case CPU_MODEL_CRYSTALWELL:

			IOLOG("MSR_PKGC3_IRTL.............(0x60a) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKGC3_IRTL));
			break;

		case CPU_MODEL_NEHALEM:
		case CPU_MODEL_NEHALEM_EX:
			break;
	}

	IOLOG("MSR_PKGC6_IRTL.............(0x60b) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKGC6_IRTL));

	if (gCheckC7)
	{
		IOLOG("MSR_PKGC7_IRTL.............(0x60c) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKGC7_IRTL));
	}

	if (aCPUModel == CPU_MODEL_NEHALEM || aCPUModel == CPU_MODEL_NEHALEM_EX)
	{
		IOLOG("MSR_PKG_C3_RESIDENCY.......(0x3f8) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C3_RESIDENCY));
	}
	else
	{
		IOLOG("MSR_PKG_C2_RESIDENCY.......(0x60d) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C2_RESIDENCY));
		/*
		 * Is package C3 auto-demotion/undemotion enabled i.e. is bit-25 or bit-27 set?
		 */
		if ((msr_pmg_cst_config_control & 0x2000000) || (msr_pmg_cst_config_control & 0x8000000))
		{
			IOLOG("MSR_PKG_C3_RESIDENCY.......(0x3f8) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C3_RESIDENCY));
		}
	}
	
	IOLOG("MSR_PKG_C6_RESIDENCY.......(0x3f9) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C6_RESIDENCY));

	if (gCheckC7)
	{
		IOLOG("MSR_PKG_C7_RESIDENCY.......(0x3fa) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C7_RESIDENCY));
	}

	if (aCPUModel == CPU_MODEL_HASWELL_ULT) // 0x45 - Intel 325462.pdf Vol.3C 35-136
	{
		IOLOG("MSR_PKG_C8_RESIDENCY.......(0x630) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C8_RESIDENCY));
		IOLOG("MSR_PKG_C9_RESIDENCY.......(0x631) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C9_RESIDENCY));
		IOLOG("MSR_PKG_C10_RESIDENCY......(0x632) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C10_RESIDENCY));
		
		IOLOG("MSR_PKG_C8_LATENCY.........(0x633) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C8_RESIDENCY));
		IOLOG("MSR_PKG_C9_LATENCY.........(0x634) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C9_RESIDENCY));
		IOLOG("MSR_PKG_C10_LATENCY........(0x635) : 0x%llX\n", (unsigned long long)rdmsr64(MSR_PKG_C10_RESIDENCY));
	}
	
	IOLOG("IA32_TSC_DEADLINE..........(0x6E0) : 0x%llX\n", (unsigned long long)rdmsr64(0x6E0));
}


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
	
	uint64_t msr = rdmsr64(0x10);
	gTSC = rdtsc64();
	
	// IOLOG("AICPUI: TSC of logical core %d is: msr(0x10) = 0x%llx, rdtsc = 0x%llx\n", logicalCoreNumber, msr, gTSC);

	if (msr > (gTSC + 4096))
	{
		IOLog("Error: TSC of logical core %d is out of sync (0x%llx)!\n", logicalCoreNumber, msr);
	}
}

//==============================================================================

IOReturn AppleIntelInfo::loopTimerEvent(void)
{
	UInt8 currentMultiplier = (rdmsr64(MSR_IA32_PERF_STS) >> 8);
	gCoreMultipliers |= (1ULL << currentMultiplier);

#if REPORT_IGPU_P_STATES
	UInt8 currentIgpuMultiplier = 0;

	if (igpuEnabled)
	{
		currentIgpuMultiplier = (UInt8)gMchbar[1];
		gIGPUMultipliers |= (1ULL << currentIgpuMultiplier);
	}
#endif

	timerEventSource->setTimeoutTicks(Interval);

	if (loopLock)
	{
		return kIOReturnTimeout;
	}

	loopLock = true;

#if REPORT_IPG_STYLE
	if (logIPGStyle)
	{
		UInt64 mPerf = (rdmsr64(IA32_MPERF));
		UInt64 aPerf = (rdmsr64(IA32_APERF));
		wrmsr64(IA32_MPERF, 0ULL);
		wrmsr64(IA32_APERF, 0ULL);
//	UInt16 busy = ((aPerf * 100) / mPerf);
		float busy = ((aPerf * 100) / mPerf);
		UInt8 pState = (UInt8)(((gClockRatio + 0.5) * busy) / 100);

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

		for (currentBit = 0; currentBit <= 16; currentBit++)
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

		for (currentBit = 0; currentBit <= 16; currentBit++)
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

		for (currentBit = 0; currentBit <= 16; currentBit++)
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

			IOLOG("\nAppleIntelInfo.kext v%s Copyright © 2012-2015 Pike R. Alpha. All rights reserved\n", VERSION);
#if REPORT_MSRS
			OSBoolean * key_logMSRs = OSDynamicCast(OSBoolean, getProperty("logMSRs"));

			if (key_logMSRs)
			{
				logMSRs = (bool)key_logMSRs->getValue();
			}

			IOLOG("\nSettings:\n------------------------------------\nlogMSRs............................: %d\n", logMSRs);
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

			IOLOG("logIGPU............................: %d\n", igpuEnabled);
			
//			wrmsr64(MSR_PP1_POWER_LIMIT, 0);
#endif

#if REPORT_INTEL_REGS
			OSBoolean * key_logIntelRegs = OSDynamicCast(OSBoolean, getProperty("logIntelRegs"));

			if (key_logIntelRegs)
			{
				logIntelRegs = (bool)key_logIntelRegs->getValue();
			}

			IOLOG("logIntelRegs.......................: %d\n", logIntelRegs);
#endif

#if REPORT_C_STATES
			OSBoolean * key_logCStates = OSDynamicCast(OSBoolean, getProperty("logCStates"));

			if (key_logCStates)
			{
				logCStates = (bool)key_logCStates->getValue();
			}

			IOLOG("logCStates.........................: %d\n", logCStates);
#endif

#if REPORT_IPG_STYLE
			OSBoolean * key_logIPGStyle = OSDynamicCast(OSBoolean, getProperty("logIPGStyle"));
			
			if (key_logIPGStyle)
			{
				logIPGStyle = (bool)key_logIPGStyle->getValue();
			}

			IOLOG("logIPGStyle........................: %d\n", logIPGStyle);
#endif
			
			UInt64 msr = rdmsr64(MSR_PLATFORM_INFO);
			gClockRatio = (UInt8)((msr >> 8) & 0xff);

			msr = rdmsr64(MSR_IA32_PERF_STS);
			gCoreMultipliers |= (1ULL << (msr >> 8));
			
			uint32_t cpuid_reg[4];
			do_cpuid(0x00000001, cpuid_reg);
			
			UInt8 cpuModel = bitfield32(cpuid_reg[eax], 7,  4) + (bitfield32(cpuid_reg[eax], 19, 16) << 4);
			
#if REPORT_C_STATES
			switch (cpuModel) // TODO: Verify me!
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
				case CPU_MODEL_SKYLAKE:			// 0x5E
					gCheckC7 = true;
					break;
			}
#endif

#if REPORT_MSRS
			gTSC = rdtsc64();
			IOLOG("InitialTSC.........................: 0x%llx\n", gTSC);

			// MWAIT information
			do_cpuid(0x00000005, cpuid_reg);
			uint32_t supportedMwaitCStates = bitfield32(cpuid_reg[edx], 31,  0);

			IOLOG("MWAIT C-States.....................: %d\n", supportedMwaitCStates);

			if (logMSRs)
			{
				reportMSRs(cpuModel);
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
			msr = rdmsr64(MSR_PLATFORM_INFO);
			gMinRatio = (UInt8)((msr >> 40) & 0xff);
			IOLOG("\nCPU Ratio Info:\n------------------------------------\nCPU Low Frequency Mode.............: %d00 MHz\n", gMinRatio);
			
			gClockRatio = (UInt8)((msr >> 8) & 0xff);
			IOLOG("CPU Maximum non-Turbo Frequency....: %d00 MHz\n", gClockRatio);
			
			if (!((rdmsr64(IA32_MISC_ENABLES) >> 32) & 0x40))	// Turbo Mode Enabled?
			{
				msr = rdmsr64(MSR_TURBO_RATIO_LIMIT);
				gMaxRatio = (UInt8)(msr & 0xff);
				IOLOG("CPU Maximum Turbo Frequency........: %d00 MHz\n", gMaxRatio);
			}
			else
			{
				gMaxRatio = gClockRatio;
				IOLOG("CPU Maximum Frequency..............: %d00 MHz\n", gMaxRatio);
			}

			timerEventSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &AppleIntelInfo::loopTimerEvent));
			workLoop = getWorkLoop();

			if (timerEventSource && workLoop && (kIOReturnSuccess == workLoop->addEventSource(timerEventSource)))
			{
				this->registerService(0);

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
								IOLOG("\nIGPU Info:\n------------------------------------\n");
								IOLOG("IGPU Current Frequency.............: %4d MHz\n", IGPU_RATIO_TO_FREQUENCY((UInt8)gMchbar[0x01])); // RP_STATE_RATIO (CURRENT_FREQUENCY)
								// Maximum RPN base frequency capability for the Integrated GFX Engine (GT).
								IOLOG("IGPU Minimum Frequency.............: %4d MHz\n", IGPU_RATIO_TO_FREQUENCY((UInt8)gMchbar[0x52])); // RPN_CAP (MIN_FREQUENCY)
								// Maximum RP1 base frequency capability for the Integrated GFX Engine (GT).
								IOLOG("IGPU Maximum Non-Turbo Frequency...: %4d MHz\n", IGPU_RATIO_TO_FREQUENCY((UInt8)gMchbar[0x51])); // RP1_CAP (MAX_NON_TURBO)
								// Maximum RP0 base frequency capability for the Integrated GFX Engine (GT).
								IOLOG("IGPU Maximum Turbo Frequency.......: %4d MHz\n", IGPU_RATIO_TO_FREQUENCY((UInt8)gMchbar[0x50])); // RP0_CAP (MAX_TURBO))

								// Maximum base frequency limit for the Integrated GFX Engine (GT) allowed during run-time.
								if (gMchbar[0x4C] == 255)
								{
									IOLOG("IGPU Maximum limit.................: No Limit\n\n"); // RPSTT_LIM
								}
								else
								{
									IOLOG("IGPU Maximum limit.................: %4d MHz\n\n", IGPU_RATIO_TO_FREQUENCY((UInt8)gMchbar[0x4C])); // RPSTT_LIM
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
