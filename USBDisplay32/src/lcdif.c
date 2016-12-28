/*
 * CProgram1.c
 *
 * Created: 16/04/2011 7:46:52 AM
 *  Author: Frank Tkalcevic
 */ 
#include <board.h>
#include <conf_board.h>
#include "asf.h"

#include "lcdif.h"
#include "r61509v.h"
#include "hx8352a.h"
#include "lcdtext.h"

#define HX8352A

#define NORMAL



 
 #ifdef USE_R61509V
static void LCDStart(void)
{
    gpio_set_gpio_pin(LCDReset); 
    delay_ms(5);
    gpio_clr_gpio_pin(LCDReset); 
    delay_ms(5);
    gpio_set_gpio_pin(LCDReset); 
    delay_ms(5);
	
    // Sync
    LCD_WriteCommand(REG_SYNC); 
    LCD_WriteCommand(REG_SYNC); 
    LCD_WriteCommand(REG_SYNC); 
    LCD_WriteCommand(REG_SYNC); 
	
    LCD_WriteCommand(REG_DEVICE_CODE);	// Read Device Code
    int n = LCD_Read_DATA();
#if 0
    //************* Power On **********//
    LCD_Write_COM(0x04,0x00); LCD_Write_DATA(0x30,0x00); 
    LCD_Write_COM(0x00,0x08); LCD_Write_DATA(0x05,0x03); 
    // ----------- Adjust the Gamma Curve ----------//
    LCD_Write_COM(0x03,0x00); LCD_Write_DATA(0x01,0x01);
    LCD_Write_COM(0x03,0x01); LCD_Write_DATA(0x0b,0x27);
    LCD_Write_COM(0x03,0x02); LCD_Write_DATA(0x13,0x2a);
    LCD_Write_COM(0x03,0x03); LCD_Write_DATA(0x2a,0x13);
    LCD_Write_COM(0x03,0x04); LCD_Write_DATA(0x2a,0x13);
    LCD_Write_COM(0x03,0x05); LCD_Write_DATA(0x27,0x0B);
    LCD_Write_COM(0x03,0x06); LCD_Write_DATA(0x01,0x01);
    LCD_Write_COM(0x03,0x07); LCD_Write_DATA(0x12,0x05);
    LCD_Write_COM(0x03,0x08); LCD_Write_DATA(0x05,0x12);
    LCD_Write_COM(0x03,0x09); LCD_Write_DATA(0x00,0x05);

    LCD_Write_COM(0x00,0x10); LCD_Write_DATA(0x00,0x12); 
    LCD_Write_COM(0x01,0x00); LCD_Write_DATA(0x06,0x30); //		
    LCD_Write_COM(0x01,0x01); LCD_Write_DATA(0x01,0x47); //		
    LCD_Write_COM(0x01,0x02); LCD_Write_DATA(0x68,0xb0); //		
    delay_ms(50);
    LCD_Write_COM(0x01,0x03); LCD_Write_DATA(0x2f,0x00); //		
	
    //************* Display On **********//
    LCD_Write_COM(0x00,0x07); LCD_Write_DATA(0x01,0x00); 
#else
    LCD_Command(REG_DISP_CTRL1, 0 );	

    LCD_Command(REG_DRIVER_OUTPUT, 0);		// set SS and SM bit
    LCD_Command(REG_LCD_DR_WAVE_CTRL,LCD_DR_WAVE_CTRL_BC);	// set 1 line inversion
	
#ifdef NORMAL	
    LCD_Command(REG_ENTRY_MODE, ENTRY_MODE_BGR | ENTRY_MODE_DFM | ENTRY_MODE_VID | ENTRY_MODE_HID | ENTRY_MODE_AM );     // Normal
#else
    LCD_Command(REG_ENTRY_MODE, ENTRY_MODE_BGR | ENTRY_MODE_DFM |  ENTRY_MODE_AM );    // 180
#endif

    LCD_Command(REG_DISP_CTRL2, DISP_CTRL2_FP(3) | DISP_CTRL2_BP(2) );
    LCD_Command(REG_DISP_CTRL3, 0);
    LCD_Command(REG_EIGHT_COLOUR_CTRL, 0 );
	
    LCD_Command(REG_EXT_DISP_CTRL1,0);
    LCD_Command(REG_EXT_DISP_CTRL2,0);

    //LCD_Command(0x000C,0x0001); // RGB 16bit interface setting
    //LCD_Command(0x000F,0x0000); // RGB interface polarity
	
    //*************Power On sequence****************//
    LCD_Command( REG_PAN_INTF_CTRL1, PWR_CTRL1_RTNI(0x12));
    LCD_Command( REG_PAN_INTF_CTRL2, PWR_CTRL2_NOWI(2) | PWR_CTRL2_SDTI(2));
    LCD_Command( REG_PAN_INTF_CTRL3, 0 );

    LCD_Command( REG_PAN_INTF_CTRL4, 0 );
    LCD_Command( REG_PAN_INTF_CTRL5, 0 );
    LCD_Command( REG_PAN_INTF_CTRL6, 0 );
    LCD_Command( REG_PAN_INTF_CTRL7, 0 );
    LCD_Command( REG_PAN_INTF_CTRL8, 0 );
    LCD_Command( REG_PAN_INTF_CTRL9, 0 );

    LCD_Command( REG_FRM_MRKR_CTRL, 0 ); // 
	
    LCD_Command(0x0100, 0x0630); //		
    LCD_Command(0x0101, 0x0147); //		
    LCD_Command(0x0102, 0x68b0); //		
    LCD_Command(0x0103, 0x2f00); //		
    //LCD_Command(0x0107,0x0000); //		
    //LCD_Command(0x0110,0x0001); //		
	
    LCD_Command(0x0280, 0x0600); //		

    // ----------- Adjust the Gamma Curve ----------//
    LCD_Command(0x0300,0x0101);
    LCD_Command(0x0301,0x0b27);
    LCD_Command(0x0302,0x132a);
    LCD_Command(0x0303,0x2a13);
    LCD_Command(0x0304,0x2a13);
    LCD_Command(0x0305,0x270B);
    LCD_Command(0x0306,0x0101);
    LCD_Command(0x0307,0x1205);
    LCD_Command(0x0308,0x0512);
    LCD_Command(0x0309,0x0005);
 
    //------------------ Set GRAM area ---------------//
    LCD_Command(REG_BIMG_NR_LINE,BIMG_NR_LINE_NL(49)|BIMG_NR_LINE_SCN(0));
    LCD_Command(REG_BIMG_DISP_CTRL, REG_BIMG_DISP_CTRL_REV );
    LCD_Command(REG_BIMG_VSCROLL_CTRL,0);
    //LCD_Command(0x0400,0x6200);
    //LCD_Command(0x0401,0x0001);
    //LCD_Command(0x0404,0x0000);
	
    // Partial Display
    LCD_Command(0x0500,0x0000);
    LCD_Command(0x0501,0x0000);
    LCD_Command(0x0502,0x0000);
	
    delay_ms(50); // Delay 50ms
#endif
}

