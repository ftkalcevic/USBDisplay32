#include <board.h>
#include <conf_board.h>
#include "asf.h"
#include "touch.h"
#include "common.h"

// TSC2046



#define CONTROL( Start, Addr, Mode, SERDFR, PD )		((Start<<7) | (Addr<<4) | (Mode<<3) | (SERDFR<<2) | PD)
#define START 1
#define ADR_MEASURE_Y		0b001
#define ADR_MEASURE_Z1		0b011
#define ADR_MEASURE_Z2		0b100
#define ADR_MEASURE_X		0b101
#define MODE_12BIT			0
#define MODE_8BIT			1
#define SER					1
#define DFR					0
#define PD_BETWEEN_CONV		0b00
#define PD_REF_OFF_ADC_ON	0b01
#define PD_REF_ON_ADC_OFF	0b10
#define PD_ALWAYS_ON		0b11

struct  __attribute__ ((packed)) STouchMsg
{
	union
	{
		struct
		{
			uint8_t ctl;
			uint8_t a;
			uint8_t b;
		};
		uint8_t raw[0];
	};
};

static volatile bool bTouchBusy;
static volatile bool bTouchComplete;
uint16_t touch_x;
uint16_t touch_y;
uint16_t touch_z1;
uint16_t touch_z2;

static struct STouchMsg msg[] =
{ 
	{ CONTROL( START, ADR_MEASURE_X, MODE_12BIT, DFR, PD_BETWEEN_CONV ), 0, 0},
	{ CONTROL( START, ADR_MEASURE_Y, MODE_12BIT, DFR, PD_BETWEEN_CONV ), 0, 0},
	{ CONTROL( START, ADR_MEASURE_Z1, MODE_12BIT, DFR, PD_BETWEEN_CONV ), 0, 0},
	{ CONTROL( START, ADR_MEASURE_Z2, MODE_12BIT, DFR, PD_BETWEEN_CONV ), 0, 0}
};

static struct STouchMsg  return_msg[countof(msg)];


//ISR(touch_gpio_interrupt, AVR32_GPIO_IRQ_0 + (TOUCH_nPENIRQ / AVR32_GPIO_IRQS_PER_GROUP), 0)
//{
	//gpio_clear_pin_interrupt_flag(TOUCH_nPENIRQ);
	//
	//// Set up for SPI read
	//static int i = 0;
	//
	//i++;
//}

ISR(touch_trc_interrupt,AVR32_PDCA_IRQ_1,0)
{
	AVR32_PDCA.channel[1].IDR.trc = 1;
	bTouchComplete = true;	
	bTouchBusy = false;
}

bool touch_busy()
{
	return bTouchBusy;
}
bool touch_complete()
{
	bool bComplete = false;
	
	if ( bTouchComplete )
	{
		cpu_irq_disable();
		bTouchComplete = false;
		cpu_irq_enable();
		bComplete = true;
	
		#define TOUCH_DATA(x)			((x.a << 8) | x.b)
		touch_x  = TOUCH_DATA( return_msg[0] );
		touch_y  = TOUCH_DATA( return_msg[1] );
		touch_z1 = TOUCH_DATA( return_msg[2] );
		touch_z2 = TOUCH_DATA( return_msg[3] );		
	}
	return bComplete;
}

