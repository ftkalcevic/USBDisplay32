/*
 * lcdtext.h
 *
 * Created: 16/04/2011 7:47:27 AM
 *  Author: Frank Tkalcevic
 */ 


#ifndef _LCDTEXT_H_
#define _LCDTEXT_H_

#define PROGMEM

#define MAX_CHARS	128

// TODO these are attributes of the font
#define FIRST_CHAR  0x20
#define CHAR_COUNT  0x60

struct SFontData
{
	uint8_t nCharWidth;
	uint8_t nCharHeight;
	uint8_t nCharsPerLine;
	uint8_t nLines;
	uint8_t * PROGMEM pFont;
	uint8_t * PROGMEM nFontOffsets[MAX_CHARS];
};


template<class TLCD, int ScreenWidth, int ScreenHeight>
class LCDText
{
private:
	TLCD &m_lcd;
	uint32_t m_TextX;
	uint32_t m_TextY;
	uint32_t m_PixelX;
	uint32_t m_PixelY;
	bool m_bPixelCursor;
	bool m_bAutoScroll;
	struct SFontData *m_pFont;
	struct SFontData *m_pFontData;

public:
	LCDText( TLCD &lcd, struct SFontData *fonts, uint8_t fontCount ) : m_lcd(lcd)
	{
		m_TextX = 0;
		m_TextY = 0;
		m_PixelX = 0;
		m_PixelY = 0;
		m_bPixelCursor = false;
		m_pFontData = fonts;
		m_pFont = fonts;
		m_bAutoScroll = false;

		int f, i;
		for ( f = 0; f < fontCount; f++ )
			for ( i = 0; i < MAX_CHARS; i++ )
				if ( i < FIRST_CHAR || i > FIRST_CHAR + CHAR_COUNT )
					m_pFontData[f].nFontOffsets[i] = 0;
				else
					m_pFontData[f].nFontOffsets[i] = m_pFontData[f].pFont + (i-FIRST_CHAR) * ((m_pFontData[f].nCharWidth>>3)+1) * m_pFontData[f].nCharHeight;

    }

	void SetTextCursor( uint8_t x, uint8_t y )
	{
		m_TextX = x;
		m_TextY = y;
		m_bPixelCursor = false;
	}

	void SetPixelCursor( uint16_t x, uint8_t y )
	{
		m_PixelX = x;
		m_PixelY = y;
		m_bPixelCursor = true;
	}

	void SetFont( uint8_t fontId )
	{
		m_pFont = m_pFontData + (fontId&1);	// currently only font 0 and 1
	}

	void SetAutoScroll(bool bAutoScroll)
	{
		m_bAutoScroll = bAutoScroll;
	}

	void DrawChar( uint16_t x, uint8_t y, char c )
	{
		m_lcd.SetWindow( x, y, x+m_pFont->nCharWidth-1, y+m_pFont->nCharHeight-1 );
		//m_lcd.SetXY( x, y );
		m_lcd.GraphicsRamMode();

		uint8_t nColMask = 7;
		if ( c < 32 || c > 127 )
			c = '?';
		uint8_t *pFont = m_pFont->nFontOffsets[(int)c];
		uint8_t row;
		for ( row = 0; row < m_pFont->nCharHeight; row++ )
		{
			uint8_t col, data, bit;
			for ( col = 0; col < m_pFont->nCharWidth; col++ )
			{
				if ( !(col & nColMask) )
				{
					data = *(pFont++);
					bit = 0x80;
				}
				if ( data & bit )
				{
					m_lcd.WriteData(m_lcd.GetForegroundColour());
				}
				else
				{
					m_lcd.WriteData(m_lcd.GetBackgroundColour());
				}
				bit >>= 1;
			}
		}
	}

	void WriteChar( char c )
	{
		if (m_bPixelCursor)
		{
			DrawChar(m_PixelX, m_PixelY, c);
			m_PixelX += m_pFont->nCharWidth;

			if (m_PixelX + m_pFont->nCharWidth >= ScreenWidth)
			{
				m_PixelX = 0;
				m_PixelY += m_pFont->nCharHeight;
				if (m_PixelY + m_pFont->nCharHeight > ScreenHeight)
				m_PixelY = 0;
			}
		}
		else
		{
			uint32_t y = m_TextY;
			y *= m_pFont->nCharHeight;
			uint32_t x = m_TextX;
			x *= m_pFont->nCharWidth;

			DrawChar(x, y, c);

			m_TextX++;
			if (m_TextX >= m_pFont->nCharsPerLine)
			{
				m_TextX = 0;
				m_TextY++;
				if (m_TextY >= m_pFont->nLines)
				m_TextY = 0; // TODO for now, just wrap.  Maybe scroll later.
			}
		}
	}

	// Writing a string is a bit faster, especially in text cursor mode (no need to multiply the coordinates)
	void WriteString( const char *s )
	{
		if (m_bPixelCursor)
		{
			char c;
			while ( (c = *(s++)) != '\0' )
			{
				DrawChar(m_PixelX, m_PixelY, c);

				m_PixelX += m_pFont->nCharWidth;

				if (m_PixelX + m_pFont->nCharWidth >= ScreenWidth)
				{
					m_PixelX = 0;
					m_PixelY += m_pFont->nCharHeight;
					if (m_PixelY + m_pFont->nCharHeight > ScreenHeight)
						m_PixelY = 0;
				}
			}
		}
		else
		{
			uint32_t y = m_TextY;
			y *= m_pFont->nCharHeight;
			uint32_t x = m_TextX;
			x *= m_pFont->nCharWidth;

			char c;
			while ( (c = *(s++)) != '\0' )
			{

				bool bNewLine = false;
				if ( c == '\n')
				{
					bNewLine = true;
				}
				else
				{
					DrawChar(x, y, c);

					m_TextX++;
					if (m_TextX >= m_pFont->nCharsPerLine)
					{
						bNewLine = true;
					}
					else
						x += m_pFont->nCharWidth;
				}
				if ( bNewLine )
				{
					m_TextX = 0;
					x = 0;

					m_TextY++;
					if (m_TextY >= m_pFont->nLines)
					{
						if ( m_bAutoScroll )
						{
							m_TextY--;
							m_lcd.ScrollScreen(m_pFont->nCharHeight);
						}
						else
						{
							m_TextY = 0; // for now, just wrap.  Maybe scroll later.
							y = 0;
						}
					}
					else
						y += m_pFont->nCharHeight;
				}
			}
		}
	}

	void WriteString( uint16_t nLen, const char *s )
	{
		while ( nLen-- )
		{
			WriteChar( *s++ );
		}
	}

};

#endif /* _LCDTEXT_H_ */
