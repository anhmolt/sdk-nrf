/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdio.h>
#include <kernel.h>
#include <zephyr.h>
#include <modem/nrf_modem_lib_trace.h>
#include <nrf_modem_at.h>
#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART
#include <modem/trace_medium_uart.h>
#endif
#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT
#include <modem/trace_medium_rtt.h>
#endif

static uint32_t max_trace_size_bytes;
static uint32_t total_trace_size_rcvd;
static bool is_transport_initialized;

static void trace_stop_timer_handler(struct k_timer *timer);

K_TIMER_DEFINE(trace_stop_timer, trace_stop_timer_handler, NULL);

static void trace_stop_timer_handler(struct k_timer *timer)
{
	nrf_modem_lib_trace_stop();
}

int nrf_modem_lib_trace_init(void)
{
#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART
	is_transport_initialized = trace_medium_uart_init();
#endif
#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT
	is_transport_initialized = trace_medium_rtt_init();
#endif

	if (!is_transport_initialized) {
		return -EBUSY;
	}
	return 0;
}

int nrf_modem_lib_trace_start(enum nrf_modem_lib_trace_mode trace_mode, uint16_t duration,
			      uint32_t max_size)
{
	if (!is_transport_initialized) {
		return -ENXIO;
	}

	total_trace_size_rcvd = 0;

	max_trace_size_bytes = max_size;

	if (nrf_modem_at_printf("AT%%XMODEMTRACE=1,%hu", trace_mode) != 0) {
		return -EOPNOTSUPP;
	}

	if (duration != 0) {
		k_timer_start(&trace_stop_timer, K_SECONDS(duration), K_SECONDS(0));
	}

	return 0;
}

int nrf_modem_lib_trace_process(const uint8_t *data, uint32_t len)
{
	if (!is_transport_initialized) {
		return -ENXIO;
	}

	if (max_trace_size_bytes != 0) {
		total_trace_size_rcvd += len;

		if (total_trace_size_rcvd > max_trace_size_bytes) {
			/* Skip sending  to transport medium as the current trace won't fit.
			 * Disable traces (see API doc for reasoning).
			 */
			nrf_modem_lib_trace_stop();

			return 0;
		}
		if (total_trace_size_rcvd == max_trace_size_bytes) {
			nrf_modem_lib_trace_stop();
		}
	}

#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART
	trace_medium_uart_write(data, len);
#endif

#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT
	trace_medium_rtt_write(data, len);
#endif

	return 0;
}

int nrf_modem_lib_trace_stop(void)
{
	k_timer_stop(&trace_stop_timer);

	if (nrf_modem_at_printf("AT%%XMODEMTRACE=0") != 0) {
		return -EOPNOTSUPP;
	}

	return 0;
}