void touch_init( void )
{
return;
	bTouchComplete = false;
	bTouchBusy = false;
	sysclk_enable_pba_module(SYSCLK_SPI1);
	
    //gpio_enable_gpio_pin(TOUCH_nCS);
    //gpio_local_enable_pin_output_driver(TOUCH_nCS);
    //gpio_local_set_gpio_pin(TOUCH_nCS);                  // always selected WRONG - CS used on SPI comms
    //
    gpio_enable_gpio_pin(TOUCH_BUSY);
    gpio_local_disable_pin_output_driver(TOUCH_BUSY);
 
    gpio_enable_gpio_pin(TOUCH_nPENIRQ);
    gpio_local_disable_pin_output_driver(TOUCH_nPENIRQ);
	//irq_register_handler(touch_gpio_interrupt, AVR32_GPIO_IRQ_0 + (TOUCH_nPENIRQ / AVR32_GPIO_IRQS_PER_GROUP), 0);
	
	// setup SPI to...
	gpio_enable_module_pin(AVR32_SPI1_MISO_0_1_PIN,AVR32_SPI1_MISO_0_1_FUNCTION);
	gpio_enable_module_pin(AVR32_SPI1_MOSI_0_1_PIN,AVR32_SPI1_MOSI_0_1_FUNCTION);
	gpio_enable_module_pin(AVR32_SPI1_NPCS_1_0_PIN,AVR32_SPI1_NPCS_1_0_FUNCTION);
	gpio_enable_module_pin(AVR32_SPI1_SCK_0_1_PIN,AVR32_SPI1_SCK_0_1_FUNCTION);

    //gpio_enable_gpio_pin(TOUCH_DCLK);
    //gpio_local_enable_pin_output_driver(TOUCH_DCLK);
//
    //gpio_enable_gpio_pin(TOUCH_DOUT);
    //gpio_local_enable_pin_output_driver(TOUCH_DOUT);
	//
    //gpio_enable_gpio_pin(TOUCH_DIN);
    //gpio_local_disable_pin_output_driver(TOUCH_DIN);
	//
    //gpio_enable_gpio_pin(TOUCH_nCS);
    //gpio_local_enable_pin_output_driver(TOUCH_nCS);

	spi_reset(&AVR32_SPI1);
	spi_set_master_mode(&AVR32_SPI1);
	spi_disable_modfault(&AVR32_SPI1);
	spi_disable_loopback(&AVR32_SPI1);
	spi_set_chipselect(&AVR32_SPI1,1);
	spi_disable_variable_chipselect(&AVR32_SPI1);
	spi_disable_chipselect_decoding(&AVR32_SPI1);
	//spi_set_delay(&AVR32_SPI1,CONFIG_SPI_MASTER_DELAY_BCS);
	spi_set_chipselect_delay_bct(&AVR32_SPI1,1,64);
	//spi_set_chipselect_delay_bs(spi,device->id,CONFIG_SPI_MASTER_DELAY_BS);
	spi_set_bits_per_transfer(&AVR32_SPI1,1, 8);
	spi_set_baudrate_register(&AVR32_SPI1,1, getBaudDiv(250000UL, sysclk_get_peripheral_bus_hz(&AVR32_SPI1)) );
	spi_enable_active_mode(&AVR32_SPI1,1);
	spi_set_mode(&AVR32_SPI1,1,0);
	spi_enable(&AVR32_SPI1);

	AVR32_PDCA.channel[0].mar = msg;
	AVR32_PDCA.channel[0].PSR.pid = AVR32_PDCA_PID_SPI1_TX;
	AVR32_PDCA.channel[0].TCR.tcv = sizeof(msg);
	AVR32_PDCA.channel[0].MR.size = AVR32_PDCA_MR_SIZE_BYTE;
		
	AVR32_PDCA.channel[1].mar = return_msg;
	AVR32_PDCA.channel[1].PSR.pid = AVR32_PDCA_PID_SPI1_RX;
	AVR32_PDCA.channel[1].TCR.tcv = sizeof(return_msg);
	AVR32_PDCA.channel[1].MR.size = AVR32_PDCA_MR_SIZE_BYTE;

	irq_register_handler(touch_trc_interrupt, AVR32_PDCA_IRQ_1, 0);
}

void touch_init_read( void )
{
	bTouchComplete = false;
	bTouchBusy = true;
	
	// Setup DMA Read
	AVR32_PDCA.channel[0].mar = msg;
	AVR32_PDCA.channel[1].mar = return_msg;
	AVR32_PDCA.channel[1].TCR.tcv = sizeof(return_msg);	// Set counts
	AVR32_PDCA.channel[0].TCR.tcv = sizeof(msg);
	AVR32_PDCA.channel[1].IER.trc = AVR32_PDCA_IER_TRC;	// Interrupt on transfer complete
	AVR32_PDCA.channel[1].CR.ten = 1;					// Enable (RX first).
	AVR32_PDCA.channel[0].CR.ten = 1;
		
	//for ( uint8_t i = 0; i < countof(msg); i++ )
	//{
		//for ( uint8_t r = 0; r < sizeof(struct STouchMsg); r++ )
		//{
			//spi_write_single(&AVR32_SPI1,msg[i].raw[r]);
			//while ( !spi_is_rx_ready(&AVR32_SPI1) )
				//continue;
			//spi_read_single(&AVR32_SPI1,return_msg[i].raw+r);
		//}
		////spi_deselect_device(&AVR32_SPI1,1);
	//}
	
	

}