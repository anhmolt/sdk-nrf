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
#include "mock_nrfx_uarte.h"
#include "mock_pinctrl.h"

extern int unity_main(void);

static const nrfx_uarte_t *p_uarte_inst_in_use;
/* Variable to store the event_handler registered by the modem_trace module.*/
static nrfx_uarte_event_handler_t uarte_callback;

/* Suite teardown shall finalize with mandatory call to generic_suiteTearDown. */
extern int generic_suiteTearDown(int num_failures);

void nrfx_isr(const void *irq_handler)
{
	/* Declared only for pleasing linker. Never expected to be called. */
	TEST_ASSERT(false);
}

void setUp(void)
{
	mock_nrfx_uarte_Init();
	mock_pinctrl_Init();
}

void tearDown(void)
{
	mock_nrfx_uarte_Verify();
	mock_pinctrl_Verify();

	p_uarte_inst_in_use = NULL;
	uarte_callback = NULL;
}

static void uart_tx_done_simulate(const uint8_t *const data, size_t len)
{
	nrfx_uarte_event_t uarte_event;

	uarte_event.type = NRFX_UARTE_EVT_TX_DONE;
	uarte_event.data.rxtx.bytes = len;
	uarte_event.data.rxtx.p_data = (uint8_t *)data;

	uarte_callback(&uarte_event, NULL);
}

static void uart_tx_error_simulate(void)
{
	nrfx_uarte_event_t uarte_event;

	uarte_event.type = NRFX_UARTE_EVT_ERROR;

	uarte_callback(&uarte_event, NULL);
}

int test_suiteTearDown(int num_failures)
{
	return generic_suiteTearDown(num_failures);
}

static nrfx_err_t nrfx_uarte_init_callback(nrfx_uarte_t const *p_instance,
					   nrfx_uarte_config_t const *p_config,
					   nrfx_uarte_event_handler_t event_handler,
					   int no_of_calls)
{
	TEST_ASSERT_NOT_EQUAL(NULL, p_config);
	TEST_ASSERT_EQUAL(true, p_config->skip_gpio_cfg);
	TEST_ASSERT_EQUAL(true, p_config->skip_psel_cfg);
	TEST_ASSERT_EQUAL(NRF_UARTE_HWFC_DISABLED, p_config->hal_cfg.hwfc);
	TEST_ASSERT_EQUAL(NRF_UARTE_PARITY_EXCLUDED, p_config->hal_cfg.parity);
	TEST_ASSERT_EQUAL(NRF_UARTE_BAUDRATE_1000000, p_config->baudrate);
	TEST_ASSERT_EQUAL(1, p_config->interrupt_priority);
	TEST_ASSERT_EQUAL(NULL, p_config->p_context);

	TEST_ASSERT_NOT_EQUAL(NULL, p_instance);
	TEST_ASSERT_EQUAL(NRFX_UARTE1_INST_IDX, p_instance->drv_inst_idx);
	p_uarte_inst_in_use = p_instance;

	TEST_ASSERT_NOT_EQUAL(NULL, event_handler);

	uarte_callback = event_handler;

	return NRFX_SUCCESS;
}

void initialize_trace_medium_uart_success(void)
{
	__wrap_pinctrl_lookup_state_ExpectAnyArgsAndReturn(0);
	__wrap_pinctrl_configure_pins_ExpectAnyArgsAndReturn(0);
	__wrap_nrfx_uarte_init_ExpectAnyArgsAndReturn(NRFX_SUCCESS);
	__wrap_nrfx_uarte_init_AddCallback(&nrfx_uarte_init_callback);
	TEST_ASSERT_EQUAL(0, trace_medium_init());
}

const uint8_t *trace_data;
uint32_t trace_data_len;
static K_SEM_DEFINE(receive_traces_sem, 0, 1);
bool is_waiting_on_traces;

void send_traces_for_processing(const uint8_t *data, uint32_t len)
{
	trace_data = data;
	trace_data_len = len;
	k_sem_give(&receive_traces_sem);
}

