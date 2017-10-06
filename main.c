//*****************************************************************************
//  Copyright 2016 Paul Chote
//  This file is part of domesim, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "usb.h"

#define POSITION_CLOSED 0
#define POSITION_OPEN 15
#define DIRECTION_OPENING 1
#define DIRECTION_STOPPED 0
#define DIRECTION_CLOSING -1

static volatile bool update_state = true;

static uint8_t east_position;
static int8_t east_direction;
static uint8_t west_position;
static int8_t west_direction;
static char status_code = 0;


void tick()
{
	// Parse move commands
	// Multiple move commands can be received per second, so we simulate opposite moves as
	// cancelling eachother out.
	while (usb_can_read())
	{
		switch (usb_read())
		{
			case 'A':
				east_direction = east_direction == DIRECTION_OPENING ?
					DIRECTION_STOPPED : DIRECTION_CLOSING;
				status_code = 'A';
				break;
			case 'a':
				east_direction = east_direction == DIRECTION_CLOSING ?
					DIRECTION_STOPPED : DIRECTION_OPENING;
				status_code = 'a';
				break;
			case 'B':
				west_direction = west_direction == DIRECTION_OPENING ?
					DIRECTION_STOPPED : DIRECTION_CLOSING;
				status_code = 'B';
				break;
			case 'b':
				west_direction = west_direction == DIRECTION_CLOSING ?
					DIRECTION_STOPPED : DIRECTION_OPENING;
				status_code = 'b';
				break;
		}
	}

    if (!update_state)
		return;

	// Update positions
	if (east_direction == DIRECTION_CLOSING && east_position > POSITION_CLOSED)
		east_position--;	
	else if (east_direction == DIRECTION_OPENING && east_position < POSITION_OPEN)
		east_position++;

	if (west_direction == DIRECTION_CLOSING && west_position > POSITION_CLOSED)
		west_position--;	
	else if (west_direction == DIRECTION_OPENING && west_position < POSITION_OPEN)
		west_position++;
	
	// Report status, using the same priority rules as the real PLC
	switch (status_code)
	{
		case 'A': usb_write(east_position == POSITION_CLOSED ? 'X' : 'A'); break;
		case 'a': usb_write(east_position == POSITION_OPEN ? 'x' : 'a'); break;
		case 'B': usb_write(west_position == POSITION_CLOSED ? 'Y' : 'B'); break;
		case 'b': usb_write(west_position == POSITION_OPEN ? 'y' : 'b'); break;
		default:
		{
			uint8_t limits = 0;
			if (east_position != POSITION_CLOSED)
				limits |= 2;
			if (west_position != POSITION_CLOSED)
				limits |= 1;
			usb_write('0' + limits);
			break;
		}
	}

	east_direction = west_direction = DIRECTION_STOPPED;
	status_code = 0;
	update_state = false;
}

int main()
{
    // Configure timer1 to interrupt every 1.00 seconds
    OCR1A = 15624;
    TCCR1B = _BV(CS12) | _BV(CS10) | _BV(WGM12);
    TIMSK1 |= _BV(OCIE1B);

	usb_initialize();

    sei();
    for (;;)
        tick();
}

ISR(TIMER1_COMPB_vect)
{
	update_state = true;
}