#include "pwm.h"
#include "mc9s12dt256.h" 
#include <hidef.h>			/* common defines and macros */
#include "derivative.h" /* derivative-specific definitions */ 
#include <stdio.h>


void pwm_init(u8 channel)
{
	u8 i;

	// Enable specific channel
	PWME |= (1 << channel);	   
	
	// Don't really care about polarity but it is good to set it
	PWMPOL |= (1 << channel);	
	
	// Enable use of divided clock
	PWMCLK |= (1 << channel);
	
	// No prescale
	PWMPRCLK = 0x00;   
	
	// Set scale register to 78 dec
	PWMSCLA = 0x4E;					  
	PWMSCLB = 0x4E;
	
	// If more will be used, channels 4+ must be set as well
	PWMPER0 = 0xFF;
	PWMPER1 = 0xFF;
	PWMPER2 = 0xFF;
	PWMPER3 = 0xFF;     
	PWMPER4 = 0xFF;
	PWMPER5 = 0xFF;
	PWMPER6 = 0xFF;
	PWMPER7 = 0xFF; 
	
	// Set sane defaults so the poor servos don't kill themselves
	for (i = 0; i < 8; i++)
	{
	 	pwm_set(i,0);
	}
}

void pwm_set(u8 channel, u8 pos)
{
	u8 set_val = 13 + (pos * 3);  //Default to a non-bad number	 
//	PWMDTY2 = set_val;
//	return;
	
	switch (channel)
	{
		case 0:
			PWMDTY0 = set_val;
			break; 
		case 1:
			PWMDTY1 = set_val;
			break;
		case 2:
			PWMDTY2 = set_val;
			break;
		case 3:
			PWMDTY3 = set_val;
			break;
		case 4:
			PWMDTY4 = set_val;
			break;
		case 5:
			PWMDTY5 = set_val;
			break;
		case 6:
			PWMDTY6 = set_val;
			break;
		case 7:
			PWMDTY7 = set_val;
			break;
	}
}