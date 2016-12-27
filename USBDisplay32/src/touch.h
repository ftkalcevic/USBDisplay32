#ifndef _TOUCH_C_
#define _TOUCH_C_


extern void touch_init( void );
extern void touch_init_read(void);
extern bool touch_complete(void);
extern bool touch_busy();


extern uint16_t touch_x;
extern uint16_t touch_y;
extern uint16_t touch_z1;
extern uint16_t touch_z2;


#endif