#define TRACE_TEST_THREAD_STACK_SIZE 512
#define TRACE_THREAD_PRIORITY CONFIG_NRF_MODEM_LIB_TRACE_THREAD_PRIO

void trace_test_thread(void)
{
	while (1) {
		is_waiting_on_traces = true;
		k_sem_take(&receive_traces_sem, K_FOREVER);
		is_waiting_on_traces = false;

		trace_medium_write(trace_data, trace_data_len);
	}
}

K_THREAD_DEFINE(trace_test_thread_id, TRACE_TEST_THREAD_STACK_SIZE, trace_test_thread,
	NULL, NULL, NULL, TRACE_THREAD_PRIORITY, 0, 0);


/* Test that uart trace medium returns zero when NRFX UART Init succeeds. */
void test_trace_medium_uart_init_success(void)
{
	initialize_trace_medium_uart_success();
}

/* Test that uart trace medium return error when NRFX UART Init fails. */
void test_trace_medium_uart_init_fails(void)
{
	/* Simulate uart init failure. */
	__wrap_pinctrl_lookup_state_ExpectAnyArgsAndReturn(0);
	__wrap_pinctrl_configure_pins_ExpectAnyArgsAndReturn(0);
	__wrap_nrfx_uarte_init_ExpectAnyArgsAndReturn(NRFX_ERROR_BUSY);
	TEST_ASSERT_EQUAL(-EBUSY, trace_medium_init());
}

/* Test uart trace medium deinit. */
void test_trace_medium_uart_deinit(void)
{
	initialize_trace_medium_uart_success();

	__wrap_nrfx_uarte_uninit_Expect(p_uarte_inst_in_use);
	__wrap_pinctrl_lookup_state_ExpectAnyArgsAndReturn(0);
	__wrap_pinctrl_configure_pins_ExpectAnyArgsAndReturn(0);
	TEST_ASSERT_EQUAL(0, trace_medium_deinit());
}

/* Test that uart trace medium divides large traces into several pieces
 * and forwards them one by one to NRFX UART.
 */
void test_trace_medium_uart_write_success(void)
{
	const uint32_t max_uart_frag_size = (1 << UARTE1_EASYDMA_MAXCNT_SIZE) - 1;
	/* Configure to send data higher than maximum number of bytes the UART trace medium
	 * will send in 'one go' over UART.
	 */
	const uint32_t sample_trace_buffer_size = max_uart_frag_size + 10;
	const uint8_t sample_trace_data[sample_trace_buffer_size];

	initialize_trace_medium_uart_success();

	__wrap_nrfx_uarte_tx_ExpectAndReturn(p_uarte_inst_in_use, sample_trace_data,
					     max_uart_frag_size, NRFX_SUCCESS);

	/* Simulate reception of a modem trace and let trace_test_thread run. */
	send_traces_for_processing(sample_trace_data, sizeof(sample_trace_data));
	k_sleep(K_MSEC(1));

	/* Simulate uart callback. Done sending 1st part of modem trace. */
	uart_tx_done_simulate(sample_trace_data, max_uart_frag_size);

	__wrap_nrfx_uarte_tx_ExpectAndReturn(p_uarte_inst_in_use,
					     &sample_trace_data[max_uart_frag_size],
					     sizeof(sample_trace_data) - max_uart_frag_size,
					     NRFX_SUCCESS);

	/* Let trace_test_thread run. */
	k_sleep(K_MSEC(1));

	/* Simulate uart callback. Done sending 2nd part of modem trace. */
	uart_tx_done_simulate(&sample_trace_data[max_uart_frag_size],
			      sizeof(sample_trace_data) - max_uart_frag_size);

	/* Let trace_test_thread run. */
	k_sleep(K_MSEC(1));

	TEST_ASSERT_EQUAL(0, k_sem_count_get(&receive_traces_sem));
	TEST_ASSERT_EQUAL(true, is_waiting_on_traces);
}

/* Test that verifies the behavior of uart trace medium when it times out, waiting
 * on the tx semaphore to send additional data of the received trace.
 */
