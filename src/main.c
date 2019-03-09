/*
# include "clock.h"
# include "setup.h"

void sys_tick_handler(void)
{
	clock_tick();
}

int main(void)
{
	setup();

    
	systick_interrupt_enable();
	while (1) {
	}

	return 0;
}


*/


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

static void gpio_setup(void)
{
	/* Enable GPIOA clock. */
	/* Manually: */
	// RCC_APB2ENR |= RCC_APB2ENR_IOPCEN;
	/* Using API functions: */
	rcc_periph_clock_enable(RCC_GPIOC);

	/* Set GPIO5 (in GPIO port A) to 'output push-pull'. */
	/* Manually: */
	// GPIOA_CRH = (GPIO_CNF_OUTPUT_PUSHPULL << (((5 - 8) * 4) + 2));
	// GPIOA_CRH |= (GPIO_MODE_OUTPUT_2_MHZ << ((5 - 8) * 4));
	/* Using API functions: */
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
}

int main(void)
{
	int i;

	gpio_setup();

	/* Blink the LED (PA5) on the board. */
	while (1) {
		/* Manually: */
		// GPIOA_BSRR = GPIO5;		/* LED off */
		// for (i = 0; i < 800000; i++)	/* Wait a bit. */
		// 	__asm__("nop");
		// GPIOA_BRR = GPIO5;		/* LED on */
		// for (i = 0; i < 800000; i++)	/* Wait a bit. */
		// 	__asm__("nop");

		/* Using API functions gpio_set()/gpio_clear(): */
		// gpio_set(GPIOA, GPIO5);	/* LED off */
		// for (i = 0; i < 800000; i++)	/* Wait a bit. */
		// 	__asm__("nop");
		// gpio_clear(GPIOA, GPIO5);	/* LED on */
		// for (i = 0; i < 800000; i++)	/* Wait a bit. */
		// 	__asm__("nop");

		/* Using API function gpio_toggle(): */
		gpio_toggle(GPIOC, GPIO13);	/* LED on/off */
		for (i = 0; i < 500000; i++)	/* Wait a bit. */
			__asm__("nop");
	}

	return 0;
}
