/**
 * \file
 *
 * \brief USB Device Human Interface Device (HID) mouse interface.
 *
 * Copyright (c) 2009-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#include "conf_usb.h"
#include "usb_protocol.h"
#include "udd.h"
#include "udc.h"
#include "udi_hid.h"
#include "udi_hid_touch.h"
#include <string.h>

/**
 * \ingroup udi_hid_mouse_group
 * \defgroup udi_hid_mouse_group_udc Interface with USB Device Core (UDC)
 *
 * Structures and functions required by UDC.
 *
 * @{
 */
bool udi_hid_touch_enable(void);
void udi_hid_touch_disable(void);
bool udi_hid_touch_setup(void);
uint8_t udi_hid_touch_getsetting(void);

//! Global structure which contains standard UDI interface for UDC
UDC_DESC_STORAGE udi_api_t udi_api_hid_touch = {
	.enable = (bool(*)(void))udi_hid_touch_enable,
	.disable = (void (*)(void))udi_hid_touch_disable,
	.setup = (bool(*)(void))udi_hid_touch_setup,
	.getsetting = (uint8_t(*)(void))udi_hid_touch_getsetting,
	.sof_notify = NULL,
};
//@}


/**
 * \ingroup udi_hid_mouse_group
 * \defgroup udi_hid_mouse_group_internal Implementation of UDI HID Mouse
 *
 * Class internal implementation
 * @{
 */

/**
 * \name Internal defines and variables to manage HID mouse
 */
//@{

//! Size of report for standard HID mouse
#define UDI_HID_TOUCH_REPORT_SIZE   6
//! To store current rate of HID mouse
COMPILER_WORD_ALIGNED
		static uint8_t udi_hid_touch_rate;
//! To store current protocol of HID mouse
COMPILER_WORD_ALIGNED
		static uint8_t udi_hid_touch_protocol;
//! To signal if a valid report is ready to send
static bool udi_hid_touch_b_report_valid;
//! Report ready to send
static uint8_t udi_hid_touch_report[UDI_HID_TOUCH_REPORT_SIZE];
//! Signal if a report transfer is on going
static bool udi_hid_touch_report_trans_ongoing;
//! Buffer used to send report
COMPILER_WORD_ALIGNED
		static uint8_t
		udi_hid_touch_report_trans[UDI_HID_TOUCH_REPORT_SIZE];


/**
 * \brief Callback for set report setup request
 *
 * \return \c 1 always, because it is not used on mouse interface
 */
static bool udi_hid_touch_setreport(void);

//@}

//! HID report descriptor for standard HID touch
UDC_DESC_STORAGE udi_hid_touch_report_desc_t udi_hid_touch_report_desc = {
	{
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x09, 0x38,                    //     USAGE (Wheel)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x0f,              //     LOGICAL_MAXIMUM (4095)
    0x75, 0x10,                    //     REPORT_SIZE (16)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0xc0,                          //     END_COLLECTION
    0xc0                           // END_COLLECTION
	}
};

/**
 * \name Internal routines
 */
//@{

/**
 * \brief Moves an axe
 *
 * \param pos           Signed value to move
 * \param index_report  Index of report to move
 *								(3=scroll wheel, 2=axe Y, 1=axe X))
 *
 * \return \c 1 if function was successfully done, otherwise \c 0.
 */
static bool udi_hid_touch_move(uint16_t x, uint16_t y, uint16_t z);


/**
 * \brief Send the report
 *
 * \return \c 1 if send on going, \c 0 if delay.
 */
static bool udi_hid_touch_send_report(void);

/**
 * \brief Callback called when the report is sent
 *
 * \param status     UDD_EP_TRANSFER_OK, if transfer finish
 * \param status     UDD_EP_TRANSFER_ABORT, if transfer aborted
 * \param nb_sent    number of data transfered
 *
 * \return \c 1 if function was successfully done, otherwise \c 0.
 */
static void udi_hid_touch_report_sent(udd_ep_status_t status,
		iram_size_t nb_sent, udd_ep_id_t ep);

//@}


//--------------------------------------------
//------ Interface for UDI HID level

bool udi_hid_touch_enable(void)
{
	// Initialize internal value
	udi_hid_touch_rate = 0;
	udi_hid_touch_protocol = 0;
	udi_hid_touch_report_trans_ongoing = false;
	memset(udi_hid_touch_report, 0, UDI_HID_TOUCH_REPORT_SIZE);
	udi_hid_touch_b_report_valid = false;
	return UDI_HID_TOUCH_ENABLE_EXT();
}


void udi_hid_touch_disable(void)
{
	UDI_HID_TOUCH_DISABLE_EXT();
}


bool udi_hid_touch_setup(void)
{
	return udi_hid_setup(&udi_hid_touch_rate,
								&udi_hid_touch_protocol,
								(uint8_t *) &udi_hid_touch_report_desc,
								udi_hid_touch_setreport);
}


uint8_t udi_hid_touch_getsetting(void)
{
	return 0;
}


static bool udi_hid_touch_setreport(void)
{
	return false;
}


//--------------------------------------------
//------ Interface for application

bool udi_hid_touch(uint16_t x, uint16_t y, uint16_t z)
{
	return udi_hid_touch_move(x,y,z);
}



//--------------------------------------------
//------ Internal routines

static bool udi_hid_touch_move(uint16_t x, uint16_t y, uint16_t z)
{
	irqflags_t flags = cpu_irq_save();

	uint16_t *u16ptr = (uint16_t *) udi_hid_touch_report;
	*u16ptr++ = x;
	*u16ptr++ = y;
	*u16ptr++ = z;

	// Valid and send report
	udi_hid_touch_b_report_valid = true;
	udi_hid_touch_send_report();

	cpu_irq_restore(flags);
	return true;
}



static bool udi_hid_touch_send_report(void)
{
	if (udi_hid_touch_report_trans_ongoing)
		return false;	// Transfer on going then send this one after transfer complete

	// Copy report on other array used only for transfer
	memcpy(udi_hid_touch_report_trans, udi_hid_touch_report,
			UDI_HID_TOUCH_REPORT_SIZE);
	memset(&udi_hid_touch_report[1], 0, 3);	// Keep status of btn for next report
	udi_hid_touch_b_report_valid = false;

	// Send report
	udi_hid_touch_report_trans_ongoing =
			udd_ep_run(	UDI_HID_TOUCH_EP_IN,
							false,
							udi_hid_touch_report_trans,
							UDI_HID_TOUCH_REPORT_SIZE,
							udi_hid_touch_report_sent);
	return udi_hid_touch_report_trans_ongoing;
}


static void udi_hid_touch_report_sent(udd_ep_status_t status,
		iram_size_t nb_sent, udd_ep_id_t ep)
{
	UNUSED(ep);
	UNUSED(status);
	UNUSED(nb_sent);
	// Valid report sending
	udi_hid_touch_report_trans_ongoing = false;
	if (udi_hid_touch_b_report_valid) {
		// Send new valid report
		udi_hid_touch_send_report();
	}
}

//@}
