/******************************************************************************
 * Timer Output Compare Demo
 *
 * Description:
 *
 * This demo configures the timer to a rate of 1 MHz, and the Output Compare
 * Channel 1 to toggle PORT T, Bit 1 at rate of 10 Hz. 
 *
 * The toggling of the PORT T, Bit 1 output is done via the Compare Result Output
 * Action bits.	
 * 
 * The Output Compare Channel 1 Interrupt is used to refresh the Timer Compare
 * value at each interrupt
 * 
 * Author:
 *	Jon Szymaniak (08/14/2009)
 *	Tom Bullinger (09/07/2011)	Added terminal framework
 *
 *****************************************************************************/


// system includes
#include <hidef.h>			/* common defines and macros */
#include <stdio.h>			/* Standard I/O Library */

// project includes
#include "derivative.h" /* derivative-specific definitions */   
#include "types.h"
#include "servo.h"
#include "state.h"
#include "opcode.h"

// Definitions

void timer_isr(void);

// Change this value to change the frequency of the output compare signal.
// The value is in Hz.
#define OC_FREQ_HZ		((u16)10)

// Macro definitions for determining the TC1 value for the desired frequency
// in Hz (OC_FREQ_HZ). The formula is:
//
// TC1_VAL = ((Bus Clock Frequency / Prescaler value) / 2) / Desired Freq in Hz
//
// Where:
//				Bus Clock Frequency		 = 2 MHz
//				Prescaler Value				 = 2 (Effectively giving us a 1 MHz timer)
//				2 --> Since we want to toggle the output at half of the period
//				Desired Frequency in Hz = The value you put in OC_FREQ_HZ
//
#define BUS_CLK_FREQ	((u32) 2000000)	 
#define PRESCALE			((u16)	2)				 
#define TC1_VAL			 ((u16)	(((BUS_CLK_FREQ / PRESCALE) / 2) / OC_FREQ_HZ))


// Initializes SCI0 for 8N1, 9600 baud, polled I/O
// The value for the baud selection registers is determined
// using the formula:
//
// SCI0 Baud Rate = ( 2 MHz Bus Clock ) / ( 16 * SCI0BD[12:0] )
//--------------------------------------------------------------
void InitializeSerialPort(void)
{
	// Set baud rate to ~9600 (See above formula)
	SCI0BD = 13;					
	
	// 8N1 is default, so we don't have to touch SCI0CR1.
	// Enable the transmitter and receiver.
	SCI0CR2_TE = 1;
	SCI0CR2_RE = 1;
}


// Initializes I/O and timer settings for the demo.
//--------------------------------------------------------------			 
void InitializeTimer(void)
{
	// Set the timer prescaler to %2, since the bus clock is at 2 MHz,
	// and we want the timer running at 1 MHz
	TSCR2_PR0 = 1;
	TSCR2_PR1 = 0;
	TSCR2_PR2 = 0;
		
	// Enable output compare on Channel 1
	TIOS_IOS1 = 1;
	
	// Set up output compare action to toggle Port T, bit 1
	TCTL2_OM1 = 0;
	TCTL2_OL1 = 1;
	
	// Set up timer compare value
	TC1 = TC1_VAL;
	
	// Clear the Output Compare Interrupt Flag (Channel 1) 
	TFLG1 = TFLG1_C1F_MASK;
	
	// Enable the output compare interrupt on Channel 1;
	TIE_C1I = 1;	
	
	//
	// Enable the timer
	// 
	TSCR1_TEN = 1;
	 
	//
	// Enable interrupts via macro provided by hidef.h
	//
	EnableInterrupts;
}


