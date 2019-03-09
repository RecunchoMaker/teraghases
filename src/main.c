# include "clock.h"
# include "setup.h"

void sys_tick_handler(void)
{
	clock_tick();
}

/**
 * @brief Initial setup and infinite wait.
 */
int main(void)
{
	setup();
	systick_interrupt_enable();
	while (1) {
	}

	return 0;
}
