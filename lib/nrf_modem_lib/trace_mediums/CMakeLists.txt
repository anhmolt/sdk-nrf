#
# Copyright (c) 2022 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
if(CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART)
# Feature toggle for experimental thread based trace processing
zephyr_library_sources_ifndef(
  CONFIG_NRF_MODEM_LIB_TRACE_THREAD_PROCESSING
  trace_medium_uart_sync.c
)
zephyr_library_sources_ifdef(
  CONFIG_NRF_MODEM_LIB_TRACE_THREAD_PROCESSING
  trace_medium_uart.c
)
endif()

zephyr_library_sources_ifdef(
  CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT
  trace_medium_rtt.c
)
