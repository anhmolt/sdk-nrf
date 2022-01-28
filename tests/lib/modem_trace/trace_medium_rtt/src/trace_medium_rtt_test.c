/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <unity.h>
#include <stdbool.h>
#include <zephyr/kernel.h>
#include <string.h>

#include "trace_medium.h"
#include "mock_SEGGER_RTT.h"

extern int unity_main(void);

static int trace_rtt_channel;

/* Suite teardown shall finalize with mandatory call to generic_suiteTearDown. */
extern int generic_suiteTearDown(int num_failures);

void setUp(void)
{
	mock_SEGGER_RTT_Init();
}

void tearDown(void)
{
	mock_SEGGER_RTT_Verify();
}

int test_suiteTearDown(int num_failures)
{
	return generic_suiteTearDown(num_failures);
}

static int rtt_allocupbuffer_callback(const char *sName, void *pBuffer, unsigned int BufferSize,
				      unsigned int Flags, int no_of_calls)
{
	char *exp_sName = "modem_trace";

	TEST_ASSERT_EQUAL_CHAR_ARRAY(exp_sName, sName, sizeof(exp_sName));
	TEST_ASSERT_NOT_EQUAL(NULL, pBuffer);
	TEST_ASSERT_EQUAL(CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT_BUF_SIZE, BufferSize);
	TEST_ASSERT_EQUAL(SEGGER_RTT_MODE_NO_BLOCK_SKIP, Flags);

	return trace_rtt_channel;
}

static void trace_session_setup_with_rtt_transport(void)
{
	__wrap_SEGGER_RTT_AllocUpBuffer_ExpectAnyArgsAndReturn(trace_rtt_channel);
	__wrap_SEGGER_RTT_AllocUpBuffer_AddCallback(&rtt_allocupbuffer_callback);
	TEST_ASSERT_EQUAL(0, trace_medium_init());
}

static void expect_rtt_write(const uint8_t *buffer, uint16_t size)
{
	__wrap_SEGGER_RTT_ASM_WriteSkipNoLock_ExpectAndReturn(trace_rtt_channel, buffer, size,
							      size);
}

void test_trace_medium_rtt_init_succeed(void)
{
	trace_rtt_channel = 1;
	trace_session_setup_with_rtt_transport();
}

void test_trace_medium_rtt_init_fail(void)
{
	/* Simulate failure by returning negative RTT channel. */
	__wrap_SEGGER_RTT_AllocUpBuffer_ExpectAnyArgsAndReturn(-1);

	TEST_ASSERT_EQUAL(-EBUSY, trace_medium_init());
}

/* Test that when RTT is configured as trace transport, the traces are forwarded to RTT API. */
void test_trace_medium_rtt_write(void)
{
	const uint16_t sample_trace_buffer_size = 300;
	const uint8_t sample_trace_data[sample_trace_buffer_size];

	trace_rtt_channel = 1;
	trace_session_setup_with_rtt_transport();

	/* Since the trace buffer size is larger than NRF_MODEM_LIB_TRACE_MEDIUM_RTT_BUF_SIZE,
	 * the modem_trace module should fragment the buffer and call the RTT API twice.
	 */
	expect_rtt_write(sample_trace_data, CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT_BUF_SIZE);
	expect_rtt_write(&sample_trace_data[CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT_BUF_SIZE],
			 sizeof(sample_trace_data) -
				CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT_BUF_SIZE);

	/* Simulate the reception of modem trace and expect the RTT API to be called. */
	trace_medium_write(sample_trace_data, sizeof(sample_trace_data));
}

void main(void)
{
	(void)unity_main();
}
