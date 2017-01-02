/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * AVR Software Framework (ASF).
 */
#include <asf.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

#include "common.h"
#include "lcdtext.h"
#include "displaycommands.h"
#include "serial.h"
#include "touch.h"
#include "lcd_conf.h"

#include "lcdiface_hx8352a.h"
#include "lcdiface_ssd1963.h"
#include "lcdtext.h"

#define USE_USB_CDC


#include "../fonts/Font6x13.h"
//#include "../fonts/FontTypewriter.h"

static struct SFontData  fontData[] =
{
	/*45x17*/    { FONT6X13_WIDTH, FONT6X13_HEIGHT, SCREEN_WIDTH / FONT6X13_WIDTH, SCREEN_HEIGHT / FONT6X13_HEIGHT, Font6x13 },
//	/*22x9 */    { FONTTYPEWRITER_WIDTH, FONTTYPEWRITER_HEIGHT, SCREEN_WIDTH / FONTTYPEWRITER_WIDTH, SCREEN_HEIGHT / FONTTYPEWRITER_HEIGHT, FontTypewriter },
};


TLCD lcd;
TLCDText lcdtext(lcd, fontData, countof(fontData) );


extern "C" void LCD_BltStart( uint16_t x, uint16_t y, uint16_t nWidth, uint16_t nHeight )
{
	lcd.BltStart(x,y,nWidth,nHeight);
}
extern "C" void LCD_SetBacklight( uint8_t intenstity )
{
	lcd.SetBacklight( intenstity );
}


static void serial_write_string( volatile avr32_usart_t *usart, const char *s )
{
    while (*s)
    {
        usart_serial_putchar( usart, *s );
        s++;
    }
}

static inline unsigned short SwapBytes( unsigned short n )
{
    unsigned short temp = n;
    uint8_t buf[2];
    buf[0] = *(((uint8_t *)(&temp))+1);
    buf[1] = *(((uint8_t *)(&temp))+0);
    return *(unsigned short *)(void *)buf;
}

extern int _write(int file, char *ptr, int len);
int _write(int file, char *ptr, int len)
{
	lcdtext.WriteString( len, ptr );
    return len;
}


static void DisplayTest(void)
{
	{
		for ( uint16_t y = 0; y <= SCREEN_WIDTH-20; y++ )
			for ( uint16_t x = 0; x <= SCREEN_WIDTH-20; x++ )
			{
				lcd.BltStart( x,y,20,20 );
				for (uint32_t i = 0; i < 20*20; i++)
				{
					lcd.WriteData(RGB( i & 1 ? 0xFF : 0, i & 1 ? 0 : 0xff,0));
				}
				//delay_ms(100);
			}
	}
	{
		uint8_t y=10;
		for ( uint16_t w = 1; w <= 10; w++ )
		{
			lcd.SetWindow( 10,y,10+w,y+w );
			//lcd.SetXY( 10,y );
			lcd.GraphicsRamMode();
			for (uint32_t i = 0; i < 100; i++)
			{
				lcd.WriteData(RGB(0xFF,0,0xFF));
				delay_ms(1);
			}
			y+=w + 1;
		}
	}
	{
		for ( uint32_t w = 271; w >= 1; w-- )
		{
			lcd.SetWindow( 0,0,w,w );
			lcd.GraphicsRamMode();
			for (uint32_t i = 0; i < w*w; i++)
			{
				lcd.WriteData(w & 1 ? RGB(0xFF,0,0) : RGB(0,0xFF,0) );
			}
			//delay_ms(500);
		}
	}
	//Command( REG_ColumnAddressStart1, 0 );
	//Command( REG_ColumnAddressStart2, 0 );
	//Command( REG_RowAddressStart1, 0 );
	//Command( REG_RowAddressStart2, 0 );
	lcd.GraphicsRamMode();
	//for (uint32_t i = 0; i < 300040l; i++)
	//{
	//WriteData(RGB(0xFF,0xFF,0xFF));
	//delay_ms(1);
	//}
	//for(;;){}
	for (int i = 0; i < 240; i++)
	{
		lcd.SetXY(i,i);
		lcd.GraphicsRamMode();
		lcd.WriteData(RGB(0xFF,0xFF,0xFF));
		delay_ms(5);
	}
	delay_ms(500);
	for ( uint16_t i = 0; i < (1<<5); i++ )
	lcd.ClearScreen(i & 1 ? RGB(127,0,0) : RGB(0,127,127) );

	int x = 0;
	int y = 0;
	int width = 400;
	int height = 240;
	lcd.SetWindow(x,y,x+width-1,y+height-1);
	lcd.SetXY(x,y);
	lcd.GraphicsRamMode();

	for ( int j = 0; j < width; j++ )
	lcd.WriteData(RGB(0xFF,0xFF,0xFF));
		
	for ( int i = 0; i < height; i++ )
	for ( int j = 0; j < width; j++ )
	{
		if ( i == j )
		lcd.WriteData(RGB(0,0,0));
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
			lcd.WriteData(c);
		}
		//delay_ms(1);
	}
		
	delay_ms(500);
	for ( uint16_t i = 0; i < (1<<5); i++ )
	lcd.ClearScreen(i);

	lcd.SetForegroundColour(RGB(0xFF,0,0));
	lcd.SolidRect( 0, 0, 133, 239 );
	lcd.SetForegroundColour(RGB(0,0xff,0));
	lcd.SolidRect( 133, 0, 133, 239 );
	lcd.SetForegroundColour(RGB(0,0,0xff));
	lcd.SolidRect( 266, 0, 133, 239 );

	lcd.SetForegroundColour(RGB(0,0xff,0));
	lcd.SetBackgroundColour(RGB(0,0,0));

	delay_ms(1000);
}