void LCD_DisplayOn( bool on )
{
    LCD_Command(REG_DISP_CTRL1, on ? DISP_CTRL1_BASEE : 0 );
}	

void LCD_SetX(unsigned int x1)
{
#ifdef NORMAL
    LCD_Command(REG_RAM_VADDR_SET, x1);
#else
    LCD_Command(REG_RAM_VADDR_SET, SCREEN_WIDTH - x1 - 1);
#endif
}

void LCD_SetY(unsigned int y1)
{
#ifdef NORMAL
    LCD_Command(REG_RAM_HADDR_SET, y1);
#else
    LCD_Command(REG_RAM_HADDR_SET, SCREEN_HEIGHT - y1 - 1);
#endif
}

void LCD_SetXY(unsigned int x1,unsigned int y1)
{
    LCD_SetX(x1);
    LCD_SetY(y1);
}

void LCD_SetWindow(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2)
{
#ifdef NORMAL
    LCD_Command(REG_RAM_HADDR_START,y1);
    LCD_Command(REG_RAM_HADDR_END,y2);
    LCD_Command(REG_RAM_VADDR_START,x1);
    LCD_Command(REG_RAM_VADDR_END,x2);
#else
    LCD_Command(REG_RAM_HADDR_END,SCREEN_HEIGHT - y1 - 1);
    LCD_Command(REG_RAM_HADDR_START, SCREEN_HEIGHT - y2 - 1);
    LCD_Command(REG_RAM_VADDR_END, SCREEN_WIDTH - x1 - 1);
    LCD_Command(REG_RAM_VADDR_START, SCREEN_WIDTH - x2 - 1);
#endif
}

void LCD_GraphicsRamMode(void)
{
    LCD_WriteCommand(REG_RW_GRAM);
}

#endif


