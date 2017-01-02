#pragma once

// LCD interface for HX8352A.
//
// Simple class to interface to the LCD hardware

#include "lcdiface.h"
#include "hx8352a.h"

// Currently only tested with a 240x400 running in landscape mode (upside down).

template<int ScreenWidth, int ScreenHeight>
class LCDIFaceHX8352A: public LCDHwIface
{
protected:
	void Start(void)
	{
		// Start up sequence based on UTFT hx8352 start up code - http://www.rinkydinkelectronics.com/library.php?id=51
		gpio_set_gpio_pin(LCDReset);
		delay_ms(5);
		gpio_clr_gpio_pin(LCDReset);
		delay_ms(5);
		gpio_set_gpio_pin(LCDReset);
		delay_ms(5);
	
		Command( REG_TestMode, TEST_Mode );
		Command( REG_VDDDcontrol, 3 );						//VDC_SEL=011
		Command( REG_VGSRESControl1, RES_VGS1 );
		Command( REG_VGSRESControl2, RES_VGS0 | 0x13 );		//STBA[7]=1,STBA[5:4]=01,STBA[1:0]=11
		Command( REG_PWMControl0, SYNC );					//DCDC_SYNC=1
		Command( REG_TestMode, 0 );

		//Gamma Setting
		Command( REG_GammaControl1, 0xB0 );
		Command( REG_GammaControl2, 0x03 );
		Command( REG_GammaControl3, 0x10 );
		Command( REG_GammaControl4, 0x56 );
		Command( REG_GammaControl5, 0x13 );
		Command( REG_GammaControl6, 0x46 );
		Command( REG_GammaControl7, 0x23 );
		Command( REG_GammaControl8, 0x76 );
		Command( REG_GammaControl9, 0x00 );
		Command( REG_GammaControl10, 0x5E );
		Command( REG_GammaControl11, 0x4F );
		Command( REG_GammaControl12, 0x40 );

		//**********Power On sequence************
        
		Command( REG_OSCControl1, 0x90 | OSC_EN );
		Command( REG_CycleControl1, 0xF9 );
		delay_ms(10);
        
		Command( REG_PowerControl3, 0x14 );
		Command( REG_PowerControl2, 0x11 );
		Command( REG_PowerControl4, 0x06 );
		Command( REG_VCOMControl, 0x42 );
		delay_ms(20);
        
		Command( REG_PowerControl1, SDK | VL_TRI );
		Command( REG_PowerControl1, SDK | VL_TRI | PON );
		delay_ms(40);
        
		Command( REG_PowerControl1, VL_TRI | PON );
		delay_ms(40);
        
		Command( REG_PowerControl6, 0x27 );
		delay_ms(100);	   
        
        
		//**********DISPLAY ON SETTING***********
		Command( REG_DisplayControl2, 0x60 );
		Command( REG_SourceControl2, 0x40 );
		Command( REG_CycleControl10, 0x38 );
		Command( REG_CycleControl11, 0x38 );
		Command( REG_DisplayControl2, 0x38 );
		delay_ms(40);
        
		Command( REG_DisplayControl2, 0x3C );
		// MV=1  MX=0  MY=1  
	//	Command( REG_MemoryAccessControl, GS | BGR | SS );
		Command( REG_MemoryAccessControl, MX | MV | GS | BGR | SS );

		Command( REG_DisplayMode, 0x06 );
		Command( REG_PANELControl, 0x00 );

		SetWindow( 0, 0, ScreenWidth-1, ScreenHeight-1 );
	}
public:
	void DisplayOn( bool on )
	{
		//if ( on )
			//Command( REG_DisplayControl2, 0x3C );
		//else
			//Command( REG_DisplayControl2, 0x34 );
	}

	void Sleep()
	{
	}

	void Wake()
	{
	}

	void GraphicsRamMode(void)
	{
		WriteCommand(REG_DataReadWrite);
	}

	void SetX(unsigned int x1)
	{
		Command(REG_ColumnAddressStart1, x1 >> 8);
		Command(REG_ColumnAddressStart2, x1 & 0xFF);
	}

	void SetY(unsigned int y1)
	{
		Command(REG_RowAddressStart1, y1 >> 8);
		Command(REG_RowAddressStart2, y1 & 0xFF);
	}

	void SetXY(unsigned int x1,unsigned int y1)
	{
		SetX(x1);
		SetY(y1);
	}

	// Note - the rectangle end point is included in the window size, eg a display of width 400, will have x1=0,x2=399
	void SetWindow(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2)
	{
		Command(REG_RowAddressStart1, y1 >> 8);
		Command(REG_RowAddressStart2, y1 & 0xFF);
		Command(REG_RowAddressEnd1, y2 >> 8);
		Command(REG_RowAddressEnd2, y2 & 0xFF);
		Command(REG_ColumnAddressStart1, x1 >> 8 );
		Command(REG_ColumnAddressStart2, x1 & 0xFF );
		Command(REG_ColumnAddressEnd1, x2 >> 8 );
		Command(REG_ColumnAddressEnd2, x2 & 0xFF);
	}

	bool ScrollScreen(uint16_t nPixels) 
	{ 
		// This only scrolls vertically (in portrait mode - it ignores the MX MY MV settings).
		//Command(REG_VerticalScrollTopFixedArea1, 0);
		//Command(REG_VerticalScrollTopFixedArea2, 0);
		//Command(REG_VerticalScrollBottomFixed1, 0);
		//Command(REG_VerticalScrollBottomFixed2, 0);
		//Command(REG_VerticalScrollHeightArea1, 0);
		//Command(REG_VerticalScrollHeightArea2, nPixels );
		//Command(REG_VerticalScrollStartAddress1, 0);
		//Command(REG_VerticalScrollStartAddress2, nPixels);
		
		////WriteCommand(REG_MemoryAccessControl);
		////uint8_t n = ReadCommand();
		////n = ReadCommand();
		////Command( REG_MemoryAccessControl, n | SRL_EN );
		//Command( REG_MemoryAccessControl, MX | MV | GS | BGR | SS | SRL_EN );

		//return true; 
		return false;
	}

	bool _SetBacklight(uint8_t intensity) { return false; }	// A bit of hack to avoid using virtual functions (slight performance gain).

};