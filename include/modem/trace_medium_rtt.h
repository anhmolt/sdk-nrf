/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**@file trace_medium_rtt.h
 *
 * @defgroup rtt trace medium for nrf_modem_lib_trace
 * @{
 */
#ifndef TRACE_MEDIUM_RTT_H__
#define TRACE_MEDIUM_RTT_H__

bool trace_medium_rtt_init(void);

int trace_medium_rtt_write(const uint8_t *data, uint32_t len);

#endif /* TRACE_MEDIUM_RTT_H__ */
/**@} */
