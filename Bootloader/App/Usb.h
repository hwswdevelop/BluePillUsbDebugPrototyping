/*
 * Usb.h
 *
 *  Created on: 18 февр. 2021 г.
 *      Author: Evgeny
 */
#pragma once

#include <libopencm3/stm32/f1/st_usbfs.h>
#include <libopencm3/usb/usbd.h>

#define BULK_EP_MAXPACKET 		64

void usbDataInCallback(usbd_device *usbd_dev, uint8_t ep);
void usbDataOutCallback(usbd_device *usbd_dev, uint8_t ep);
void usbdInit();
