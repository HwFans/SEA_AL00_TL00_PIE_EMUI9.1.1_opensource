#ifndef __DEVDRV_PLATFORM_REGISTER_H
#define __DEVDRV_PLATFORM_REGISTER_H

#define SOC_HARDWARE_VERSION (DEVDRV_PLATFORM_LITE_ORLANDO)

#include "soc_npu_ts_sysctrl_reg_offset.h"
#include "soc_npu_tscpu_cfg_reg_offset.h"

#define DEVDRV_SC_TESTREG0_OFFSET \
		(SOC_TS_SYSCTRL_SC_TESTREG0_REG - SOC_TS_SYSCTRL_BASE)
#define DEVDRV_SC_TESTREG1_OFFSET \
		(SOC_TS_SYSCTRL_SC_TESTREG1_REG - SOC_TS_SYSCTRL_BASE)
#define TS_BOOT_STATUS_OBSERVATION_REG \
		(SOC_TS_SYSCTRL_SC_TESTREG6_REG - SOC_TS_SYSCTRL_BASE)
#define DEVDRV_SC_TESTREG8_OFFSET \
		(SOC_TS_SYSCTRL_SC_TESTREG8_REG - SOC_TS_SYSCTRL_BASE)
#define DEVDRV_CFG_STAT0_OFFSET \
		(SOC_NPU_TSCPU_CFG_TS_CPU_STAT0_REG - SOC_TS_SYSCTRL_BASE)

#endif
