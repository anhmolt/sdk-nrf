/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <unity.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <zephyr/kernel.h>

#include "nrf_modem_lib_trace.h"
#include "mock_trace_medium.h"
#include "mock_nrf_modem.h"

extern int unity_main(void);

static const char *start_trace_at_cmd_fmt = "AT%%XMODEMTRACE=1,%hu";
static unsigned int exp_trace_mode;
static int nrf_modem_at_printf_retval;

static K_HEAP_DEFINE(trace_heap, 128);

/* Suite teardown shall finalize with mandatory call to generic_suiteTearDown. */
extern int generic_suiteTearDown(int num_failures);

void setUp(void)
{
	mock_nrf_modem_Init();
	mock_trace_medium_Init();
}

void tearDown(void)
{
	mock_nrf_modem_Verify();
	mock_trace_medium_Verify();
}

static void nrf_modem_at_printf_ExpectTraceModeAndReturn(unsigned int trace_mode, int retval)
{
	exp_trace_mode = trace_mode;
	nrf_modem_at_printf_retval = retval;
}

/* Custom mock written for nrf_modem_at_printf since CMock can not be made to test parameters
 * of variadic functions.
 */
int nrf_modem_at_printf(const char *fmt, ...)
{
	va_list args;
	unsigned int trace_mode;

	TEST_ASSERT_EQUAL_STRING(start_trace_at_cmd_fmt, fmt);

	va_start(args, fmt);
	trace_mode = va_arg(args, unsigned int);
	va_end(args);

	TEST_ASSERT_EQUAL(exp_trace_mode, trace_mode);

	return nrf_modem_at_printf_retval;
}

int test_suiteTearDown(int num_failures)
{
	return generic_suiteTearDown(num_failures);
}

static const uint8_t *trace_processed_callback_buf_expect;
static uint32_t trace_processed_callback_len_expect;
static int trace_processed_callback_retval;

static int trace_processed_callback_stub(const uint8_t *buf, uint32_t len, int cmock_calls)
{
	TEST_ASSERT_EQUAL(buf, trace_processed_callback_buf_expect);
	TEST_ASSERT_EQUAL(len, trace_processed_callback_len_expect);
	return trace_processed_callback_retval;
}

/* Custom mock written for nrf_modem_trace_processed_callback since the regular mock
 * function, __wrap_nrf_modem_trace_processed_callback_ExpectAndReturn, erroneously
 * calls the fault handler when compiler is optimizing for size.
 */
static void trace_processed_callback_ExpectAndReturn(const uint8_t *buf, uint32_t len, int retval)
{
	trace_processed_callback_buf_expect = buf;
	trace_processed_callback_len_expect = len;
	trace_processed_callback_retval = retval;
	__wrap_nrf_modem_trace_processed_callback_Stub(trace_processed_callback_stub);
}

/* Test that nrf_modem_lib_trace_start returns error when modem trace module is not initialized. */
void test_modem_trace_start_when_not_initialized(void)
{
	TEST_ASSERT_EQUAL(-ENXIO, nrf_modem_lib_trace_start(NRF_MODEM_LIB_TRACE_ALL));
}

void test_modem_trace_init_medium(void)
{
	__wrap_trace_medium_init_ExpectAndReturn(0);

	TEST_ASSERT_EQUAL(0, nrf_modem_lib_trace_init(&trace_heap));
}

void test_modem_trace_start_coredump_only(void)
{
	nrf_modem_at_printf_ExpectTraceModeAndReturn(NRF_MODEM_LIB_TRACE_COREDUMP_ONLY, 0);
	TEST_ASSERT_EQUAL(0, nrf_modem_lib_trace_start(NRF_MODEM_LIB_TRACE_COREDUMP_ONLY));
}

void test_modem_trace_start_all(void)
{
	nrf_modem_at_printf_ExpectTraceModeAndReturn(NRF_MODEM_LIB_TRACE_ALL, 0);
	TEST_ASSERT_EQUAL(0, nrf_modem_lib_trace_start(NRF_MODEM_LIB_TRACE_ALL));
}

void test_modem_trace_start_ip_only(void)
{
	nrf_modem_at_printf_ExpectTraceModeAndReturn(NRF_MODEM_LIB_TRACE_IP_ONLY, 0);
	TEST_ASSERT_EQUAL(0, nrf_modem_lib_trace_start(NRF_MODEM_LIB_TRACE_IP_ONLY));
}

