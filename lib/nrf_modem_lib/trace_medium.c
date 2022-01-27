/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <stddef.h>
#include <string.h>
#include <zephyr.h>
#include <modem/trace_medium.h>

static struct trace_medium_entry *medium = NULL;

int trace_medium_select(const char *name)
{
	int err;
	struct trace_medium_entry *new_medium = NULL;
	struct trace_medium_entry *old_medium = NULL;

	/* Deselect current trace backend, if any. */
	if (name == NULL) {
		if (medium != NULL) {
			old_medium = medium;
			medium = NULL;
			old_medium->deinit();
		}
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

	/* Return success if backend is already in use. */
	if (new_medium == medium) {
		return 0;
	}

	/* Initialize new backend first. */
	err = new_medium->init();
	if (err) {
		return err;
	}

	/* Switch to the new backend. */
	old_medium = medium;
	medium = new_medium;

	/* Lastly, deinitialize old backend. */
	if (old_medium != NULL) {
		old_medium->deinit();
	}

	return 0;
}

int trace_medium_write(const uint8_t *data, uint32_t len)
{
	if (medium == NULL) {
		return -EACCES;
	}
	return medium->write(data, len);
}
