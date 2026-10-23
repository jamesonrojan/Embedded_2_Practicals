/**
  ******************************************************************************
  * @file    prac2a.h
  * @brief   Practical 2A - every function and shared variable, in one place.
  *
  * Each task has its own file:
  *   task1_gpio.c           Task 1  memory-mapped GPIO on PC13
  *   task2_spi_config.c     Task 2  SPI peripheral configuration
  *   task3_spi_transfer.c   Task 3  single-byte transfer
  *   task4_eeprom.c         Task 4  EEPROM driver
  *   task5_fault_case.c     Task 5  the fault case - do not edit (RUN_TASK 5)
  *   task6_fsm.c            Task 6  non-blocking state machine
  *   board_io.c             given   LEDs and buttons
  *
  * main.c runs ONE task at a time, chosen by RUN_TASK.
  ******************************************************************************
  */
#ifndef __PRAC2A_H
#define __PRAC2A_H

#include "main.h"
#include "board_config.h"

/* ---- which task is running (RUN_TASK in main.c) -------------------------- */
extern volatile uint8_t run_task;

/* ---- board_io.c (given) ---------------------------------------------------- */
typedef enum { STATUS_OFF = 0, STATUS_PASS, STATUS_FAIL } status_led_t;

void     board_io_init(void);
uint32_t button_pressed(uint32_t pin);
void     read_inputs(uint32_t now);         /* sets the two edge flags below */
void     status_leds_show(status_led_t s);  /* green / red / both off        */

extern volatile uint8_t btn_start_edge;      /* PA0 pressed - clear once used */
extern volatile uint8_t btn_abort_edge;      /* PA3 pressed - clear once used */

/* ---- Task 1 ---------------------------------------------------------------- */
void task1_gpio_init(void);
void task1_gpio_update(uint32_t now);
void task1_setup(void);
void task1_loop(uint32_t now);

extern volatile uint32_t task1_half_period_ms;
extern volatile uint32_t task1_toggle_count;

/* ---- Task 2 ---------------------------------------------------------------- */
void eeprom_spi_init(void);
void task2_setup(void);
void task2_loop(uint32_t now);

extern volatile uint32_t dbg_gpiob_moder;
extern volatile uint32_t dbg_gpiob_afrh;
extern volatile uint32_t dbg_spi_cr1;
extern volatile uint32_t dbg_spi_cr2;
extern volatile uint32_t dbg_spi_sr;
extern volatile uint32_t dbg_sck_hz_predicted;

/* ---- Task 3 ---------------------------------------------------------------- */
uint8_t spi_transfer(uint8_t tx);
void    eeprom_cs_low(void);
void    eeprom_cs_high(void);
void    task3_setup(void);
void    task3_loop(uint32_t now);

extern volatile uint8_t  task3_test_byte;
extern volatile uint8_t  task3_rx_byte;
extern volatile uint32_t task3_tx_count;

/* ---- Task 4 ---------------------------------------------------------------- */
uint8_t eeprom_read_status(void);
void    eeprom_write_enable(void);
void    eeprom_write_byte(uint16_t address, uint8_t value);
uint8_t eeprom_read_byte(uint16_t address);
void    leds_write_byte(uint8_t v);
void    eeprom_read_only_path(void);
void    eeprom_write_verify_path(void);
void    task4_setup(void);
void    task4_loop(uint32_t now);

extern volatile uint16_t eeprom_test_addr;
extern volatile uint8_t  eeprom_test_byte;
extern volatile uint8_t  eeprom_status_before;
extern volatile uint8_t  eeprom_status_after;
extern volatile uint8_t  eeprom_read_value;
extern volatile uint8_t  eeprom_verify_ok;
extern volatile uint32_t eeprom_write_wait_ms;
extern volatile uint32_t eeprom_timeout_count;
extern volatile uint8_t  eeprom_read_loop_enable;

/* ---- Task 5 ---------------------------------------------------------------- */
void task5_fault_hook(void);

/* ---- Task 6 ---------------------------------------------------------------- */
void update_eeprom_state_machine(uint32_t now);
void update_outputs(void);
void task6_setup(void);
void task6_loop(uint32_t now);

extern volatile uint8_t ee_state;       /* your current state, as a number   */
extern volatile uint8_t ee_last_read;   /* last byte read, for the LEDs      */
extern volatile uint8_t ee_use_fsm;     /* 1 = your FSM, 0 = Task 4 blocking */

#endif /* __PRAC2A_H */
