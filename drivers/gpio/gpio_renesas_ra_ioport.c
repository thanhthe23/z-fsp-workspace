/*
 * Copyright (c) 2025 The.Thanh Nguyen <the.nguyen.yf@outlook.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT renesas_ra_ioport

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <soc.h>

#include "r_ioport.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(renesas_ra_ioport);

struct renesas_ra_ioport_config {
	const uint32_t port_num;
	const uint32_t pin_mask;
};

struct renesas_ra_ioport_data {
	struct st_ioport_instance fsp_instance;
	struct st_ioport_instance_ctrl fsp_instance_ctrl;
	struct st_ioport_cfg fsp_instance_cfg;
};

static int renesas_ra_ioport_pin_configure(const struct device *port, gpio_pin_t pin,
					   gpio_flags_t flags);
{
	const struct renesas_ra_ioport_config *config = port->config;
	struct renesas_ra_ioport_data *data = port->data;
	struct st_ioport_instance *fsp_instance = &data->fsp_instance;
	ioport_cfg_t ioport_cfg;
	ioport_pin_cfg_t pin_cfg_data;
	fsp_err_t err;

	if (BIT(pin) & ~config->pin_mask) {
		LOG_DBG("Pin %d is out of range", pin);
		return -EINVAL;
	}

	if (mask & GPIO_INT_MASK) {
		LOG_DBG("Interrupt flags are not supported");
		return -ENOTSUP;
	}

	ioport_cfg.number_of_pins = 1;
	ioport_cfg.p_pin_cfg_data = &pin_cfg_data;
	pin_cfg_data.pin = pin;

	if (flags & GPIO_OUTPUT) {
		pin_cfg_data.pin_cfg = IOPORT_CFG_PORT_DIRECTION_OUTPUT;
		if (flags & GPIO_OUTPUT_INIT_HIGH) {
			pin_cfg_data.pin_cfg |= IOPORT_CFG_PORT_OUTPUT_HIGH;
		} else if (flags & GPIO_OUTPUT_INIT_LOW) {
			pin_cfg_data.pin_cfg |= IOPORT_CFG_PORT_OUTPUT_LOW;
		}
	} else {
		pin_cfg_data.pin_cfg = IOPORT_CFG_PORT_DIRECTION_INPUT;
	}

	err = fsp_instance->p_api.pinCfg(fsp_instance->p_ctrl, &ioport_cfg);
	if (err != FSP_SUCCESS) {
		LOG_DBG("Failed to configure pin %d: fsp_err: %d", pin, err);
		return -EIO;
	}

	return 0;
}

static int renesas_ra_ioport_port_get_raw(const struct device *port, gpio_port_value_t *value)
{
	const struct renesas_ra_ioport_config *config = port->config;
	struct renesas_ra_ioport_data *data = port->data;
	struct st_ioport_instance *fsp_instance = &data->fsp_instance;
	fsp_err_t err;

	err = fsp_instance->p_api.portRead(fsp_instance->p_ctrl, (bsp_io_port_t)config->port_num,
					   (ioport_size_t)value);
	if (err != FSP_SUCCESS) {
		LOG_DBG("Failed to read port %d: fsp_err: %d", config->port_num, err);
		return -EIO;
	}

	return 0;
}

static int renesas_ra_ioport_port_set_masked_raw(const struct device *port, gpio_port_pins_t mask,
						 gpio_port_value_t value)
{
	const struct renesas_ra_ioport_config *config = port->config;
	struct renesas_ra_ioport_data *data = port->data;
	struct st_ioport_instance *fsp_instance = &data->fsp_instance;
	fsp_err_t err;

	if (mask & ~config->pin_mask) {
		LOG_DBG("Mask %08x is out of range for port %d", mask, config->port_num);
		return -EINVAL;
	}

	err = fsp_instance->p_api.portWrite(fsp_instance->p_ctrl, (bsp_io_port_t)config->port_num,
					    (ioport_size_t)value, (ioport_size_t)mask);
	if (err != FSP_SUCCESS) {
		LOG_DBG("Failed to set masked raw on port %d: fsp_err: %d", config->port_num, err);
		return -EIO;
	}

	return 0;
}

static int renesas_ra_ioport_port_set_bits_raw(const struct device *port, gpio_port_pins_t pins)
{
	const struct renesas_ra_ioport_config *config = port->config;
	struct renesas_ra_ioport_data *data = port->data;
	struct st_ioport_instance *fsp_instance = &data->fsp_instance;
	bsp_io_port_pin_t port_pin = (bsp_io_port_pin_t)((config->port_num << 8) | pins);
	fsp_err_t err;

	if (pins & ~config->pin_mask) {
		LOG_DBG("Pins %08x are out of range for port %d", pins, config->port_num);
		return -EINVAL;
	}

	err = fsp_instance->p_api.pinWrite(fsp_instance->p_ctrl, port_pin, BSP_IO_LEVEL_HIGH);
	if (err != FSP_SUCCESS) {
		LOG_DBG("Failed to set bits raw on port %d: fsp_err: %d", config->port_num, err);
		return -EIO;
	}

	return 0;
}

static int renesas_ra_ioport_port_clear_bits_raw(const struct device *port, gpio_port_pins_t pins)
{
	const struct renesas_ra_ioport_config *config = port->config;
	struct renesas_ra_ioport_data *data = port->data;
	struct st_ioport_instance *fsp_instance = &data->fsp_instance;
	bsp_io_port_pin_t port_pin = (bsp_io_port_pin_t)((config->port_num << 8) | pins);
	fsp_err_t err;

	if (pins & ~config->pin_mask) {
		LOG_DBG("Pins %08x are out of range for port %d", pins, config->port_num);
		return -EINVAL;
	}

	err = fsp_instance->p_api.pinWrite(fsp_instance->p_ctrl, port_pin, BSP_IO_LEVEL_LOW);
	if (err != FSP_SUCCESS) {
		LOG_DBG("Failed to set bits raw on port %d: fsp_err: %d", config->port_num, err);
		return -EIO;
	}

	return 0;
}

static int renesas_ra_ioport_port_toggle_bits(const struct device *port, gpio_port_pins_t pins)
{
	const struct renesas_ra_ioport_config *config = port->config;
	struct renesas_ra_ioport_data *data = port->data;
	struct st_ioport_instance *fsp_instance = &data->fsp_instance;
	bsp_io_port_pin_t port_pin = (bsp_io_port_pin_t)((config->port_num << 8) | pins);
	bsp_io_level_t pin_level;
	fsp_err_t err;

	if (pins & ~config->pin_mask) {
		LOG_DBG("Pins %08x are out of range for port %d", pins, config->port_num);
		return -EINVAL;
	}

	err = fsp_instance->p_api.pinRead(fsp_instance->p_ctrl, port_pin, &pin_level);
	if (err != FSP_SUCCESS) {
		LOG_DBG("Failed to read pin %d: fsp_err: %d", pins, err);
		return -EIO;
	}

	err = fsp_instance->p_api.pinWrite(fsp_instance->p_ctrl, port_pin, !pin_level);
	if (err != FSP_SUCCESS) {
		LOG_DBG("Failed to set bits raw on port %d: fsp_err: %d", config->port_num, err);
		return -EIO;
	}

	return 0;
}

static int renesas_ra_ioport_init(const struct device *dev)
{
	const struct renesas_ra_ioport_config *config = dev->config;
	struct renesas_ra_ioport_data *data = dev->data;
	struct st_ioport_instance *fsp_instance = &data->fsp_instance;
	fsp_err_t err;

	err = fsp_instance->p_api.open(fsp_instance->p_ctrl, fsp_instance->p_cfg);
	if (err != FSP_SUCCESS) {
		LOG_DBG("Failed to open IOPORT: fsp_err: %d", err);
		return -EIO;
	}

	return 0;
}

static DEVICE_API(gpio, renesas_ra_ioport_api) = {
	.pin_configure = renesas_ra_ioport_pin_configure,
	.port_get_raw = renesas_ra_ioport_port_get_raw,
	.port_set_masked_raw = renesas_ra_ioport_port_set_masked_raw,
	.port_set_bits_raw = renesas_ra_ioport_port_set_bits_raw,
	.port_clear_bits_raw = renesas_ra_ioport_port_clear_bits_raw,
	.port_toggle_bits = renesas_ra_ioport_port_toggle_bits,
};

#define GPIO_RENESAS_RA_IOPORT_INIT(inst)                                                          \
	static struct st_ioport_cfg renesas_ra_ioport_cfg_##inst[DT_INST_PROP(inst, ngpios)];      \
                                                                                                   \
	static struct renesas_ra_ioport_data renesas_ra_ioport_data_##inst = {                     \
		.fsp_instance =                                                                    \
			{                                                                          \
				.p_ctrl = &renesas_ra_ioport_data_##inst.fsp_instance_ctrl,        \
				.p_cfg = &renesas_ra_ioport_data_##inst.fsp_instance_cfg,          \
				.p_api = &g_ioport_on_ioport,                                      \
			},                                                                         \
		.fsp_instance_cfg =                                                                \
			{                                                                          \
				.number_of_pins = DT_INST_PROP(inst, ngpios),                      \
				.p_pin_cfg_data = renesas_ra_ioport_cfg_##inst,                    \
			},                                                                         \
	};                                                                                         \
                                                                                                   \
	static const struct renesas_ra_ioport_config renesas_ra_ioport_config_##inst = {           \
		.port_num = (DT_INST_REG_ADDR(inst) - R_PORT0_BASE) / DT_INST_REG_SIZE(inst),      \
		.pin_mask =                                                                        \
			GPIO_DT_INST_PORT_PIN_MASK_NGPIOS_EXC(inst, DT_INST_PROP(inst, ngpios)),   \
	};                                                                                         \
                                                                                                   \
	DEVICE_DT_INST_DEFINE(inst, renesas_ra_ioport_init, NULL, &renesas_ra_ioport_data_##inst,  \
			      &renesas_ra_ioport_config_##inst, PRE_KERNEL_1,                      \
			      CONFIG_GPIO_INIT_PRIORITY, &renesas_ra_ioport_api);

DT_INST_FOREACH_STATUS_OKAY(GPIO_RENESAS_RA_IOPORT_INIT)
