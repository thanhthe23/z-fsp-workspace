/*
 * Copyright (c) 2025 The.Thanh Nguyen <the.nguyen.yf@outlook.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/irq.h>
#include <soc.h>

#include "r_sci_uart.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(renesas_ra_uart_sci);

#define SETTING_INVALID     -1
#define BAUDATE_ERROR_X1000 CONFIG_UART_RENESAS_RA_BAUDRATE_ERROR

typedef R_SCI0_Type sci_uart_regs_t;

struct uart_renesas_ra_sci_config {
	const struct pinctrl_dev_config *pcfg;
	sci_uart_regs_t *const regs;
#ifndef CONFIG_UART_RUNTIME_CONFIGURE
	const struct uart_config *uart_config;
#endif /* CONFIG_UART_RUNTIME_CONFIGURE */
};

struct uart_renesas_ra_sci_data {
	struct st_sci_uart_instance_ctrl fsp_instance_ctrl;
	struct st_uart_cfg fsp_instance_cfg;
	struct st_sci_uart_extended_cfg fsp_extended_cfg;
	struct st_baud_setting_t fsp_baud_setting;
#ifdef CONFIG_UART_RUNTIME_CONFIGURE
	struct uart_config uart_config;
#endif /* CONFIG_UART_RUNTIME_CONFIGURE */
};

static const int uart_parity_cfg_lut[] = {
	[UART_CFG_PARITY_NONE] = UART_PARITY_OFF,   [UART_CFG_PARITY_ODD] = UART_PARITY_ODD,
	[UART_CFG_PARITY_EVEN] = UART_PARITY_EVEN,  [UART_CFG_PARITY_MARK] = SETTING_INVALID,
	[UART_CFG_PARITY_SPACE] = UART_PARITY_ZERO,
};

static const int uart_stop_bits_cfg_lut[] = {
	[UART_CFG_STOP_BITS_0_5] = SETTING_INVALID,
	[UART_CFG_STOP_BITS_1] = UART_STOP_BITS_1,
	[UART_CFG_STOP_BITS_1_5] = SETTING_INVALID,
	[UART_CFG_STOP_BITS_2] = UART_STOP_BITS_2,
};

static const int uart_data_bits_cfg_lut[] = {
	[UART_CFG_DATA_BITS_5] = SETTING_INVALID,  [UART_CFG_DATA_BITS_6] = SETTING_INVALID,
	[UART_CFG_DATA_BITS_7] = UART_DATA_BITS_7, [UART_CFG_DATA_BITS_8] = UART_DATA_BITS_8,
	[UART_CFG_DATA_BITS_9] = UART_DATA_BITS_9,
};

static int uart_renesas_ra_configure(const struct device *dev, const struct uart_config *cfg)
{
	struct uart_renesas_ra_sci_data *data = dev->data;
	struct st_sci_uart_instance_ctrl *instance_ctrl = &data->fsp_instance_ctrl;
	struct st_uart_cfg *instance_cfg = &data->fsp_instance_cfg;
	struct st_sci_uart_extended_cfg *extended_cfg = &data->fsp_extended_cfg;
	fsp_err_t err;

	instance_cfg->parity = uart_parity_cfg_lut[cfg->parity];
	if (instance_cfg->parity == SETTING_INVALID) {
		LOG_DBG("Parity setting not support: %d", cfg->parity);
		return -EINVAL;
	}

	instance_cfg->stop_bits = uart_stop_bits_cfg_lut[cfg->stop_bits];
	if (instance_cfg->stop_bits == SETTING_INVALID) {
		LOG_DBG("Stop bits setting not support: %d", cfg->stop_bits);
		return -EINVAL;
	}

	instance_cfg->data_bits = uart_data_bits_cfg_lut[cfg->data_bits];
	if (instance_cfg->data_bits == SETTING_INVALID) {
		LOG_DBG("Data bits setting not support: %d", cfg->data_bits);
		return -EINVAL;
	}

	if (cfg->flow_ctrl != UART_CFG_FLOW_CTRL_NONE) {
		LOG_DBG("Flow control setting not support: %d", cfg->data_bits);
		return -EINVAL;
	}

	err = R_SCI_UART_BaudCalculate(cfg->baudrate, false, BAUDATE_ERROR_X1000,
				       &data->fsp_baud_setting);
	if (err != FSP_SUCCESS) {
		LOG_DBG("Failed to calculate baud rate: fsp_err: %d", err);
		return -EINVAL;
	}

	if (instance_ctrl->open != 0U) {
		err = R_SCI_UART_Close(instance_ctrl);
		assert(err == FSP_SUCCESS);
	}

	err = R_SCI_UART_Open(instance_ctrl, instance_cfg);
	assert(err == FSP_SUCCESS);

#ifdef CONFIG_UART_RUNTIME_CONFIGURE
	memcpy(&data->uart_config, cfg, sizeof(data->uart_config));
#endif

	return 0;
}

#ifdef CONFIG_UART_RUNTIME_CONFIGURE
static int uart_renesas_ra_sci_config_get(const struct device *dev, struct uart_config *cfg)
{
	struct uart_renesas_ra_sci_data *data = dev->data;

	if (cfg == NULL) {
		LOG_ERR("Configuration pointer is NULL");
		return -EINVAL;
	}

	memcpy(cfg, &data->uart_config, sizeof(struct uart_config));

	return 0;
}
#endif /* CONFIG_UART_RUNTIME_CONFIGURE */

