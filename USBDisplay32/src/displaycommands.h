// graphicdisplay, firmware for the lcd graphic display
// Copyright (C) 2009, Frank Tkalcevic, www.franksworkshop.com

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#ifndef _DISPLAYCOMMANDS_H_
#define _DISPLAYCOMMANDS_H_

#define CMD_SET_BACKGROUND_COLOUR	0x80	    // Set Background Colour CMD, R, G, B
#define CMD_SET_FOREGROUND_COLOUR	0x81	    // Set Foreground Colour CMD, R, G, B
#define CMD_CLEAR_SCREEN		0x82	    // Clear Screen CMD,
#define CMD_WRITE_TEXT			0x83	    // Write Text - CMD, data..., NULL
#define CMD_SET_TEXT_POSITION		0x84	    // Set position CMD, x, y
#define CMD_SET_GRAPHICS_POSITION	0x85	    // set position CMD, xlsb, xmsb, y
#define CMD_SET_FONT			0x86	    // Set font CMD, 0/1
#define CMD_SET_BACKLIGHT		0x87	    // set backlight CMD, 0-100
#define CMD_SET_TEXT_ATTRIBUTE		0x88	    // set attribute CMD, [underline][Reverse?bg/fg?][italic][bold][strikethrough]
#define     TEXT_ATTRIBUTE_UNDERLINE	    0x01
#define     TEXT_ATTRIBUTE_STRIKEOUT	    0x02
#define CMD_DRAW_RECTANGLE		0x89	    // rect xlsb, xmsg, y, widthlsb, widthmsb, height, fill
#define CMD_DRAW_PIXEL		    0x8A	    // cmd, xlsb, xmsg, y
#define CMD_BITBLT              0x8B	    // cmd, xlsb, xmsg, y, widthlsb, widthmsb, height, [width*height*16bit pixels MSB:LSB:R5G6B5]
#define CMD_BITBLT_RLE          0x8C	    // cmd, xlsb, xmsg, y, widthlsb, widthmsb, height, datalenlsb, datalenb1, datalenb2, [datalen*bytes]
#define CMD_BOOTLOADER          0x8D	    // cmd, MAGIC1, MAGIC2, MAGIC3, MAGIC4

#define CMD_MIN				0x80
#define CMD_MAX				0x8C

#pragma pack(1)

typedef struct _CmdSetBackgroundColour
{
    uint8_t cmd;
    uint8_t r;
    uint8_t g;
    uint8_t b;
} CmdSetBackgroundColour;

typedef struct _CmdSetForegroundColour
{
    uint8_t cmd;
    uint8_t r;
    uint8_t g;
    uint8_t b;
} CmdSetForegroundColour;

typedef struct _CmdClearScreen
{
    uint8_t cmd;
} CmdClearScreen;

typedef struct _CmdWriteText
{
    uint8_t cmd;
    uint8_t data[0];   // null terminated string.
} CmdWriteText;

typedef struct _CmdSetTextPosition
{
    uint8_t cmd;
    uint8_t x;
    uint8_t y;
} CmdSetTextPosition;

typedef struct _CmdSetGraphicsPosition
{
    uint8_t cmd;
    uint16_t x;
    uint16_t y;
} CmdSetGraphicsPosition;

typedef struct _CmdSetFont
{
    uint8_t cmd;
    uint8_t font;
} CmdSetFont;

typedef struct _CmdSetBacklight
{
    uint8_t cmd;
    uint8_t intensity;
} CmdSetBacklight;

typedef struct _CmdSetAttribute
{
    uint8_t cmd;
    uint8_t attribute;
} CmdSetAttribute;

typedef struct _CmdDrawRectangle
{
    uint8_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint8_t fill;
} CmdDrawRectangle;

typedef struct _CmdDrawPixel
{
    uint8_t cmd;
    uint16_t x;
    uint16_t y;
} CmdDrawPixel;

typedef struct _CmdBitBlt
{
    uint8_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t data[0];
} CmdBitBlt;

typedef struct _CmdBitBltRLE
{
    uint8_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint8_t datalen[3]; // 24 bit data length.
    uint16_t data[0];
} CmdBitBltRLE;

typedef struct _CmdBootLoader
{
    uint8_t cmd;
    uint8_t magic[5];
} CmdBootLoader;

#define BOOTLOADER_MAGIC1   0x70
#define BOOTLOADER_MAGIC2   0x74
#define BOOTLOADER_MAGIC3   0x84
#define BOOTLOADER_MAGIC4   0x19
#define BOOTLOADER_MAGIC5   0x68

typedef union _CmdAll
{
	uint8_t cmd;
    CmdSetBackgroundColour bg;
    CmdSetForegroundColour fg;
    CmdClearScreen cls;
    CmdWriteText text;
    CmdSetTextPosition tpos;
    CmdSetGraphicsPosition gpos;
    CmdSetFont font;
    CmdSetBacklight light;
    CmdSetAttribute attr;
    CmdDrawRectangle rect;
    CmdDrawPixel pixel;
    CmdBitBlt blt;
    CmdBitBltRLE bltrle;
	CmdBootLoader boot;
} CmdAll;

#define MAX_CMD_LEN			(sizeof(CmdAll))
#pragma pack()

#endif

