/**
  ******************************************************************************
  * @file    board_io.c
  * @brief   Given: LEDs, buttons and button edges, shared by Tasks 4 and 6.
  *          Not part of any assessed task - you do not need to change it.
  ******************************************************************************
  */

#include "prac2a.h"

volatile uint8_t btn_start_edge = 0u;
volatile uint8_t btn_abort_edge = 0u;

uint32_t button_pressed(uint32_t pin)
{
    /* Given. The switches pull to GND when pressed: active low. */
    return ((BTN_GPIO->IDR & (1UL << pin)) == 0u) ? 1u : 0u;
}

void board_io_init(void)
{
    /* Given: LED and button set-up. Not part of any assessed task. */
    uint32_t moder;
    uint32_t pupdr;

    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN;
    (void)RCC->AHBENR;

    moder = LED_BYTE_GPIO->MODER;
    for (uint32_t pin = 0u; pin <= 7u; pin++)
    {
        moder = (moder & ~MODER2_MASK(pin)) | MODER2(pin, 1u);
    }
    moder = (moder & ~(MODER2_MASK(LED_RED_PIN) | MODER2_MASK(LED_GREEN_PIN)))
          | MODER2(LED_RED_PIN, 1u) | MODER2(LED_GREEN_PIN, 1u);
    LED_BYTE_GPIO->MODER = moder;

    LED_BYTE_GPIO->BRR = LED_BYTE_MASK | (1UL << LED_RED_PIN) | (1UL << LED_GREEN_PIN);

    pupdr = BTN_GPIO->PUPDR;
    pupdr = (pupdr & ~(MODER2_MASK(BTN_START_PIN) | MODER2_MASK(BTN_ABORT_PIN)))
          | MODER2(BTN_START_PIN, 1u) | MODER2(BTN_ABORT_PIN, 1u);
    BTN_GPIO->PUPDR = pupdr;
}

void read_inputs(uint32_t now)
{
    /* Edge detection with a 20 ms debounce guard. */
    static uint32_t prev_start = 0u, prev_abort = 0u;
    static uint32_t last_start = 0u, last_abort = 0u;

    uint32_t s = button_pressed(BTN_START_PIN);
    uint32_t a = button_pressed(BTN_ABORT_PIN);

    if (s && !prev_start && ((uint32_t)(now - last_start) >= 20u))
    {
        last_start     = now;
        btn_start_edge = 1u;
    }
    if (a && !prev_abort && ((uint32_t)(now - last_abort) >= 20u))
    {
        last_abort     = now;
        btn_abort_edge = 1u;
    }

    prev_start = s;
    prev_abort = a;
}

void status_leds_show(status_led_t s)
{
    /* Given: green (PB11) for PASS, red (PB10) for FAIL, both off for OFF. */
    switch (s)
    {
    case STATUS_PASS:
        LED_BYTE_GPIO->BSRR = (1UL << LED_GREEN_PIN);
        LED_BYTE_GPIO->BRR  = (1UL << LED_RED_PIN);
        break;
    case STATUS_FAIL:
        LED_BYTE_GPIO->BSRR = (1UL << LED_RED_PIN);
        LED_BYTE_GPIO->BRR  = (1UL << LED_GREEN_PIN);
        break;
    default:
        LED_BYTE_GPIO->BRR = (1UL << LED_RED_PIN) | (1UL << LED_GREEN_PIN);
        break;
    }
}
