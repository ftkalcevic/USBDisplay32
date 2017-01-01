/**
 * \file
 *
 * \brief USB Device Communication Device Class (CDC) interface.
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

#include "asf.h"
#include "conf_usb.h"
#include "usb_protocol.h"
#include "udd.h"
#include "udc.h"
#include "udi_cdc.h"
#include <string.h>


#include "displaycommands.h"
#include "user_board.h"
#include "lcd_conf.h"

#ifdef UDI_CDC_LOW_RATE
#  define UDI_CDC_TX_BUFFERS     (2)
#	define UDI_CDC_RX_BUFFERS     (2)
#else
#  ifdef USB_DEVICE_HS_SUPPORT
#    define UDI_CDC_TX_BUFFERS     (2)
#	  define UDI_CDC_RX_BUFFERS     (2)
#  else
#    define UDI_CDC_TX_BUFFERS     (2)
#	  define UDI_CDC_RX_BUFFERS     (2)
#  endif
#endif

#define NEXT_BUFFER(idx) ((idx==0)?1:0)
//#define NEXT_BUFFER(idx) ((idx==UDI_CDC_RX_BUFFERS-1)?0:idx+1)
/**
 * \addtogroup udi_cdc_group
 *
 * @{
 */

/**
 * \name Interface for UDC
 */
//@{

bool udi_cdc_data_enable(void);
void udi_cdc_data_disable(void);
bool udi_cdc_data_setup(void);
uint8_t udi_cdc_getsetting(void);
void udi_cdc_data_sof_notify(void);


UDC_DESC_STORAGE udi_api_t udi_api_cdc_data = {
	.enable = udi_cdc_data_enable,
	.disable = udi_cdc_data_disable,
	.setup = udi_cdc_data_setup,
	.getsetting = udi_cdc_getsetting,
	.sof_notify = 	udi_cdc_data_sof_notify,
};

//@}

//! Status of CDC interface
static volatile bool udi_cdc_running;

/**
 * \name Variables to manage RX/TX transfer requests
 * Two buffers for each sense are used to optimize the speed.
 */
//@{

typedef union _RXBuffer
{
    CmdAll cmd;
    uint8_t buf[UDI_CDC_DATA_EPS_SIZE];
} RXBuffer;

//! Buffer to receive data
COMPILER_WORD_ALIGNED static RXBuffer rx_buffer[UDI_CDC_RX_BUFFERS];
//! Data available in RX buffers
static uint16_t udi_cdc_rx_buf_nb[UDI_CDC_RX_BUFFERS];
//! Give the current RX buffer used (rx0 if 0, rx1 if 1)
static volatile uint8_t udi_cdc_rx_buf_sel;
//! Read position in current RX buffer
static volatile uint16_t udi_cdc_rx_pos;
static volatile uint32_t udi_cdc_blt_bytes_to_go;
//! Signal a transfer on-going
static volatile bool udi_cdc_rx_trans_ongoing;

//! Define a transfer halted
#define  UDI_CDC_TRANS_HALTED    2

//@}


/**
 * \brief Enable the reception of data from the USB host
 *
 * The value udi_cdc_rx_trans_sel indicate the RX buffer to fill.
 *
 * \return \c 1 if function was successfully done, otherwise \c 0.
 */
static bool udi_cdc_rx_start(void);

/**
 * \brief Update RX buffer management with a new data
 * Callback called after data reception on USB line
 *
 * \param status     UDD_EP_TRANSFER_OK, if transfer finish
 * \param status     UDD_EP_TRANSFER_ABORT, if transfer aborted
 * \param n          number of data received
 */
void udi_cdc_data_recevied(udd_ep_status_t status, iram_size_t n);



bool udi_cdc_data_enable(void)
{
	// Initialize RX management
	udi_cdc_rx_trans_ongoing = false;
	udi_cdc_rx_buf_sel = 0;
	udi_cdc_rx_buf_nb[0] = 0;
	udi_cdc_rx_pos = 0;
	udi_cdc_blt_bytes_to_go = 0;
	udi_cdc_running = udi_cdc_rx_start();
	return udi_cdc_running;
}


void udi_cdc_data_disable(void)
{
}


bool udi_cdc_data_setup(void)
{
	return false;  // request Not supported
}

uint8_t udi_cdc_getsetting(void)
{
	return 0;      // CDC don't have multiple alternate setting
}

void udi_cdc_data_sof_notify(void)
{
	//udi_cdc_tx_send();
}

//-------------------------------------------------
//------- Internal routines to control serial line


//-------------------------------------------------
//------- Internal routines to process data transfer


