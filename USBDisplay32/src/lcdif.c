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

uint16_t g_BackgroundColour;
uint16_t g_ForegroundColour;

#define NORMAL

#ifdef LCDBITINTERFACE	// BitBang interface

static void SetDataOutput(void)
{
    gpio_configure_pin(LCDD0, GPIO_DIR_OUTPUT);
    gpio_configure_pin(LCDD1, GPIO_DIR_OUTPUT);
    gpio_configure_pin(LCDD2, GPIO_DIR_OUTPUT);
    gpio_configure_pin(LCDD3, GPIO_DIR_OUTPUT);
    gpio_configure_pin(LCDD4, GPIO_DIR_OUTPUT);
    gpio_configure_pin(LCDD5, GPIO_DIR_OUTPUT);
    gpio_configure_pin(LCDD6, GPIO_DIR_OUTPUT);
    gpio_configure_pin(LCDD7, GPIO_DIR_OUTPUT);
    gpio_configure_pin(LCDD8, GPIO_DIR_OUTPUT);
    gpio_configure_pin(LCDD9, GPIO_DIR_OUTPUT);
    gpio_configure_pin(LCDD10, GPIO_DIR_OUTPUT);
    gpio_configure_pin(LCDD11, GPIO_DIR_OUTPUT);
    gpio_configure_pin(LCDD12, GPIO_DIR_OUTPUT);
    gpio_configure_pin(LCDD13, GPIO_DIR_OUTPUT);
    gpio_configure_pin(LCDD14, GPIO_DIR_OUTPUT);
    gpio_configure_pin(LCDD15, GPIO_DIR_OUTPUT);
}
	
static void SetDataInput(void)
{
    gpio_configure_pin(LCDD0, GPIO_DIR_INPUT);
    gpio_configure_pin(LCDD1, GPIO_DIR_INPUT);
    gpio_configure_pin(LCDD2, GPIO_DIR_INPUT);
    gpio_configure_pin(LCDD3, GPIO_DIR_INPUT);
    gpio_configure_pin(LCDD4, GPIO_DIR_INPUT);
    gpio_configure_pin(LCDD5, GPIO_DIR_INPUT);
    gpio_configure_pin(LCDD6, GPIO_DIR_INPUT);
    gpio_configure_pin(LCDD7, GPIO_DIR_INPUT);
    gpio_configure_pin(LCDD8, GPIO_DIR_INPUT);
    gpio_configure_pin(LCDD9, GPIO_DIR_INPUT);
    gpio_configure_pin(LCDD10, GPIO_DIR_INPUT);
    gpio_configure_pin(LCDD11, GPIO_DIR_INPUT);
    gpio_configure_pin(LCDD12, GPIO_DIR_INPUT);
    gpio_configure_pin(LCDD13, GPIO_DIR_INPUT);
    gpio_configure_pin(LCDD14, GPIO_DIR_INPUT);
    gpio_configure_pin(LCDD15, GPIO_DIR_INPUT);
}
#endif	

