/* 

Real-Time Operating Systems
Project 2
Michael Moffitt & Chaithanya Gadiyam
*/
#include "types.h"
#include "servo.h"
#include "state.h"

// Servos start at timer 2 (1 used for ISR)
#define INIT_CHANNEL 2

#define MOVE_DELAY_CYCLES 2

void output_pwm(u32 value, u8 channel)
{
	
}

void build_test_prog_1(u8 *prog)
{
	prog[0] = 0x21; // MOV+1 
	prog[1] = 0x24; // MOV+4
	prog[2] = 0x85; // LOOP_START+5
	prog[3] = 0x23; // MOV+3
	prog[4] = 0x20; // MOV+0
	prog[5] = 0xA0; // END_LOOP+0
	prog[6] = 0x42; // WAIT+2
	prog[7] = 0x00; // RECIPE_END
}

void build_test_prog_2(u8 *prog)
{
	prog[0] = 0x22; // MOV+2 
	prog[1] = 0x23; // MOV+3
	prog[2] = 0x83; // LOOP_START+3
	prog[3] = 0x25; // MOV+5
	prog[4] = 0x21; // MOV+1
	prog[5] = 0xA0; // END_LOOP+0
	prog[6] = 0x44; // WAIT+4
	prog[7] = 0x00; // RECIPE_END
}

void timer_isr(void)
{
	// Example test recipe
	u8 i;
	for (i = 0; i < NUM_SERVOS; i++)
	{
		servo_kernel((servo *)(servos + i),i);
	}
}

int main(void)
{
	u8 i;
	char usr[2];
	
	u8 program1[8];
	u8 program2[8];
	
	build_test_prog_1(program1);
	build_test_prog_2(program2);
	
	for (i = 0; i < NUM_SERVOS; i++)
	{
		servo_init((servo *)(servos + i));
		servos[i].channel_num = i + INIT_CHANNEL;
	}
	
	servos[0].recipe = program1;
	servos[1].recipe = program2;
	
	for (;;)
	{
		//usr = block_on_user_input();
		
		for (i = 0; i < NUM_SERVOS; i++)
		{
			usr[i] = usr[i] | 0x20;
			if (usr[i] == USER_RESTART)
			{
				for (i = 0; i < NUM_SERVOS; i++)
				{
					servo_init((servo *)(servos + i),i);
				}
				servos[i].state = STATE_RUN;
			}
			// State-specific commands
			switch(servos[i].state)
			{
				case STATE_BEGIN:
					if (usr[i] == USER_CONTINUE)
					{
						for (i = 0; i < NUM_SERVOS; i++)
						{
							servo_init((servo *)(servos + i),i);
						}
						servos[i].state = STATE_RUN;
					}
					break;
				case STATE_RUN:
					if (usr[i] == USER_PAUSE)
					{
						servos[i].state = STATE_PAUSE;
					}
					break;
				case STATE_PAUSE:
					if (usr[i] == USER_LEFT && servos[i].pos != SERVO_MIN_POS)
					{
						servos[i].pos -= 1;
					}
					if (usr[i] == USER_RIGHT && servos[i].pos != SERVO_MAX_POS)
					{
						servos[i].pos += 1;
					}
					break;
				case STATE_ERROR:
				case STATE_END:
					break;
			}
		}
		
	}
}