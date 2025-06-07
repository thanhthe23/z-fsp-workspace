/*
 * Copyright (c) 2025 The.Thanh Nguyen <the.nguyen.yf@outlook.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#if DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(rom_registers))

SECTION_PROLOGUE(.rom_registers,,)
{
	KEEP(*(.rom_registers*))
} GROUP_LINK_IN(ROM_REGISTERS)

#endif

#if DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(id_code))

SECTION_PROLOGUE(.id_code,,)
{
	KEEP(*(.id_code*))
} GROUP_LINK_IN(ID_CODE)

#endif
