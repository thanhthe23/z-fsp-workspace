/*
 * Copyright (c) 2025 The.Thanh Nguyen <the.nguyen.yf@outlook.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "r_sci_uart.h"

int rp_sci_uart_get_char(uart_ctrl_t *const p_api_ctrl, uint8_t *p_char);
void rp_sci_uart_put_char(uart_ctrl_t *const p_api_ctrl, uint8_t out_char);
int rp_sci_uart_err_check(uart_ctrl_t *const p_api_ctrl);
