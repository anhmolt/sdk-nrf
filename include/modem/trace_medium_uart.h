/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**@file trace_medium_uart.h
 *
 * @defgroup uart trace medium for nrf_modem_lib_trace
 * @{
 */
#ifndef TRACE_MEDIUM_UART_H__
#define TRACE_MEDIUM_UART_H__

#include <stdbool.h>
#include <stdint.h>

bool trace_medium_uart_init(void);

void trace_medium_uart_deinit(void);

int trace_medium_uart_write(const uint8_t *data, uint32_t len);

#endif /* TRACE_MEDIUM_UART_H__ */
/**@} */