// Initialise IO pins for manual LCD mode
static void LCDInitIO(void)
{
#ifdef LCDBITINTERFACE	// BitBang interface

    // Claim the IO pins
    gpio_enable_gpio_pin(LCDD0);
    gpio_enable_gpio_pin(LCDD1);
    gpio_enable_gpio_pin(LCDD2);
    gpio_enable_gpio_pin(LCDD3);
    gpio_enable_gpio_pin(LCDD4);
    gpio_enable_gpio_pin(LCDD5);
    gpio_enable_gpio_pin(LCDD6);
    gpio_enable_gpio_pin(LCDD7);
    gpio_enable_gpio_pin(LCDD8);
    gpio_enable_gpio_pin(LCDD9);
    gpio_enable_gpio_pin(LCDD10);
    gpio_enable_gpio_pin(LCDD11);
    gpio_enable_gpio_pin(LCDD12);
    gpio_enable_gpio_pin(LCDD13);
    gpio_enable_gpio_pin(LCDD14);
    gpio_enable_gpio_pin(LCDD15);
    gpio_enable_gpio_pin(LCDReset);
    gpio_enable_gpio_pin(LCDRS);
    gpio_enable_gpio_pin(LCDRD);
    gpio_enable_gpio_pin(LCDWR);

    // Set reset as an output, and pull low.
	gpio_configure_pin(LCDReset, GPIO_DIR_OUTPUT);
    gpio_clr_gpio_pin(LCDReset);
	
    // Set data pins as outputs
    SetDataOutput();
	
	gpio_configure_pin(LCDRS, GPIO_DIR_OUTPUT);
	gpio_configure_pin(LCDRD, GPIO_DIR_OUTPUT);
	gpio_configure_pin(LCDWR, GPIO_DIR_OUTPUT);

    // RD/WR on falling edge.
    gpio_set_gpio_pin(LCDWR);
    gpio_set_gpio_pin(LCDRD);
	
#else	// EBI
    // Set up the pins
    gpio_enable_module_pin(EBI_D00, AVR32_EBI_DATA_0_FUNCTION);
    gpio_enable_module_pin(EBI_D01, AVR32_EBI_DATA_1_FUNCTION);
    gpio_enable_module_pin(EBI_D02, AVR32_EBI_DATA_2_FUNCTION);
    gpio_enable_module_pin(EBI_D03, AVR32_EBI_DATA_3_FUNCTION);
    gpio_enable_module_pin(EBI_D04, AVR32_EBI_DATA_4_FUNCTION);
    gpio_enable_module_pin(EBI_D05, AVR32_EBI_DATA_5_FUNCTION);
    gpio_enable_module_pin(EBI_D06, AVR32_EBI_DATA_6_FUNCTION);
    gpio_enable_module_pin(EBI_D07, AVR32_EBI_DATA_7_FUNCTION);
    gpio_enable_module_pin(EBI_D08, AVR32_EBI_DATA_8_FUNCTION);
    gpio_enable_module_pin(EBI_D09, AVR32_EBI_DATA_9_FUNCTION);
    gpio_enable_module_pin(EBI_D10, AVR32_EBI_DATA_10_FUNCTION);
    gpio_enable_module_pin(EBI_D11, AVR32_EBI_DATA_11_FUNCTION);
    gpio_enable_module_pin(EBI_D12, AVR32_EBI_DATA_12_FUNCTION);
    gpio_enable_module_pin(EBI_D13, AVR32_EBI_DATA_13_FUNCTION);
    gpio_enable_module_pin(EBI_D14, AVR32_EBI_DATA_14_FUNCTION);
    gpio_enable_module_pin(EBI_D15, AVR32_EBI_DATA_15_FUNCTION);

    gpio_enable_module_pin(EBI_ADDR21, AVR32_EBI_ADDR_21_FUNCTION);
    gpio_enable_module_pin(EBI_NRD, AVR32_EBI_NRD_0_FUNCTION);
    gpio_enable_module_pin(EBI_NWE0, AVR32_EBI_NWE0_0_FUNCTION);
	
    AVR32_SMC.cs[0].SETUP.ncs_rd_setup = 0;
    AVR32_SMC.cs[0].SETUP.nrd_setup = 0;
    AVR32_SMC.cs[0].SETUP.ncs_wr_setup = 0;
    AVR32_SMC.cs[0].SETUP.nwe_setup = 1;
	
    AVR32_SMC.cs[0].PULSE.ncs_rd_pulse = 3;
    AVR32_SMC.cs[0].PULSE.nrd_pulse = 2;
    AVR32_SMC.cs[0].PULSE.ncs_wr_pulse = 2;
    AVR32_SMC.cs[0].PULSE.nwe_pulse = 1;
	
    AVR32_SMC.cs[0].CYCLE.nrd_cycle = 2;
    AVR32_SMC.cs[0].CYCLE.nwe_cycle = 4;
	
    AVR32_SMC.cs[0].MODE.ps = 0;	// Page Size - not using
    AVR32_SMC.cs[0].MODE.pmen = 0;	// Page Mode Enabel = false
    AVR32_SMC.cs[0].MODE.tdf_mode = 0;	// TDF optimisation - false
    AVR32_SMC.cs[0].MODE.tdf_cycles = 0;
    AVR32_SMC.cs[0].MODE.dbw = 1;		// Data bus width
    AVR32_SMC.cs[0].MODE.bat = 0;		// Byte access type - don't care 16bit only.
    AVR32_SMC.cs[0].MODE.exnw_mode = 0;	// don't use NWAIT
    AVR32_SMC.cs[0].MODE.write_mode = 1;// Write mode controlled by NWE
    AVR32_SMC.cs[0].MODE.read_mode = 1;	// Read mode controlled by NRD
	
	
#endif	
}

