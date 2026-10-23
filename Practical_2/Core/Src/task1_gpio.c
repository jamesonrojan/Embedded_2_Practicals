/**
  ******************************************************************************
  * @file    task1_gpio.c
  * @brief   TASK 1 : MEMORY-MAPPED GPIO ACCESS
  *
  * Drive PC13 as a digital output using EXPLICIT volatile register pointers,
  * built from addresses you find in RM0091:
  *
  *     register address = peripheral base address + register offset
  *
  * Do not use HAL_GPIO_Init(), HAL_GPIO_WritePin() or HAL_GPIO_TogglePin().
  * You must be able to show where every base and offset came from.
  ******************************************************************************
  */

#include "prac2a.h"

/* TODO 1.1  Peripheral base addresses: the memory map in RM0091 section 2. */
#define RCC_BASE_ADDR       0x40021000UL    /* <- TODO */
#define GPIOC_BASE_ADDR     0x48000800UL    /* <- TODO */

/* TODO 1.2  Register offsets: the register map at the end of the RCC chapter
 *           and of the GPIO chapter of RM0091. */
#define RCC_AHBENR_OFFSET   0x14UL          /* <- TODO */
#define GPIO_MODER_OFFSET   0x00UL          /* <- TODO */
#define GPIO_ODR_OFFSET     0x14UL          /* <- TODO */
#define GPIO_BSRR_OFFSET    0x18UL          /* <- TODO */
#define GPIO_BRR_OFFSET     0x28UL          /* <- TODO */

/* One pointer is declared for you to show the pattern. Be ready to explain
 * what `volatile` does here, and what can go wrong without it.
 *
 * TODO 1.3  Declare pointers for the GPIOC registers you use, the same way. */
static volatile uint32_t * const pRCC_AHBENR =
        (volatile uint32_t *)(RCC_BASE_ADDR + RCC_AHBENR_OFFSET);
static volatile uint32_t * const pGPIOC_MODER =
        (volatile uint32_t *)(GPIOC_BASE_ADDR + GPIO_MODER_OFFSET);
static volatile uint32_t * const pGPIOC_ODR =
        (volatile uint32_t *)(GPIOC_BASE_ADDR + GPIO_ODR_OFFSET);
static volatile uint32_t * const pGPIOC_BSRR =
        (volatile uint32_t *)(GPIOC_BASE_ADDR + GPIO_BSRR_OFFSET);
static volatile uint32_t * const pGPIOC_BRR =
        (volatile uint32_t *)(GPIOC_BASE_ADDR + GPIO_BRR_OFFSET);

/* TODO 1.4  Once TODO 1.1 and 1.2 are done, UNCOMMENT these checks. They
 *           compare your addresses with the CMSIS device header, so a wrong
 *           base or offset fails the build and names the culprit. Fix your
 *           address - do not edit the right-hand side.
 *           (They are commented out only so the project builds before you
 *           start, letting you check your toolchain first.)
 */
_Static_assert(RCC_BASE_ADDR   == RCC_BASE,   "RCC base mismatch");
_Static_assert(GPIOC_BASE_ADDR == GPIOC_BASE, "GPIOC base mismatch");
_Static_assert(RCC_BASE_ADDR + RCC_AHBENR_OFFSET
               == (uint32_t)(uintptr_t)&RCC->AHBENR,  "AHBENR offset");
_Static_assert(GPIOC_BASE_ADDR + GPIO_MODER_OFFSET
               == (uint32_t)(uintptr_t)&GPIOC->MODER, "MODER offset");
_Static_assert(GPIOC_BASE_ADDR + GPIO_ODR_OFFSET
               == (uint32_t)(uintptr_t)&GPIOC->ODR,   "ODR offset");
_Static_assert(GPIOC_BASE_ADDR + GPIO_BSRR_OFFSET
               == (uint32_t)(uintptr_t)&GPIOC->BSRR,  "BSRR offset");
_Static_assert(GPIOC_BASE_ADDR + GPIO_BRR_OFFSET
               == (uint32_t)(uintptr_t)&GPIOC->BRR,   "BRR offset");


volatile uint32_t task1_half_period_ms = 5u;
volatile uint32_t task1_toggle_count   = 0u;

static uint32_t t1_last = 0u;

void task1_gpio_init(void)
{
    /* TODO 1.5  Enable the GPIOC peripheral clock in RCC_AHBENR. Find the
     *           bit. Use a read-modify-write so clocks other code has already
     *           enabled are kept. Be ready to explain what the board does if
     *           you forget this. */
	*pRCC_AHBENR |= (1UL << 19);

    /* TODO 1.6  Put PC13 into general purpose output mode. MODER has two bits
     *           per pin: clear both of PC13's bits, then set the output value,
     *           leaving every other pin unchanged. */
	*pGPIOC_MODER = (*pGPIOC_MODER & ~(3UL << 26)) | (1UL << 26);

    /* TODO 1.7  Drive PC13 to a known starting level. */
	*pGPIOC_BRR = (1UL << 13);
}

void task1_gpio_update(uint32_t now)
{
    if ((uint32_t)(now - t1_last) < task1_half_period_ms)
    {
        return;
    }
    t1_last = now;

    /* TODO 1.8  Toggle PC13: read its present level, then drive the opposite
     *           level. There are set-only and reset-only registers as well as
     *           ODR - be ready to say why you chose the one you used. */
    if (*pGPIOC_ODR & (1UL << 13))
        {
            *pGPIOC_BRR  = (1UL << 13);
        }
        else
        {
            *pGPIOC_BSRR = (1UL << 13);
        }

    task1_toggle_count++;
}

/* ==========================================================================
 * RUN_TASK 1 - given
 * ========================================================================== */

void task1_setup(void)
{
    task1_gpio_init();
}

void task1_loop(uint32_t now)
{
    task1_gpio_update(now);
}
