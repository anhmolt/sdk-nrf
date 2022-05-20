/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <modem/nrf_modem_lib_trace.h>
#include <modem/trace_medium.h>
#include <nrf_modem.h>
#include <nrf_modem_at.h>

LOG_MODULE_REGISTER(nrf_modem_lib_trace, CONFIG_NRF_MODEM_LIB_LOG_LEVEL);

static bool is_transport_initialized;
static bool is_stopped;

int nrf_modem_lib_trace_init(struct k_heap *trace_heap)
{
	int err;

	ARG_UNUSED(trace_heap);
	is_stopped = false;

	err = trace_medium_init();
	if (err) {
		is_transport_initialized = false;
		return -EBUSY;
	}

	is_transport_initialized = true;
	return 0;
}

int nrf_modem_lib_trace_start(enum nrf_modem_lib_trace_mode trace_mode)
{
	if (!is_transport_initialized) {
		return -ENXIO;
	}

	if (nrf_modem_at_printf("AT%%XMODEMTRACE=1,%hu", trace_mode) != 0) {
		return -EOPNOTSUPP;
	}

	is_stopped = false;

	return 0;
}

int nrf_modem_lib_trace_process(const uint8_t *data, uint32_t len)
{
	int err;

	if (!is_stopped && is_transport_initialized) {
		trace_medium_write(data, len);
	}

	err = nrf_modem_trace_processed_callback(data, len);
	if (err) {
		LOG_ERR("nrf_modem_trace_processed_callback failed, err %d", err);
	}

	if (!is_transport_initialized) {
		return -ENXIO;
	}

	return 0;
}

int nrf_modem_lib_trace_stop(void)
{
	__ASSERT(!k_is_in_isr(), "%s cannot be called from interrupt context", __func__);

	/* Don't use the AT%%XMODEMTRACE=0 command to disable traces because the
	 * modem won't respond if the modem has crashed and is outputting the modem
	 * core dump.
	 */

	is_stopped = true;

	return 0;
}

void nrf_modem_lib_trace_deinit(void)
{
	is_transport_initialized = false;
	trace_medium_deinit();
}