static void speedtest( void )
{
    Set_sys_count(0);
	int i;
    for ( i = 0; i < 1000; i++ )
    {
	    lcd.ClearScreen(i);
    }
    int n = Get_sys_count();
    double t1 = ((double)n)/CLOCK;
	
	lcd.SetBackgroundColour(RGB(0,0,0));
	lcd.SetForegroundColour(RGB(0,0xff,0));
    const char *sTest = "The rain in spain falls mainly on the plain.";
    Set_sys_count(0);
    for ( int i = 0; i < 1000; i++ )
    {
		lcd.SetBackgroundColour( (i&3) == 1 ? RGB(0xFF,0xFF,0xFF) : (i&3) == 2 ? RGB(0xFF,0,0) : (i&3) == 3 ? RGB(0,0xFF,0) : RGB(0,0,0xFF) );
		lcd.SetForegroundColour( (i&3) == 0 ? RGB(0xFF,0xFF,0xFF) : (i&3) == 1 ? RGB(0xFF,0,0) : (i&3) == 2 ? RGB(0,0xFF,0) : RGB(0,0,0xFF) );
	    lcdtext.WriteString( sTest );
    }
    n = Get_sys_count();
    double t2 = ((double)n)/CLOCK;
	
	lcdtext.SetAutoScroll(true);
    Set_sys_count(0);
    for ( int i = 0; i < 100; i++ )
    {
	    lcdtext.WriteString( sTest );
		lcd.SetForegroundColour( (i&3) == 0 ? RGB(0xFF,0xFF,0xFF) : (i&3) == 1 ? RGB(0xFF,0,0) : (i&3) == 2 ? RGB(0,0xFF,0) : RGB(0,0,0xFF) );
    }
    n = Get_sys_count();
    double t3 = ((double)n)/CLOCK;

    char s[100];
    lcd.ClearScreen(0);
    snprintf(s,sizeof(s),"Clear=%.3f, Text=%.3f, Scroll=%.3f", t1, t2, t3 );
	s[sizeof(s)-1] = 0;
    lcdtext.SetPixelCursor( SCREEN_WIDTH/2 - 7*strlen(s)/2, SCREEN_HEIGHT/2 );
    lcdtext.WriteString( s );
	delay_ms(10000);
}


