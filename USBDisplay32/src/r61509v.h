/*
 * Register definitions for the Renesas R61509V LCD chip set.  Based on R61509.h from ....
 */

/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id: r61509.h 22080 2009-07-29 12:57:42Z mcuelenaere $
 *
 * Copyright (C) 2008 by Maurus Cuelenaere
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

#ifndef __R61509V_H
#define __R61509V_H

/* Register list */
#define REG_DEVICE_CODE						0x000
	#define DEVICE_CODE_R61509V						0xB509
#define REG_SYNC							0x000
#define REG_DRIVER_OUTPUT					0x001
	#define DRIVER_OUTPUT_SS						(1 << 8)
	#define DRIVER_OUTPUT_SM						(1 << 10)
#define REG_LCD_DR_WAVE_CTRL				0x002
	#define LCD_DR_WAVE_CTRL_BC						(1 << 8)
#define REG_ENTRY_MODE						0x003
	#define ENTRY_MODE_TRI							(1 << 15)
	#define ENTRY_MODE_DFM							(1 << 14)
	#define ENTRY_MODE_BGR							(1 << 12)
	#define ENTRY_MODE_ORG							(1 << 7)
	#define ENTRY_MODE_VID							(1 << 5)
	#define ENTRY_MODE_HID							(1 << 4)
	#define ENTRY_MODE_AM							(1 << 3)
#define REG_DISP_CTRL1						0x007
	#define DISP_CTRL1_PTDE							(1 << 12)
	#define DISP_CTRL1_BASEE						(1 << 8)
#define REG_DISP_CTRL2						0x008
	#define DISP_CTRL2_FP(n)						(((n)&0xFF)<<8)
	#define DISP_CTRL2_BP(n)						((n)&0xFF)
#define REG_DISP_CTRL3						0x009
	#define DISP_CTRL3_PTV							(1<<11)
	#define DISP_CTRL3_PTS							(1<<10)
#define REG_EIGHT_COLOUR_CTRL               0x00B
	#define EIGHT_COLOUR_CTRL_COL					(1)
#define REG_EXT_DISP_CTRL1					0x00C
#define REG_EXT_DISP_CTRL2					0x00F
#define REG_PAN_INTF_CTRL1					0x010
	#define PWR_CTRL1_DVI(n)						((n & 3) << 8)
	#define PWR_CTRL1_RTNI(n)						(n & 0x1f)
#define REG_PAN_INTF_CTRL2					0x011
	#define PWR_CTRL2_NOWI(n)						((n & 7) << 8)
	#define PWR_CTRL2_SDTI(n)						(n & 0x7)
#define REG_PAN_INTF_CTRL3					0x012
	#define PWR_CTRL3_VEQWI(n)						((n & 3) << 8)
	#define PWR_CTRL3_SEQWI(n)						((n & 3) << 8)
#define REG_PAN_INTF_CTRL4					0x013
#define REG_PAN_INTF_CTRL5					0x014
#define REG_PAN_INTF_CTRL6					0x020
#define REG_PAN_INTF_CTRL7					0x021
#define REG_PAN_INTF_CTRL8					0x022
#define REG_PAN_INTF_CTRL9					0x023
#define REG_FRM_MRKR_CTRL					0x090
	#define FRM_MRKR_CTRL_FMK						(1<<15)
	#define FRM_MRKR_CTRL_FMI(n)					(((n)&7)<<15)
	#define FRM_MRKR_CTRL_FMP(n)					((n)&0x1FF)

#define REG_PWR_CTRL1                    0x100
#define REG_PWR_CTRL2                    0x101
#define REG_PWR_CTRL3                    0x102
#define REG_PWR_CTRL4                    0x103
#define REG_PWR_CTRL5                    0x107
#define REG_PWR_CTRL6                    0x110
#define REG_PWR_CTRL7                    0x112

#define REG_RAM_HADDR_SET                0x200
#define REG_RAM_VADDR_SET                0x201
#define REG_RW_GRAM                      0x202
#define REG_RAM_HADDR_START              0x210
#define REG_RAM_HADDR_END                0x211
#define REG_RAM_VADDR_START              0x212
#define REG_RAM_VADDR_END                0x213
#define REG_RW_NVM                       0x280

#define REG_GAMMA_CTRL1                  0x300
#define REG_GAMMA_CTRL2                  0x301
#define REG_GAMMA_CTRL3                  0x302
#define REG_GAMMA_CTRL4                  0x303
#define REG_GAMMA_CTRL5                  0x304
#define REG_GAMMA_CTRL6                  0x305
#define REG_GAMMA_CTRL7                  0x306
#define REG_GAMMA_CTRL8                  0x307
#define REG_GAMMA_CTRL9                  0x308


#define REG_BIMG_NR_LINE					0x400
	#define BIMG_NR_LINE_GS						(1 << 15)
	#define BIMG_NR_LINE_NL(n)					(((n)&0x3f) << 9)
	#define BIMG_NR_LINE_SCN(n)					(((n)&0x3f) << 1)
#define REG_BIMG_DISP_CTRL					0x401
	#define REG_BIMG_DISP_CTRL_NDL				(1 << 2)
	#define REG_BIMG_DISP_CTRL_VLE				(1 << 1)
	#define REG_BIMG_DISP_CTRL_REV				(1 << 0)
#define REG_BIMG_VSCROLL_CTRL				0x404
	#define BIMG_VSCROLL_CTRL(n)				((n)&0x1Ff)

#define REG_PARTIMG1_POS					0x500
#define REG_PARTIMG1_RAM_START				0x501
#define REG_PARTIMG1_RAM_END				0x502

#define REG_SOFT_RESET						0x600
#define REG_NVM_ACCESS_CTRL					0x6F0
	#define NVM_ACCESS_CTRL_TE					(1<<7)
	#define NVM_ACCESS_CTRL_CALB				(1<<6)

/* Bits */



#define SOFT_RESET(n)                    (n << 0)

#endif /* __R61509V_H */
