#ifndef _LCD_CONF_H_
#define _LCD_CONF_H_

#define SCREEN_WIDTH		400
#define SCREEN_HEIGHT		240

#ifndef RGB
	#define RGB(r,g,b) ((uint16_t)(((((r)>>3)&0x1f) << 11) | ((((g)>>2)&0x3f) << 5) | (((b)>>3)&0x1f) ))
#endif

#endif
