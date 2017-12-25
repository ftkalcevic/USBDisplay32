/**
 * \file
 *
 * \brief USB Device Communication Device Class (CDC) interface definitions.
 *
 * Copyright (C) 2009 Atmel Corporation. All rights reserved.
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 * Atmel AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#ifndef _UDI_LCD_H_
#define _UDI_LCD_H_

#include "conf_usb.h"
#include "usb_protocol.h"
#include "udd.h"
#include "udc_desc.h"
#include "udi.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \name Interface Descriptor
 *
 * The following structures provide the interface descriptor.
 * It must be implemented in USB configuration descriptor.
 */
//@{




/**
 * \brief LCD Class interface descriptor
 *
 */
typedef struct {
	//! Standard interface descriptor
	usb_iface_desc_t iface;
	//! endpoint descriptor
	usb_ep_desc_t ep_out;
} udi_lcd_iface_desc_t;


//!  data enpoints size for all speed (no need to use 512B for HS)
#ifdef USB_DEVICE_HS_SUPPORT
    #define UDI_LCD_DATA_EPS_SIZE       512
#else
    #define UDI_LCD_DATA_EPS_SIZE       64
#endif

#define	LCD_DISPLAY_CLASS		0xFF
#define	LCD_DISPLAY_SUBCLASS	0
#define	LCD_DISPLAY_PROTOCOL	0
#define USB_DT_LCD				0x30
#define LCD_VERSION				1


//! Content of CDC COMM interface descriptor for all speed
#define UDI_LCD_IFACE_DESC        {\
   .iface.bInterfaceNumber       = UDI_LCD_IFACE_NUMBER, \
   .iface.bLength                = sizeof(usb_iface_desc_t),\
   .iface.bDescriptorType        = USB_DT_INTERFACE,\
   .iface.bAlternateSetting      = 0,\
   .iface.bNumEndpoints          = 1,\
   .iface.bInterfaceClass        = LCD_DISPLAY_CLASS,\
   .iface.bInterfaceSubClass     = LCD_DISPLAY_SUBCLASS,\
   .iface.bInterfaceProtocol     = LCD_DISPLAY_PROTOCOL,\
   .iface.iInterface             = 0,\
   .ep_out.bLength               = sizeof(usb_ep_desc_t),\
   .ep_out.bDescriptorType       = USB_DT_ENDPOINT,\
   .ep_out.bEndpointAddress      = UDI_LCD_DATA_EP_OUT,\
   .ep_out.wMaxPacketSize        = LE16(UDI_LCD_DATA_EPS_SIZE),\
   .ep_out.bmAttributes          = USB_EP_TYPE_BULK,\
   .ep_out.bInterval             = 1,\
   }

#define UDI_LCD_DESC {\
   .bFunctionLength			 = sizeof(usb_lcd_desc_t),\
   .bDescriptorType			 = USB_DT_LCD, \
   .bVersion				 = LCD_VERSION, \
   .wWidth					 = LE16(SCREEN_WIDTH),\
   .wHeight					 = LE16(SCREEN_HEIGHT),\
   .wWidthMM				 = LE16(SCREEN_WIDTH_MM),\
   .wHeightMM				 = LE16(SCREEN_HEIGHT_MM),\
   .wScanlineLen			 = LE16(SCREEN_SCANLINE_LEN),\
   .bBitsPerPixel			 = SCREEN_BITS_PER_PIXEL,\
   .bDisplayType			 = DISPLAY_TYPE,\
}

//! Global struture which contains standard UDI API for UDC
extern UDC_DESC_STORAGE udi_api_t udi_api_lcd_data;


/**
 * \brief This function checks if a character has been received on the CDC line
 *
 * \return \c 1 if a byte is ready to be read.
 */
bool udi_lcd_is_rx_ready(void);


#ifdef __cplusplus
}
#endif
#endif 
