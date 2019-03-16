
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>


void setup_gpio()
{
    // port A
    rcc_periph_clock_enable(RCC_GPIOA);

    // PA1 led
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO1);

    // PA8 button
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO8);

    // configure as pull-up
    gpio_set(GPIOA, GPIO8);

    // configure as pull-down
    //gpio_clear(GPIOA, GPIO8);
}


void setup()
{
    // system clock
    rcc_clock_setup_in_hse_8mhz_out_72mhz();

    // GPIO
    setup_gpio();
}


int main(void)
{
    setup();

    while (1)
    {
        // button is pull-up (pressed = GND)
        bool button = (gpio_get(GPIOA, GPIO8) == 0);

        // led is VCC sourced and GPIO sinks the current (LOW = on)
        if (button)
            gpio_clear(GPIOA, GPIO1);
        else
            gpio_set(GPIOA, GPIO1);
    }

    return 0;
}