#ifdef LCDBITINTERFACE	// BitBang interface
static void SetPinValue( int nPin, int nValue )
{
    if ( nValue )
		gpio_set_gpio_pin(nPin);
    else
		gpio_clr_gpio_pin(nPin);
}

static void LCD_Writ_Bus( uint16_t V)   //Write Data
{
    SetPinValue( LCDD0, V & 0x0001 );
    SetPinValue( LCDD1, V & 0x0002 );
    SetPinValue( LCDD2, V & 0x0004 );
    SetPinValue( LCDD3, V & 0x0008 );
    SetPinValue( LCDD4, V & 0x0010 );
    SetPinValue( LCDD5, V & 0x0020 );
    SetPinValue( LCDD6, V & 0x0040 );
    SetPinValue( LCDD7, V & 0x0080 );
    SetPinValue( LCDD8, V & 0x0100 );
    SetPinValue( LCDD9, V & 0x0200 );
    SetPinValue( LCDD10, V & 0x0400 );
    SetPinValue( LCDD11, V & 0x0800 );
    SetPinValue( LCDD12, V & 0x1000 );
    SetPinValue( LCDD13, V & 0x2000 );
    SetPinValue( LCDD14, V & 0x4000 );
    SetPinValue( LCDD15, V & 0x8000 );
	
    gpio_clr_gpio_pin(LCDWR);
    gpio_set_gpio_pin(LCDWR);
}


static int LCD_Read_Bus(void)
{
    SetDataInput();
    gpio_clr_gpio_pin(LCDRD);
	
    int n  = (gpio_pin_is_high(LCDD0) << 0) |
             (gpio_pin_is_high(LCDD1) << 1) |
             (gpio_pin_is_high(LCDD2) << 2) |
             (gpio_pin_is_high(LCDD3) << 3) |
             (gpio_pin_is_high(LCDD4) << 4) |
             (gpio_pin_is_high(LCDD5) << 5) |
             (gpio_pin_is_high(LCDD6) << 6) |
             (gpio_pin_is_high(LCDD7) << 7) |
             (gpio_pin_is_high(LCDD8) << 8) |
             (gpio_pin_is_high(LCDD9) << 9) |
             (gpio_pin_is_high(LCDD10) << 10) |
             (gpio_pin_is_high(LCDD11) << 11) |
             (gpio_pin_is_high(LCDD12) << 12) |
             (gpio_pin_is_high(LCDD13) << 13) |
             (gpio_pin_is_high(LCDD14) << 14) |
             (gpio_pin_is_high(LCDD15) << 15);
    gpio_set_gpio_pin(LCDRD);
    SetDataOutput();
	
    return n;
}
#endif

#ifdef LCDBITINTERFACE	// BitBang interface
static void LCD_WriteCommand(unsigned short cmd)
{
    gpio_clr_gpio_pin(LCDRS);
    LCD_Writ_Bus(cmd);
}
#endif