// Output Compare Channel 1 Interrupt Service Routine
// Refreshes TC1 and clears the interrupt flag.
//					
// The first CODE_SEG pragma is needed to ensure that the ISR
// is placed in non-banked memory. The following CODE_SEG
// pragma returns to the default scheme. This is neccessary
// when non-ISR code follows. 
//
// The TRAP_PROC tells the compiler to implement an
// interrupt funcion. Alternitively, one could use
// the __interrupt keyword instead.
// 
// The following line must be added to the Project.prm
// file in order for this ISR to be placed in the correct
// location:
//		VECTOR ADDRESS 0xFFEC OC1_isr 
#pragma push
#pragma CODE_SEG __SHORT_SEG NON_BANKED
//--------------------------------------------------------------			 
void interrupt 9 OC1_isr( void )
{
	timer_isr();
	TC1		 +=	TC1_VAL;			
	TFLG1	 =	 TFLG1_C1F_MASK;	
}
#pragma pop


// This function is called by printf in order to
// output data. Our implementation will use polled
// serial I/O on SCI0 to output the character.
//
// Remember to call InitializeSerialPort() before using printf!
//
// Parameters: character to output
//--------------------------------------------------------------			 
void TERMIO_PutChar(s8 ch)
{
	// Poll for the last transmit to be complete
	do
	{
		// Nothing	
	} while (SCI0SR1_TC == 0);
	
	// write the data to the output shift register
	SCI0DRL = ch;
}


// Polls for a character on the serial port.
//
// Returns: Received character
//--------------------------------------------------------------			 
u8 GetChar(void)
{ 
	// Poll for data
	do
	{
		// Nothing
	} while(SCI0SR1_RDRF == 0);
	 
	// Fetch and return data from SCI0
	return SCI0DRL;
}
/* 

Real-Time Operating Systems
Project 2
Michael Moffitt & Chaithanya Gadiyam
*/
#include "types.h"
#include "servo.h"
#include "state.h"

servo *global_servos;

// Servos start at timer 2 (1 used for ISR)
#define INIT_CHANNEL 2
							
#define OP_RECIPE_END 0x00
#define OP_MOV 0x01
#define OP_WAIT 0x02	  
#define OP_ERR1 0x03
#define OP_LOOP_START 0x04
#define OP_END_LOOP 0x05  
#define OP_ERR2 0x07
#define OP_ERR3 0x06

void print_recipe_line(u8 data)
{
	switch (get_opcode(data))
	{
		case OP_RECIPE_END:
			(void)printf("RECIPE_END ");
			break;	
		case OP_MOV:
			(void)printf("MOV        ");
			break;		 
		case OP_WAIT:
			(void)printf("WAIT       ");
			break;		 
		case OP_ERR1:
			(void)printf("MOVC       ");
			break;		 
		case OP_LOOP_START:
			(void)printf("LOOP_START ");
			break;		 
		case OP_END_LOOP:
			(void)printf("END_LOOP   ");
			break;		 
		case OP_ERR2:
			(void)printf("INVALID 06 ");
			break;		 
		case OP_ERR3:
			(void)printf("INVALID 07 ");
			break;		 	  
	}
	(void)printf("%X\r\n",get_param(data));
}

void print_recipe(u8 *recipe)
{
	int i;
	for(i = 0; i < 255; i++)
	{
		print_recipe_line(recipe[i]);
		if (get_opcode(recipe[i]) == OP_RECIPE_END)
		{
		 	return;
		}
	}
}

// All programs are null terminated (same as OP_RECIPE_END)
const char prog0[] = { 0x24, 0x82, 0x23, 0x20, 0xA0, 0x42, 0x64, 0x00 };	  
const char prog1[] = { 0x22, 0x23, 0x83, 0x24, 0x81, 0xA0, 0x63, 0x44, 0x00 };

// examples from Project 2 PDF
const char example0[] = { 
	(OP_MOV << 5) | 0x00,
	(OP_MOV << 5) | 0x05,
	(OP_MOV << 5) | 0x00,
	(OP_RECIPE_END)
}; 
	   
const char example1[] = { 
	(OP_MOV << 5) | 0x03,
	(OP_LOOP_START << 5) | 0x08,
	(OP_MOV << 5) | 0x01,
	(OP_MOV << 5) | 0x04,
	(OP_END_LOOP << 5),
	(OP_MOV  << 5) | 0x00,
	(OP_RECIPE_END)
};

