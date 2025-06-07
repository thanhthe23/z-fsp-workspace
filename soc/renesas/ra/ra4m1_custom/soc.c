/*
 * Copyright (c) 2025 The.Thanh Nguyen <the.nguyen.yf@outlook.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief System/hardware module for Renesas RA4M1 family processor
 */

#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/arch/cpu.h>
#include <cmsis_core.h>
#include <zephyr/arch/arm/nmi.h>
#include <zephyr/irq.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(soc, CONFIG_SOC_LOG_LEVEL);

#include "bsp_cfg.h"
#include <bsp_api.h>

/**
 * @brief Perform basic hardware initialization at boot.
 *
 * This needs to be run from the very beginning.
 */
void soc_early_init_hook(void)
{
	/* Configure system clocks. */
	bsp_clock_init();

#ifdef CONFIG_ENTROPY
	/* To prevent an undesired current draw, this MCU requires a reset
	 * of the TRNG circuit after the clocks are initialized */
	bsp_reset_trng_circuit();
#endif /* CONFIG_ENTROPY */

	/* Initialize SystemCoreClock variable. */
	SystemCoreClockUpdate();
}
