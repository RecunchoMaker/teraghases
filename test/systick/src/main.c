
#include <libopencmsis/core_cm3.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>


// 72 MHz / 8
#define SYSTEM_CLOCK (72000000 / 8)


volatile uint32_t sysMillis = 0;


void SysTick_Handler(void)
{
      sysMillis++;
}


void setup()
{
    // clocks setup
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
    rcc_periph_clock_enable(RCC_GPIOB);

    // gpio setup
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
            GPIO_CNF_OUTPUT_PUSHPULL, GPIO1);

    // systick every millisecond
    SysTick_Config(SYSTEM_CLOCK / 1000);
}


int main(void)
{
    setup();

    uint32_t last_second = 0;
    while (1)
    {
        uint32_t seconds_now = sysMillis / 1000;
        if (last_second != seconds_now)
        {
            last_second = seconds_now;
            gpio_toggle(GPIOB, GPIO1);
        }
    }

    return 0;
}


