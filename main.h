/**
* wiegand-avr fimrware for ATTINY44/84
*
* main.h - main header file
*
* (c) 2017 Sam Thompson <git@samt.us>
* The MIT License
*/
#ifndef _MAIN_H
#define _MAIN_H

#include <util/parity.h>

/**
 * Wiegand I/O
 */
#define WGD_DIR_REG             DDRA
#define WGD_OUT_REG             PORTA
#define WGD_IN_REG              PINA
#define WGD_D0                  PA0
#define WGD_D1                  PA1
#define WGD_PCINT_D0            PCINT0
#define WGD_PCINT_D1            PCINT1

/**
 * IRQ settings
 */
#define IRQ_DIR_REG             DDRB
#define IRQ_OUT_REG             PORTB
#define IRQ_SIGNAL              PB2

/**
 * USI I/O
 */
#define USI_DIR_REG	            DDRA
#define USI_OUT_REG             PORTA
#define USI_IN_REG	            PINA
#define USI_CLOCK_PIN	        PA4
#define USI_DATAIN_PIN	        PA6
#define USI_DATAOUT_PIN	        PA5

/** Just like _BV() but for 64-bit integers */
#define _BV_ULL(b)              (1ULL<<(b))

#endif
