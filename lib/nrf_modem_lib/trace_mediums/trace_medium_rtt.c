/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <modem/trace_medium.h>
#include <SEGGER_RTT.h>

LOG_MODULE_DECLARE(modem_trace_medium, CONFIG_MODEM_TRACE_MEDIUM_LOG_LEVEL);

static int trace_rtt_channel;
static char rtt_buffer[CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT_BUF_SIZE];

int trace_medium_init(void)
{
	trace_rtt_channel = SEGGER_RTT_AllocUpBuffer("modem_trace", rtt_buffer, sizeof(rtt_buffer),
						     SEGGER_RTT_MODE_NO_BLOCK_SKIP);

	if (trace_rtt_channel <= 0) {
		return -EBUSY;
	}
	return 0;
}

int trace_medium_deinit(void)
{
	/* trace_medium_write_Expect */
	return 0;
}

int trace_medium_write(const void *data, size_t len)
{
	uint8_t *buf = (uint8_t *)data;
	size_t remaining_bytes = len;

	while (remaining_bytes) {
		uint16_t transfer_len = MIN(remaining_bytes,
			CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT_BUF_SIZE);
		size_t idx = len - remaining_bytes;

		SEGGER_RTT_WriteSkipNoLock(trace_rtt_channel, &buf[idx], transfer_len);
		remaining_bytes -= transfer_len;
	}

	return len;
}
