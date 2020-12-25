
#include <libopencmsis/core_cm3.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/cm3/nvic.h>
#include <string.h>



#define SYSTEM_CLOCK (72000000 / 8)

volatile uint32_t sysMillis = 0;

void SysTick_Handler(void)
{
      sysMillis++;
}



volatile static bool tx_done = true;

void dma1_channel3_isr(void)
{
    if ((DMA1_ISR &DMA_ISR_TCIF3) != 0) DMA1_IFCR |= DMA_IFCR_CTCIF3;
    dma_disable_transfer_complete_interrupt(DMA1, DMA_CHANNEL3);
    spi_disable_tx_dma(SPI1);
    dma_disable_channel(DMA1, DMA_CHANNEL3);
    tx_done = true;
}

void spi_dma_write(const uint8_t* buffer, size_t size)
{
    // wait for previous transmission
    while (!tx_done);
    tx_done = false;

    // send buffer to SPI via DMA
    dma_channel_reset(DMA1, DMA_CHANNEL3);
    dma_set_peripheral_address(DMA1, DMA_CHANNEL3, (uint32_t)&SPI1_DR);
    dma_set_memory_address(DMA1, DMA_CHANNEL3, (uint32_t)buffer);
    dma_set_number_of_data(DMA1, DMA_CHANNEL3, size);
    dma_set_read_from_memory(DMA1, DMA_CHANNEL3);
    dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL3);
    dma_set_peripheral_size(DMA1, DMA_CHANNEL3, DMA_CCR_PSIZE_8BIT);
    dma_set_memory_size(DMA1, DMA_CHANNEL3, DMA_CCR_MSIZE_8BIT);
    dma_set_priority(DMA1, DMA_CHANNEL3, DMA_CCR_PL_HIGH);
    dma_enable_transfer_complete_interrupt(DMA1, DMA_CHANNEL3);
    dma_enable_channel(DMA1, DMA_CHANNEL3);
    spi_enable_tx_dma(SPI1);
}




#define PIXELS 7
#define PIXELS_BYTES (PIXELS * 3)
#define SPI_BUFFER_SIZE ((PIXELS_BYTES + 1) * 4)
static uint8_t encoded_bits[SPI_BUFFER_SIZE];
static const uint8_t codex [] = { 0x88, 0x8e, 0xe8, 0xee };

void update_neopixels(const uint8_t* buffer)
{
    memset(encoded_bits, 0x00, SPI_BUFFER_SIZE);
    uint8_t b = 2;

    for (int c = 0; c < PIXELS_BYTES; c++)
    {
        int eightbits = buffer[c];
        for (int bit = 0; bit < 8; bit += 2)
        {
            encoded_bits[b++] = codex[(eightbits & 0xc0) >> 6];
            eightbits <<= 2;
        }
    }

    spi_dma_write(encoded_bits, SPI_BUFFER_SIZE);
}



void setup_spi()
{
    // clocks
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_SPI1);
    rcc_periph_clock_enable(RCC_DMA1);

    // MOSI is PA7
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
            GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO7);

    // SPI1
    spi_reset(SPI1);
    spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_16,
            SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE, SPI_CR1_CPHA_CLK_TRANSITION_2,
            SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST);
    spi_set_unidirectional_mode(SPI1);
    spi_enable(SPI1);

    // DMA interrupt
    nvic_set_priority(NVIC_DMA1_CHANNEL3_IRQ, 0);
    nvic_enable_irq(NVIC_DMA1_CHANNEL3_IRQ);
}


void setup()
{
    // clocks setup
    rcc_clock_setup_in_hse_8mhz_out_72mhz();

    // systick every millisecond
    SysTick_Config(SYSTEM_CLOCK / 1000);

    setup_spi();
}


int main(void)
{
    setup();

    uint8_t strip[PIXELS_BYTES];

    // turn off
    memset(strip, 0x00, PIXELS_BYTES);
    update_neopixels(strip);

    // main loop
    uint32_t last_second = 0xffffffff;
    while (1)
    {
        uint32_t seconds_now = sysMillis / 1000;
        if (last_second != seconds_now)
        {
            // every second
            last_second = seconds_now;

            // rotate RGB over 3 leds
            int idx = 0;
            for (int p = 0; p < PIXELS; p++)
            {
                uint8_t r = 0x00, g = 0x00, b = 0x00;
                switch ((p + last_second) % 3)
                {
                    case 0: r = 0x10; break;
                    case 1: g = 0x10; break;
                    case 2: b = 0x10; break;
                }

                // GRB format
                strip[idx++] = g;
                strip[idx++] = r;
                strip[idx++] = b;
            }

            // show
            update_neopixels(strip);
        }
    }

    return 0;
}


