
#include <libopencmsis/core_cm3.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/adc.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>


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


int _write(int file, char *ptr, int len)
{
	if (file == STDOUT_FILENO || file == STDERR_FILENO)
		return serial_send(ptr, len);

	errno = EIO;
	return -1;
}


/*
inline int serial_print(const char* str)
{
    return serial_send((const uint8_t*)str, strlen(str));
}
*/

void setup()
{
    // clocks setup
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_USART1);
    rcc_periph_clock_enable(RCC_ADC1);

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

    // ADC1 in PA0(ADC1_IN0)
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO0);
    adc_power_off(ADC1);
    adc_set_right_aligned(ADC1);
    adc_set_single_conversion_mode(ADC1);
    adc_disable_external_trigger_regular(ADC1);
    adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_239DOT5CYC);
    adc_power_on(ADC1);
    for (int i = 0; i < 800000; i++) __asm__("nop");
    adc_reset_calibration(ADC1);
    adc_calibrate(ADC1);

    // systick every millisecond
    SysTick_Config(SYSTEM_CLOCK / 1000);
}


uint16_t analog_read()
{
    uint8_t channel_array[16];
    channel_array[0] = 0;
    adc_set_regular_sequence(ADC1, 1, channel_array);
    adc_start_conversion_direct(ADC1);
    while (!adc_eoc(ADC1));
    return (uint16_t) adc_read_regular(ADC1);
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

			uint16_t v = analog_read();
            printf("ADC value: %u\n", v);
        }
    }

    return 0;
}


