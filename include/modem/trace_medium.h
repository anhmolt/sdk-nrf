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

typedef int (*trace_medium_init_t)(void);
typedef int (*trace_medium_write_t)(const uint8_t *data, uint32_t len);
typedef int (*trace_medium_deinit_t)(void);

struct trace_medium_entry {
	char *name;
	const trace_medium_init_t init;
	const trace_medium_write_t write;
	const trace_medium_deinit_t deinit;
};

#define TRACE_MEDIUM_REGISTER(name, _init, _write, _deinit)                                        \
	static int _init(void);                                                                    \
	static int _write(const uint8_t *, uint32_t);                                              \
	static int _deinit(void);                                                                  \
	STRUCT_SECTION_ITERABLE(trace_medium_entry, trace_medium_##name) = {                       \
		.name = #name,                                                                     \
		.init = _init,                                                                     \
		.write = _write,                                                                   \
		.deinit = _deinit                                                                  \
	}

int trace_medium_select(const char *name);

int trace_medium_write(const uint8_t *data, uint32_t len);

#endif /* TRACE_MEDIUM_H__ */
/**@} */