#ifdef LCDBITINTERFACE	// BitBang interface
void LCD_WriteData(uint16_t data)//sent data
{
    gpio_set_gpio_pin(LCDRS);
    LCD_Writ_Bus(data);
}
#endif
 

static int LCD_Read_DATA(void)
{
#ifdef LCDBITINTERFACE	// BitBang interface
    gpio_set_gpio_pin(LCDRS);
    return LCD_Read_Bus();
#else	// EBI
    return *LCD_DATA;
#endif
}

static void LCD_Command(uint16_t reg, uint16_t data )
{
    LCD_WriteCommand(reg);
    LCD_WriteData(data);
}

 
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






#ifdef  HX8352A
static void LCDStart(void)
{
	gpio_set_gpio_pin(LCDReset);
	delay_ms(5);
	gpio_clr_gpio_pin(LCDReset);
	delay_ms(5);
	gpio_set_gpio_pin(LCDReset);
	delay_ms(5);
	
	LCD_Command( REG_TestMode, TEST_Mode );
	LCD_Command( REG_VDDDcontrol, 3 );						//VDC_SEL=011
	LCD_Command( REG_VGSRESControl1, RES_VGS1 );
	LCD_Command( REG_VGSRESControl2, RES_VGS0 | 0x13 );		//STBA[7]=1,STBA[5:4]=01,STBA[1:0]=11
	LCD_Command( REG_PWMControl0, SYNC );					//DCDC_SYNC=1
	LCD_Command( REG_TestMode, 0 );

	//Gamma Setting
	LCD_Command( REG_GammaControl1, 0xB0 );
	LCD_Command( REG_GammaControl2, 0x03 );
	LCD_Command( REG_GammaControl3, 0x10 );
	LCD_Command( REG_GammaControl4, 0x56 );
	LCD_Command( REG_GammaControl5, 0x13 );
	LCD_Command( REG_GammaControl6, 0x46 );
	LCD_Command( REG_GammaControl7, 0x23 );
	LCD_Command( REG_GammaControl8, 0x76 );
	LCD_Command( REG_GammaControl9, 0x00 );
	LCD_Command( REG_GammaControl10, 0x5E );
	LCD_Command( REG_GammaControl11, 0x4F );
	LCD_Command( REG_GammaControl12, 0x40 );

	//**********Power On sequence************
        
	LCD_Command( REG_OSCControl1, 0x90 | OSC_EN );
	LCD_Command( REG_CycleControl1, 0xF9 );
	delay_ms(10);
        
	LCD_Command( REG_PowerControl3, 0x14 );
	LCD_Command( REG_PowerControl2, 0x11 );
	LCD_Command( REG_PowerControl4, 0x06 );
	LCD_Command( REG_VCOMControl, 0x42 );
	delay_ms(20);
        
	LCD_Command( REG_PowerControl1, SDK | VL_TRI );
	LCD_Command( REG_PowerControl1, SDK | VL_TRI | PON );
	delay_ms(40);
        
	LCD_Command( REG_PowerControl1, VL_TRI | PON );
	delay_ms(40);
        
	LCD_Command( REG_PowerControl6, 0x27 );
	delay_ms(100);	   
        
        
	//**********DISPLAY ON SETTING***********
	LCD_Command( REG_DisplayControl2, 0x60 );
	LCD_Command( REG_SourceControl2, 0x40 );
	LCD_Command( REG_CycleControl10, 0x38 );
	LCD_Command( REG_CycleControl11, 0x38 );
	LCD_Command( REG_DisplayControl2, 0x38 );
	delay_ms(40);
        
	LCD_Command( REG_DisplayControl2, 0x3C );
	// MV=1  MX=0  MY=1  
//	LCD_Command( REG_MemoryAccessControl, GS | BGR | SS );
	LCD_Command( REG_MemoryAccessControl, MX | MV | GS | BGR | SS );

	LCD_Command( REG_DisplayMode, 0x06 );
	LCD_Command( REG_PANELControl, 0x00 );

	LCD_Command( REG_ColumnAddressStart1, 0x00 );
	LCD_Command( REG_ColumnAddressStart2, 0x00 );
	LCD_Command( REG_ColumnAddressEnd1, 0x00 );
	LCD_Command( REG_ColumnAddressEnd1, 0xef );

	LCD_Command( REG_RowAddressStart1, 0x00 );
	LCD_Command( REG_RowAddressStart2, 0x00 );
	LCD_Command( REG_RowAddressEnd1, 0x01 );
	LCD_Command( REG_RowAddressEnd2, 0x8F );
        

	LCD_WriteCommand(REG_DataReadWrite);
}

