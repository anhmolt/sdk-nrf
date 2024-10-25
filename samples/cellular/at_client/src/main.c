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

#if defined(CONFIG_DK_LIBRARY)
#include <dk_buttons_and_leds.h>
#endif /* CONFIG_DK_LIBRARY */

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

#if defined(CONFIG_DK_LIBRARY)
void buttons_handler(uint32_t button_state, uint32_t has_changed)
{
	printk("button_handler: button 1: %u, button 2: %u "
	       "button 3: %u, button 4: %u\n",
	       (bool)(button_state & DK_BTN1_MSK), (bool)(button_state & DK_BTN2_MSK),
	       (bool)(button_state & DK_BTN3_MSK), (bool)(button_state & DK_BTN4_MSK));
}
#endif /* CONFIG_DK_LIBRARY */

static char buf[128] = {};

int main(void)
{
	int err;

	printk("The AT host sample started\n");


#if defined(CONFIG_DK_LIBRARY)
	err = dk_leds_init();
	if (err) {
		printk("Failed led init, err %d\n", err);
	}

	err = dk_set_leds_state(0x0, DK_ALL_LEDS_MSK);
	if (err) {
		printk("Failed setting leds state, err: %d\n", err);
	}

	k_msleep(1000);

	err = dk_set_leds_state(0x1, DK_ALL_LEDS_MSK);
	if (err) {
		printk("Failed setting leds state, err: %d\n", err);
	}

	k_msleep(1000);

	err = dk_set_leds_state(0x2, DK_ALL_LEDS_MSK);
	if (err) {
		printk("Failed setting leds state, err: %d\n", err);
	}

	k_msleep(1000);

	err = dk_set_leds_state(0x4, DK_ALL_LEDS_MSK);
	if (err) {
		printk("Failed setting leds state, err: %d\n", err);
	}

	k_msleep(1000);

	err = dk_set_leds_state(0x8, DK_ALL_LEDS_MSK);
	if (err) {
		printk("Failed setting leds state, err: %d\n", err);
	}

	k_msleep(1000);

	err = dk_set_leds_state(0xF, DK_ALL_LEDS_MSK);
	if (err) {
		printk("Failed setting leds state, err: %d\n", err);
	}

	k_msleep(1000);

	err = dk_buttons_init(buttons_handler);
	if (err) {
		printk("Failed buttons init, err %d\n", err);
	}
#endif /* CONFIG_DK_LIBRARY */

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
