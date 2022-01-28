/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <stddef.h>
#include <string.h>
#include <zephyr.h>
#include <modem/trace_medium.h>
#include <nrf_modem.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(trace_medium);

static struct trace_medium_entry *active_medium;
static struct trace_medium_entry *new_medium;
static bool switch_active_medium;
static bool should_deinit_mediums;
static const uint8_t *trace_data;
static uint32_t trace_data_length;

static void select_first_ready_medium(void)
{
	struct trace_medium_entry *medium = NULL;

	STRUCT_SECTION_FOREACH(trace_medium_entry, e) {
		if (e->ready) {
			medium = e;
			break;
		}
	}

	new_medium = medium;
	switch_active_medium = true;
	if (new_medium == NULL) {
		LOG_INF("Active trace medium: none");
	} else {
		LOG_INF("Active trace medium: %s", new_medium->name);
	}
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



void trace_medium_deinit(void)
{
	should_deinit_mediums = true;
}

static void deinit_mediums(void)
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

void trace_medium_write(const uint8_t *data, uint32_t len)
{
	trace_data = data;
	trace_data_length = len;

	if (switch_active_medium) {
		active_medium = new_medium;
		switch_active_medium = true;
	}

	if (active_medium != NULL && active_medium->ready) {
		active_medium->write(trace_data, trace_data_length);
	} else {
		trace_medium_write_done();
	}
}

void trace_medium_write_done(void)
{
	int err;

	if (should_deinit_mediums) {
		should_deinit_mediums = false;
		deinit_mediums();
	}
}

int trace_medium_select(const char *name)
{
	struct trace_medium_entry *medium = NULL;

	if (name == NULL) {
		/* Deselect current trace backend, if any. */
		new_medium = NULL;
		switch_active_medium = true;
		LOG_INF("Active trace medium: none");
		return 0;
	}

	/* Loop through registered backends, searching for backend with name. */
	STRUCT_SECTION_FOREACH(trace_medium_entry, e) {
		if (strcmp(e->name, name) == 0) {
			medium = e;
			break;
		}
	}

	/* Return error if no backend with name was found. */
	if (medium == NULL) {
		return -ENXIO;
	}

	new_medium = medium;
	switch_active_medium = true;
	LOG_INF("Active trace medium: %s", new_medium->name);
	return 0;
}