void test_trace_medium_uart_write_tx_sem_take_failure(void)
{
	const uint32_t max_uart_frag_size = (1 << UARTE1_EASYDMA_MAXCNT_SIZE) - 1;
	/* Configure to send data higher than maximum number of bytes the UART trace medium
	 * will send in 'one go' over UART.
	 */
	const uint32_t sample_trace_buffer_size = max_uart_frag_size + 10;
	const uint8_t sample_trace_data[sample_trace_buffer_size];

	initialize_trace_medium_uart_success();

	__wrap_nrfx_uarte_tx_ExpectAndReturn(p_uarte_inst_in_use, sample_trace_data,
					     max_uart_frag_size, NRFX_SUCCESS);

	/* Simulate reception of a modem trace and let trace_test_thread run. */
	send_traces_for_processing(sample_trace_data, sizeof(sample_trace_data));
	k_sleep(K_MSEC(1));

	/* Simulate a condition when the UART TX is not completed for 100 ms (i.e
	 * UART_TX_WAIT_TIME_MS). The modem trace module is expected to not make any further
	 * calls to the UART API for this trace buffer.
	 */
	k_sleep(K_MSEC(100));

	/* Simulate a TX complete now and check if the modem trace module handles it gracefully. */
	uart_tx_done_simulate(sample_trace_data, max_uart_frag_size);

	/* Let trace_test_thread run. */
	k_sleep(K_MSEC(1));

	TEST_ASSERT_EQUAL(0, k_sem_count_get(&receive_traces_sem));
	TEST_ASSERT_EQUAL(true, is_waiting_on_traces);
}

/* Test that verifies the behavior of the uart trace medium when UART API returns error. */
void test_trace_medium_uart_write_tx_returns_error(void)
{
	const uint32_t max_uart_frag_size = (1 << UARTE1_EASYDMA_MAXCNT_SIZE) - 1;
	/* Configure to send data higher than maximum number of bytes the UART trace medium
	 * will send in 'one go' over UART.
	 */
	const uint32_t sample_trace_buffer_size = max_uart_frag_size + 10;
	const uint8_t sample_trace_data[sample_trace_buffer_size];

	initialize_trace_medium_uart_success();

	/* Make the nrfx_uarte_tx return error. This should make the modem trace module
	 * abort sending the second part of the trace buffer.
	 */
	__wrap_nrfx_uarte_tx_ExpectAndReturn(p_uarte_inst_in_use, sample_trace_data,
					     max_uart_frag_size, NRFX_ERROR_NO_MEM);

	/* Simulate reception of a modem trace and let trace_test_thread run. */
	send_traces_for_processing(sample_trace_data, sizeof(sample_trace_data));
	k_sleep(K_MSEC(1));

	TEST_ASSERT_EQUAL(0, k_sem_count_get(&receive_traces_sem));
	TEST_ASSERT_EQUAL(true, is_waiting_on_traces);
}

/* Test that verifies that the uart trace medium handles an TX Error event from UART gracefully.
 */
void test_trace_medium_uart_write_tx_error(void)
{
	const uint32_t sample_trace_buffer_size = 10;
	const uint8_t sample_trace_data[sample_trace_buffer_size];

	initialize_trace_medium_uart_success();

	__wrap_nrfx_uarte_tx_ExpectAndReturn(p_uarte_inst_in_use, sample_trace_data,
					     sizeof(sample_trace_data), NRFX_SUCCESS);

	/* Simulate reception of a modem trace and let trace_test_thread run. */
	send_traces_for_processing(sample_trace_data, sizeof(sample_trace_data));
	k_sleep(K_MSEC(1));

	uart_tx_error_simulate();

	/* Let trace_test_thread run. */
	k_sleep(K_MSEC(1));

	TEST_ASSERT_EQUAL(0, k_sem_count_get(&receive_traces_sem));
	TEST_ASSERT_EQUAL(true, is_waiting_on_traces);
}

void main(void)
{
	(void)unity_main();
}