//#define MAIN_BUFFER
#ifdef MAIN_BUFFER
    #define RX_BUFFER_SIZE  (4*2048)
    #define RX_BUFFER_MASK ( RX_BUFFER_SIZE - 1 )
    #if ( RX_BUFFER_SIZE & RX_BUFFER_MASK )
        #error RX buffer size is not a power of 2
    #endif

    typedef uint16_t circ_buffer_ptr_t;
    static uint8_t RxBuf[RX_BUFFER_SIZE];
    static circ_buffer_ptr_t RxHead=0;
    static circ_buffer_ptr_t RxTail=0;
#endif

static bool IsDataAvailable(void)
{
    return false;
}    

static int ReadByte( void )
{
    return -1;
}

#define RECEIVE_BUFFER_LEN		MAX_CMD_LEN
static union
{
    uint8_t ReceiveBuffer[RECEIVE_BUFFER_LEN];
	uint16_t colour;
	CmdAll cmd;
} buf;
	
static int ReceiveBufferBytes = 0;
static int WaitBytes;
static enum
{
    WAIT_FOR_COMMAND, 
    WAIT_FOR_BYTES, 
    WAIT_FOR_TEXT,
	WAIT_FOR_BLT,
	WAIT_FOR_BLT1,
	WAIT_FOR_BLT_RLE
} state = WAIT_FOR_COMMAND;

