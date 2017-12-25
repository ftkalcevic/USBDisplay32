#pragma  once

//#define ITDB02_32WD
#define ITDB02_43

#ifdef ITDB02_32WD

#define DISPLAY_TYPE		1
#define SCREEN_WIDTH		400
#define SCREEN_HEIGHT		240
#define SCREEN_WIDTH_MM		70
#define SCREEN_HEIGHT_MM	42
#define SCREEN_SCANLINE_LEN	(SCREEN_WIDTH*2)
#define SCREEN_BITS_PER_PIXEL 16


#define RGB(r,g,b) ((uint16_t)(((((r)>>3)&0x1f) << 11) | ((((g)>>2)&0x3f) << 5) | (((b)>>3)&0x1f) ))

// EBI SMC timings
//#define LCD_NCS_RD_SETUP		0
//#define LCD_NCS_RD_PULSE		3
//#define LCD_NRD_SETUP			0
//#define LCD_NRD_PULSE			2
//#define LCD_NRD_CYCLE			2
//
//#define LCD_NCS_WR_SETUP		0
//#define LCD_NCS_WR_PULSE		1
//#define LCD_NWE_SETUP			2
//#define LCD_NWE_PULSE			2
////          NWE_HOLD			2
//#define LCD_NWE_CYCLE			6

#define LCD_NCS_RD_SETUP		1
#define LCD_NCS_RD_PULSE		4
#define LCD_NRD_SETUP			1
#define LCD_NRD_PULSE			3
#define LCD_NRD_CYCLE			3

#define LCD_NCS_WR_SETUP		1
#define LCD_NCS_WR_PULSE		2
#define LCD_NWE_SETUP			3
#define LCD_NWE_PULSE			3
//          NWE_HOLD			2
#define LCD_NWE_CYCLE			7

#define TLCD		LCDIFace<LCDIFaceHX8352A<SCREEN_WIDTH,SCREEN_HEIGHT>,SCREEN_WIDTH,SCREEN_HEIGHT>	
#define TLCDText	LCDText< LCDIFace<LCDIFaceHX8352A<SCREEN_WIDTH,SCREEN_HEIGHT>,SCREEN_WIDTH,SCREEN_HEIGHT>, SCREEN_WIDTH, SCREEN_HEIGHT >	

#endif

#ifdef ITDB02_43

#define DISPLAY_TYPE		1
#define SCREEN_WIDTH		480
#define SCREEN_HEIGHT		272
#define SCREEN_WIDTH_MM		95
#define SCREEN_HEIGHT_MM	54
#define SCREEN_SCANLINE_LEN	(SCREEN_WIDTH*2)
#define SCREEN_BITS_PER_PIXEL 16

#define RGB(r,g,b) ((uint16_t)(((((r)>>3)&0x1f) << 11) | ((((g)>>2)&0x3f) << 5) | (((b)>>3)&0x1f) ))

#define TLCD		LCDIFace<LCDIFaceSSD1963<SCREEN_WIDTH,SCREEN_HEIGHT>,SCREEN_WIDTH,SCREEN_HEIGHT>	
#define TLCDText	LCDText< LCDIFace<LCDIFaceSSD1963<SCREEN_WIDTH,SCREEN_HEIGHT>,SCREEN_WIDTH,SCREEN_HEIGHT>, SCREEN_WIDTH, SCREEN_HEIGHT >	

// EBI SMC timings
#define LCD_NCS_RD_SETUP		0
#define LCD_NCS_RD_PULSE		3
#define LCD_NRD_SETUP			0
#define LCD_NRD_PULSE			2
#define LCD_NRD_CYCLE			2

#define LCD_NCS_WR_SETUP		0
#define LCD_NCS_WR_PULSE		1
#define LCD_NWE_SETUP			1
#define LCD_NWE_PULSE			1
//          NWE_HOLD			1
#define LCD_NWE_CYCLE			3

#endif

#ifdef __cplusplus
extern "C" {
#endif

	extern void LCD_BltStart( uint16_t x, uint16_t y, uint16_t nWidth, uint16_t nHeight );
	extern void LCD_SetBacklight( uint8_t intenstity );

#ifdef __cplusplus
}
#endif