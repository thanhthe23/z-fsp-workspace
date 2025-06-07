# Copyright (c) 2025 The.Thanh Nguyen <the.nguyen.yf@outlook.com>
# SPDX-License-Identifier: Apache-2.0

board_runner_args(pyocd "--target=r7fa4m1ab")
board_runner_args(jlink "--device=r7fa4m1ab")

include(${ZEPHYR_BASE}/boards/common/pyocd.board.cmake)
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
