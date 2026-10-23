/**
  ******************************************************************************
  * @file    task3_spi_transfer.c
  * @brief   TASK 3 : SEND ONE BYTE AND PROVE THE WAVEFORM
  ******************************************************************************
  */

#include "prac2a.h"

/* ==========================================================================
 * Chip select
 * ========================================================================== */

void eeprom_cs_low(void)
{
    EE_SPI_GPIO->BRR = EE_CS_MASK;          /* RM0091 GPIOx_BRR             */
}

void eeprom_cs_high(void)
{
    /* TODO 3.3  Raise CS - but only once the peripheral has completely
     *           finished transmitting. A received byte does not mean the
     *           last bits have left the shift register. RM0091 lists the
     *           status flags to wait for (see its procedure for disabling the
     *           SPI). Raising CS early cuts the end off a command. */
	while ((EE_SPI->SR & SPI_SR_TXE) == 0u){}

	while ((EE_SPI->SR & SPI_SR_BSY) != 0u){}

	EE_SPI_GPIO->BSRR = EE_CS_MASK;
}

/* ==========================================================================
 * TASK 3
 * ========================================================================== */

uint8_t spi_transfer(uint8_t tx)
{
    /* TODO 3.4  Wait until the transmit buffer has room (SPI_SR). */
	while ((EE_SPI->SR & SPI_SR_TXE) == 0u){}

    /* TODO 3.5  Write tx to the data register.
     *           HINT: SPI_DR is declared 16 bits wide in the CMSIS header,
     *           and on the STM32F0 the WIDTH of the write matters. Count the
     *           clock pulses per call on your scope: 16 instead of 8 means
     *           this is the problem. */
	*(__IO uint8_t *)&EE_SPI->DR = tx;

    /* TODO 3.6  Wait for the received byte (SPI_SR), then read it from the
     *           data register and return it. Read it every time, even when
     *           you do not need the value. */
	while ((EE_SPI->SR & SPI_SR_RXNE) == 0u)
	{
	}

	return *(__IO uint8_t *)&EE_SPI->DR;
}

/* ==========================================================================
 * RUN_TASK 3 - given
 * Sends task3_test_byte every 100 ms with CS held HIGH, so the EEPROM ignores
 * it and your oscilloscope has a repeating burst to trigger on.
 * task3_test_byte can be changed from Live Expressions without a rebuild.
 * ========================================================================== */

#define TASK3_TX_PERIOD_MS  100u

volatile uint8_t  task3_test_byte = (uint8_t)TEST_BYTE_B;
volatile uint8_t  task3_rx_byte   = 0u;
volatile uint32_t task3_tx_count  = 0u;

static uint32_t t3_last = 0u;

void task3_setup(void)
{
    task1_gpio_init();
    eeprom_spi_init();
}

void task3_loop(uint32_t now)
{
    task1_gpio_update(now);

    if ((uint32_t)(now - t3_last) < TASK3_TX_PERIOD_MS)
    {
        return;
    }
    t3_last = now;

    task3_rx_byte = spi_transfer(task3_test_byte);
    task3_tx_count++;
}
