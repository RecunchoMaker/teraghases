
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>


void setup_pwm()
{
    // enable GPIOA clock
    rcc_periph_clock_enable(RCC_GPIOA);

    // enable TIM2 clock
    rcc_periph_clock_enable(RCC_TIM2);

    // configure PA1 alt function (TIM2_CH2)
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO1);

    // configure PA2 alt function (TIM2_CH3)
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO2);

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

    // configure output-compare CH2 in PWM1 mode
    timer_disable_oc_output(TIM2, TIM_OC2);
    timer_set_oc_mode(TIM2, TIM_OC2, TIM_OCM_PWM1);
    timer_set_oc_value(TIM2, TIM_OC2, 0);
    timer_enable_oc_output(TIM2, TIM_OC2);

    // configure output-compare CH3 in PWM1 mode
    timer_disable_oc_output(TIM2, TIM_OC3);
    timer_set_oc_mode(TIM2, TIM_OC3, TIM_OCM_PWM1);
    timer_set_oc_value(TIM2, TIM_OC3, 0);
    timer_enable_oc_output(TIM2, TIM_OC3);

    // enable TIM2
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
    int16_t ms = 0;
    int dir = +1;

    while (1)
    {
        // little pause
        for (int n = 0; n < 20000; n++);

        // up and down value
        ms += dir;
        if (ms == 1000) dir = -1;
        if (ms == 0) dir = +1;

        // ms2 is complementary
        int32_t ms2 = 1000 - ms;

        // duty to the quadratic function on CH2 and CH3 of TIM2
        timer_set_oc_value(TIM2, TIM_OC2, ms * ms / 1000);
        timer_set_oc_value(TIM2, TIM_OC3, ms2 * ms2 / 1000);
    }

    return 0;
}


