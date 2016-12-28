#pragma once

// Base class implementation for the higher level LCD interface.
//
// Provides some basic implementation 


#include "lcdhwiface.h"

template<class TIFaceClass, int ScreenWidth, int ScreenHeight>
class LCDIFace: public TIFaceClass
{
	uint16_t m_nBackgroundColour;
	uint16_t m_nForegroundColour;

public:
	LCDIFace()
	{
		m_nBackgroundColour = RGB(0,0,0);
		m_nForegroundColour = RGB(0xff,0xff,0xff);
	}

public:

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
		this->SetWindow(0,0,ScreenWidth-1,ScreenHeight-1);
		this->SetXY(0,0);
		this->GraphicsRamMode();
		for ( uint16_t j = 0; j < ScreenHeight * ScreenWidth / 128; j++ )	// Here we are assuming the display width*height is a multiple of 128
		{
			REPEAT_COMMAND128(this->WriteData(colour));		// Unroll 128 iterations
		}		
	}


	void ScrollScreen(uint16_t nPixels, bool bErase = true)
	{
		if ( TIFaceClass::ScrollScreen(nPixels) )	// A bit of hack to avoid using virtual functions (slight performance gain).
			return;

		this->SetWindow(0,0,ScreenWidth-1,ScreenHeight-1);
		// Scrolling the whole screen nPixels
		for ( uint16_t r = nPixels; r < ScreenHeight; r++)
		{
			uint16_t source_row = r;
			uint16_t dest_row = r - nPixels;
			uint16_t row[ScreenWidth];
			this->SetXY(0,source_row);
			for ( uint16_t c = 0; c < ScreenWidth; c++ )
			{
				this->SetX(c);
				this->GraphicsRamMode();
				uint16_t temp = this->ReadData();
				temp = this->ReadData();
				row[c] = temp;
			}
		
			this->SetXY(0,dest_row);
			this->GraphicsRamMode();
			for ( uint16_t c = 0; c < ScreenWidth; c++ )
			{
				this->WriteData( row[c] );
			}
		}
		if ( bErase )
		{
			uint16_t nOldForeColour = GetForegroundColour();
			SetForegroundColour( GetBackgroundColour() );
			SolidRect(0,ScreenHeight-1-nPixels, ScreenWidth-1,ScreenHeight-1);
			SetForegroundColour( nOldForeColour );
		}
	}

	void BltStart( int x, uint8_t y, int nWidth, uint8_t nHeight )
	{
		this->SetWindow(x, y, x+nWidth-1, y+nHeight-1);
		this->SetXY( x, y );
		this->GraphicsRamMode();
	}

	void SolidRect( int x, uint8_t y, int nWidth, uint8_t nHeight )
	{
		this->SetWindow(x, y, x+nWidth-1, y+nHeight-1);
		this->SetXY( x, y );
		this->GraphicsRamMode();
	
		unsigned int nBytesToWrite = (int)nHeight * (int)nWidth;
		unsigned int nBlocksToWrite = nBytesToWrite / 128;
		unsigned int nRemainderBytes = nBytesToWrite % 128;
	
		for ( unsigned int i = 0; i < nBlocksToWrite; i++ )
		{
			REPEAT_COMMAND128(this->WriteData(m_nForegroundColour));		// Unroll 128 iterations
		}

		for ( unsigned int i = 0; i < nRemainderBytes; i++ )
		{
			this->WriteData(m_nForegroundColour);
		}
	}

	void Rect( int x, uint8_t y, int nWidth, uint8_t nHeight )
	{
		if ( nHeight <= 1 )	// Horizontal Line
		{
			this->SolidRect( x, y, nWidth, 1 );
		}
		else if ( nWidth <= 1 )  // Vertical Line
		{
			this->SolidRect( x, y, 1, nHeight );
		}
		else
		{		
			this->SolidRect( x, y, nWidth, 1 );
			this->SolidRect( x, y, 1, nHeight );
			this->SolidRect( x+nWidth-1, y, 1, nHeight );
			this->SolidRect( x, y+nHeight-1, nWidth, 1 );
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
		this->SetXY( x, y );
		this->GraphicsRamMode();
		this->WriteData(m_nForegroundColour);
	}

	void Init(void)
	{
		this->LCDInitIO();

		this->Start();	
		this->DisplayOn(true);
		ClearScreen(0);
	}
};

