#pragma once

// Base class implementation for the higher level LCD interface.
//
// Provides some basic implementation 


#include "lcdhwiface.h"

template<int ScreenWidth, int ScreenHeight >
class LCDIFace: public LCDHwIface
{
	uint16_t m_nBackgroundColour;
	uint16_t m_nForegroundColour;

private:
	virtual void Start(void) = 0;
public:
	virtual void DisplayOn( bool on ) = 0;
	virtual void GraphicsRamMode(void) = 0;
	virtual void SetX(unsigned int x1) = 0;
	virtual void SetY(unsigned int y1) = 0;

	void SetXY(unsigned int x1,unsigned int y1)
	{
		SetX(x1);
		SetY(y1);
	}

	// Note - the rectangle end point is included in the window size, eg a display of width 400, will have x1=0,x2=399
	virtual void SetWindow(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2) = 0;

	#define REPEAT_COMMAND1(d)			d
	#define REPEAT_COMMAND2(d)			REPEAT_COMMAND1(d); REPEAT_COMMAND1(d)
	#define REPEAT_COMMAND4(d)			REPEAT_COMMAND2(d); REPEAT_COMMAND2(d)
	#define REPEAT_COMMAND8(d)			REPEAT_COMMAND4(d); REPEAT_COMMAND4(d)
	#define REPEAT_COMMAND16(d)			REPEAT_COMMAND8(d); REPEAT_COMMAND8(d)
	#define REPEAT_COMMAND32(d)			REPEAT_COMMAND16(d); REPEAT_COMMAND16(d)
	#define REPEAT_COMMAND64(d)			REPEAT_COMMAND32(d); REPEAT_COMMAND32(d)
	#define REPEAT_COMMAND128(d)		REPEAT_COMMAND64(d); REPEAT_COMMAND64(d)

	void ClearScreen(uint16_t colour)
	{
		SetWindow(0,0,ScreenWidth-1,ScreenHeight-1);
		SetXY(0,0);
		GraphicsRamMode();
		for ( uint16_t j = 0; j < ScreenHeight * ScreenWidth / 128; j++ )	// Here we are assuming the display width*height is a multiple of 128
		{
			REPEAT_COMMAND128(WriteData(colour));		// Unroll 128 iterations
		}		
	}


	void ScrollScreen(uint16_t nPixels)
	{
		SetWindow(0,0,ScreenWidth-1,ScreenHeight-1);
		// Scrolling the whole screen nPixels
		for ( uint16_t r = nPixels; r < ScreenHeight; r++)
		{
			uint16_t source_row = r;
			uint16_t dest_row = r - nPixels;
			uint16_t row[ScreenWidth];
			SetXY(0,source_row);
			for ( uint16_t c = 0; c < ScreenWidth; c++ )
			{
				SetX(c);
				GraphicsRamMode();
				uint16_t temp = ReadData();
				row[c] = ReadData();
			}
		
			SetXY(0,dest_row);
			GraphicsRamMode();
			for ( uint16_t c = 0; c < ScreenWidth; c++ )
			{
				WriteData( row[c] );
			}
		}
	}

	void BltStart( int x, uint8_t y, int nWidth, uint8_t nHeight )
	{
		SetWindow(x, y, x+nWidth-1, y+nHeight-1);
		SetXY( x, y );
		GraphicsRamMode();
	}

	void SolidRect( int x, uint8_t y, int nWidth, uint8_t nHeight )
	{
		SetWindow(x, y, x+nWidth-1, y+nHeight-1);
		SetXY( x, y );
		GraphicsRamMode();
	
		unsigned int nBytesToWrite = (int)nHeight * (int)nWidth;
		unsigned int nBlocksToWrite = nBytesToWrite / 128;
		unsigned int nRemainderBytes = nBytesToWrite % 128;
	
		for ( unsigned int i = 0; i < nBlocksToWrite; i++ )
		{
			REPEAT_COMMAND128(WriteData(m_nForegroundColour));		// Unroll 128 iterations
		}

		for ( unsigned int i = 0; i < nRemainderBytes; i++ )
		{
			WriteData(m_nForegroundColour);
		}
	}

