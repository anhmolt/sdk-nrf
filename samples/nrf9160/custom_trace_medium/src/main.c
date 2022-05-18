/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <string.h>
#include <zephyr/kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include <nrf_modem_at.h>
#include <modem/nrf_modem_lib.h>

/* Network registration semaphore */
static K_SEM_DEFINE(cfun_sem, 0, 1);

/* define callback */
LTE_LC_ON_CFUN(cfun_hook, on_cfun, NULL);

/* callback implementation */
static void on_cfun(enum lte_lc_func_mode mode, void *context)
{
	printk("LTE mode changed to %d\n", mode);
	k_sem_give(&cfun_sem);
}

void main(void)
{
	int err;

	printk("Custom trace medium sample started\n");

	printk("Connecting to network\n");
	err = nrf_modem_at_printf("AT+CFUN=1");
	if (err) {
		printk("AT+CFUN failed\n");
		return;
	}

	err = k_sem_take(&cfun_sem, K_SECONDS(5));

	printk("Shutting down modem\n");
	err = nrf_modem_at_printf("AT+CFUN=0");
	if (err) {
		printk("AT+CFUN failed\n");
		return;
	}
	nrf_modem_lib_shutdown();
	printk("Bye\n");
}