void LCD_DisplayOn( bool on )
{
	//LCD_Command(REG_DISP_CTRL1, on ? DISP_CTRL1_BASEE : 0 );
}

void LCD_GraphicsRamMode(void)
{
	LCD_WriteCommand(REG_DataReadWrite);
}

void LCD_SetX(unsigned int x1)
{
	#ifdef NORMAL
		LCD_Command(REG_ColumnAddressStart1, x1 >> 8);
		LCD_Command(REG_ColumnAddressStart2, x1 & 0xFF);
	#else
		LCD_Command(REG_ColumnAddressStart1, SCREEN_WIDTH - x1 - 1);
	#endif
}

void LCD_SetY(unsigned int y1)
{
	#ifdef NORMAL
		LCD_Command(REG_RowAddressStart1, y1 >> 8);
		LCD_Command(REG_RowAddressStart2, y1 & 0xFF);
	#else
		LCD_Command(REG_RowAddressStart1, SCREEN_HEIGHT - y1 - 1);
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
		LCD_Command(REG_RowAddressStart1, 0);
		LCD_Command(REG_RowAddressStart2, y1);
		LCD_Command(REG_RowAddressEnd1, 0);
		LCD_Command(REG_RowAddressEnd2, y2-1);
		LCD_Command(REG_ColumnAddressStart1, x1 >> 8 );
		LCD_Command(REG_ColumnAddressStart2, x1 & 0xFF );
		LCD_Command(REG_ColumnAddressEnd1, (x2 - 1) >> 8 );
		LCD_Command(REG_ColumnAddressEnd2, (x2 - 1) & 0xFF);
	#endif
}

#endif



#define REPEAT_COMMAND1(d)			d
#define REPEAT_COMMAND2(d)			REPEAT_COMMAND1(d); REPEAT_COMMAND1(d)
#define REPEAT_COMMAND4(d)			REPEAT_COMMAND2(d); REPEAT_COMMAND2(d)
#define REPEAT_COMMAND8(d)			REPEAT_COMMAND4(d); REPEAT_COMMAND4(d)
#define REPEAT_COMMAND16(d)			REPEAT_COMMAND8(d); REPEAT_COMMAND8(d)
#define REPEAT_COMMAND32(d)			REPEAT_COMMAND16(d); REPEAT_COMMAND16(d)
#define REPEAT_COMMAND64(d)			REPEAT_COMMAND32(d); REPEAT_COMMAND32(d)
#define REPEAT_COMMAND128(d)			REPEAT_COMMAND64(d); REPEAT_COMMAND64(d)

void LCD_ClearScreen(uint16_t colour)
{
    LCD_SetWindow(0,0,SCREEN_WIDTH-1,SCREEN_HEIGHT-1);
	LCD_SetXY(0,0);
    LCD_GraphicsRamMode();
    for ( uint16_t j = 0; j < SCREEN_HEIGHT * SCREEN_WIDTH / 128; j++ )	// Here we are assuming the display width*height is a multiple of 128
    {
	    REPEAT_COMMAND128(LCD_WriteData(colour));		// Unroll 128 iterations
    }		
}


