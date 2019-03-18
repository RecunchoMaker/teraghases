
#include <libopencmsis/core_cm3.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/timer.h>
#include <stdio.h>
#include <string.h>


// 72 MHz / 8
#define SYSTEM_CLOCK (72000000 / 8)


volatile uint32_t sysMillis = 0;


void SysTick_Handler(void)
{
      sysMillis++;
}


// circular TX buffer
#define TX_BUFFER_SIZE 256
volatile uint8_t tx_buffer[TX_BUFFER_SIZE];
volatile int tx_buffer_a = 0, tx_buffer_b = 0;


// IRQ service
void usart1_isr()
{
    // check if TX int is enabled and previous TX completed
    if (((USART_CR1(USART1) & USART_CR1_TXEIE) != 0)
            && ((USART_SR(USART1) & USART_SR_TXE) != 0))
    {
        if (tx_buffer_b == tx_buffer_a)
        {
            // no more data to send, disable TX int
            USART_CR1(USART1) &= ~USART_CR1_TXEIE;
        }
        else
        {
            // send the first byte in circular buffer
            tx_buffer_a++;
            tx_buffer_a %= TX_BUFFER_SIZE;
            usart_send(USART1, tx_buffer[tx_buffer_a]);
        }
    }
}


// put a string buffer in the circular TX buffer
int serial_send(const uint8_t* buffer, size_t length)
{
    // next index in the circular buffer
    int next_idx = (tx_buffer_b + 1) % TX_BUFFER_SIZE;

    // put bytes in TX buffer until full or no more bytes
    while (next_idx != tx_buffer_a && length > 0)
    {
        // put byte in the circular buffer
        tx_buffer[next_idx++] = *buffer++;
        tx_buffer_b++;
        next_idx %= TX_BUFFER_SIZE;
        length--;
    }

    tx_buffer_b %= TX_BUFFER_SIZE;

    // enable usart1 TX interrupt
    USART_CR1(USART1) |= USART_CR1_TXEIE;

    // return > 0 when buffer overrun
    return length;
}


inline int serial_print(const char* str)
{
    return serial_send((const uint8_t*)str, strlen(str));
}


void setup_uart()
{
    // usart1 PA9(TX) PA10(RX)
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_USART1);
    nvic_enable_irq(NVIC_USART1_IRQ);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);
    usart_set_baudrate(USART1, 115200);
    usart_set_databits(USART1, 8);
    usart_set_parity(USART1, USART_PARITY_NONE);
    usart_set_stopbits(USART1, USART_STOPBITS_1);
    usart_set_mode(USART1, USART_MODE_TX_RX);
    usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
    usart_enable(USART1);
}


uint16_t read_encoder()
{
    return timer_get_counter(TIM4);
}


void reset_encoder()
{
    timer_set_counter(TIM4, 0);
}


void setup_encoder()
{
    // clocks
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_TIM4);

    // channel pins
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO_TIM4_CH1);
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO_TIM4_CH2);

    // ARR register to the maximum
    timer_set_period(TIM4, UINT16_MAX);

    // up/down on TI1FP1+TI2FP2 edges depending on complementary input (4x)
    timer_slave_set_mode(TIM4, TIM_SMCR_SMS_EM3);

    // quadrature encoder mode, input capture channels correspondency
    timer_ic_set_input(TIM4, TIM_IC1, TIM_IC_IN_TI1);
    timer_ic_set_input(TIM4, TIM_IC2, TIM_IC_IN_TI2);

    // reset counter
    timer_set_counter(TIM4, 0);

    // turn on counter
    timer_enable_counter(TIM4);
}


void setup()
{
    // clocks setup
    rcc_clock_setup_in_hse_8mhz_out_72mhz();

    // UART
    setup_uart();

    // encoder
    setup_encoder();

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

            char buffer[100];
            int left = read_encoder();
            int right = 0;
            sprintf(buffer, "%u: %i - %i\n", last_second, left, right);
            serial_print(buffer);
        }
    }

    return 0;
}


