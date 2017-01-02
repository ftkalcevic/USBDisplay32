#pragma once

// LCD Hardware interface.  
//
// Simple class to interface to the LCD hardware

//#define  LCD_BITBANG_INTERFACE
class LCDHwIface
{
#ifdef LCD_BITBANG_INTERFACE
	// Bit bang implementation of the hardware interface.  The IO pins are set up for use by the EBI interface
	// so they io pins are not in a useful order for writing words directly to ports.
	// This interface is usually only used when doing slow testing.
	private:
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

		static void SetPinValue( int nPin, int nValue )
		{
			if ( nValue )
				gpio_set_gpio_pin(nPin);
			else
				gpio_clr_gpio_pin(nPin);
		}

		static void WriteBus( uint16_t V)   //Write Data
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


		static int ReadBus(void)
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

	public:
		static void LCDInitIO(void)
		{
			// Note - we don't use the gpio_local* functions as they require PB clock = CPU clock. (why can't we change PB clock?)
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

			// RD/WR on falling edge, so keep high
			gpio_set_gpio_pin(LCDWR);
			gpio_set_gpio_pin(LCDRD);
		}

		static void WriteCommand(unsigned short cmd)
		{
			gpio_clr_gpio_pin(LCDRS);
			WriteBus(cmd);
		}

		static void WriteData(uint16_t data)//sent data
		{
			gpio_set_gpio_pin(LCDRS);
			WriteBus(data);
		}

		static int ReadData(void)
		{
			gpio_set_gpio_pin(LCDRS);
			return ReadBus();
		}
#else
	// EBI implementation (external bus interface).
	// The data pins and control pins are managed directly by the MCU, treating the lcd as a memory device.
	// This reduces writing commands and data to writing to memory.  This is also very fast.
	public:
		static void LCDInitIO(void)
		{
			// EBI
			// Set up the pins
			// Note - we don't use the gpio_local* functions as they require PB clock = CPU clock. (why can't we change PB clock?)
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

			gpio_enable_module_pin(EBI_ADDR21, AVR32_EBI_ADDR_21_FUNCTION);		// Map Address line 21 to LCD RS (data/command) so we have 2 different memory addresses for data and commands.
			gpio_enable_module_pin(EBI_NRD,    AVR32_EBI_NRD_0_FUNCTION);		// Map ~RD to LCD ~RD
			gpio_enable_module_pin(EBI_NWE0,   AVR32_EBI_NWE0_0_FUNCTION);		// Map ~WE to LCD ~WR
	
			// Set reset as an output, and pull low.
			gpio_configure_pin(LCDReset, GPIO_DIR_OUTPUT);
			gpio_clr_gpio_pin(LCDReset);

			AVR32_SMC.cs[0].SETUP.ncs_rd_setup = LCD_NCS_RD_SETUP;
			AVR32_SMC.cs[0].SETUP.nrd_setup = LCD_NRD_SETUP;
			AVR32_SMC.cs[0].SETUP.ncs_wr_setup = LCD_NCS_WR_SETUP;
			AVR32_SMC.cs[0].SETUP.nwe_setup = LCD_NWE_SETUP;
	
			AVR32_SMC.cs[0].PULSE.ncs_rd_pulse = LCD_NCS_RD_PULSE;
			AVR32_SMC.cs[0].PULSE.nrd_pulse = LCD_NRD_PULSE;
			AVR32_SMC.cs[0].PULSE.ncs_wr_pulse = LCD_NCS_WR_PULSE;
			AVR32_SMC.cs[0].PULSE.nwe_pulse = LCD_NWE_PULSE;
	
			AVR32_SMC.cs[0].CYCLE.nrd_cycle = LCD_NRD_CYCLE;
			AVR32_SMC.cs[0].CYCLE.nwe_cycle = LCD_NWE_CYCLE;
	
			AVR32_SMC.cs[0].MODE.ps = 0;		// Page Size - not using
			AVR32_SMC.cs[0].MODE.pmen = 0;		// Page Mode Enable = false
			AVR32_SMC.cs[0].MODE.tdf_mode = 0;	// TDF optimisation - false
			AVR32_SMC.cs[0].MODE.tdf_cycles = 0;
			AVR32_SMC.cs[0].MODE.dbw = 1;		// Data bus width
			AVR32_SMC.cs[0].MODE.bat = 0;		// Byte access type - don't care 16bit only.
			AVR32_SMC.cs[0].MODE.exnw_mode = 0;	// don't use NWAIT
			AVR32_SMC.cs[0].MODE.write_mode = 1;// Write mode controlled by NWE
			AVR32_SMC.cs[0].MODE.read_mode = 1;	// Read mode controlled by NRD
		}

		static inline void WriteData(uint16_t data)
		{
			*LCD_DATA = data;
		}

		static inline void WriteCommand(uint16_t cmd)
		{
			*LCD_COMMAND = cmd;
		}
 
		static int ReadData(void)
		{
			return *LCD_DATA;
		}

		static int ReadCommand(void)
		{
			return *LCD_COMMAND;
		}
 #endif

	static void Command(uint16_t reg, uint16_t data )
	{
		WriteCommand(reg);
		WriteData(data);
	}
};
