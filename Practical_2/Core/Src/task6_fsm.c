/**
  ******************************************************************************
  * @file    task6_fsm.c
  * @brief   TASK 6 : NON-BLOCKING EEPROM TRANSACTION STATE MACHINE
  *
  * Restructure the Task 4 transaction so the main loop never stops:
  *
  *   request -> write-enable -> write -> wait for EEPROM
  *           -> read-back -> verify -> result
  *
  * Rules (handout, Task 6):
  *   - No HAL_Delay(), and no software busy-wait for the EEPROM's internal
  *     write cycle. You may use HAL_GetTick() to decide when the next status
  *     check is due.
  *   - Each call does a small amount of work, updates the state, and returns.
  *   - Do not put the whole transaction inside one blocking function called
  *     from the loop.
  *   - While a transaction is in progress the loop must still respond to PA3.
  *
  * Controls: PA0 starts, PA3 aborts. PB0..PB7 show the last byte read, PB11
  * (green) a successful verification, PB10 (red) a failed one.
  ******************************************************************************
  */

#include "prac2a.h"

/* TODO 6.1  Design your states. You need enough of them to tell apart at
 *           least: idle, write preparation, write transaction, EEPROM busy /
 *           status checking, read transaction, verification, and success or
 *           failure.
 *
 *           Draw the diagram first. It must show the initial state, the
 *           condition on every transition, and the success, failure and
 *           abort paths.
 *
 *           typedef enum { ... } ee_state_t;
 */

typedef enum {
    EE_STATE_IDLE       = 0u,
    EE_STATE_WREN       = 1u,
    EE_STATE_WRITE      = 2u,
    EE_STATE_POLL_BUSY  = 3u,
    EE_STATE_READ       = 4u,
    EE_STATE_VERIFY     = 5u,
    EE_STATE_SUCCESS    = 6u,
    EE_STATE_FAILURE    = 7u,
    EE_STATE_ABORTED    = 8u
} ee_state_t;

volatile uint8_t ee_state       = (uint8_t)EE_STATE_IDLE;
volatile uint8_t ee_last_read   = 0u;
volatile uint8_t ee_use_fsm     = 1u;

volatile uint32_t ee_dwell_ms   = 0u;

static uint32_t write_start_ms  = 0u;
static uint32_t state_entry_ms  = 0u;
static uint8_t  prev_state      = 0xFFu;

void update_eeprom_state_machine(uint32_t now)
{
    /* TODO 6.2  One step of your state machine.
     *
     *   - btn_start_edge (PA0) starts a transaction from idle.
     *   - btn_abort_edge (PA3) abandons the transaction in progress and returns
     *     to idle. Leave the SPI bus in a state the next transaction can use.
     *   - While the EEPROM is busy writing, do NOT wait in here. Work out when
     *     the next status check is due, remember it, and return.
     *   - Decide that the write has finished from the status register, never
     *     from elapsed time alone. Also decide what happens if the EEPROM never
     *     reports ready.
     *   - Store the byte read back in ee_last_read.
     *   - Clear each button edge once you have acted on it. */

    uint8_t active = (ee_state >= (uint8_t)EE_STATE_WREN) &&
                     (ee_state <= (uint8_t)EE_STATE_VERIFY);

    if (btn_abort_edge)
    {
        btn_abort_edge = 0u;
        if (active)
        {
            eeprom_cs_high();
            ee_state = (uint8_t)EE_STATE_ABORTED;
            return;
        }
    }

    if (active)
    {
        btn_start_edge = 0u;
    }

    if (ee_state != prev_state)
    {
        prev_state     = ee_state;
        state_entry_ms = now;
    }
    if (((ee_state == (uint8_t)EE_STATE_WREN)  ||
         (ee_state == (uint8_t)EE_STATE_WRITE) ||
         (ee_state == (uint8_t)EE_STATE_READ)  ||
         (ee_state == (uint8_t)EE_STATE_VERIFY)) &&
        ((uint32_t)(now - state_entry_ms) < ee_dwell_ms))
    {
        return;
    }

    switch ((ee_state_t)ee_state)
    {
    case EE_STATE_IDLE:
    case EE_STATE_SUCCESS:
    case EE_STATE_FAILURE:
    case EE_STATE_ABORTED:
        if (btn_start_edge)
        {
            btn_start_edge = 0u;
            ee_state = (uint8_t)EE_STATE_WREN;
        }
        break;

    case EE_STATE_WREN:
        eeprom_write_enable();
        ee_state = (uint8_t)EE_STATE_WRITE;
        break;

    case EE_STATE_WRITE:
        eeprom_cs_low();
        spi_transfer(EEPROM_CMD_WRITE);
        spi_transfer((uint8_t)(eeprom_test_addr >> 8));
        spi_transfer((uint8_t)(eeprom_test_addr & 0xFFu));
        spi_transfer(eeprom_test_byte);
        eeprom_cs_high();

        write_start_ms = now;
        ee_state = (uint8_t)EE_STATE_POLL_BUSY;
        break;

    case EE_STATE_POLL_BUSY:
        if ((uint32_t)(now - write_start_ms) >= EEPROM_WRITE_TIMEOUT_MS)
        {
            eeprom_timeout_count++;
            eeprom_verify_ok = 0u;
            ee_state = (uint8_t)EE_STATE_FAILURE;
        }
        else
        {
            uint8_t status = eeprom_read_status();
            if ((status & EEPROM_SR_RDY) == 0u)
            {
                eeprom_write_wait_ms = (uint32_t)(now - write_start_ms);
                ee_state = (uint8_t)EE_STATE_READ;
            }
        }
        break;

    case EE_STATE_READ:
        ee_last_read = eeprom_read_byte(eeprom_test_addr);
        eeprom_read_value = ee_last_read;
        ee_state = (uint8_t)EE_STATE_VERIFY;
        break;

    case EE_STATE_VERIFY:
        if (ee_last_read == eeprom_test_byte)
        {
            eeprom_verify_ok = 1u;
            ee_state = (uint8_t)EE_STATE_SUCCESS;
        }
        else
        {
            eeprom_verify_ok = 0u;
            ee_state = (uint8_t)EE_STATE_FAILURE;
        }
        break;

    default:
        ee_state = (uint8_t)EE_STATE_IDLE;
        break;
    }
}

