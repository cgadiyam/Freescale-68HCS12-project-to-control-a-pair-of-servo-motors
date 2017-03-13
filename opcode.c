/* 

Real-Time Operating Systems
Project 2
Michael Moffitt & Chaithanya Gadiyam
*/
#include "opcode.h"

u8 get_opcode(u8 byte)
{
	return byte >> 5;
}

u8 get_param(u8 byte)
{
	return byte & 0x1F;
}