void LCD_ScrollScreen(uint16_t nPixels)
{
    LCD_SetWindow(0,0,SCREEN_WIDTH-1,SCREEN_HEIGHT-1);
    // Scrolling the whole screen nPixels
    for ( uint16_t r = nPixels; r < SCREEN_HEIGHT; r++)
    {
	    uint16_t source_row = r;
	    uint16_t dest_row = r - nPixels;
	    uint16_t row[SCREEN_WIDTH];
	    LCD_SetXY(0,source_row);
	    for ( uint16_t c = 0; c < SCREEN_WIDTH; c++ )
	    {
	        LCD_SetX(c);
	        LCD_GraphicsRamMode();
	        uint16_t temp = LCD_Read_DATA();
	        row[c] = LCD_Read_DATA();
	    }
		
	    LCD_SetXY(0,dest_row);
	    LCD_GraphicsRamMode();
	    for ( uint16_t c = 0; c < SCREEN_WIDTH; c++ )
	    {
	        LCD_WriteData( row[c] );
	    }
    }
}

void LCD_BltStart( int x, uint8_t y, int nWidth, uint8_t nHeight )
{
    LCD_SetWindow(x, y, x+nWidth-1, y+nHeight-1);
    LCD_SetXY( x, y );
    LCD_GraphicsRamMode();
}

void LCD_SolidRect( int x, uint8_t y, int nWidth, uint8_t nHeight )
{
    LCD_SetWindow(x, y, x+nWidth, y+nHeight);
    LCD_SetXY( x, y );
    LCD_GraphicsRamMode();
	
    unsigned int nBytesToWrite = (int)nHeight * (int)nWidth;
    unsigned int nBlocksToWrite = nBytesToWrite / 128;
    unsigned int nRemainderBytes = nBytesToWrite % 128;
	
    for ( int i = 0; i < nBlocksToWrite; i++ )
    {
	    REPEAT_COMMAND128(LCD_WriteData(g_ForegroundColour));		// Unroll 128 iterations
    }

    for ( int i = 0; i < nRemainderBytes; i++ )
    {
	    LCD_WriteData(g_ForegroundColour);
    }
}

void LCD_Rect( int x, uint8_t y, int nWidth, uint8_t nHeight )
{
    if ( nHeight <= 1 )	// Horizontal Line
    {
	    LCD_SolidRect( x, y, nWidth, 1 );
    }
    else if ( nWidth <= 1 )  // Vertical Line
    {
	    LCD_SolidRect( x, y, 1, nHeight );
    }
    else
    {		
	    LCD_SolidRect( x, y, nWidth, 1 );
	    LCD_SolidRect( x, y, 1, nHeight );
	    LCD_SolidRect( x+nWidth-1, y, 1, nHeight );
	    LCD_SolidRect( x, y+nHeight-1, nWidth, 1 );
    }		
}


void LCD_SetBacklight( uint16_t intensity )	// intensity 0 - 100
{
    tc_write_ra(&AVR32_TC0, 0, intensity * 4);		// pwm value 0 to 400
}

void LCD_SetBackgroundColour( uint16_t colour )
{
    g_BackgroundColour = colour;
}

void LCD_SetForegroundColour( uint16_t colour )
{
    g_ForegroundColour = colour;
}

void LCD_DrawPixel( uint16_t x, uint8_t y )
{
    LCD_SetWindow( x, y, x, y );
    LCD_SetXY( x, y );
    LCD_GraphicsRamMode();
	LCD_WriteData(g_ForegroundColour);
}

