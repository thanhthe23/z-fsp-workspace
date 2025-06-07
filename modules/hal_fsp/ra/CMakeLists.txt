# Copyright (c) 2025 The.Thanh Nguyen <the.nguyen.yf@outlook.com>
# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(
  ${ZEPHYR_HAL_FSP_MODULE_DIR}/ra/fsp/ra/fsp/inc
  ${ZEPHYR_HAL_FSP_MODULE_DIR}/ra/fsp/ra/fsp/inc/api
  ${ZEPHYR_HAL_FSP_MODULE_DIR}/ra/fsp/ra/fsp/inc/instances
  inc
  ra_cfg
  ra_cfg/bsp/${SOC_SERIES}
  ra_gen
  ra_gen/${SOC_SERIES}
)
zephyr_library_sources(
  ${ZEPHYR_HAL_FSP_MODULE_DIR}/ra/fsp/ra/fsp/src/bsp/mcu/all/bsp_clocks.c
  ${ZEPHYR_HAL_FSP_MODULE_DIR}/ra/fsp/ra/fsp/src/bsp/mcu/all/bsp_common.c
  ${ZEPHYR_HAL_FSP_MODULE_DIR}/ra/fsp/ra/fsp/src/bsp/mcu/all/bsp_delay.c
  ${ZEPHYR_HAL_FSP_MODULE_DIR}/ra/fsp/ra/fsp/src/bsp/mcu/all/bsp_group_irq.c
  ${ZEPHYR_HAL_FSP_MODULE_DIR}/ra/fsp/ra/fsp/src/bsp/mcu/all/bsp_guard.c
  ${ZEPHYR_HAL_FSP_MODULE_DIR}/ra/fsp/ra/fsp/src/bsp/mcu/all/bsp_io.c
  ${ZEPHYR_HAL_FSP_MODULE_DIR}/ra/fsp/ra/fsp/src/bsp/mcu/all/bsp_macl.c
  ${ZEPHYR_HAL_FSP_MODULE_DIR}/ra/fsp/ra/fsp/src/bsp/mcu/all/bsp_register_protection.c
  ${ZEPHYR_HAL_FSP_MODULE_DIR}/ra/fsp/ra/fsp/src/bsp/mcu/all/bsp_rom_registers.c
  ${ZEPHYR_HAL_FSP_MODULE_DIR}/ra/fsp/ra/fsp/src/bsp/mcu/all/bsp_sbrk.c
  ${ZEPHYR_HAL_FSP_MODULE_DIR}/ra/fsp/ra/fsp/src/bsp/mcu/all/bsp_sdram.c
  ${ZEPHYR_HAL_FSP_MODULE_DIR}/ra/fsp/ra/fsp/src/bsp/mcu/all/bsp_security.c
  portable/bsp/bsp_irq.c
)

zephyr_library_sources_ifdef(CONFIG_RENESAS_RA_FSP_IOPORT
  ${ZEPHYR_HAL_FSP_MODULE_DIR}/ra/fsp/ra/fsp/src/r_ioport/r_ioport.c
)
