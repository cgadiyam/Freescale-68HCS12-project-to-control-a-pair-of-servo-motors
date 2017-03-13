/* 

Real-Time Operating Systems
Project 2
Michael Moffitt & Chaithanya Gadiyam
*/
#ifndef OPCODE_H
#define OPCODE_H

#include "types.h"
				  
#define OP_RECIPE_END 0x00
#define OP_MOV 0x01
#define OP_WAIT 0x02	  
#define OP_MOVC 0x03
#define OP_LOOP_START 0x04
#define OP_END_LOOP 0x05  
#define OP_ERR2 0x07
#define OP_ERR3 0x06

u8 get_opcode(u8 byte);
u8 get_param(u8 byte);

#endif