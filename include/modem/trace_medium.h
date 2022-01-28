/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**@file trace_medium.h
 *
 * @defgroup trace_medium nRF91 Modem trace medium API
 * @{
 */
#ifndef TRACE_MEDIUM_H__
#define TRACE_MEDIUM_H__

#include <zephyr.h>

typedef int (*trace_medium_init_t)(void);
typedef void (*trace_medium_write_t)(const uint8_t *data, uint32_t len);
typedef int (*trace_medium_deinit_t)(void);

struct trace_medium_entry {
	const char *name;
	const trace_medium_init_t init;
	const trace_medium_write_t write;
	const trace_medium_deinit_t deinit;
	bool ready;
};

#define TRACE_MEDIUM_REGISTER(_name, _init, _write, _deinit)                                       \
	COND_CODE_0(_init, (), (static int _init(void);))                                          \
	COND_CODE_0(_write, (), (static void _write(const uint8_t *, uint32_t);))                  \
	COND_CODE_0(_deinit, (), (static int _deinit(void);))                                      \
	STRUCT_SECTION_ITERABLE(trace_medium_entry, trace_medium_##_name) = {                      \
		.name = #_name,                                                                    \
		.init = COND_CODE_0(_init, (NULL), (_init)),                                       \
		.write = COND_CODE_0(_write, (NULL), (_write)),                                    \
		.deinit = COND_CODE_0(_deinit, (NULL), (_deinit)),                                 \
		.ready = false,                                                                    \
	}

void trace_medium_init(void);

void trace_medium_deinit(void);

void trace_medium_write(const uint8_t *data, uint32_t len);

void trace_medium_write_done(void);

int trace_medium_select(const char *name);

#endif /* TRACE_MEDIUM_H__ */
/**@} */
