/**
 * Copyright (c) 2017 IpTronix
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <board.h>

#include "wifi_winc1500_nm_bsp_internal.h"

#include <bsp/include/nm_bsp.h>
#include <common/include/nm_common.h>

#include "wifi_winc1500_config.h"

struct winc1500_device winc1500;

void (*isr_function)(void);

static inline void chip_isr(struct device *port,
			    struct gpio_callback *cb,
			    uint32_t pins)
{
	if (isr_function) {
		isr_function();
	}
}

s8_t nm_bsp_init(void)
{
	isr_function = NULL;

	/* Initialize chip IOs. */
	winc1500.gpios = winc1500_configure_gpios();

	winc1500.spi_dev = device_get_binding(CONFIG_WINC1500_SPI_DRV_NAME);
	if (!winc1500.spi_dev) {
		return -1;
	}

	/* Perform chip reset. */
	nm_bsp_reset();

	return 0;
}

s8_t nm_bsp_deinit(void)
{
	/* TODO */
	return 0;
}

void nm_bsp_reset(void)
{
	gpio_pin_write(winc1500.gpios[WINC1500_GPIO_IDX_CHIP_EN],
		       CONFIG_WINC1500_GPIO_CHIP_EN, 0);
	gpio_pin_write(winc1500.gpios[WINC1500_GPIO_IDX_RESET_N],
		       CONFIG_WINC1500_GPIO_RESET_N, 0);
	nm_bsp_sleep(100);

	gpio_pin_write(winc1500.gpios[WINC1500_GPIO_IDX_CHIP_EN],
		       CONFIG_WINC1500_GPIO_CHIP_EN, 1);
	nm_bsp_sleep(10);

	gpio_pin_write(winc1500.gpios[WINC1500_GPIO_IDX_RESET_N],
		       CONFIG_WINC1500_GPIO_RESET_N, 1);
	nm_bsp_sleep(10);
}

void nm_bsp_sleep(uint32 u32TimeMsec)
{
	k_busy_wait(u32TimeMsec * MSEC_PER_SEC);
}

void nm_bsp_register_isr(void (*isr_fun)(void))
{
	isr_function = isr_fun;

	gpio_init_callback(&winc1500.gpio_cb,
			   chip_isr,
			   BIT(CONFIG_WINC1500_GPIO_IRQN));

	gpio_add_callback(winc1500.gpios[WINC1500_GPIO_IDX_IRQN],
			  &winc1500.gpio_cb);
}

void nm_bsp_interrupt_ctrl(u8_t enable)
{
	if (enable) {
		gpio_pin_enable_callback(
			winc1500.gpios[WINC1500_GPIO_IDX_IRQN],
			CONFIG_WINC1500_GPIO_IRQN);
	} else {
		gpio_pin_disable_callback(
			winc1500.gpios[WINC1500_GPIO_IDX_IRQN],
			CONFIG_WINC1500_GPIO_IRQN);
	}
}