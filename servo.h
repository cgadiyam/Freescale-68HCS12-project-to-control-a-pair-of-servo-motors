/* 

Real-Time Operating Systems
Project 2
Michael Moffitt & Chaithanya Gadiyam

Servo period: 20mS

0.388ms		0 deg
0.535ms		45 deg
1.264ms		90 deg
1.605ms		135 deg
2.14ms		180 deg

*/
#ifndef SERVO_H
#define SERVO_H
				 
// system includes
#include <hidef.h>			/* common defines and macros */
#include <stdio.h>			/* Standard I/O Library */

// project includes											 
#include "derivative.h" /* derivative-specific definitions */   
#include "types.h"
#include "opcode.h"
#include "state.h"
#include "pwm.h"

#define SERVO_MIN_POS 0
#define SERVO_MAX_POS 5
#define SERVO_MAX_WAIT 31
#define MOVE_DELAY_CYCLES 130
#define NUM_SERVOS 2
#define SERVO_PROG_LEN 16

// Status codes
#define ERR_LOOP 1
#define ERR_END 2
#define ERR_PAUSE 3 
#define ERR_RECIPE 4

typedef struct servo
{
	u32 pwm_val;
	u16 pc; 
	u16 wait;
	u16 loop_ptr;
	u8 pos;
	u8 loop_num;
	u8 err;
	u8 finished;
	u8 channel_num;	  
	u8 errno;
	u8 *recipe;
	volatile u8 state;
} servo;

void servo_set_recipe(servo *s, u8 *r);
void servo_led_output(servo *s);
void servo_print_state(servo *s);
void servo_init(servo *s, u8 timer);
void servo_process_opcode(servo* s, u8 opcode);
void servo_kernel(servo* s);


#endif