void test_modem_trace_start_lte_ip(void)
{
	nrf_modem_at_printf_ExpectTraceModeAndReturn(NRF_MODEM_LIB_TRACE_LTE_IP, 0);
	TEST_ASSERT_EQUAL(0, nrf_modem_lib_trace_start(NRF_MODEM_LIB_TRACE_LTE_IP));
}

/* Test that traces are forwarded to the trace medium when the module is correctly initialized
 * and nrf_modem_lib_trace_start has been called.
 */
void test_modem_trace_write_to_medium(void)
{
	int ret;
	const uint8_t sample_trace_data[10];

	__wrap_trace_medium_init_ExpectAndReturn(0);
	TEST_ASSERT_EQUAL(0, nrf_modem_lib_trace_init(&trace_heap));

	nrf_modem_at_printf_ExpectTraceModeAndReturn(NRF_MODEM_LIB_TRACE_ALL, 0);
	TEST_ASSERT_EQUAL(0, nrf_modem_lib_trace_start(NRF_MODEM_LIB_TRACE_ALL));

	/* Simulate reception of a modem trace. */
	ret = nrf_modem_lib_trace_process(sample_trace_data, sizeof(sample_trace_data));
	TEST_ASSERT_EQUAL(0, ret);

	__wrap_trace_medium_write_ExpectAndReturn(sample_trace_data, sizeof(sample_trace_data),
					   sizeof(sample_trace_data));
	trace_processed_callback_ExpectAndReturn(sample_trace_data, sizeof(sample_trace_data), 0);

	/* Let trace thread run. */
	k_sleep(K_MSEC(1));
}

/* With the module correctly initialized, test that traces are forwarded to the trace medium
 * even when nrf_modem_lib_trace_start has not yet been called.
 */
void test_modem_trace_process_when_modem_trace_start_was_not_called(void)
{
	int ret;
	const uint8_t sample_trace_data[10];

	__wrap_trace_medium_init_ExpectAndReturn(0);
	TEST_ASSERT_EQUAL(0, nrf_modem_lib_trace_init(&trace_heap));

	/* Simulate reception of a modem trace. */
	ret = nrf_modem_lib_trace_process(sample_trace_data, sizeof(sample_trace_data));
	TEST_ASSERT_EQUAL(0, ret);

	__wrap_trace_medium_write_ExpectAndReturn(sample_trace_data, sizeof(sample_trace_data),
					   sizeof(sample_trace_data));
	trace_processed_callback_ExpectAndReturn(sample_trace_data, sizeof(sample_trace_data), 0);

	/* Let trace thread run. */
	k_sleep(K_MSEC(1));
}

/* Test that trace_medium_write is not called when trace module fails to initialize, and that
 * nrf_modem_trace_processed_callback is called even when initialization fails.
 */
void test_modem_trace_write_when_medium_init_fails(void)
{
	int ret;
	const uint8_t sample_trace_data[10];

	__wrap_trace_medium_init_ExpectAndReturn(-EBUSY);
	TEST_ASSERT_EQUAL(-EBUSY, nrf_modem_lib_trace_init(&trace_heap));
	TEST_ASSERT_EQUAL(-ENXIO, nrf_modem_lib_trace_start(NRF_MODEM_LIB_TRACE_ALL));

	/* Verify that nrf_modem_trace_processed_callback is called even when initialization
	 * of trace module failed.
	 */
	trace_processed_callback_ExpectAndReturn(sample_trace_data, sizeof(sample_trace_data), 0);

	/* Simulate the reception of a modem trace and expect no calls to trace_medium_write. */
	ret = nrf_modem_lib_trace_process(sample_trace_data, sizeof(sample_trace_data));
	TEST_ASSERT_EQUAL(-ENXIO, ret);
}

/* Test that trace_medium_write is not called when trace module has not been initialized, and that
 * nrf_modem_trace_processed_callback is called even when trace module has not been initialized.
 */
void test_modem_trace_process_when_not_initialized(void)
{
	int ret;
	const uint8_t sample_trace_data[10];

	/* Verify that nrf_modem_trace_processed_callback is called even when the trace module
	 * has not been initialized.
	 */
	trace_processed_callback_ExpectAndReturn(sample_trace_data, sizeof(sample_trace_data), 0);

	/* Simulate the reception of a modem trace and expect no calls to trace_medium_write. */
	ret = nrf_modem_lib_trace_process(sample_trace_data, sizeof(sample_trace_data));
	TEST_ASSERT_EQUAL(-ENXIO, ret);
}

