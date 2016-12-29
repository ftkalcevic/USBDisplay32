/**
 * \file
 *
 * \brief User board initialization template
 *
 */

#include <board.h>
#include <conf_board.h>
#include "asf.h"
#include "lcdif.h"
#include "sleepmgr.h"
#include "sysclk.h"
#include "udc.h"
#include "touch.h"
#include "serial.h"

static void InitGPIOs( void )
{
    //gpio_local_init();

    //gpio_enable_gpio_pin(LED0);
    //gpio_local_enable_pin_output_driver(LED0);

    //gpio_enable_gpio_pin(LCDFMark);
    //gpio_local_disable_pin_output_driver(LCDFMark);
}

static void InitClocks( void )
{
    sysclk_init();
//#define OSC0_66MHz
#ifdef OSC0_66MHz
    // Set the clock to 66 MHz
    pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);
    pm_pll_setup( &AVR32_PM, 0,  // pll.
	          10,  // mul.
	          1,   // div.
	          0,   // osc.
	          16); // lockcount.
    pm_pll_set_option( &AVR32_PM, 0, // pll.
	               0,  // pll_freq.
	               0,  // pll_div2.
	               0); // pll_wbwdisable.
    pm_pll_enable(&AVR32_PM, 0);
    pm_wait_for_pll0_locked(&AVR32_PM);
    pm_cksel( &AVR32_PM,
	      1,   // pbadiv.	PBA = CLK/2 = 33MHz
	      0,   // pbasel.
	      1,   // pbbdiv.	PBB = CLK/2 = 33MHz
	      0,   // pbbsel.
	      1,   // hsbdiv.	HSB = CLK/2 = 33MHz
	      0);  // hsbsel.
    flashc_set_wait_state(1);
    pm_switch_to_clock(&AVR32_PM, AVR32_PM_MCCTRL_MCSEL_PLL0);
	
#endif
	
    //pm_freq_param_t params;
    //params.osc0_f = FOSC0;
    //params.osc0_startup = OSC0_STARTUP;
    //params.cpu_f = CLOCK;
    //params.pba_f = 16500000;
    //pm_configure_clocks( &params );
	    //

    delay_init(CLOCK);
	
	//// Real time clock 
	//while ( AVR32_RTC.CTRL.busy );
	//AVR32_RTC.CTRL.clk32 = 0;
	//while ( AVR32_RTC.CTRL.busy );
	//AVR32_RTC.CTRL.psel = 10;	// ~ 56Hz
	//while ( AVR32_RTC.CTRL.busy );
	//AVR32_RTC.CTRL.clken = 1;
	//while ( AVR32_RTC.CTRL.busy );
	//AVR32_RTC.CTRL.en = 1;

	// Real time clock 
	while ( AVR32_RTC.CTRL.busy );
	AVR32_RTC.CTRL.clk32 = 0;
	while ( AVR32_RTC.CTRL.busy );
	AVR32_RTC.CTRL.psel = 15;	// 1.7Hz
	while ( AVR32_RTC.CTRL.busy );
	AVR32_RTC.CTRL.clken = 1;
	while ( AVR32_RTC.CTRL.busy );
	AVR32_RTC.CTRL.en = 1;

}



static void InitBacklightPWM( void )
{
#define NOPWM
#ifdef NOPWM
	gpio_enable_gpio_pin(LCD_BACKLIGHT_PWM);

	// Set reset as an output, and pull low.
	gpio_configure_pin(LCD_BACKLIGHT_PWM, GPIO_DIR_OUTPUT);
	gpio_set_gpio_pin(LCD_BACKLIGHT_PWM);
#else
    // Options for waveform generation.
    tc_waveform_opt_t waveform_opt =
    {
        .channel  = 0,				            // Channel selection.

        .bswtrg   = TC_EVT_EFFECT_NOOP,                     // Software trigger effect on TIOB.
        .beevt    = TC_EVT_EFFECT_NOOP,                     // External event effect on TIOB.
        .bcpc     = TC_EVT_EFFECT_NOOP,                     // RC compare effect on TIOB.
        .bcpb     = TC_EVT_EFFECT_NOOP,                     // RB compare effect on TIOB.

        .aswtrg   = TC_EVT_EFFECT_NOOP,                     // Software trigger effect on TIOA.
        .aeevt    = TC_EVT_EFFECT_NOOP,                     // External event effect on TIOA.
        .acpc     = TC_EVT_EFFECT_CLEAR,                    // RC compare effect on TIOA.
        .acpa     = TC_EVT_EFFECT_SET,                      // RA compare effect on TIOA.

        .wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,     // Waveform selection: Up mode with automatic trigger on RC compare.
        .enetrg   = FALSE,                                  // External event trigger enable.
        .eevt     = TC_EXT_EVENT_SEL_TIOB_INPUT,            // External event selection.
        .eevtedg  = TC_SEL_NO_EDGE,                         // External event edge selection.
        .cpcdis   = FALSE,                                  // Counter disable when RC compare.
        .cpcstop  = FALSE,                                  // Counter clock stopped with RC compare.

        .burst    = TC_BURST_NOT_GATED,                     // Burst signal selection.
        .clki     = TC_CLOCK_RISING_EDGE,                   // Clock inversion.
        .tcclks   = TC_CLOCK_SOURCE_TC3                     // Internal source clock 3, connected to fPBA / 8. (PBA=33MHz, /8, /0x2000) = 500Hz
    };

    // Assign the pin to the module.
    gpio_enable_module_pin(AVR32_TC0_A0_0_1_PIN, AVR32_TC0_A0_0_1_FUNCTION);
	
    // Initialize the timer/counter waveform.
    tc_init_waveform(&AVR32_TC0, &waveform_opt);
	
    // Set the compare trigger.
    tc_write_ra(&AVR32_TC0, 0, 200);		// Default to 50%
    tc_write_rc(&AVR32_TC0, 0, 400);

    // Start the timers/counters.
    tc_start(&AVR32_TC0, 0);
#endif
}


