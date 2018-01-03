/**
 * \file
 *
 * \brief USB configuration file for CDC application
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

#ifndef _CONF_USB_H_
#define _CONF_USB_H_

#include "compiler.h"

/**
 * USB Device Configuration
 * @{
 */

//! Device definition (mandatory)
#define  USB_DEVICE_VENDOR_ID             0x1C40 
#define  USB_DEVICE_PRODUCT_ID            0x04DC  // Framebuffer device
//#define  USB_DEVICE_PRODUCT_ID            0x04DB    // Display
#define  USB_DEVICE_MAJOR_VERSION         1
#define  USB_DEVICE_MINOR_VERSION         1
#define  USB_DEVICE_POWER                 200 // Consumption on Vbus line (mA)
#define  USB_DEVICE_ATTR                  \
                                          (USB_CONFIG_ATTR_BUS_POWERED)
//	(USB_CONFIG_ATTR_SELF_POWERED)
//	(USB_CONFIG_ATTR_REMOTE_WAKEUP|USB_CONFIG_ATTR_SELF_POWERED)
//	(USB_CONFIG_ATTR_REMOTE_WAKEUP|USB_CONFIG_ATTR_BUS_POWERED)

//! USB Device string definitions (Optional)
#define  USB_DEVICE_MANUFACTURE_NAME      "Frank's Workshop"
#define  USB_DEVICE_PRODUCT_NAME          "USB Display"
#define  USB_DEVICE_CHIP_SERIAL_NUMBER
// #define  USB_DEVICE_SERIAL_NAME           "1234"


/**
 * Device speeds support
 * Low speed not supported by CDC
 * @{
 */
//! To authorize the High speed
#if (UC3A3||UC3A4)
#define  USB_DEVICE_HS_SUPPORT
#endif
//@}


/**
 * USB Device Callbacks definitions (Optional)
 * @{
 */
// #define  UDC_VBUS_EVENT(b_vbus_high)      user_callback_vbus_action(b_vbus_high)
// #define  UDC_SOF_EVENT()                  user_callback_sof_action()
#define  UDC_SUSPEND_EVENT()              user_callback_suspend_action()
#define  UDC_RESUME_EVENT()               user_callback_resume_action()
//! Mandatory when USB_DEVICE_ATTR authorizes remote wakeup feature
// #define  UDC_REMOTEWAKEUP_ENABLE()        user_callback_remotewakeup_enable()
// #define  UDC_REMOTEWAKEUP_DISABLE()       user_callback_remotewakeup_disable()
//! When a extra string descriptor must be supported
//! other than manufacturer, product and serial string
// #define  UDC_GET_EXTRA_STRING()
//@}
extern void user_callback_suspend_action(void);
extern void user_callback_resume_action(void);

/**
 * Configuration of CDC interface
 * @{
 */
//! Interface callback definition
#define  UDI_LCD_RX_NOTIFY()              

#define  USB_DEVICE_NB_INTERFACE       2

#define UDI_HID_TOUCH_EP_IN           (1 | USB_EP_DIR_IN)
#define UDI_LCD_DATA_EP_OUT           (2 | USB_EP_DIR_OUT)	// RX

#define UDI_LCD_IFACE_NUMBER			0
#define UDI_HID_TOUCH_IFACE_NUMBER		1



/**
 * Configuration of HID Mouse interface (if used)
 * @{
 */
//! Interface callback definition
#define  UDI_HID_TOUCH_ENABLE_EXT()       true
#define  UDI_HID_TOUCH_DISABLE_EXT()
// #define UDI_HID_MOUSE_ENABLE_EXT() my_callback_mouse_enable()
// extern bool my_callback_mouse_enable(void);
// #define UDI_HID_MOUSE_DISABLE_EXT() my_callback_mouse_disable()
// extern void my_callback_mouse_disable(void);





#define UDI_COMPOSITE_DESC_T	\
udi_lcd_iface_desc_t udi_lcd;	\
udi_hid_touch_desc_t udi_hid_touch;


#define UDI_COMPOSITE_DESC_FS					\
.udi_lcd				= UDI_LCD_IFACE_DESC,	\
.udi_hid_touch			= UDI_HID_TOUCH_DESC

#define UDI_COMPOSITE_DESC_HS \
.udi_lcd				   = UDI_LCD_IFACE_DESC,\
.udi_hid_touch			   = UDI_HID_TOUCH_DESC

#define UDI_COMPOSITE_API	\
&udi_api_lcd_data,			\
&udi_api_hid_touch

//! The includes of classes and other headers must be done at the end of this file to avoid compile error
#include "udi_lcd_conf.h"
#include "udi_hid_touch.h"

#endif // _CONF_USB_H_