static int uart_renesas_ra_sci_poll_in(const struct device *dev, unsigned char *c)
{
	struct uart_renesas_ra_sci_data *data = dev->data;

	return rp_sci_uart_get_char(&data->fsp_instance_ctrl, c);
}

static void uart_renesas_ra_sci_poll_out(const struct device *dev, unsigned char c)
{
	struct uart_renesas_ra_sci_data *data = dev->data;

	rp_sci_uart_put_char(&data->fsp_instance_ctrl, c);
}

static int uart_renesas_ra_sci_err_check(const struct device *dev)
{
	struct uart_renesas_ra_sci_data *data = dev->data;

	return rp_sci_uart_err_check(&data->fsp_instance_ctrl);
}

static int uart_renesas_ra_sci_init(const struct device *dev)
{
	const struct uart_renesas_ra_sci_config *config = dev->config;
#ifdef CONFIG_UART_RUNTIME_CONFIGURE
	struct uart_renesas_ra_sci_data *data = dev->data;
	const struct uart_config *uart_config = &data->uart_config;
#else
	const struct uart_config *uart_config = &config->uart_config;
#endif /* CONFIG_UART_RUNTIME_CONFIGURE */
	int ret;

	ret = pinctrl_apply_state(config->pcfg, PINCTRL_STATE_DEFAULT);
	if (ret < 0) {
		return ret;
	}

	ret = uart_renesas_ra_configure(dev, uart_config);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

static DEVICE_API(uart, uart_renesas_ra_sci_api) = {
	.poll_in = uart_renesas_ra_sci_poll_in,
	.poll_out = uart_renesas_ra_sci_poll_out,
	.err_check = uart_renesas_ra_sci_err_check,
#ifdef CONFIG_UART_RUNTIME_CONFIGURE
	.configure = uart_renesas_ra_configure,
	.config_get = uart_renesas_ra_sci_config_get,
#endif
};

#define DT_DRV_COMPAT renesas_ra_uart_sci

#define UART_CFG_GET(inst)                                                                         \
	.uart_config = {                                                                           \
		.baudrate = DT_INST_PROP(inst, current_speed),                                     \
		.parity = DT_INST_ENUM_IDX(inst, parity),                                          \
		.stop_bits = DT_INST_ENUM_IDX(inst, stop_bits),                                    \
		.data_bits = DT_INST_ENUM_IDX(inst, data_bits),                                    \
		.flow_ctrl = DT_INST_ENUM_IDX(inst, flow_control),                                 \
	}

#define UART_RENESAS_RA_SCI_INIT(inst)                                                             \
	PINCTRL_DT_DEFINE(DT_INST_PARENT(inst));                                                   \
                                                                                                   \
	static const struct uart_renesas_ra_sci_config uart_renesas_ra_sci_config_##inst = {       \
		.pcfg = PINCTRL_DT_DEV_CONFIG_GET(DT_INST_PARENT(inst)),                           \
		.regs = (sci_uart_regs_t *)DT_REG_ADDR(DT_INST_PARENT(inst)),                      \
		IF_DISABLED(CONFIG_UART_RUNTIME_CONFIGURE, (UART_CFG_GET(inst)))};                 \
                                                                                                   \
	static struct uart_renesas_ra_sci_data uart_renesas_ra_sci_data_##inst = {                 \
		.fsp_instance_ctrl = {0},                                                          \
		.fsp_instance_cfg = {                                                              \
			.channel = (DT_REG_ADDR(DT_INST_PARENT(inst)) - R_SCI0_BASE) /             \
				    DT_REG_SIZE(DT_INST_PARENT(inst)),                             \
			.rxi_ipl = DT_IPL_GET_BY_NAME(DT_INST_PARENT(inst), rxi),                  \
			.rxi_irq = DT_IRQ_GET_BY_NAME(DT_INST_PARENT(inst), rxi),                  \
			.txi_ipl = DT_IPL_GET_BY_NAME(DT_INST_PARENT(inst), txi),                  \
			.txi_irq = DT_IRQ_GET_BY_NAME(DT_INST_PARENT(inst), txi),                  \
			.tei_ipl = DT_IPL_GET_BY_NAME(DT_INST_PARENT(inst), tei),                  \
			.tei_irq = DT_IRQ_GET_BY_NAME(DT_INST_PARENT(inst), tei),                  \
			.eri_ipl = DT_IPL_GET_BY_NAME(DT_INST_PARENT(inst), eri),                  \
			.eri_irq = DT_IRQ_GET_BY_NAME(DT_INST_PARENT(inst), eri),                  \
			.p_callback = NULL,                                                        \
			.p_context = NULL,                                                         \
			.p_extend = &uart_renesas_ra_sci_config_##inst.fsp_extended_cfg,           \
		},                                                                                 \
		.fsp_extended_cfg = {                                                              \
			.p_baud_setting = &uart_renesas_ra_sci_config_##inst.fsp_baud_setting,     \
		},                                                                                 \
		IF_DISABLED(CONFIG_UART_RUNTIME_CONFIGURE, (UART_CFG_GET(inst)))};                 \
                                                                                                   \
	DEVICE_DT_INST_DEFINE(inst, uart_renesas_ra_sci_init, NULL,                                \
			      &uart_renesas_ra_sci_data_##inst,                                    \
			      &uart_renesas_ra_sci_config_##inst, POST_KERNEL,                     \
			      CONFIG_SERIAL_INIT_PRIORITY, &uart_renesas_ra_sci_api);

DT_INST_FOREACH_STATUS_OKAY(UART_RENESAS_RA_SCI_INIT)
