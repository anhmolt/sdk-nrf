/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**@file trace_medium.h
 *
 * @defgroup trace_medium nRF91 Modem trace transport medium interface
 * @{
 */
#ifndef TRACE_MEDIUM_H__
#define TRACE_MEDIUM_H__

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Initialize the compile-time selected trace medium.
 *
 * @retval Zero on success.
 * @retval Negative error code on failure.
 */
int trace_medium_init(void);

/**
 * @brief Deinitialize the compile-time selected trace medium.
 *
 * @retval Zero on success.
 * @retval Negative error code on failure.
 */
int trace_medium_deinit(void);

/**
 * @brief Send trace data to the compile-time selected trace medium.
 *
 * @param data Memory buffer containing modem trace data.
 * @param len  Memory buffer length.
 * 
 * @return Number of bytes written. Negative error code on failure.
 */
int trace_medium_write(const void *data, size_t len);

#endif /* TRACE_MEDIUM_H__ */
/**@} */