	void Rect( int x, uint8_t y, int nWidth, uint8_t nHeight )
	{
		if ( nHeight <= 1 )	// Horizontal Line
		{
			SolidRect( x, y, nWidth, 1 );
		}
		else if ( nWidth <= 1 )  // Vertical Line
		{
			SolidRect( x, y, 1, nHeight );
		}
		else
		{		
			SolidRect( x, y, nWidth, 1 );
			SolidRect( x, y, 1, nHeight );
			SolidRect( x+nWidth-1, y, 1, nHeight );
			SolidRect( x, y+nHeight-1, nWidth, 1 );
		}		
	}


	void SetBacklight( uint16_t intensity )	// intensity 0 - 100
	{
		tc_write_ra(&AVR32_TC0, 0, intensity * 4);		// pwm value 0 to 400
	}

	void SetBackgroundColour( uint16_t colour )
	{
		m_nBackgroundColour = colour;
	}

	uint16_t GetBackgroundColour( )
	{
		return m_nBackgroundColour;
	}

	void SetForegroundColour( uint16_t colour )
	{
		m_nForegroundColour = colour;
	}

	uint16_t GetForegroundColour()
	{
		return m_nForegroundColour;
	}

	void DrawPixel( uint16_t x, uint8_t y )
	{
		//SetWindow( x, y, x, y );
		SetXY( x, y );
		GraphicsRamMode();
		WriteData(m_nForegroundColour);
	}

	void DisplayTest(void)
	{
		{
			uint8_t y=10;
			for ( uint16_t w = 1; w <= 10; w++ )
			{
				SetWindow( 10,y,10+w,y+w );
				SetXY( 10,y );
				GraphicsRamMode();
				for (uint32_t i = 0; i < 100; i++)
				{
					WriteData(RGB(0xFF,0,0xFF));
					delay_ms(1);
				}
				y+=w + 1;
			}
		}
		//Command( REG_ColumnAddressStart1, 0 );
		//Command( REG_ColumnAddressStart2, 0 );
		//Command( REG_RowAddressStart1, 0 );
		//Command( REG_RowAddressStart2, 0 );
		GraphicsRamMode();
		//for (uint32_t i = 0; i < 300040l; i++)		
		//{
			//WriteData(RGB(0xFF,0xFF,0xFF));
			//delay_ms(1);
		//}
		//for(;;){}
		for (int i = 0; i < 240; i++)		
		{
			SetXY(i,i);
			GraphicsRamMode();
			WriteData(RGB(0xFF,0xFF,0xFF));
			delay_ms(5);
		}
		delay_ms(500);
		for ( uint16_t i = 0; i < (1<<5); i++ )
    		ClearScreen(i & 1 ? RGB(127,0,0) : RGB(0,127,127) );

		int x = 0;
		int y = 0;
		int width = 400;
		int height = 240;
		SetWindow(x,y,x+width-1,y+height-1);
		SetXY(x,y);
		GraphicsRamMode();

		for ( int j = 0; j < width; j++ )
			WriteData(RGB(0xFF,0xFF,0xFF));
	
		for ( int i = 0; i < height; i++ )
    		for ( int j = 0; j < width; j++ )
			{
				if ( i == j )
					WriteData(RGB(0,0,0));
				else
				{
					int c = 0;
					int n = i > j ? 0xFF : i;
					if ( i < 50 )
						c = RGB(n,0,0);
					else if ( i < 100 )
						c = RGB(0,n,0);
					else if ( i < 150 )
						c = RGB(0,0,n);
					else
						c = RGB(0xFF,0xFF,0x80);
					WriteData(c);
				}	
			//delay_ms(1);				
			}
			
		delay_ms(500);
		for ( uint16_t i = 0; i < (1<<5); i++ )
			ClearScreen(i);

		SetForegroundColour(RGB(0xFF,0,0));
		SolidRect( 0, 0, 133, 239 );
		SetForegroundColour(RGB(0,0xff,0));
		SolidRect( 133, 0, 133, 239 );
		SetForegroundColour(RGB(0,0,0xff));
		SolidRect( 266, 0, 133, 239 );

		SetForegroundColour(RGB(0,0xff,0));		
		SetBackgroundColour(RGB(0,0,0));		

		delay_ms(1000);
	}

	void Init(void)
	{
		m_nBackgroundColour=RGB(0,0,0);
		m_nForegroundColour=RGB(0xff,0xff,0xff);

		LCDInitIO();

		Start();	
		DisplayOn(true);
		ClearScreen(0);
		//LCDText_Init();
		//DisplayTest();
	}
};