static void ProcessDisplayData( int b )
{
	char s[50];

    switch (state)
    {
	    case WAIT_FOR_COMMAND:
	        if (b >= CMD_MIN && b <= CMD_MAX)
	        {
		        ReceiveBufferBytes = 0;
		        buf.ReceiveBuffer[ReceiveBufferBytes++] = b;
		        state = WAIT_FOR_BYTES;
		        switch (b)
		        {
		            case CMD_SET_BACKGROUND_COLOUR: // Set Background Colour 0x00, r, g, b
			            WaitBytes = sizeof(CmdSetBackgroundColour) - 1;
			            break;
		            case CMD_SET_FOREGROUND_COLOUR: // Set Foreground Colour 0x01, r, g, b
			            WaitBytes = sizeof(CmdSetForegroundColour) - 1;
			            break;
		            case CMD_CLEAR_SCREEN: // Clear Screen 0x02,
	                    sprintf(s, "cls\r\n" );
						serial_write_string( &DEVICE_USART, s );
			            lcd.ClearScreen(lcd.GetBackgroundColour());
			            state = WAIT_FOR_COMMAND;
			            break;
		            case CMD_WRITE_TEXT: // Write Text - 0x03, data..., NULL,
			            state = WAIT_FOR_TEXT;
			            break;
		            case CMD_SET_TEXT_POSITION: // Set position 0x04, x, y
			            WaitBytes = sizeof(CmdSetTextPosition) - 1;
			            break;
		            case CMD_SET_GRAPHICS_POSITION: // Set position 0x0?, xmsb, xlsb, y
			            WaitBytes = sizeof(CmdSetGraphicsPosition) - 1;
			            break;
		            case CMD_SET_FONT: // Set font? 0x05, 0/1
			            WaitBytes = sizeof(CmdSetFont) - 1;
			            break;
		            case CMD_SET_BACKLIGHT: // set backlight 0x06, 0-100
			            WaitBytes = sizeof(CmdSetBacklight) - 1;
			            break;
		            case CMD_SET_TEXT_ATTRIBUTE: // set attribute 0x07, [underline][Reverse?bg/fg?][italic][bold][strikethrough]
			            WaitBytes = sizeof(CmdSetAttribute) - 1;
			            break;
		            case CMD_DRAW_PIXEL:
			            WaitBytes = sizeof(CmdDrawPixel) - 1;
			            break;
		            case CMD_DRAW_RECTANGLE:
			            WaitBytes = sizeof(CmdDrawRectangle) - 1;
			            break;
				    case CMD_BITBLT:
			            WaitBytes = sizeof(CmdBitBlt) - 1;
				        break;
				    case CMD_BITBLT_RLE:
			            WaitBytes = sizeof(CmdBitBltRLE) - 1;
				        break;
		            default:
			            state = WAIT_FOR_COMMAND;
			            break;
		        }
	        }
	        break;

	    case WAIT_FOR_BYTES:
	        buf.ReceiveBuffer[ReceiveBufferBytes++] = b;
	        WaitBytes--;
	        if (WaitBytes == 0)
	        {
		        switch (buf.cmd.cmd)
		        {
		            case CMD_SET_BACKGROUND_COLOUR: // Set Background Colour 0x00, LSB, MSB
			            lcd.SetBackgroundColour( RGB( buf.cmd.bg.r, buf.cmd.bg.g, buf.cmd.bg.b) );
		                state = WAIT_FOR_COMMAND;
			            break;
						
		            case CMD_SET_FOREGROUND_COLOUR: // Set Foreground Colour 0x01, LSB, MSB
			            lcd.SetForegroundColour( RGB( buf.cmd.fg.r, buf.cmd.fg.g, buf.cmd.fg.b) );
		                state = WAIT_FOR_COMMAND;
			            break;
						
		            case CMD_SET_TEXT_POSITION: // Set position 0x04, x, y
			            lcdtext.SetTextCursor( buf.cmd.tpos.x, buf.cmd.tpos.y );
		                state = WAIT_FOR_COMMAND;
			            break;
						
		            case CMD_SET_GRAPHICS_POSITION: // Set position 0x0?, xlsb, xmsb, y
			            lcdtext.SetPixelCursor( buf.cmd.gpos.x, buf.cmd.gpos.y );
		                state = WAIT_FOR_COMMAND;
			            break;
						
		            case CMD_SET_FONT: // Set font? 0x05, 0/1
			            lcdtext.SetFont( buf.cmd.font.font );
		                state = WAIT_FOR_COMMAND;
			            break;
						
		            case CMD_SET_BACKLIGHT: // set backlight 0x06, 0-100
			            lcd.SetBacklight( buf.cmd.light.intensity );
		                state = WAIT_FOR_COMMAND;
			            break;

						
		            case CMD_SET_TEXT_ATTRIBUTE: // set attribute 0x07, [underline][Reverse?bg/fg?][italic][bold][strikethrough]
		                state = WAIT_FOR_COMMAND;
    			        break;
						
		            case CMD_DRAW_PIXEL:
				        lcd.DrawPixel( buf.cmd.pixel.x, buf.cmd.pixel.y );
		                state = WAIT_FOR_COMMAND;
				        break;
					
		            case CMD_DRAW_RECTANGLE:
	                    sprintf(s, "rect %d,%d %dx%d %c\r\n",buf.cmd.rect.x, buf.cmd.rect.y, buf.cmd.rect.width, buf.cmd.rect.height, buf.cmd.rect.fill ? 'F' : ' ' );
	                    serial_write_string( &DEVICE_USART, s );
			            if ( buf.cmd.rect.fill )
			            {
                            lcd.SolidRect( buf.cmd.rect.x, buf.cmd.rect.y, buf.cmd.rect.width, buf.cmd.rect.height );
			            }											
			            else
			            {
                            lcd.Rect( buf.cmd.rect.x, buf.cmd.rect.y, buf.cmd.rect.width, buf.cmd.rect.height );
		                }
		                state = WAIT_FOR_COMMAND;
			            break;
					
				    case CMD_BITBLT:
				        lcd.BltStart(buf.cmd.blt.x, buf.cmd.blt.y, buf.cmd.blt.width, buf.cmd.blt.height );
	                    sprintf(s, "blt %d,%d %dx%d\r\n",buf.cmd.blt.x,buf.cmd.blt.y, buf.cmd.blt.width,buf.cmd.blt.height );
	                    serial_write_string( &DEVICE_USART, s );
						WaitBytes = buf.cmd.blt.width * buf.cmd.blt.height * 2;
						state = WAIT_FOR_BLT;
				        break;
					
				    case CMD_BITBLT_RLE:
				        lcd.BltStart(buf.cmd.bltrle.x, buf.cmd.bltrle.y, buf.cmd.bltrle.width, buf.cmd.bltrle.height );
						WaitBytes = buf.cmd.bltrle.datalen[0];
						WaitBytes |= buf.cmd.bltrle.datalen[1] << 8;
						WaitBytes |= buf.cmd.bltrle.datalen[2] << 16;
						state = WAIT_FOR_BLT_RLE;
				        break;
		            }
	        }
	        break;
        case WAIT_FOR_TEXT:
	        if (b == 0)
		        state = WAIT_FOR_COMMAND;
	        else
	        {
		        if (IsDataAvailable())
		        {
		            // we can grab more bytes from the buffer and write them as
		            // as string while they are arriving.  Writing a string is a bit faster.
		            char buf[32];
		            uint8_t nBufLen = sizeof(buf);
		            uint8_t n = 0;
		            buf[n++] = b;
		            do
		            {
			            b = ReadByte();
			            if (b == 0)
			            {
			                state = WAIT_FOR_COMMAND;
			                break;
			            }
			            buf[n++] = b;
		            } while (IsDataAvailable() && n < nBufLen);

		            lcdtext.WriteString(n, buf);
		        }
		        else
                {
                    lcdtext.WriteChar(b);
                }
	        }
	        break;
			
		case WAIT_FOR_BLT:
		case WAIT_FOR_BLT1:
		    for(;;)
		    {
		        if ( state == WAIT_FOR_BLT )
			    {
	                buf.ReceiveBuffer[0] = b;
				    state = WAIT_FOR_BLT1;
			    }				
			    else
			    {
	                buf.ReceiveBuffer[1] = b;
			        lcd.WriteData( buf.colour );
			        state = WAIT_FOR_BLT;
			    }				
	            WaitBytes--;
	            if (WaitBytes == 0)
				{
			        state = WAIT_FOR_COMMAND;
			        break;
				}
			    else if ( IsDataAvailable() )
				{
					b = ReadByte();
				}
				else
				{
					break;
				}
				
			}				
			break;
    }
    //}
    //else
    //{
        //// check for a time out. If we haven't got anything for 100ms, go back to wait state
        //unsigned int nTime = Get_sys_count();
        //if ( nTime - nLastTime > CLOCK/10 )
	        //state = WAIT_FOR_COMMAND;
        //nLastTime = nTime;
    //}
}

