//*****************************************************************************
//  Copyright 2016 Paul Chote
//  This file is part of domesim, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#include <stdarg.h>
#include <stdint.h>

#ifndef DOMESIM_USB_H
#define DOMESIM_USB_H

void usb_initialize();
bool usb_can_read();
uint8_t usb_read();
void usb_write(uint8_t b);

#endif