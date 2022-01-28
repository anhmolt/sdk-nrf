/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdio.h>
#include <kernel.h>
#include <zephyr.h>
#include <SEGGER_RTT.h>
#include <modem/trace_medium.h>

TRACE_MEDIUM_REGISTER(rtt, trace_medium_rtt_init, trace_medium_rtt_write, 0,
	CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT_PAUSED);

static int trace_rtt_channel;
static char rtt_buffer[CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT_BUF_SIZE];

int trace_medium_rtt_init(void)
{
	trace_rtt_channel = SEGGER_RTT_AllocUpBuffer("modem_trace",
		rtt_buffer, sizeof(rtt_buffer), SEGGER_RTT_MODE_NO_BLOCK_SKIP);

	if (trace_rtt_channel <= 0) {
		return -EFAULT;
	}

	return 0;
}

void trace_medium_rtt_write(const uint8_t *data, uint32_t len)
{
	uint32_t remaining_bytes = len;

	while (remaining_bytes) {
		uint8_t transfer_len = MIN(remaining_bytes,
					CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT_BUF_SIZE);
		uint32_t idx = len - remaining_bytes;

		SEGGER_RTT_WriteSkipNoLock(trace_rtt_channel, &data[idx], transfer_len);
		remaining_bytes -= transfer_len;
	}

	return;
}
