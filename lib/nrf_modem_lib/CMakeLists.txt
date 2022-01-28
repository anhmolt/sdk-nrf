#
# Copyright (c) 2018 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
zephyr_library()
zephyr_library_sources(nrf_modem_lib.c)
zephyr_library_sources(nrf_modem_os.c)
zephyr_library_sources_ifdef(CONFIG_NET_SOCKETS nrf91_sockets.c)
zephyr_library_sources_ifdef(CONFIG_NRF_MODEM_LIB_TRACE_ENABLED nrf_modem_lib_trace.c)
zephyr_library_sources_ifdef(CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART trace_medium_uart.c)
zephyr_library_sources_ifdef(CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT trace_medium_rtt.c)
zephyr_library_sources(errno_sanity.c)
zephyr_library_sources(shmem_sanity.c)
