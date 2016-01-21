//*****************************************************************************
//  Copyright 2016 Paul Chote
//  This file is part of domesim, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdint.h>

static uint8_t output_buffer[256];
static volatile uint8_t output_read = 0;
static volatile uint8_t output_write = 0;

static uint8_t input_buffer[256];
static uint8_t input_read = 0;
static volatile uint8_t input_write = 0;

void usb_initialize()
{
#define BAUD 9600
#include <util/setbaud.h>
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
#if USE_2X
    UCSR0A = _BV(U2X0);
#endif

    // Enable receive, transmit, data received interrupt
    UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);

    input_read = input_write = 0;
    output_read = output_write = 0;
}

bool usb_can_read()
{
    return input_write != input_read;
}

// Read a byte from the receive buffer
// Will block if the buffer is empty
uint8_t usb_read()
{
    while (input_read == input_write);
    return input_buffer[input_read++];
}

// Add a byte to the send buffer.
// Will block if the buffer is full
void usb_write(uint8_t b)
{
    // Don't overwrite data that hasn't been sent yet
    while (output_write == (uint8_t)(output_read - 1));

    output_buffer[output_write++] = b;

    // Enable transmit if necessary
    UCSR0B |= _BV(UDRIE0);
}

ISR(USART_UDRE_vect)
{
    if (output_write != output_read)
        UDR0 = output_buffer[output_read++];

    // Ran out of data to send - disable the interrupt
    if (output_write == output_read)
        UCSR0B &= ~_BV(UDRIE0);
}

ISR(USART_RX_vect)
{
    input_buffer[(uint8_t)(input_write++)] = UDR0;
}