/* Test nrf_modem_lib_trace_start when nrf_modem_at_printf returns fails. */
void test_modem_trace_start_when_nrf_modem_at_printf_fails(void)
{
	__wrap_trace_medium_init_ExpectAndReturn(0);
	TEST_ASSERT_EQUAL(0, nrf_modem_lib_trace_init(&trace_heap));

	/* Make nrf_modem_at_printf return failure. */
	nrf_modem_at_printf_ExpectTraceModeAndReturn(NRF_MODEM_LIB_TRACE_ALL, -1);
	TEST_ASSERT_EQUAL(-EOPNOTSUPP, nrf_modem_lib_trace_start(NRF_MODEM_LIB_TRACE_ALL));
}

void test_modem_trace_stop(void)
{
	TEST_ASSERT_EQUAL(0, nrf_modem_lib_trace_stop());
}

/* Test that no more traces are forwarded to the trace medium after trace_stop have been called. */
void test_modem_trace_no_traces_after_trace_stop(void)
{
	int ret;
	const uint8_t sample_trace_data[10];

	__wrap_trace_medium_init_ExpectAndReturn(0);
	TEST_ASSERT_EQUAL(0, nrf_modem_lib_trace_init(&trace_heap));

	TEST_ASSERT_EQUAL(0, nrf_modem_lib_trace_stop());

	/* Simulate reception of a modem trace. */
	ret = nrf_modem_lib_trace_process(sample_trace_data, sizeof(sample_trace_data));
	TEST_ASSERT_EQUAL(0, ret);

	/* The test will fail if traces are forwarded to the trace medium because
	 * trace_medium_write will be called more times than expected (0).
	 */
	trace_processed_callback_ExpectAndReturn(sample_trace_data, sizeof(sample_trace_data), 0);

	/* Let trace thread run. */
	k_sleep(K_MSEC(1));
}

/* Test trace data sent to trace medium before and after trace_stop, and again after trace_start. */
void test_modem_trace_more_traces_after_trace_stop_start(void)
{
	int ret;
	const uint8_t sample_trace_data_1[10];
	const uint8_t sample_trace_data_2[11];
	const uint8_t sample_trace_data_3[12];

	__wrap_trace_medium_init_ExpectAndReturn(0);
	TEST_ASSERT_EQUAL(0, nrf_modem_lib_trace_init(&trace_heap));

	/* Simulate reception of 1st trace data piece. */
	ret = nrf_modem_lib_trace_process(sample_trace_data_1, sizeof(sample_trace_data_1));
	TEST_ASSERT_EQUAL(0, ret);

	/* Let trace thread run, expect 1st trace data piece to be forwarded to trace medium. */
	__wrap_trace_medium_write_ExpectAndReturn(sample_trace_data_1, sizeof(sample_trace_data_1),
					   sizeof(sample_trace_data_1));
	trace_processed_callback_ExpectAndReturn(sample_trace_data_1,
						 sizeof(sample_trace_data_1), 0);
	k_sleep(K_MSEC(1));

	/* Simulate reception of 2nd trace data piece. */
	ret = nrf_modem_lib_trace_process(sample_trace_data_2, sizeof(sample_trace_data_2));
	TEST_ASSERT_EQUAL(0, ret);

	/* Stop forwarding of trace data to medium. */
	TEST_ASSERT_EQUAL_MESSAGE(0, nrf_modem_lib_trace_stop(), "stop failed");

	/* Let trace thread run, expect 2nd trace data piece to be 'discarded', not forwarded. */
	trace_processed_callback_ExpectAndReturn(sample_trace_data_2,
						 sizeof(sample_trace_data_2), 0);
	k_sleep(K_MSEC(1));

	/* Start forwarding of trace data. */
	nrf_modem_at_printf_ExpectTraceModeAndReturn(NRF_MODEM_LIB_TRACE_ALL, 0);
	TEST_ASSERT_EQUAL_MESSAGE(0,
		nrf_modem_lib_trace_start(NRF_MODEM_LIB_TRACE_ALL), "start failed");

	/* Simulate reception of 3rd trace data piece. */
	ret = nrf_modem_lib_trace_process(sample_trace_data_3, sizeof(sample_trace_data_3));
	TEST_ASSERT_EQUAL(0, ret);

	/* Let trace thread run, expect 3rd trace data piece to be forwarded to trace medium. */
	__wrap_trace_medium_write_ExpectAndReturn(sample_trace_data_3, sizeof(sample_trace_data_3),
					   sizeof(sample_trace_data_3));
	trace_processed_callback_ExpectAndReturn(sample_trace_data_3,
						 sizeof(sample_trace_data_3), 0);
	k_sleep(K_MSEC(1));
}

void main(void)
{
	(void)unity_main();
}