static bool udi_cdc_rx_start(void)
{
	irqflags_t flags;
	uint8_t buf_sel_trans;

	flags = cpu_irq_save();
	buf_sel_trans = udi_cdc_rx_buf_sel;
	if (udi_cdc_rx_trans_ongoing ||
		(udi_cdc_rx_pos < udi_cdc_rx_buf_nb[buf_sel_trans])) {
		// Transfer already on-going or current buffer no empty
		cpu_irq_restore(flags);
		return false;
	}

	// Change current buffer
	udi_cdc_rx_pos = 0;
	udi_cdc_rx_buf_sel = NEXT_BUFFER(buf_sel_trans);

	// Start transfer on RX
	udi_cdc_rx_trans_ongoing = true;
	cpu_irq_restore(flags);
	
	if (udi_cdc_is_rx_ready()) {
		UDI_CDC_RX_NOTIFY();
	}

	if ( udi_cdc_blt_bytes_to_go == 0 )
	{
		// read next command
	    return udd_ep_run( UDI_LCD_DATA_EP_OUT,
					    true,
					    rx_buffer[buf_sel_trans].buf,
					    UDI_CDC_DATA_EPS_SIZE,
					    udi_cdc_data_recevied);
	}
	else
	{
		// copy data into graphics memory
	    return udd_ep_run( UDI_LCD_DATA_EP_OUT,
					    true,
					    LCD_DATA,
					    UDI_CDC_DATA_EPS_SIZE,
					    udi_cdc_data_recevied);
	}						
}


void udi_cdc_data_recevied(udd_ep_status_t status, iram_size_t n)
{
	uint8_t buf_sel_trans;

	if (UDD_EP_TRANSFER_OK != status) {
		// Abort reception
		return;
	}
	
	buf_sel_trans = NEXT_BUFFER(udi_cdc_rx_buf_sel);
	if ( udi_cdc_blt_bytes_to_go == 0 )
	{
		// New cmd
		if ( rx_buffer[buf_sel_trans].cmd.blt.cmd == CMD_BITBLT )
		{
			udi_cdc_blt_bytes_to_go = rx_buffer[buf_sel_trans].cmd.blt.width;
			udi_cdc_blt_bytes_to_go *= rx_buffer[buf_sel_trans].cmd.blt.height;
			udi_cdc_blt_bytes_to_go *= 2;
			
			LCD_BltStart(rx_buffer[buf_sel_trans].cmd.bltrle.x, rx_buffer[buf_sel_trans].cmd.bltrle.y, rx_buffer[buf_sel_trans].cmd.bltrle.width, rx_buffer[buf_sel_trans].cmd.bltrle.height );
		}
		else if ( rx_buffer[buf_sel_trans].cmd.light.cmd == CMD_SET_BACKLIGHT && n >= sizeof(rx_buffer[buf_sel_trans].cmd.light) )
		{
			
			LCD_SetBacklight( rx_buffer[buf_sel_trans].cmd.light.intensity );
		}
		else if ( rx_buffer[buf_sel_trans].cmd.boot.cmd == CMD_BOOTLOADER  && n >= sizeof(rx_buffer[buf_sel_trans].cmd.boot) )
		{
		    if ( rx_buffer[buf_sel_trans].cmd.boot.magic[0] == BOOTLOADER_MAGIC1 &&
		         rx_buffer[buf_sel_trans].cmd.boot.magic[1] == BOOTLOADER_MAGIC2 &&
		         rx_buffer[buf_sel_trans].cmd.boot.magic[2] == BOOTLOADER_MAGIC3 &&
		         rx_buffer[buf_sel_trans].cmd.boot.magic[3] == BOOTLOADER_MAGIC4 &&
		         rx_buffer[buf_sel_trans].cmd.boot.magic[4] == BOOTLOADER_MAGIC5 )
            {
				// set the ISP_FORCE bit. Force the Watchdog to trigger
                flashc_erase_gp_fuse_bit(31, true); 
                flashc_write_gp_fuse_bit(31, true); 

                cpu_irq_disable(); 
                wdt_opt_t opt = { 
                   .us_timeout_period = 1000000 
                }; 
                wdt_enable(&opt); 
                while(1);            
			}
		}
	}
	else
	{
		// Receive data.  That should have been copied via DMA.
		udi_cdc_blt_bytes_to_go -= n;
	}

	udi_cdc_rx_buf_nb[buf_sel_trans] = 0;
	udi_cdc_rx_trans_ongoing = false;
	udi_cdc_rx_start();
}


bool udi_cdc_is_rx_ready(void)
{
	return (udi_cdc_rx_pos < udi_cdc_rx_buf_nb[udi_cdc_rx_buf_sel]);
}





