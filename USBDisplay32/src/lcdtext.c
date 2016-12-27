/*
 * CProgram1.c
 *
 * Created: 22/04/2011 4:17:19 PM
 *  Author: Frank Tkalcevic
 */ 

#include <board.h>
#include <conf_board.h>
#include "asf.h"

#include "lcdif.h"
#include "lcdtext.h"

#define PROGMEM

#include "../fonts/Font6x13.h"
#include "../fonts/FontTypewriter.h"


struct SFontData
{
    uint8_t nCharWidth;
    uint8_t nCharHeight;
    uint8_t nCharsPerLine;
    uint8_t nLines;
    uint8_t * PROGMEM pFont;
    uint8_t * PROGMEM nFontOffsets[128];
};

#define FIRST_CHAR  0x20
#define CHAR_COUNT  0x60


static struct SFontData  fontData[] =
{
/*45x17*/    { FONT6X13_WIDTH, FONT6X13_HEIGHT, SCREEN_WIDTH / FONT6X13_WIDTH, SCREEN_HEIGHT / FONT6X13_HEIGHT, Font6x13 },
/*22x9 */    { FONTTYPEWRITER_WIDTH, FONTTYPEWRITER_HEIGHT, SCREEN_WIDTH / FONTTYPEWRITER_WIDTH, SCREEN_HEIGHT / FONTTYPEWRITER_HEIGHT, FontTypewriter },
};


static uint32_t g_TextX;
static uint32_t g_TextY;
static uint32_t g_PixelX;
static uint32_t g_PixelY;
static bool g_bPixelCursor;
static struct SFontData *g_pFont = fontData;


static void DrawChar( uint16_t x, uint8_t y, char c )
{
    LCD_SetWindow( x, y, x+g_pFont->nCharWidth, y+g_pFont->nCharHeight );
    LCD_SetXY( x, y );
    LCD_GraphicsRamMode();

    uint8_t nColMask = 7;
    if ( c < 32 || c > 127 )
    	c = '?';
    uint8_t *pFont = g_pFont->nFontOffsets[(int)c];
    uint8_t row;
    for ( row = 0; row < g_pFont->nCharHeight; row++ )
    {
	    uint8_t col, data, bit;
	    for ( col = 0; col < g_pFont->nCharWidth; col++ )
	    {
	        if ( !(col & nColMask) )
	        {
		        data = *(pFont++);
		        bit = 0x80;
	        }
	        if ( data & bit )
	        {
		        LCD_WriteData(g_ForegroundColour);
	        }
	        else
	        {
		        LCD_WriteData(g_BackgroundColour);
	        }
	        bit >>= 1;
	    }
    }
}

void LCDText_WriteChar( char c )
{
    if (g_bPixelCursor)
    {
	    DrawChar(g_PixelX, g_PixelY, c);
	    g_PixelX += g_pFont->nCharWidth;

	    if (g_PixelX + g_pFont->nCharWidth >= SCREEN_WIDTH)
	    {
	        g_PixelX = 0;
	        g_PixelY += g_pFont->nCharHeight;
	        if (g_PixelY + g_pFont->nCharHeight > SCREEN_HEIGHT)
		        g_PixelY = 0;
	    }
    }
    else
    {
	    uint32_t y = g_TextY;
	    y *= g_pFont->nCharHeight;
	    uint32_t x = g_TextX;
	    x *= g_pFont->nCharWidth;

	    DrawChar(x, y, c);

	    g_TextX++;
	    if (g_TextX >= g_pFont->nCharsPerLine)
	    {
	        g_TextX = 0;
	        g_TextY++;
	        if (g_TextY >= g_pFont->nLines)
		    g_TextY = 0; // for now, just wrap.  Maybe scroll later.
        }
    }
}

 // Writing a string is a bit faster, especially in text cursor mode (no need to multiply the coordinates)
void LCDText_WriteString( uint8_t nLen, const char *s )
{
    if (g_bPixelCursor)
    {
	    while (nLen)
	    {
	        DrawChar(g_PixelX, g_PixelY, *s);
	        s++;
	        nLen--;

	        g_PixelX += g_pFont->nCharWidth;

	        if (g_PixelX + g_pFont->nCharWidth >= SCREEN_WIDTH)
	        {
		        g_PixelX = 0;
		        g_PixelY += g_pFont->nCharHeight;
		        if (g_PixelY + g_pFont->nCharHeight > SCREEN_HEIGHT)
		            g_PixelY = 0;
	        }
	    }
    }
    else
    {
	    uint32_t y = g_TextY;
	    y *= g_pFont->nCharHeight;
	    uint32_t x = g_TextX;
	    x *= g_pFont->nCharWidth;

	    while (nLen)
	    {
	        char c = *s;
	        s++;
	        nLen--;

	        if ( c == '\n')
	        {
		        g_TextX = 0;
		        x = 0;

		        g_TextY++;
		        if (g_TextY >= g_pFont->nLines)
		        {
		            g_TextY = 0; // for now, just wrap.  Maybe scroll later.
		            y = 0;
		        }
		        else
		            y += g_pFont->nCharHeight;
	        }
	        else
	        {
		        DrawChar(x, y, c);

		        g_TextX++;
		        if (g_TextX >= g_pFont->nCharsPerLine)
		        {
		            g_TextX = 0;
		            x = 0;

		            g_TextY++;
		            if (g_TextY >= g_pFont->nLines)
		            {
			            g_TextY = 0; // for now, just wrap.  Maybe scroll later.
			            y = 0;
		            }
		            else
			           y += g_pFont->nCharHeight;
		        }
		        else
		            x += g_pFont->nCharWidth;
	        }
	    }
    }
}

void LCDText_SetTextCursor( uint8_t x, uint8_t y )
{
    g_TextX = x;
    g_TextY = y;
    g_bPixelCursor = false;
}

void LCDText_SetPixelCursor( uint16_t x, uint8_t y )
{
    g_PixelX = x;
    g_PixelY = y;
    g_bPixelCursor = true;
}

void LCDText_SetFont( uint8_t fontId )
{
    g_pFont = fontData + (fontId&1);	// currently only font 0 and 1
}


void LCDText_Init( void )
{
    g_TextX = 0;
    g_TextY = 0;
    g_PixelX = 0;
    g_PixelY = 0;
    g_bPixelCursor = false;
    g_pFont = &(fontData[0]);
	
    int f, i;
    for ( f = 0; f < 2; f++ )
    	for ( i = 0; i < 128; i++ )
    	    if ( i < FIRST_CHAR || i > FIRST_CHAR + CHAR_COUNT )
    		    fontData[f].nFontOffsets[i] = 0;
    	    else
    		    fontData[f].nFontOffsets[i] = fontData[f].pFont + (i-FIRST_CHAR) * ((fontData[f].nCharWidth>>3)+1) * fontData[f].nCharHeight;
}
