/*
 * lcdtext.h
 *
 * Created: 16/04/2011 7:47:27 AM
 *  Author: Frank Tkalcevic
 */ 


#ifndef _LCDTEXT_H_
#define _LCDTEXT_H_

void LCDText_Init( void );
void LCDText_WriteChar( char c );
void LCDText_WriteString( uint8_t nLen, const char *s );
void LCDText_SetFont( uint8_t fontId );
void LCDText_SetPixelCursor( uint16_t x, uint8_t y );
void LCDText_SetTextCursor( uint8_t x, uint8_t y );

#endif /* _LCDTEXT_H_ */
