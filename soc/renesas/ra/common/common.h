/*
 * Copyright (c) 2025 The.Thanh Nguyen <the.nguyen.yf@outlook.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_INST_IRQ_GET_BY_NAME(inst, name)                                                        \
	COND_CODE_1(DT_INST_IRQ_HAS_NAME(inst, name),                                              \
		    (DT_INST_IRQ_BY_NAME(inst, name, irq)),                                        \
		    (FSP_INVALID_VECTOR))

#define DT_INST_IPL_GET_BY_NAME(inst, name)                                                        \
	COND_CODE_1(DT_INST_IRQ_HAS_NAME(inst, name),                                              \
		    (DT_INST_IRQ_BY_NAME(inst, name, priority)),                                   \
		    (0))

#define DT_IRQ_GET_BY_NAME(node_id, name)                                                          \
	COND_CODE_1(DT_IRQ_HAS_NAME(node_id, name),                                                \
		    (DT_IRQ_BY_NAME(node_id, name)),                                               \
		    (FSP_INVALID_VECTOR))

#define DT_IPL_GET_BY_NAME(node_id, name)                                                          \
	COND_CODE_1(DT_IRQ_HAS_NAME(node_id, name),                                                \
		    (DT_IRQ_BY_NAME(node_id, name, priority)),                                     \
		    (0))
