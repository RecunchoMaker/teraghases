
#include <libopencmsis/core_cm3.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <string.h>


// 72 MHz / 8
#define SYSTEM_CLOCK (72000000 / 8)


volatile uint32_t sysMillis = 0;


void SysTick_Handler(void)
{
      sysMillis++;
}


// circular TX buffer
#define TX_BUFFER_SIZE 1000
volatile uint8_t tx_buffer[TX_BUFFER_SIZE];
volatile int tx_buffer_a = 0, tx_buffer_b = 0;


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
            usart_send(USART1, tx_buffer[++tx_buffer_a]);
            tx_buffer_a %= TX_BUFFER_SIZE;
        }
    }
}


int serial_send(const uint8_t* buffer, int length)
{
    int next_idx = (tx_buffer_b + 1) % TX_BUFFER_SIZE;
    while (length > 0 && next_idx != tx_buffer_a)
    {
        // put byte in the circular buffer
        tx_buffer[next_idx++] = *buffer++;
        next_idx %= TX_BUFFER_SIZE;
        tx_buffer_b++;
        length--;
    }

    // enable usart1 TX interrupt
    USART_CR1(USART1) |= USART_CR1_TXEIE;

    return length;
}


inline int serial_print(const char* str)
{
    return serial_send((const uint8_t*)str, strlen(str));
}


void setup()
{
    // clocks setup
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_USART1);

    // gpio setup
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
            GPIO_CNF_OUTPUT_PUSHPULL, GPIO1);

    // usart1 PA9(TX) PA10(RX)
    nvic_enable_irq(NVIC_USART1_IRQ);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
            GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);
    usart_set_baudrate(USART1, 115200);
    usart_set_databits(USART1, 8);
    usart_set_parity(USART1, USART_PARITY_NONE);
    usart_set_stopbits(USART1, USART_STOPBITS_1);
    usart_set_mode(USART1, USART_MODE_TX_RX);
    usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
    usart_enable(USART1);

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

            serial_print("heello there!\n");
        }
    }

    return 0;
}