const char example2[] = {
	(OP_MOV << 5) | 0x02,
	(OP_WAIT << 5) | 0x00,
	(OP_MOV << 5) | 0x03,
	(OP_RECIPE_END)
};

const char example3[] = {
	(OP_MOV << 5) | 0x02,
	(OP_MOV << 5) | 0x03,
	(OP_WAIT << 5) | 0x1F,	 
	(OP_WAIT << 5) | 0x1F,
	(OP_WAIT << 5) | 0x1F,
	(OP_MOV << 5) | 0x04,
	(OP_RECIPE_END)
};

void timer_isr(void)
{
	u8 i;
	// Run the kernel on each servo
	for (i = 0; i < NUM_SERVOS; i++)
	{
		servo_kernel((servo *)(global_servos + i));
	}
}

int main(void)
{
	servo servos[NUM_SERVOS];

	u8 i; 
	u8 input_num;
	char user_input;
	char usr[NUM_SERVOS];
								 
	InitializeSerialPort();
	InitializeTimer();
	
	// Set up LEDs
	DDRB |= 0xF0;
	
	global_servos = servos;
	input_num = 0;
	
	for (i = 0; i < NUM_SERVOS; i++)
	{
		// Start assigning channels
		servo_init(&(servos[i]),i);
		pwm_init(servos[i].channel_num);
	}
	
	// Set up servos with the program and print out status
	servo_set_recipe(&(servos[0]), &prog1);
	servo_print_state(&(servos[0]));  
	print_recipe(servos[0].recipe); 
	
	servo_set_recipe(&(servos[1]), &prog0);
	servo_print_state(&(servos[1])); 
	print_recipe(servos[1].recipe);  
	
	for (;;)
	{	 
		// Take user input. Record only [n] servos' worth of chars.
		// Once we have enough chars and the user hits enter, reset the
		// input position index and run the servo state modifier block.
		user_input = GetChar();	 
		// Echo input
		(void)printf("%c", user_input);
		if (user_input == '\n' || user_input == '\r')
		{
			input_num = 0;
		}
		else if (input_num < NUM_SERVOS)
		{
			usr[input_num] = user_input;
			input_num++;
		}
		
		// Modify the servo state based on user input when user hits enter
		if (input_num == 0)
		{
			for (i = 0; i < NUM_SERVOS; i++)
			{
				usr[i] = usr[i] | 0x20;
				if (usr[i] == USER_RESTART)
				{
					servo_init((servo *)(servos + i),i); 
					servos[i].state = STATE_RUN;   
					(void)printf("\r\nServo #%d -> STATE_RUN",servos[i].channel_num);
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
							(void)printf("\r\nServo #%d -> STATE_RUN",servos[i].channel_num);
						}
						break;
					case STATE_RUN:
						if (usr[i] == USER_PAUSE)
						{
							servos[i].state = STATE_PAUSE;  
							(void)printf("\r\nServo #%d -> STATE_PAUSE",servos[i].channel_num);
						}
						break;
					case STATE_PAUSE:
						if (usr[i] == USER_LEFT && 
							servos[i].pos != SERVO_MIN_POS)
						{
							servos[i].pos -= 1;	 
						}
						else if (usr[i] == USER_RIGHT && 
							servos[i].pos != SERVO_MAX_POS)
						{
							servos[i].pos += 1;	 
						}	
						else if (usr[i] == USER_CONTINUE)
						{	
							servos[i].state = STATE_RUN; 
							(void)printf("\r\nServo #%d -> STATE_RUN",servos[i].channel_num);
							break;
						}	  						 			
						pwm_set(servos[i].channel_num,servos[i].pos);
						(void)printf("\r\nServo #%d Pos = %d",
							servos[i].channel_num,servos[i].pos,servos[i].pc);
						break;
					case STATE_ERROR:
						(void)printf("\r\nServo #%d has encountered an error - PC==%d",
							servos[i].channel_num,servos[i].pc);
					case STATE_END:
						(void)printf("\r\nServo #%d: Execution has ended",
							servos[i].channel_num,servos[i].pc);
						break;
				}
			}
			(void)printf("\r\n");
		}
	}
	return 0;
}