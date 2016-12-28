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
#include "lcdif.h"
#include "lcdtext.h"
#include "displaycommands.h"
#include "serial.h"
#include "touch.h"
#include "lcd_conf.h"

#include "lcdiface_hx8352a.h"
#include "lcdtext.h"

#define USE_USB_CDC


#include "../fonts/Font6x13.h"
#include "../fonts/FontTypewriter.h"

static struct SFontData  fontData[] =
{
	/*45x17*/    { FONT6X13_WIDTH, FONT6X13_HEIGHT, SCREEN_WIDTH / FONT6X13_WIDTH, SCREEN_HEIGHT / FONT6X13_HEIGHT, Font6x13 },
	/*22x9 */    { FONTTYPEWRITER_WIDTH, FONTTYPEWRITER_HEIGHT, SCREEN_WIDTH / FONTTYPEWRITER_WIDTH, SCREEN_HEIGHT / FONTTYPEWRITER_HEIGHT, FontTypewriter },
};


static LCDIFaceHX8352A<SCREEN_WIDTH,SCREEN_HEIGHT> lcd;
static LCDText<LCDIFaceHX8352A<SCREEN_WIDTH,SCREEN_HEIGHT>, SCREEN_WIDTH, SCREEN_HEIGHT > text(lcd, fontData, countof(fontData) );



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
    text.WriteString( len, ptr );
    return len;
}

static void speedtest( void )
{
    Set_sys_count(0);
    for ( int i = 0; i < 1000; i++ )
    {
	    lcd.ClearScreen(i);
    }
    int n = Get_sys_count();
    double t1 = ((double)n)/CLOCK;
	
    Set_sys_count(0);
    const char *sTest = "The rain in spain falls mainly on the plain.";
    int nLen = strlen(sTest);
    for ( int i = 0; i < 1000; i++ )
    {
	    text.WriteString( nLen, sTest );
    }
    n = Get_sys_count();
    double t2 = ((double)n)/CLOCK;
	
    char s[100];
    lcd.ClearScreen(0);
    sprintf(s,"Clear=%.3f, Text=%.3f", t1, t2 );
    text.WriteString( strlen(s), s );
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
#ifdef USE_USB_CDC
    #ifdef MAIN_BUFFER
        // If there are byte available in the usb rx buffer, copy them all out.
        while ( udi_cdc_is_rx_ready() )
        {
	        circ_buffer_ptr_t tmphead;
	        /* Calculate buffer index */
	        tmphead = ( RxHead + 1 ) & RX_BUFFER_MASK; 
	        if ( tmphead == RxTail )
	        {
	            // Buffer Overflow.
		        break;
	        }
	        else
	        {
	            RxBuf[tmphead] = udi_cdc_getc();    /* Store data in buffer */
	            RxHead = tmphead;                   /* Store new index */
	        }
        }	
        return RxHead != RxTail;
    #else
	    //delay_ms(1);
        return udi_cdc_is_rx_ready();
    #endif
#else
    return false;
#endif
}    

static int ReadByte( void )
{
#ifdef USE_USB_CDC
    #ifdef MAIN_BUFFER
        circ_buffer_ptr_t tmptail;
		
        if ( RxHead == RxTail ) 
        {
            // Buffer under flow
            return -1;
        }        
        else
        {
            tmptail = ( RxTail + 1 ) & RX_BUFFER_MASK; /* Calculate buffer index */
            RxTail = tmptail;                       /* Store new index */
            return RxBuf[tmptail];                  /* Return data */
        } 
    #else
        return udi_cdc_getc();                   
    #endif
#endif
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
			            text.SetTextCursor( buf.cmd.tpos.x, buf.cmd.tpos.y );
		                state = WAIT_FOR_COMMAND;
			            break;
						
		            case CMD_SET_GRAPHICS_POSITION: // Set position 0x0?, xlsb, xmsb, y
			            text.SetPixelCursor( buf.cmd.gpos.x, buf.cmd.gpos.y );
		                state = WAIT_FOR_COMMAND;
			            break;
						
		            case CMD_SET_FONT: // Set font? 0x05, 0/1
			            text.SetFont( buf.cmd.font.font );
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

		            text.WriteString(n, buf);
		        }
		        else
                {
                    text.WriteChar(b);
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


int main (void)
{
    board_init();
    lcd.Init();
	lcd.SetBackgroundColour(RGB(0xFF,0Xff,0));
	lcd.SetForegroundColour(RGB(0,0,0));
	text.WriteString(17,"The rain in spain");
    //gpio_local_set_gpio_pin(LCD_BACKLIGHT_PWM);
    //gpio_local_clr_gpio_pin(LCD_BACKLIGHT_PWM);
    // Insert application code here, after the board has been initialized.
	
     speedtest();
    
    //for ( int i = 0; i < 400; i+= 50 )
    //{
        //for ( int j = 0; j <240; j += 40 )
        //{
            //LCD_SetForegroundColour( ((i/50)^(j/40))&1?RGB(255,255,255):RGB(0,0,0));
            //LCD_SolidRect(i,j,50,40);
        //}
    //}        
    lcd.SetBacklight( 50 );
	
    printf( "%s %s %d.%d\n", USB_DEVICE_PRODUCT_NAME, USB_DEVICE_MANUFACTURE_NAME, USB_DEVICE_MAJOR_VERSION, USB_DEVICE_MINOR_VERSION );
    serial_write_string( &DEVICE_USART, "USB Display\r\n" );

    //printf( "The rain in spain\n" );
    //for ( int i = 0; i < 20; i++ )
    //{
        //for ( int j = 0; j < i; j++ )
        //{
            //printf("%c", 'A' + j );
        //}
        //printf("\n");
    //}		
	
	//gpio_enable_pin_interrupt( TOUCH_nPENIRQ, GPIO_FALLING_EDGE);
//	uint16_t x = 0;
//	uint8_t y = 0;
	
	// Force read for the first time to reset the touch controller
	//touch_init_read();

//    int bLast=0;
	uint32_t oldrtc = AVR32_RTC.val;
    while (true)
    {
        //avr32_usbb_t *usb = &AVR32_USBB;
	//int byte_count = usb->UESTA2.byct;
	//int status = usb->uesta2;
	//int rxouti = usb->UESTA2.rxouti;
	//int rwall = usb->UESTA2.rwall;
	//int fifocon = usb->UECON2.fifocon;
	//int rxoute = usb->UECON2.rxoute;
	//int hs = udd_is_high_speed();
	//
        ProcessComms();

		static uint16_t n = 0;
		n++;

		if ( oldrtc != AVR32_RTC.val )
		{
			oldrtc = AVR32_RTC.val;
			if ( touch_busy() == 0 )
			{
				if ( gpio_get_pin_value(TOUCH_nPENIRQ) == 0 )
				{
					touch_init_read();
					n = 0;
				}
			}
		}
		if ( touch_complete() )
		{
			char s[30];
			snprintf(s, sizeof(s), "%d,%d  %d ", touch_x, touch_y,n);
			text.SetTextCursor(0,0);
			text.WriteString(strlen(s),s);
			uint16_t x = (touch_y >> 6);
			uint8_t y = SCREEN_HEIGHT - (touch_x >> 7);
			lcd.DrawPixel( x, y );
		}
		//delay_ms(1);
        //extern int bytes_read;
        //char s[30];
        //sprintf( s, "%d\n", bytes_read );
        //serial_write_string(&DEVICE_USART, s);
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