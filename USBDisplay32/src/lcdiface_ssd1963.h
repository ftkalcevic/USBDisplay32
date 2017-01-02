#pragma once

// LCD interface for HX8352A.
//
// Simple class to interface to the LCD hardware

#include "lcdiface.h"
#include "ssd1963.h"

// Currently only tested with a 240x400 running in landscape mode (upside down).

template<int ScreenWidth, int ScreenHeight>
class LCDIFaceSSD1963: public LCDHwIface
{
protected:
	void Start(void)
	{
		// Start up sequence based on UTFT SSD1963 start up code - http://www.rinkydinkelectronics.com/library.php?id=51
		gpio_set_gpio_pin(LCDReset);
		delay_ms(5);
		gpio_clr_gpio_pin(LCDReset);
		delay_ms(5);
		gpio_set_gpio_pin(LCDReset);
		delay_ms(5);
		
		WriteCommand( REG_SET_PLL_MN );
		WriteData( 0x23 );	    //N=0x36 for 6.5M, 0x23 for 10M crystal
		WriteData(0x02);
		WriteData(0x54);

		WriteCommand( REG_SET_PLL );
		WriteData(0x03);
		delay_ms(10);


		WriteCommand( REG_SOFT_RESET );
		delay_ms(100);

		WriteCommand(REG_SET_LSHIFT_FREQ);		//PLL setting for PCLK, depends on resolution
		WriteData(0x01);
		WriteData(0x1F);
		WriteData(0xFF);

		WriteCommand(REG_SET_LCD_MODE);		//LCD SPECIFICATION
		WriteData(0x20);
		WriteData(0x00);
		WriteData(0x01);		//Set HDP	479
		WriteData(0xDF);
		WriteData(0x01);		//Set VDP	271
		WriteData(0x0F);
		WriteData(0x00);

		WriteCommand(REG_SET_HORI_PERIOD);		//HSYNC
		WriteData(0x02);		//Set HT	531
		WriteData(0x13);
		WriteData(0x00);		//Set HPS	8
		WriteData(0x08);
		WriteData(0x2B);		//Set HPW	43
		WriteData(0x00);		//Set LPS	2
		WriteData(0x02);
		WriteData(0x00);

		WriteCommand(REG_SET_VERT_PERIOD);		//VSYNC
		WriteData(0x01);		//Set VT	288
		WriteData(0x20);
		WriteData(0x00);		//Set VPS	4
		WriteData(0x04);
		WriteData(0x0c);		//Set VPW	12
		WriteData(0x00);		//Set FPS	2
		WriteData(0x02);

		WriteCommand(REG_SET_GPIO_VALUE);
		WriteData(0x0F);		//GPIO[3:0] out 1

		WriteCommand(REG_SET_GPIO_CONF);
		WriteData(0x07);	    //GPIO3=input, GPIO[2:0]=output
		WriteData(0x01);		//GPIO0 normal

		WriteCommand(REG_SET_ADDRESS_MODE);		//rotation
		WriteData(0);

		WriteCommand(REG_SET_PIXEL_DATA_INTERFACE);		//pixel data interface
		WriteData(0x03);


		delay_ms(1);

		SetWindow( 0, 0, ScreenWidth-1, ScreenHeight-1 );
	//	SetXY(0, 0, 479, 271);
		DisplayOn(true);

		_SetBacklight( 30 );

		WriteCommand(REG_SET_DBC_CONF);
		WriteData(0x0d);

		GraphicsRamMode();
	}
public:
	void DisplayOn( bool on )
	{
		if ( on )
		{
			WriteCommand(REG_SET_DISPLAY_ON);		//display on
		}
		else
		{
			WriteCommand(REG_SET_DISPLAY_OFF);		//display off
		}
	}

	void Sleep()
	{
		//WriteCommand(REG_ENTER_SLEEP_MODE);
		//delay_ms(5);
		//WriteCommand(REG_SET_DEEP_SLEEP);
		//delay_ms(5);
	}

	void Wake()
	{
		//WriteCommand(REG_EXIT_SLEEP_MODE);
		//delay_ms(120);
	}

	void GraphicsRamMode(void)
	{
		WriteCommand(REG_WRITE_MEMORY_START);
	}

	void SetX(uint16_t x1)
	{
		//Command(REG_ColumnAddressStart1, x1 >> 8);
		//Command(REG_ColumnAddressStart2, x1 & 0xFF);
	}

	void SetY(uint16_t y1)
	{
		//Command(REG_RowAddressStart1, y1 >> 8);
		//Command(REG_RowAddressStart2, y1 & 0xFF);
	}

	void SetXY(uint16_t x1, uint16_t y1)
	{
		//SetX(x1);
		//SetY(y1);
		SetWindow(x1,y1,x1,y1);
	}

	// Note - the rectangle end point is included in the window size, eg a display of width 400, will have x1=0,x2=399
	void SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
	{
		WriteCommand( REG_SET_PAGE_ADDRESS );
		WriteData(y1 >> 8);
		WriteData(y1);
		WriteData(y2 >> 8);
		WriteData(y2);
		WriteCommand( REG_SET_COLUMN_ADDRESS );
		WriteData(x1 >> 8);
		WriteData(x1);
		WriteData(x2 >> 8);
		WriteData(x2);
	}

	bool ScrollScreen(uint16_t nPixels) 
	{ 
		return false;
		// ? Does this actually scroll?  Or just change the display memory start address?
		WriteCommand(REG_SET_SCROLL_AREA);
		WriteData(0);	// TFA MSB
		WriteData(0);	// TFA LSB
		WriteData(ScreenHeight>>8);	// VSA MSB
		WriteData(ScreenHeight);	// VSA LSB
		WriteData(0);	// BFA MSB
		WriteData(0);	// BFA LSB
		WriteCommand(REG_SET_SCROLL_START);
		WriteData(nPixels>>8);
		WriteData(nPixels);
		
		//WriteCommand(REG_MemoryAccessControl);
		//uint8_t n = ReadCommand();
		//n = ReadCommand();
		//Command( REG_MemoryAccessControl, n | SRL_EN );
		//Command( REG_MemoryAccessControl, MX | MV | GS | BGR | SS | SRL_EN );

		return true; 
	}

	bool _SetBacklight(uint8_t intensity) // A bit of hack to avoid using virtual functions (slight performance gain).
	{ 
		// the itead 4.3 doesn't appear to have back light control.
		return true;

		WriteCommand(REG_SET_PWM_CONF);		//set PWM for B/L
		WriteData(0x06);	// Freq
		WriteData(255*intensity/100);	// Duty cycle intensity is 0-100
		WriteData(0x01);	// PWM Enable
		WriteData(0xFF);	// DBC...
		WriteData(0x00);
		WriteData(0xf);

		return true; 
	}	

};