/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/logging/log.h>
#include <modem/trace_medium_uart.h>
#include <nrfx_uarte.h>

LOG_MODULE_REGISTER(trace_medium_uart, CONFIG_NRF_MODEM_LIB_LOG_LEVEL);

#define UART1_NL DT_NODELABEL(uart1)
PINCTRL_DT_DEFINE(UART1_NL);
static const nrfx_uarte_t uarte_inst = NRFX_UARTE_INSTANCE(1);

/* Maximum time to wait for a UART transfer to complete before giving up.*/
#define UART_TX_WAIT_TIME_MS 100
#define UNUSED_FLAGS 0

/* Semaphore used to check if the last UART transfer was complete. */
static K_SEM_DEFINE(tx_sem, 1, 1);

static void uarte_callback(nrfx_uarte_event_t const *event, void *p_context)
{
	__ASSERT(k_sem_count_get(&tx_sem) == 0,
			"UART TX semaphore not in use");

	if (event->type == NRFX_UARTE_EVT_ERROR) {
		LOG_ERR("uarte error 0x%04x", event->data.error.error_mask);

		k_sem_give(&tx_sem);
	}

	if (event->type == NRFX_UARTE_EVT_TX_DONE) {
		k_sem_give(&tx_sem);
	}
}

static void wait_for_tx_done(void)
{
	/* Attempt to take the TX semaphore to stop the thread execution until
	 * UART is done sending. Immediately give the semaphore when it becomes
	 * available.
	 */
	if (k_sem_take(&tx_sem, K_MSEC(UART_TX_WAIT_TIME_MS)) != 0) {
		LOG_WRN("UARTE TX not available");
	}
	k_sem_give(&tx_sem);
}

bool trace_medium_uart_init(void)
{
	int ret;
	const uint8_t irq_priority = DT_IRQ(UART1_NL, priority);
	const nrfx_uarte_config_t config = {
		.skip_gpio_cfg = true,
		.skip_psel_cfg = true,

		.hal_cfg.hwfc = NRF_UARTE_HWFC_DISABLED,
		.hal_cfg.parity = NRF_UARTE_PARITY_EXCLUDED,
		.baudrate = NRF_UARTE_BAUDRATE_1000000,

		.interrupt_priority = irq_priority,
		.p_context = NULL,
	};

	ret = pinctrl_apply_state(PINCTRL_DT_DEV_CONFIG_GET(UART1_NL), PINCTRL_STATE_DEFAULT);
	__ASSERT_NO_MSG(ret == 0);

	IRQ_CONNECT(DT_IRQN(UART1_NL),
		irq_priority,
		nrfx_isr,
		&nrfx_uarte_1_irq_handler,
		UNUSED_FLAGS);
	return (nrfx_uarte_init(&uarte_inst, &config, &uarte_callback) ==
		NRFX_SUCCESS);
}

void trace_medium_uart_deinit(void)
{
	nrfx_uarte_uninit(&uarte_inst);
}

int trace_medium_uart_write(const uint8_t *data, uint32_t len)
{
	/* Split RAM buffer into smaller chunks to be transferred using DMA. */
	const uint32_t MAX_BUF_LEN = (1 << UARTE1_EASYDMA_MAXCNT_SIZE) - 1;
	uint32_t remaining_bytes = len;
	nrfx_err_t err;

	while (remaining_bytes) {
		size_t transfer_len = MIN(remaining_bytes, MAX_BUF_LEN);
		uint32_t idx = len - remaining_bytes;

		if (k_sem_take(&tx_sem, K_MSEC(UART_TX_WAIT_TIME_MS)) != 0) {
			LOG_WRN("UARTE TX not available!");
			break;
		}
		err = nrfx_uarte_tx(&uarte_inst, &data[idx], transfer_len);
		if (err != NRFX_SUCCESS) {
			LOG_ERR("nrfx_uarte_tx error: %d", err);
			k_sem_give(&tx_sem);
			break;
		}
		remaining_bytes -= transfer_len;
	}
	wait_for_tx_done();

	return 0;
}
