/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <stdio.h>
#include <string.h>
#include <modem/nrf_modem_lib.h>
#include <nrf_modem_at.h>

#if defined(CONFIG_SOC_SERIES_NRF91X)
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/drivers/clock_control/nrf_clock_control.h>

/* To strictly comply with UART timing, enable external XTAL oscillator */
void enable_xtal(void)
{
	struct onoff_manager *clk_mgr;
	static struct onoff_client cli = {};

	clk_mgr = z_nrf_clock_control_get_onoff(CLOCK_CONTROL_NRF_SUBSYS_HF);
	sys_notify_init_spinwait(&cli.notify);
	(void)onoff_request(clk_mgr, &cli);
}
#endif /* CONFIG_SOC_SERIES_NRF91X */

static char buf[128] = {};

int main(void)
{
	int err;

	printk("The AT host sample started\n");

	err = nrf_modem_lib_init();
	if (err) {
		printk("Modem library initialization failed, error: %d\n", err);
		return 0;
	}
#if defined(CONFIG_SOC_SERIES_NRF91X)
	enable_xtal();
#endif /* CONFIG_SOC_SERIES_NRF91X */
	printk("Ready\n");

#if defined(CONFIG_SOC_SERIES_NRF92X)
	/* Print IMEI */
	err = nrf_modem_at_cmd(buf, sizeof(buf), "AT+CGSN=1");
	if (err) {
		printk("Failed AT+CGSN=1, err %d\n", err);
	} else {
		printk("Success: buf: %.127s\n", buf);
	}
#endif /* CONFIG_SOC_SERIES_NRF92X */

	return 0;
}