static void DisplayTest(void)
{

{
	uint8_t y=10;
	for ( uint16_t w = 1; w <= 10; w++ )
	{
		LCD_SetWindow( 10,y,10+w,y+w );
		LCD_SetXY( 10,y );
		LCD_GraphicsRamMode();
		for (uint32_t i = 0; i < 100; i++)
		{
			LCD_WriteData(RGB(0xFF,0,0xFF));
			delay_ms(1);
		}
		y+=w + 1;
	}
}
	//LCD_Command( REG_ColumnAddressStart1, 0 );
	//LCD_Command( REG_ColumnAddressStart2, 0 );
	//LCD_Command( REG_RowAddressStart1, 0 );
	//LCD_Command( REG_RowAddressStart2, 0 );
	LCD_GraphicsRamMode();
    //for (uint32_t i = 0; i < 300040l; i++)		
    //{
	    //LCD_WriteData(RGB(0xFF,0xFF,0xFF));
	    //delay_ms(1);
    //}
	//for(;;){}
    for (int i = 0; i < 240; i++)		
    {
	    LCD_SetXY(i,i);
	    LCD_GraphicsRamMode();
	    LCD_WriteData(RGB(0xFF,0xFF,0xFF));
	    delay_ms(5);
    }
    delay_ms(500);
    for ( uint16_t i = 0; i < (1<<5); i++ )
    	LCD_ClearScreen(i & 1 ? RGB(127,0,0) : RGB(0,127,127) );

    int x = 0;
    int y = 0;
    int width = 400;
    int height = 240;
    LCD_SetWindow(x,y,x+width-1,y+height-1);
    LCD_SetXY(x,y);
    LCD_GraphicsRamMode();

    for ( int j = 0; j < width; j++ )
	    LCD_WriteData(RGB(0xFF,0xFF,0xFF));
	
    for ( int i = 0; i < height; i++ )
    	for ( int j = 0; j < width; j++ )
	    {
		    if ( i == j )
		        LCD_WriteData(RGB(0,0,0));
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
		        LCD_WriteData(c);
		    }	
		//delay_ms(1);				
	    }
			
    delay_ms(500);
    for ( uint16_t i = 0; i < (1<<5); i++ )
	    LCD_ClearScreen(i);

	LCD_SetForegroundColour(RGB(0xFF,0,0));
	LCD_SolidRect( 0, 0, 133, 239 );
	LCD_SetForegroundColour(RGB(0,0xff,0));
	LCD_SolidRect( 133, 0, 133, 239 );
	LCD_SetForegroundColour(RGB(0,0,0xff));
	LCD_SolidRect( 266, 0, 133, 239 );

	LCD_SetForegroundColour(RGB(0,0xff,0));		
	LCD_SetBackgroundColour(RGB(0,0,0));		
	LCDText_SetPixelCursor( 0, 20 );
    LCDText_WriteString(17,"IIIIIIIIIIIIIIIII");
    LCDText_WriteString(17,"The rain in spain");
	delay_ms(1000);
}


void LCD_Init(void)
{
    g_BackgroundColour=0;
    g_ForegroundColour=0xffff;

    LCDInitIO();

    //gpio_clr_gpio_pin(LCDRS);
    //gpio_clr_gpio_pin(LCDRD);
    //gpio_clr_gpio_pin(LCDWR);
    //gpio_set_gpio_pin(LCDRS);
    //gpio_set_gpio_pin(LCDRD);
    //gpio_set_gpio_pin(LCDWR);

	//for (;;)
	//{
		//uint16_t n = 1;
		//for ( int i = 0; i < 8; i++ )
		//{
			//LCD_WriteData( n );
			////LCD_Writ_Bus( n );
			//n <<= 1;
			//delay_ms(1000);
		//}
//
		//n = 0xFFFF;
		//for ( int i = 0; i < 8; i++ )
		//{
			//LCD_WriteData( n );
			////LCD_Writ_Bus( n );
			//n <<= 1;
			//n &= 0xFFFE;
			//delay_ms(1000);
		//}
	//}

    LCDStart();	
    LCD_DisplayOn(true);
    LCD_ClearScreen(0);
	
    LCDText_Init();
	
    DisplayTest();
}

