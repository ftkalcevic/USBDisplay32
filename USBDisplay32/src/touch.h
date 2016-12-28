#ifndef _TOUCH_C_
#define _TOUCH_C_


#ifdef __cplusplus
extern "C" {
#endif

extern void touch_init( void );
extern void touch_init_read(void);
extern bool touch_complete(void);
extern bool touch_busy(void);


extern uint16_t touch_x;
extern uint16_t touch_y;
extern uint16_t touch_z1;
extern uint16_t touch_z2;

#ifdef __cplusplus
}
#endif

#endif