static void InitUSB(void)
{
    sleepmgr_init();
    udc_start();
    udc_attach();
}

static void serial_init(void)
{
    gpio_enable_module_pin(RS232_TX, AVR32_USART0_TXD_0_0_FUNCTION);
    gpio_enable_module_pin(RS232_RX, AVR32_USART0_RXD_0_0_FUNCTION);

    usart_serial_options_t opt;
    opt.baudrate = 115200;
    opt.charlength = 8;
    opt.paritytype = USART_NO_PARITY;
    opt.stopbits = USART_1_STOPBIT;
    opt.channelmode = USART_NORMAL_CHMODE;    
    
    usart_serial_init( &DEVICE_USART, &opt);
    setup_tx_interrupt();
}

static void test(void)
{
	/* Insert system clock initialization code here (sysclk_init()). */
	//sysclk_init();
	////board_init();
//
	//gpio_local_init();
	//gpio_enable_gpio_pin(LCDD0);
	//gpio_enable_gpio_pin(LCDD1);
	//gpio_enable_gpio_pin(LCDD2);
	//gpio_enable_gpio_pin(LCDD3);
	//gpio_enable_gpio_pin(LCDD4);
	//gpio_enable_gpio_pin(LCDD5);
	//gpio_enable_gpio_pin(LCDD6);
	//gpio_enable_gpio_pin(LCDD7);
	//gpio_enable_gpio_pin(LCDD8);
	//gpio_enable_gpio_pin(LCDD9);
	//gpio_enable_gpio_pin(LCDD10);
	//gpio_enable_gpio_pin(LCDD11);
	//gpio_enable_gpio_pin(LCDD12);
	//gpio_enable_gpio_pin(LCDD13);
	//gpio_enable_gpio_pin(LCDD14);
	//gpio_enable_gpio_pin(LCDD15);
	//gpio_local_enable_pin_output_driver(LCDD0);
	//gpio_local_enable_pin_output_driver(LCDD1);
	//gpio_local_enable_pin_output_driver(LCDD2);
	//gpio_local_enable_pin_output_driver(LCDD3);
	//gpio_local_enable_pin_output_driver(LCDD4);
	//gpio_local_enable_pin_output_driver(LCDD5);
	//gpio_local_enable_pin_output_driver(LCDD6);
	//gpio_local_enable_pin_output_driver(LCDD7);
	//gpio_local_enable_pin_output_driver(LCDD8);
	//gpio_local_enable_pin_output_driver(LCDD9);
	//gpio_local_enable_pin_output_driver(LCDD10);
	//gpio_local_enable_pin_output_driver(LCDD11);
	//gpio_local_enable_pin_output_driver(LCDD12);
	//gpio_local_enable_pin_output_driver(LCDD13);
	//gpio_local_enable_pin_output_driver(LCDD14);
	//gpio_local_enable_pin_output_driver(LCDD15);

	gpio_set_gpio_pin(LCDD0);
	gpio_clr_gpio_pin(LCDD1);
	gpio_set_gpio_pin(LCDD2);
	gpio_clr_gpio_pin(LCDD3);
	gpio_set_gpio_pin(LCDD4);
	gpio_clr_gpio_pin(LCDD5);
	gpio_set_gpio_pin(LCDD6);
	gpio_clr_gpio_pin(LCDD7);
	gpio_set_gpio_pin(LCDD8);
	gpio_clr_gpio_pin(LCDD9);
	gpio_set_gpio_pin(LCDD10);
	gpio_clr_gpio_pin(LCDD11);
	gpio_set_gpio_pin(LCDD12);
	gpio_clr_gpio_pin(LCDD13);
	gpio_set_gpio_pin(LCDD14);
	gpio_clr_gpio_pin(LCDD15);

	/* Insert application code here, after the board has been initialized. */
	for (;;)
	{
		gpio_tgl_gpio_pin(LCDD0);
		gpio_tgl_gpio_pin(LCDD1);
		gpio_tgl_gpio_pin(LCDD2);
		gpio_tgl_gpio_pin(LCDD3);
		gpio_tgl_gpio_pin(LCDD4);
		gpio_tgl_gpio_pin(LCDD5);
		gpio_tgl_gpio_pin(LCDD6);
		gpio_tgl_gpio_pin(LCDD7);
		gpio_tgl_gpio_pin(LCDD8);
		gpio_tgl_gpio_pin(LCDD9);
		gpio_tgl_gpio_pin(LCDD10);
		gpio_tgl_gpio_pin(LCDD11);
		gpio_tgl_gpio_pin(LCDD12);
		gpio_tgl_gpio_pin(LCDD13);
		gpio_tgl_gpio_pin(LCDD14);
		gpio_tgl_gpio_pin(LCDD15);
		delay_ms(100);
	}
}


void board_init(void)
{
    /* This function is meant to contain board-specific initialization code
	* for, e.g., the I/O pins. The initialization can rely on application-
	* specific board configuration, found in conf_board.h.
	*/
    irq_initialize_vectors();
    cpu_irq_enable();
		
    InitClocks();
    InitGPIOs();
//	test();
    //InitUSB();
    InitBacklightPWM();
    touch_init();
    serial_init();
    
}
