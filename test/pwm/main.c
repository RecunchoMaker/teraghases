
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>


void setup_pwm()
{
    // enable GPIO port A
    rcc_periph_clock_enable(RCC_GPIOA);

    // enable TIM2 timer
    rcc_periph_clock_enable(RCC_TIM2);

    // configure PA1 alt function
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO1);

    // init TIM2
    timer_disable_counter(TIM2);

    // set PWM mode (clock to CK_INT, edge-aligned, upcounter)
    timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

    // prescaler 72 ==> 1 MHz cycles ==> counter incremented by 1 us
    timer_set_prescaler(TIM2, 72);
    timer_enable_preload(TIM2);

    // set period to 1000 us (1 ms)
    timer_continuous_mode(TIM2);
    timer_set_period(TIM2, 1000);

    // configure output-compare in PWM1 mode
    timer_disable_oc_output(TIM2, TIM_OC2);
    timer_set_oc_mode(TIM2, TIM_OC2, TIM_OCM_PWM1);
    timer_enable_oc_output(TIM2, TIM_OC2);

    // set duty 0% and enable TIM2
    timer_set_oc_value(TIM2, TIM_OC2, 0);
    timer_enable_counter(TIM2);
}


void setup()
{
    // system clock
    rcc_clock_setup_in_hse_8mhz_out_72mhz();

    // setup pwm
    setup_pwm();
}


int main(void)
{
    setup();

    // milliseconds and direction
    uint16_t ms = 0;
    int dir = +1;

    while (1)
    {
        // little pause
        for (int n = 0; n < 10000; n++);

        // up and down value
        ms += dir;
        if (ms == 1000) dir = -1;
        if (ms == 0) dir = +1;

        // set the duty to the quadratic function
        timer_set_oc_value(TIM2, TIM_OC2, ms * ms / 1000);
    }

    return 0;
}


