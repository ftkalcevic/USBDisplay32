#pragma once

// LCD interface for HX8352A.
//
// Simple class to interface to the LCD hardware

#include "lcdiface.h"
#include "hx8352a.h"

// Currently only tested with a 240x400 running in landscape mode (upside down).

template<int ScreenWidth, int ScreenHeight>
class LCDIFaceHX8352A: public LCDIFace<ScreenWidth, ScreenHeight>
{
private:
	virtual void Start(void)
	{
		// Start up sequence based on UTFT hx8352 start up code - http://www.rinkydinkelectronics.com/library.php?id=51
		gpio_set_gpio_pin(LCDReset);
		delay_ms(5);
		gpio_clr_gpio_pin(LCDReset);
		delay_ms(5);
		gpio_set_gpio_pin(LCDReset);
		delay_ms(5);
	
		this->Command( REG_TestMode, TEST_Mode );
		this->Command( REG_VDDDcontrol, 3 );						//VDC_SEL=011
		this->Command( REG_VGSRESControl1, RES_VGS1 );
		this->Command( REG_VGSRESControl2, RES_VGS0 | 0x13 );		//STBA[7]=1,STBA[5:4]=01,STBA[1:0]=11
		this->Command( REG_PWMControl0, SYNC );					//DCDC_SYNC=1
		this->Command( REG_TestMode, 0 );

		//Gamma Setting
		this->Command( REG_GammaControl1, 0xB0 );
		this->Command( REG_GammaControl2, 0x03 );
		this->Command( REG_GammaControl3, 0x10 );
		this->Command( REG_GammaControl4, 0x56 );
		this->Command( REG_GammaControl5, 0x13 );
		this->Command( REG_GammaControl6, 0x46 );
		this->Command( REG_GammaControl7, 0x23 );
		this->Command( REG_GammaControl8, 0x76 );
		this->Command( REG_GammaControl9, 0x00 );
		this->Command( REG_GammaControl10, 0x5E );
		this->Command( REG_GammaControl11, 0x4F );
		this->Command( REG_GammaControl12, 0x40 );

		//**********Power On sequence************
        
		this->Command( REG_OSCControl1, 0x90 | OSC_EN );
		this->Command( REG_CycleControl1, 0xF9 );
		delay_ms(10);
        
		this->Command( REG_PowerControl3, 0x14 );
		this->Command( REG_PowerControl2, 0x11 );
		this->Command( REG_PowerControl4, 0x06 );
		this->Command( REG_VCOMControl, 0x42 );
		delay_ms(20);
        
		this->Command( REG_PowerControl1, SDK | VL_TRI );
		this->Command( REG_PowerControl1, SDK | VL_TRI | PON );
		delay_ms(40);
        
		this->Command( REG_PowerControl1, VL_TRI | PON );
		delay_ms(40);
        
		this->Command( REG_PowerControl6, 0x27 );
		delay_ms(100);	   
        
        
		//**********DISPLAY ON SETTING***********
		this->Command( REG_DisplayControl2, 0x60 );
		this->Command( REG_SourceControl2, 0x40 );
		this->Command( REG_CycleControl10, 0x38 );
		this->Command( REG_CycleControl11, 0x38 );
		this->Command( REG_DisplayControl2, 0x38 );
		delay_ms(40);
        
		this->Command( REG_DisplayControl2, 0x3C );
		// MV=1  MX=0  MY=1  
	//	Command( REG_MemoryAccessControl, GS | BGR | SS );
		this->Command( REG_MemoryAccessControl, MX | MV | GS | BGR | SS );

		this->Command( REG_DisplayMode, 0x06 );
		this->Command( REG_PANELControl, 0x00 );

		SetWindow( 0, 0, ScreenWidth-1, ScreenHeight-1 );
	}
public:
	virtual void DisplayOn( bool on )
	{
		//Command(REG_DISP_CTRL1, on ? DISP_CTRL1_BASEE : 0 );
	}

	virtual void GraphicsRamMode(void)
	{
		this->WriteCommand(REG_DataReadWrite);
	}

	virtual void SetX(unsigned int x1)
	{
		this->Command(REG_ColumnAddressStart1, x1 >> 8);
		this->Command(REG_ColumnAddressStart2, x1 & 0xFF);
	}

	virtual void SetY(unsigned int y1)
	{
		this->Command(REG_RowAddressStart1, y1 >> 8);
		this->Command(REG_RowAddressStart2, y1 & 0xFF);
	}

	// Note - the rectangle end point is included in the window size, eg a display of width 400, will have x1=0,x2=399
	virtual void SetWindow(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2)
	{
		this->Command(REG_RowAddressStart1, 0);
		this->Command(REG_RowAddressStart2, y1);
		this->Command(REG_RowAddressEnd1, 0);
		this->Command(REG_RowAddressEnd2, y2);
		this->Command(REG_ColumnAddressStart1, x1 >> 8 );
		this->Command(REG_ColumnAddressStart2, x1 & 0xFF );
		this->Command(REG_ColumnAddressEnd1, x2 >> 8 );
		this->Command(REG_ColumnAddressEnd2, x2 & 0xFF);
	}

};