static void ProcessComms( void )
{
    while ( IsDataAvailable() )	
    {
        int c = ReadByte();
        //printf("Got %02X %c\n", c, isprint(c) ? c : '?');
        //usart_serial_putchar( &DEVICE_USART, c );
	    ProcessDisplayData( c );
	    //text.WriteChar( c );
    }
}


static void DisplayStartupMsg()
{
	lcdtext.SetAutoScroll(false);
	lcdtext.SetTextCursor(0,0);	
	char s[200];
	snprintf( s, sizeof(s), "%s %s %d.%d\n", USB_DEVICE_PRODUCT_NAME, USB_DEVICE_MANUFACTURE_NAME, USB_DEVICE_MAJOR_VERSION, USB_DEVICE_MINOR_VERSION );
	s[sizeof(s)-1] = '\0';
	lcdtext.WriteString( s );
	serial_write_string( &DEVICE_USART, s );
}

static void speedtest2( void )
{
	lcd.ClearScreen(0);
	lcdtext.SetTextCursor(0,0);
	//lcdtext.WriteString( "This is another test" );
	//lcd.SetBackgroundColour(RGB(0,0,0));
	//lcd.SetForegroundColour(RGB(0xFF,0xFF,0xFF));
	//lcdtext.WriteString( "This is another test" );
	//lcd.SetBackgroundColour(RGB(0xFF,0xFF,0xFF));
	//lcd.SetForegroundColour(RGB(0,0,0));
	//lcdtext.WriteString( "This is another test" );
	//lcd.SetBackgroundColour(RGB(0xFF,0,0));
	//lcd.SetForegroundColour(RGB(0,0xFF,0xFF));
	//lcdtext.WriteString( "This is another test" );
	//lcd.SetBackgroundColour(RGB(0,0xFF,0xFF));
	//lcd.SetForegroundColour(RGB(0xFF,0,0));
	//lcdtext.WriteString( "This is another test" );

	char s[100];
	//Set_sys_count(0);
	int i;
	//for ( i = 0; i < 1000; i++ )
	//{
	//lcd.ClearScreen(i);
	//}
	int n = 0;//Get_sys_count();
	double t1 = 0;// ((double)n)/CLOCK;
	
	lcd.SetBackgroundColour(RGB(0,0,0));
	lcd.SetForegroundColour(RGB(0,0xff,0));
	const char *sTest = "The rain in spain falls mainly on the plain.";
	Set_sys_count(0);
	for ( int i = 0; i < 50; i++ )
	{
		lcd.SetForegroundColour( (i&3) == 0 ? RGB(0xFF,0xFF,0xFF) : (i&3) == 1 ? RGB(0xFF,0,0) : (i&3) == 2 ? RGB(0,0xFF,0) : RGB(0,0,0xFF) );
		lcdtext.WriteString( sTest );
	}
	n = 0;//Get_sys_count();
	double t2 = 0;//((double)n)/CLOCK;
	
	//text.SetAutoScroll(true);
	//Set_sys_count(0);
	//for ( int i = 0; i < 100; i++ )
	//{
	//text.WriteString( sTest );
	//}
	n = 0;//Get_sys_count();
	double t3 = 0;//((double)n)/CLOCK;

	lcd.ClearScreen(0);
	lcdtext.WriteString( "This is another test" );
	lcd.SetBackgroundColour(RGB(0,0,0));
	lcdtext.WriteString( "This is another test" );
	lcd.SolidRect(200,100,50,50);
	lcd.SetForegroundColour(RGB(0xFF,0xFF,0xFF));
	lcd.SolidRect(250,100,50,50);
	lcdtext.WriteString( "This is another test" );
	snprintf(s,sizeof(s),"Clear=%.3f, Text=%.3f, Scroll=%.3f", t1, t2, t3 );
	s[sizeof(s)-1] = 0;
	lcdtext.SetPixelCursor( SCREEN_WIDTH/2 - 7*strlen(s)/2, SCREEN_HEIGHT/2 );
	lcdtext.WriteString( s );
	lcdtext.WriteString( "This is another test" );
	delay_ms(1000);

	{
		lcd.ClearScreen(RGB(0,0xff,0));
		lcd.SetForegroundColour(RGB(0,0,0));
		lcd.SetBackgroundColour(RGB(0xFF,0xFF,0xFF));

		lcdtext.SetAutoScroll(false);
		lcdtext.SetTextCursor(1,1);
		//char s[200];
		snprintf( s, sizeof(s), "%s %s %d.%d\n", USB_DEVICE_PRODUCT_NAME, USB_DEVICE_MANUFACTURE_NAME, USB_DEVICE_MAJOR_VERSION, USB_DEVICE_MINOR_VERSION );
		s[sizeof(s)-1] = '\0';
		for ( int i = 0; i < 5; i++ )
		{
			lcdtext.WriteString( s );
			delay_ms(1000);
		}
		delay_ms(10000);
	}
	lcd.ClearScreen(RGB(0,0xff,0));
	lcd.SetForegroundColour(RGB(0xFF,0xFF,0xFF));
	lcd.SetBackgroundColour(RGB(0,0,0));

	DisplayStartupMsg();
}