void update_outputs(void)
{
    /* TODO 6.3  PB0..PB7 show ee_last_read (leds_write_byte). Green after a
     *           successful verification, red after a failed one
     *           (status_leds_show). Decide what the status LEDs should show
     *           while a transaction is in progress, and after an abort. */

    leds_write_byte(ee_last_read);

    switch ((ee_state_t)ee_state)
    {
    case EE_STATE_SUCCESS:
        status_leds_show(STATUS_PASS);
        break;

    case EE_STATE_FAILURE:
        status_leds_show(STATUS_FAIL);
        break;

    case EE_STATE_IDLE:
        status_leds_show(eeprom_verify_ok ? STATUS_PASS : STATUS_OFF);
        break;

    case EE_STATE_ABORTED:
    case EE_STATE_WREN:
    case EE_STATE_WRITE:
    case EE_STATE_POLL_BUSY:
    case EE_STATE_READ:
    case EE_STATE_VERIFY:
    default:
        status_leds_show(STATUS_OFF);
        break;
    }
}

/* ==========================================================================
 * RUN_TASK 6 - given
 *
 * The main loop has exactly the shape the handout asks for:
 *     read_inputs();  update_eeprom_state_machine();  update_outputs();
 *
 * Set ee_use_fsm = 0 in Live Expressions to run the Task 4 blocking path on
 * PA0 instead - useful when you explain why yours is non-blocking. PC13 keeps
 * toggling as a heartbeat; watch it on the scope in both modes.
 * ========================================================================== */

void task6_setup(void)
{
    task1_gpio_init();
    eeprom_spi_init();
    board_io_init();

    eeprom_read_only_path();
    ee_last_read = eeprom_read_value;

    /* TODO 6.4  Your state machine starts in its idle state. Make sure the
     *           boot-time result (eeprom_verify_ok) still shows on the status
     *           LEDs, so a reset still shows green for the persistence test. */
    ee_state = (uint8_t)EE_STATE_IDLE;
    update_outputs();
}

void task6_loop(uint32_t now)
{
    task1_gpio_update(now);
    read_inputs(now);

    if (ee_use_fsm)
    {
        update_eeprom_state_machine(now);
        update_outputs();
    }
    else
    {
        if (btn_start_edge)
        {
            btn_start_edge = 0u;
            eeprom_write_verify_path();     /* Task 4: blocks for the write */
            ee_last_read = eeprom_read_value;
        }
        btn_abort_edge = 0u;
    }
}
