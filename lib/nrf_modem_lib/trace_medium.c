/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <stddef.h>
#include <string.h>
#include <zephyr.h>
#include <modem/trace_medium.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(trace_medium);

static struct trace_medium_entry *active_medium;

static void select_first_ready_medium(void)
{
	struct trace_medium_entry *new_medium = NULL;

	STRUCT_SECTION_FOREACH(trace_medium_entry, e) {
		if (e->ready) {
			new_medium = e;
			break;
		}
	}

	if (new_medium == NULL) {
		LOG_INF("Active trace medium: none");
		return;
	}

	active_medium = new_medium;
	LOG_INF("Active trace medium: %s", active_medium->name);
}

void trace_medium_init(void)
{
	int err;

	STRUCT_SECTION_FOREACH(trace_medium_entry, e) {
		if (e->ready || e->write == NULL) {
			continue;
		}
		if (e->init == NULL) {
			e->ready = true;
			continue;
		}

		err = e->init();
		if (err) {
			LOG_WRN("Failed to init trace medium %s, err %d", e->name, err);
		} else {
			e->ready = true;
		}
	}

#if defined(CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART)
	err = trace_medium_select("uart");
#elif defined(CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT)
	err = trace_medium_select("rtt");
#else
	err = -ENXIO;
#endif
	if (err) {
		select_first_ready_medium();
	}
}

void trace_medium_write(const uint8_t *data, uint32_t len)
{
	if (active_medium != NULL && active_medium->ready == true) {
		active_medium->write(data, len);
	}
}

void trace_medium_deinit(void)
{
	int err;

	STRUCT_SECTION_FOREACH(trace_medium_entry, e) {
		if (!e->ready) {
			continue;
		}
		if (e->deinit == NULL) {
			e->ready = false;
			continue;
		}

		err = e->deinit();
		if (err) {
			LOG_WRN("Failed to deinit trace medium %s, err %d", e->name, err);
		} else {
			e->ready = false;
		}
	}
}

int trace_medium_select(const char *name)
{
	struct trace_medium_entry *new_medium = NULL;

	if (name == NULL) {
		/* Deselect current trace backend, if any. */
		active_medium = NULL;
		LOG_INF("Active trace medium: none");
		return 0;
	}

	/* Loop through registered backends, searching for backend with name. */
	STRUCT_SECTION_FOREACH(trace_medium_entry, e) {
		if (strcmp(e->name, name)) {
			new_medium = e;
			break;
		}
	}

	/* Return error if no backend with name was found. */
	if (new_medium == NULL) {
		return -ENXIO;
	}

	active_medium = new_medium;
	LOG_INF("Active trace medium: %s", active_medium->name);
	return 0;
}
