# Copyright (c) 2025 The.Thanh Nguyen <the.nguyen.yf@outlook.com>
# SPDX-License-Identifier: Apache-2.0

zephyr_library_amend()
zephyr_library_sources_ifdef(CONFIG_UART_RENESAS_RA_SCI uart_renesas_ra_sci.c)