static volatile bool bSuspend;
static volatile bool bResume;

void Suspend(void)
{
	bSuspend = false;
	//lcd.ClearScreen(RGB(0,0,0));
	//lcd.DisplayOn(false);
	//lcd.SetBacklight(0);
	//lcd.Sleep();
}

void user_callback_suspend_action(void)
{
	bSuspend = true;
}

static void Resume(void)
{
	bResume = false;
	//lcd.Wake();
	//lcd.DisplayOn(true);
	//lcd.ClearScreen(RGB(0,0,0));
	//lcd.SetBacklight(50);
}


void user_callback_resume_action(void)
{
	bResume = true;
}

int main (void)
{
	bSuspend = bResume = false;

    board_init();
    lcd.Init();

	//DisplayTest();
    //speedtest();
    //speedtest2();

    //lcd.SetBacklight( 100 );
	//lcd.ClearScreen(RGB(0,0,0));
	lcd.ClearScreen(0);
	//lcd.SetForegroundColour(RGB(0x7F,0x7F,0x7F));

	DisplayStartupMsg();

	
	//gpio_enable_pin_interrupt( TOUCH_nPENIRQ, GPIO_FALLING_EDGE);
	uint16_t x = 0;
	uint8_t y = 0;
	
	// Force read for the first time to reset the touch controller
	touch_init_read();

//    int bLast=0;
	uint32_t oldrtc = AVR32_RTC.val;
    while (true)
    {
        volatile avr32_usbb_t *usb = &AVR32_USBB;
		int byte_count = usb->UESTA2.byct;
		int status = usb->uesta2;
		int rxouti = usb->UESTA2.rxouti;
		int rwall = usb->UESTA2.rwall;
		int fifocon = usb->UECON2.fifocon;
		int rxoute = usb->UECON2.rxoute;
		int hs = udd_is_high_speed();
	
        ProcessComms();

		static uint16_t n = 0;
		n++;

		if ( oldrtc != AVR32_RTC.val )
		{
			oldrtc = AVR32_RTC.val;
			//lcdtext.WriteString("Tick ");
		}
		if ( touch_busy() == 0 )
		{
			if ( gpio_get_pin_value(TOUCH_nPENIRQ) == 0 )
			{
				touch_init_read();
				n = 0;
			}
		}
		else if ( touch_complete() )
		{
			char s[50];
			snprintf(s, sizeof(s), "%d,%d  %d           ", touch_x, touch_y,n);
			s[sizeof(s)-1]=0;
			lcdtext.SetTextCursor(0,1);
			lcdtext.WriteString(s);
			uint16_t x = (touch_x >> 7);
			uint8_t y = (touch_y >> 7);
			if ( x > SCREEN_WIDTH ) x = SCREEN_WIDTH - 1;
			if ( y > SCREEN_HEIGHT ) x = SCREEN_HEIGHT - 1;
			lcd.DrawPixel( x, y );
		}
		//delay_ms(1);
        //extern int bytes_read;
        //char s[30];
        //sprintf( s, "%d\n", bytes_read );
        //serial_write_string(&DEVICE_USART, s);

		if ( bSuspend ) Suspend();
		if ( bResume ) Resume();

    }

/*	
		delay_ms(2000);
	for ( int i = 0; i < 240; i++ )
		LCD_ScrollScreen(1);
	for (;;)
	{
		gpio_local_clr_gpio_pin(LED0);
		//cpu_delay_ms(500,CLOCK);
		delay_ms(500);
		gpio_local_set_gpio_pin(LED0);
		//cpu_delay_ms(500,CLOCK);
		delay_ms(500);
	}	
	
		
*/	
}


/*

Composite device
    - CDC - serial commands
        - flow control
        - ack
        - timeout
    - ANSI 
    - USB display
    - mouse (touch screen)
SPI
TTL Serial?
ASCII - curses

Framebuffer Device
    - Unix
	    - Clean up code
		- support deferred
		- optimize buffer copy - reduce Xorg cpu from 25% when running glxgears
		- investigate rle
		    - as this will not use DMA, try just not using DMA.
		- setup with no window manager
		- try dual monitor setup.
		- write Nexis.
		- investigate a dedicated X driver
	- Firmware
	    - Tidy
	    - Support blanking
	    - Support backlight intensity
		- support more lcd panels/con
		- bootloader (hardware switch?)
		- composite device?
		    - lcd
			- framebuffer
			- touch screen
			- sideshow
	
*/