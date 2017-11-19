/**
* wiegand-avr fimrware for ATTINY44/84
*
* main.c - main file
*
* (c) 2017 Sam Thompson <git@samt.us>
* The MIT License
*/
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>

#include "main.h"

/**
 * Counter
 *
 * Counts the incoming bits to determine whether or not to validate and sent
 * the transmission off to the RPi.
 */
volatile uint8_t counter;

/**
 * Raw input buffer from the Wiegand reader (reverse order)
 *
 * Wiegand transmits the MSB first, but the last byte in the ID first. We
 * accept the ID, storing each byte reversed, and consequently fixing the byte
 * order. When read back, we shift off each byte, reverse the order, and send
 * it down the USIDR.
 */
volatile uint64_t buffer;

/**
 * Pin Change0 Interrupt
 *
 * Register incomming bits from the Wiegand reader
 */
ISR(PCINT0_vect)
{
    if (bit_is_clear(WGD_IN_REG, WGD_D0) ^ bit_is_clear(WGD_IN_REG, WGD_D1)) {
        // Stop timer before we go any further
        TCCR0B = 0;

        // If we have data but a new transmission is starting, just clear it out
        if (bit_is_set(IRQ_OUT_REG, IRQ_SIGNAL)) {
            IRQ_OUT_REG &= ~_BV(IRQ_SIGNAL);
            buffer = 0ULL;
            counter = 0;
        }

        // Buffer is zero-ed before anything happens so we only need to add
        // the ones from the reader.
        if (bit_is_clear(WGD_IN_REG, WGD_D1)) {
            buffer |= _BV_ULL(counter);
        }

        counter++;

        // Start transmission timeout clock
        TCNT0 = 0;              // Start timer at 0
        TCCR0B = _BV(CS02);     // Start clock by setting prescaler (clk/256)

        return;
    }

    // When both bits are clear, the D1 and D0 lines are not high and
    // can therefore be considered disconnected.
    if (bit_is_clear(WGD_IN_REG, WGD_D0) && bit_is_clear(WGD_IN_REG, WGD_D1)) {
        TCCR0B = 0; // Stop timer
    }
}

/**
 * Timer0 Compare Match on A
 *
 * Wiegand transmission is done, find results
 */
ISR(TIM0_COMPA_vect) {
    // Stop timer, no need to clear TCNT0 as we are in CTC mode.
    TCCR0B = 0;

    // Only support even bitcounts
    if (counter & 0x01) {
        counter = 0;
        buffer = 0ULL;
        return;
    }

    // If counter is NOT divisible by 4, there is party to remove
    if (counter & 0x03) {
        // Discard Parity
        buffer >>= 1;
        buffer &= ~_BV_ULL(counter - 2);
    }

    // Send USI data
    USISR = _BV(USIOIF); // Reset flag to enable USI interrupt
    USIDR = counter;     // Store bit count in register

    // Signal that the transmission has completed
    IRQ_OUT_REG |= _BV(IRQ_SIGNAL);
    counter = 0;
}

/**
 * USI Overflow Interrupt
 *
 * Send the last byte of the buffer var to the USIDR.
 */
ISR(USI_OVF_vect)
{
    // Clear IRQ, USI counter
    IRQ_OUT_REG &= ~_BV(IRQ_SIGNAL);
    USISR = _BV(USIOIF);

    // Buffer to send (still backwards bits)
    USIDR = (uint8_t) (buffer & 0xFF);

    // Flip the order of bits in register
    USIDR = (USIDR & 0xF0) >> 4 | (USIDR & 0x0F) << 4;
    USIDR = (USIDR & 0xCC) >> 2 | (USIDR & 0x33) << 2;
    USIDR = (USIDR & 0xAA) >> 1 | (USIDR & 0x55) << 1;

    // Shift off the byte we've just read
    buffer >>= 8;
}

int main(void)
{
    // Wiegand Setup
    WGD_DIR_REG &= ~(_BV(WGD_D0) | _BV(WGD_D1)); // Inputs

    // IRQ Setup
    IRQ_DIR_REG |= _BV(IRQ_SIGNAL);
    IRQ_OUT_REG &= ~_BV(IRQ_SIGNAL);

    PCMSK0 |= _BV(WGD_PCINT_D0) | _BV(WGD_PCINT_D1); // PCINT0 enable
    GIMSK |= _BV(PCIE0); // Pin Change Interrupt Enable

    // Timer setup
    TCCR0A = _BV(WGM01);  // CTC Mode
    TIMSK0 = _BV(OCIE0A); // Interrupt on register A match
    OCR0A = 100;          // 100 ticks at clk/256 ~= 50ms

    // USI Setup
    USI_DIR_REG |= _BV(USI_DATAOUT_PIN);                        // Outputs
    USI_DIR_REG &= ~(_BV(USI_DATAIN_PIN) | _BV(USI_CLOCK_PIN)); // Inputs
    USI_OUT_REG |= _BV(USI_DATAIN_PIN) | _BV(USI_CLOCK_PIN);    // Pull-ups

    USICR = _BV(USIOIE)  // USI Overflow interrupt
        | _BV(USIWM0)    // 3 wire (SPI)
        | _BV(USICS1);   // Sample positive edge (SPI mode 0)

    sei();

    // Initialize our state
    counter = 0;
    buffer = 0ULL;

    for (;;) {
        _NOP();
    }

    return 0;
}
