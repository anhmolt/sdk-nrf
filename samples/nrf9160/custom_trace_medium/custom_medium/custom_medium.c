/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/zephyr.h>
#include <zephyr/logging/log.h>
#include <debug/cpu_load.h>
#include <modem/trace_medium.h>
#include <sys/errno.h>

LOG_MODULE_DECLARE(modem_trace_medium, CONFIG_MODEM_TRACE_MEDIUM_LOG_LEVEL);

#define TIME_INTERVAL_MSEC 1000

static int64_t  timeout;
static uint32_t num_bytes;
static uint32_t num_bytes_prev;
static bool show_cpu_load;

int trace_medium_init(void)
{
	int err;

	err = cpu_load_init();
	if (err) {
		if (err == -ENODEV) {
			LOG_WRN("Failed to allocate PPI channels for cpu load estimation");
		} else if (err == -EBUSY) {
			LOG_WRN("Failed to allocate TIMER instance for cpu load estimation");
		} else {
			LOG_WRN("Failed to initialize cpu load module");
		}
		show_cpu_load = false;
	} else {
		show_cpu_load = true;
		cpu_load_reset();
	}

	timeout = k_uptime_get() + TIME_INTERVAL_MSEC;
	num_bytes_prev = num_bytes;

	LOG_INF("Custom trace medium initialized");
	return 0;
}

int trace_medium_deinit(void)
{
	printk("trace medium printk deinitialized\n");
	return 0;
}

int trace_medium_write(const uint8_t *data, uint32_t len)
{
	int64_t time_now;
	float throughput;
	float cpu_load_percent;

	ARG_UNUSED(data);

	num_bytes += len;

	time_now = k_uptime_get();
	if (time_now < timeout) {
		return len;
	}

	throughput = (num_bytes - num_bytes_prev)/(TIME_INTERVAL_MSEC/1000.0);
	LOG_INF("Trace data rate: %.3f kB/s, total: %.3f kB", throughput, num_bytes/1000.0);

	if (show_cpu_load) {
		cpu_load_percent = cpu_load_get()/1000.0;
		LOG_INF("CPU load: %.1f", cpu_load_percent);
		cpu_load_reset();
	}

	timeout = time_now + TIME_INTERVAL_MSEC;
	num_bytes_prev = num_bytes;

	return len;
}
