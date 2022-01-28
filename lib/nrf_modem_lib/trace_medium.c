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
}

void trace_medium_write(const uint8_t *data, uint32_t len)
{
	STRUCT_SECTION_FOREACH(trace_medium_entry, e) {
		if (!e->paused && e->ready) {
			e->write(data, len);
		}
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

int trace_medium_pause(const char *name)
{
	int err = -EINVAL;

	STRUCT_SECTION_FOREACH(trace_medium_entry, e) {
		if (!strcmp(name, e->name)) {
			continue;
		}
		if (!e->ready) {
			err = -ENXIO;
			break;
		}
		if (!e->paused) {
			err = 0;
			e->paused = true;
		} else {
			err = -EALREADY;
		}
		break;
	}

	return err;
}

int trace_medium_resume(const char *name)
{
	int err = -EINVAL;

	STRUCT_SECTION_FOREACH(trace_medium_entry, e) {
		if (!strcmp(name, e->name)) {
			continue;
		}
		if (!e->ready) {
			err = -ENXIO;
			break;
		}
		if (e->paused) {
			err = 0;
			e->paused = false;
		} else {
			err = -EALREADY;
		}
		break;
	}

	return err;
}
