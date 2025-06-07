/*
 * Copyright (c) 2025 The.Thanh Nguyen <the.nguyen.yf@outlook.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "rp_uart.h"

int rp_sci_uart_get_char(uart_ctrl_t *const p_api_ctrl, uint8_t *p_char)
{
	sci_uart_instance_ctrl_t *p_ctrl = (sci_uart_instance_ctrl_t *)p_api_ctrl;

#if (SCI_UART_CFG_PARAM_CHECKING_ENABLE)
	FSP_ASSERT(p_ctrl);
	FSP_ASSERT(p_char);
	FSP_ASSERT(0 != p_ctrl->open);
#endif

#if SCI_UART_CFG_FIFO_SUPPORT
	if (p_ctrl->fifo_depth > 0 ? p_ctrl->p_regs->FDR_b.R : p_ctrl->p_regs->SSR_b.RDRF)
#else
	if (p_ctrl->p_regs->SSR_b.RDRF)
#endif
	{
		/* Read the received data */
		*p_char = (uint8_t)(p_ctrl->p_regs->RDR & SCI_UART_FIFO_DAT_MASK);

		/* Clear the RDRF flag */
		p_ctrl->p_regs->SSR_b.RDRF = 0U;

		return 0;
	}

	return -1;
}

void rp_sci_uart_put_char(uart_ctrl_t *const p_api_ctrl, uint8_t out_char)
{
	sci_uart_instance_ctrl_t *p_ctrl = (sci_uart_instance_ctrl_t *)p_api_ctrl;

#if (SCI_UART_CFG_PARAM_CHECKING_ENABLE)
	FSP_ASSERT(p_ctrl);
	FSP_ASSERT(0 != p_ctrl->open);
#endif

#if SCI_UART_CFG_FIFO_SUPPORT
	if (p_ctrl->fifo_depth > 0) {
		while (p_ctrl->p_regs->FDR_b.T > 0x8) {
		}
		p_ctrl->p_regs->FTDRL = out_char;
	} else
#endif
	{
		while (p_ctrl->p_regs->SSR_b.TDRE == 0) {
		}
		p_ctrl->p_regs->TDR = out_char;
	}
}

int rp_sci_uart_err_check(uart_ctrl_t *const p_api_ctrl)
{
	sci_uart_instance_ctrl_t *p_ctrl = (sci_uart_instance_ctrl_t *)p_api_ctrl;
	int errors = 0;

#if (SCI_UART_CFG_PARAM_CHECKING_ENABLE)
	FSP_ASSERT(p_ctrl);
	FSP_ASSERT(0 != p_ctrl->open);
#endif

#if SCI_UART_CFG_FIFO_SUPPORT
	if (p_ctrl->fifo_depth > 0) {
		const uint8_t status = p_ctrl->p_regs->SSR_FIFO;

		if (status & R_SCI0_SSR_FIFO_ORER_Msk) {
			errors |= UART_ERROR_OVERRUN;
		}

		if (status & R_SCI0_SSR_FIFO_PER_Msk) {
			errors |= UART_ERROR_PARITY;
		}

		if (status & R_SCI0_SSR_FIFO_FER_Msk) {
			errors |= UART_ERROR_FRAMING;
		}

		p_ctrl->p_regs->SSR_FIFO &=
			~(status & (R_SCI0_SSR_FIFO_ORER_Msk | R_SCI0_SSR_FIFO_FER_Msk |
				    R_SCI0_SSR_FIFO_PER_Msk));
	} else
#endif
	{
		const uint8_t status = p_ctrl->p_regs->SSR;

		if (status & R_SCI0_SSR_ORER_Msk) {
			errors |= UART_ERROR_OVERRUN;
		}

		if (status & R_SCI0_SSR_PER_Msk) {
			errors |= UART_ERROR_PARITY;
		}

		if (status & R_SCI0_SSR_FER_Msk) {
			errors |= UART_ERROR_FRAMING;
		}

		p_ctrl->p_regs->SSR &=
			~(status & (R_SCI0_SSR_ORER_Msk | R_SCI0_SSR_FER_Msk | R_SCI0_SSR_PER_Msk));
	}

	return errors;
}
