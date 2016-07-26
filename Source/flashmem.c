/*  flashmem.c
 *
 *  Created on : Mar 19, 2016
 *  Author     : Erasmo Morales, MSCp.E
 *
 *  Description: Header File for use with the MSP430G3553 Flash Memory Controller.
 *
 *  Usage      : See flashmem.h for details and usage.
 */

#include "common.h"
#include "flashmem.h"
#include <stdarg.h>

//FUNCTIONS
void flashConfig(unsigned int clockSource, unsigned int clockDiv)
{
	FCTL2 = FWKEY + clockSource + clockDiv;

	return;
}

void flashInterrupts(iState emergencyExitInterrupt, ...)
{
	//Handle Arguements
	va_list args;
	va_start(args, emergencyExitInterrupt);
	iState eraseInterrupt  = va_arg(args, iState);
	iState accessViolation = va_arg(args, iState);
	va_end(args);

	if(emergencyExitInterrupt == enable_all)
	{
		FCTL1 = FWKEY + BIT3 + BIT4;
		return;
	}

	if(emergencyExitInterrupt == disable_all)
	{
		FCTL1 = FWKEY;
		return;
	}

	(emergencyExitInterrupt == disable) & (eraseInterrupt == disable) ? FCTL1 = FWKEY               : 0;
	(emergencyExitInterrupt == disable) & (eraseInterrupt == enable ) ? FCTL1 = FWKEY + BIT3        : 0;
	(emergencyExitInterrupt	== enable ) & (eraseInterrupt == disable) ? FCTL1 = FWKEY + BIT4        : 0;
	(emergencyExitInterrupt == enable ) & (eraseInterrupt == enable ) ? FCTL1 = FWKEY + BIT3 + BIT4 : 0;

	(accessViolation == enable) ? (IE1 |= ACCVIE) : (IE1 &= ~ACCVIE);

	return;
}

void flashWrite(void *data, int *address, ...)
{
	//Handle Arguements
	va_list args;
	va_start(args, address);
	volatile dataType writetype = va_arg(args, dataType);
	unsigned int    blocklength = va_arg(args, unsigned int);
	va_end(args);

	//Check and store state of Interrupts if enabled.
	boolie inturrupts_enabled = false;
	if((__get_SR_register() & GIE) == GIE)
	{
		inturrupts_enabled = true;
		_disable_interrupts();
	}

	 while(FCTL3 & BUSY);	//Check BUSY bit
	 FCTL3 = FWKEY;         //Clear Lock bit
	 FCTL1 = FWKEY + WRT;   //Set WRT bit for Write Operation

	switch(writetype)
	{
		case bit:
		{
			volatile char bits          = (unsigned char *)data;
			*(unsigned char *)address  &= ~bits;
			break;
		}
		case byte:
		{
			volatile char Byte         = (unsigned char *)data;
			*(unsigned char *)address  = Byte;
			break;
		}
		case word:
		{
			volatile int  Word         = (unsigned int  *)data;
			*(unsigned int *)address   = Word;
			break;
		}
		case block:
		{
			blocklength = (blocklength <=  0) ?  1 : blocklength; //if 0 is msitakenly entered.
			blocklength = (blocklength >  32) ? 32 : blocklength; //if value greter than 32 is msitakenly entered.

			//NOTE: Not a 'true' block write as those can only be done from RAM. Writes in word increments.

			int *a = (int *)data;
			int   i;
			for(i = blocklength; i > 0; i--)
			{
				*address++ = a[blocklength - i];
				while(FCTL3 & BUSY);							  //Check BUSY bit (check to see if done writing)
			}
		}
	}

	 while(FCTL3 & BUSY);	//Check BUSY bit
	 FCTL3 = FWKEY;         //Clear WRT  bit
	 FCTL3 = FWKEY + LOCK;  //Reset LOCK bit

	//Restore state of Interrupts if they were enabled.
	inturrupts_enabled == true ? _enable_interrupts() : 0;

	return;
}

void flashErase(int *address)
{
	//Check and store state of Interrupts if enabled.
	boolie inturrupts_enabled = false;
	if((__get_SR_register() & GIE) == GIE)
	{
		inturrupts_enabled = true;
		_disable_interrupts();
	}

	while(FCTL3 & BUSY);	//Check BUSY bit
	FCTL3 = FWKEY;          //Clear Lock bit
	FCTL1 = FWKEY + ERASE;  //Set ERASE bit for Erase Operation

	 *address = 0; 			//Dummy write to initiate erasing of Segment

	while(FCTL3 & BUSY);	//Check BUSY  bit
	FCTL3 = FWKEY;          //Clear ERASE bit
	FCTL3 = FWKEY + LOCK;   //Reset LOCK  bit

	//Restore state of Interrupts if they were enabled.
	inturrupts_enabled == true ? _enable_interrupts() : 0;

	return;
}

void flashCopy(int *address_S, int *address_D, ...)
{
	//Handle Arguements
	va_list args;
	va_start(args, address_D);
	volatile dataType  copytype = va_arg(args, dataType);
	unsigned int    blocklength = va_arg(args, unsigned int);
	va_end(args);

	//Check and store state of Interrupts if enabled.
	boolie inturrupts_enabled = false;
	if((__get_SR_register() & GIE) == GIE)
	{
		inturrupts_enabled = true;
		_disable_interrupts();
	}

	 while(FCTL3 & BUSY);	//Check BUSY bit
	 FCTL3 = FWKEY;         //Clear Lock bit
	 FCTL1 = FWKEY + WRT;   //Set WRT bit for Write Operation

	switch(copytype)
	{
		case bit :
		case byte:
		{
			volatile char Byte           = (unsigned char *)(*address_S);
			*(unsigned char *)address_D  = Byte;
			break;
		}
		case word:
		{
			volatile int  Word           = (unsigned int  *)(*address_S);
			*(unsigned int *)address_D   = Word;
			break;
		}
		case block:
		{
			blocklength = (blocklength <=  0) ?  1 : blocklength; //if 0 is msitakenly entered.
			blocklength = (blocklength >  32) ? 32 : blocklength; //if value greater than 32 is msitakenly entered.

			//NOTE: Not a 'true' block write as those can only be done from RAM. Writes in word increments.

			int   i;
			for(i = blocklength; i > 0; i--)
			{
				*address_D++ = (int *)(*address_S++);
				while(FCTL3 & BUSY);							  //Check BUSY bit (check to see if done writing)
			}
			break;
		}
		case segment:
		{
			//Correct Segment Address
			address_S = SegmentAddress(*(unsigned int *)&address_S);
			address_D = SegmentAddress(*(unsigned int *)&address_D);

			blocklength = 32;

			//NOTE: Not a 'true' block write as those can only be done from RAM. Writes in word increments.

			volatile unsigned int   i, j, s, k = 0;

			if( SegmentSizeCompare(address_S, address_D) == false ){ break; } //Exit if segments are mismatched.

			//Get Segment Size
			s = SegmentSize(address_S);
			j = (s == 512) ? 8 : 1;

			for(j = j; j > 0; j--)
			{
				for(i = blocklength; i > 0; i--)
				{
					*address_D++ = (int *)(*address_S++);
					while(FCTL3 & BUSY);	k++;					  //Check BUSY bit (check to see if done writing)
				}
				__delay_cycles(100);								  //Flash write cool down time.
			}
		}
	}

	 while(FCTL3 & BUSY);	//Check BUSY bit
	 FCTL3 = FWKEY;         //Clear WRT  bit
	 FCTL3 = FWKEY + LOCK;  //Reset LOCK bit

	//Restore state of Interrupts if they were enabled.
	inturrupts_enabled == true ? _enable_interrupts() : 0;

	return;
}

void flashMove(int *address_S, int *address_D, ...)
{
	//Handle Arguements
	va_list args;
	va_start(args, address_D);
	volatile dataType  movetype = va_arg(args, dataType);
	unsigned int    blocklength = va_arg(args, unsigned int);
	va_end(args);

	switch(movetype)
	{
		case bit :
		case byte:
		{
			flashCopy(address_S, address_D, byte, 0);
			break;
		}
		case word:
		{
			flashCopy(address_S, address_D, word, 0);
			break;
		}
		case block:
		{
			flashCopy(address_S, address_D, block, blocklength);
			break;
		}
		case segment:
		{
			//Correct Segment Address
			address_S = SegmentAddress(*(unsigned int *)&address_S);
			address_D = SegmentAddress(*(unsigned int *)&address_D);

			if( SegmentSizeCompare(address_S, address_D) == false ){ return; } //Exit if segments are mismatched.

			flashCopy(address_S, address_D, segment, 0);
		}
	}

	flashErase(address_S);

	return;
}

boolie isItEmpty(int *address, ...)
{
	//Handle Arguements
	va_list args;
	va_start(args, address);
	volatile dataType emptytype = va_arg(args, dataType);
	va_end(args);

	switch(emptytype)
	{
		case bit :
		case byte:
		{
			if(*(unsigned char *)address == 0xFF)  { return true; }
			break;
		}
		case word:
		{
			if(*(unsigned int *)address  == 0xFFFF){ return true; }
		}
	}

	return false;
}

unsigned int areTheyEmpty(int *address, ...)
{
	//Handle Arguements
	va_list args;
	va_start(args, address);
	volatile dataType  emptytype   = va_arg(args, dataType);
	unsigned int       blocklength = va_arg(args, unsigned int);
	va_end(args);

	switch(emptytype)
	{
		case bit :
		case byte:
		case block:
		{
			blocklength = (blocklength <=  0) ?  1 : blocklength; //if 0 is msitakenly entered.
			blocklength = (blocklength >  32) ? 32 : blocklength; //if value greater than 32 is msitakenly entered.

			int   i;
			for(i = blocklength; i > 0; i--)
			{
				if(*(unsigned int *)address != 0xFFFF)  { return *(unsigned int *)&address; }else{ *address++; }
			}
			break;
		}
		case segment:
		{
			//Correct Segment Address
			address = SegmentAddress(*(unsigned int *)&address);

			blocklength = 32;

			volatile unsigned int   i, j, s;

			if( SegmentSizeCompare(address, address) == false ){ break; } //Exit if segment doesn't belong to Code / Data or Information Sections.

			//Get Segment Size
			s = SegmentSize(address);
			j = (s == 512) ? 8 : 1;

			for(j = j; j > 0; j--)
			{
				for(i = blocklength; i > 0; i--)
				{
					if(*(unsigned int *)address != 0xFFFF)  { return *(unsigned int *)&address; }else{ *address++; }
				}
			}
		}
	}

	return 0x0000;
}

unsigned char whereAreU(int *address)
{
	volatile unsigned int  where  = *(unsigned int *)&address;
	volatile unsigned char there  = 0xFF; //Unaddressable

	//Code / Data
	if     ((unsigned int)Segment0		<= where && where <=	                0xFFFF) { there = 0x00; }
	else if((unsigned int)Segment1		<= where && where < 	(unsigned int)Segment0) { there = 0x01; }
	else if((unsigned int)Segment2		<= where && where < 	(unsigned int)Segment1) { there = 0x02; }
	else if((unsigned int)Segment3		<= where && where < 	(unsigned int)Segment2) { there = 0x03; }
	else if((unsigned int)Segment4		<= where && where < 	(unsigned int)Segment3) { there = 0x04; }
	else if((unsigned int)Segment5		<= where && where < 	(unsigned int)Segment4) { there = 0x05; }
	else if((unsigned int)Segment6		<= where && where < 	(unsigned int)Segment5) { there = 0x06; }
	else if((unsigned int)Segment7		<= where && where < 	(unsigned int)Segment6) { there = 0x07; }
	else if((unsigned int)Segment8		<= where && where < 	(unsigned int)Segment7) { there = 0x08; }
	else if((unsigned int)Segment9		<= where && where < 	(unsigned int)Segment8) { there = 0x09; }
	else if((unsigned int)Segment10		<= where && where < 	(unsigned int)Segment9) { there = 0x10; }
	else if((unsigned int)Segment11		<= where && where < 	(unsigned int)Segment10){ there = 0x11; }
	else if((unsigned int)Segment12		<= where && where < 	(unsigned int)Segment11){ there = 0x12; }
	else if((unsigned int)Segment13		<= where && where < 	(unsigned int)Segment12){ there = 0x13; }
	else if((unsigned int)Segment14		<= where && where < 	(unsigned int)Segment13){ there = 0x14; }
	else if((unsigned int)Segment15		<= where && where < 	(unsigned int)Segment14){ there = 0x15; }
	else if((unsigned int)Segment16		<= where && where < 	(unsigned int)Segment15){ there = 0x16; }
	else if((unsigned int)Segment17		<= where && where < 	(unsigned int)Segment16){ there = 0x17; }
	else if((unsigned int)Segment18		<= where && where < 	(unsigned int)Segment17){ there = 0x18; }
	else if((unsigned int)Segment19		<= where && where < 	(unsigned int)Segment18){ there = 0x19; }
	else if((unsigned int)Segment20		<= where && where < 	(unsigned int)Segment19){ there = 0x20; }
	else if((unsigned int)Segment21		<= where && where < 	(unsigned int)Segment20){ there = 0x21; }
	else if((unsigned int)Segment22		<= where && where < 	(unsigned int)Segment21){ there = 0x22; }
	else if((unsigned int)Segment23		<= where && where < 	(unsigned int)Segment22){ there = 0x23; }
	else if((unsigned int)Segment24		<= where && where < 	(unsigned int)Segment23){ there = 0x24; }
	else if((unsigned int)Segment25		<= where && where < 	(unsigned int)Segment24){ there = 0x25; }
	else if((unsigned int)Segment26		<= where && where < 	(unsigned int)Segment25){ there = 0x26; }
	else if((unsigned int)Segment27		<= where && where < 	(unsigned int)Segment26){ there = 0x27; }
	else if((unsigned int)Segment28		<= where && where < 	(unsigned int)Segment27){ there = 0x28; }
	else if((unsigned int)Segment29		<= where && where < 	(unsigned int)Segment28){ there = 0x29; }
	else if((unsigned int)Segment30		<= where && where < 	(unsigned int)Segment29){ there = 0x30; }
	else if((unsigned int)Segment31		<= where && where < 	(unsigned int)Segment30){ there = 0x31; }

	//Information
	else if((unsigned int)SegmentA		<= where && where <= 	                 0x10FF){ there = 0x0A; }
	else if((unsigned int)SegmentB		<= where && where <  	 (unsigned int)SegmentA){ there = 0x0B; }
	else if((unsigned int)SegmentC		<= where && where < 	 (unsigned int)SegmentB){ there = 0x0C; }
	else if((unsigned int)SegmentD		<= where && where < 	 (unsigned int)SegmentC){ there = 0x0D; }

	//RAM
	else if(0x0200                  	<= where && where <= 	                 0x03FF){ there = 0xA0; }

	//Peripherals - 16bit
	else if(0x0100                  	<= where && where <= 	                 0x01FF){ there = 0xFB; }

	//Peripherals -  8bit
	else if(0x0010                  	<= where && where <= 	                 0x00FF){ there = 0x8B; }

	//Peripherals -  8bit SFR
	else if(0x0000                  	<= where && where <= 	                 0x000F){ there = 0x88; }

	return there; //There you are!
}

int * SegmentAddress(int *address)
{
	char  addresscheck = whereAreU(address);
	int * segment	   = (int *)(0x00FFu); //Unaddressable

	//Code / Data
	if     (addresscheck == 0x00){ segment = Segment0;  }
	else if(addresscheck == 0x01){ segment = Segment1;  }
	else if(addresscheck == 0x02){ segment = Segment2;  }
	else if(addresscheck == 0x03){ segment = Segment3;  }
	else if(addresscheck == 0x04){ segment = Segment4;  }
	else if(addresscheck == 0x05){ segment = Segment5;  }
	else if(addresscheck == 0x06){ segment = Segment6;  }
	else if(addresscheck == 0x07){ segment = Segment7;  }
	else if(addresscheck == 0x08){ segment = Segment8;  }
	else if(addresscheck == 0x09){ segment = Segment9;  }
	else if(addresscheck == 0x10){ segment = Segment10; }
	else if(addresscheck == 0x11){ segment = Segment11; }
	else if(addresscheck == 0x12){ segment = Segment12; }
	else if(addresscheck == 0x13){ segment = Segment13; }
	else if(addresscheck == 0x14){ segment = Segment14; }
	else if(addresscheck == 0x15){ segment = Segment15; }
	else if(addresscheck == 0x16){ segment = Segment16; }
	else if(addresscheck == 0x17){ segment = Segment17; }
	else if(addresscheck == 0x18){ segment = Segment18; }
	else if(addresscheck == 0x19){ segment = Segment19; }
	else if(addresscheck == 0x20){ segment = Segment20; }
	else if(addresscheck == 0x21){ segment = Segment21; }
	else if(addresscheck == 0x22){ segment = Segment22; }
	else if(addresscheck == 0x23){ segment = Segment23; }
	else if(addresscheck == 0x24){ segment = Segment24; }
	else if(addresscheck == 0x25){ segment = Segment25; }
	else if(addresscheck == 0x26){ segment = Segment26; }
	else if(addresscheck == 0x27){ segment = Segment27; }
	else if(addresscheck == 0x28){ segment = Segment28; }
	else if(addresscheck == 0x29){ segment = Segment29; }
	else if(addresscheck == 0x30){ segment = Segment29; }
	else if(addresscheck == 0x31){ segment = Segment29; }

	//Information
	else if(addresscheck == 0x0A){ segment = SegmentA;  }
	else if(addresscheck == 0x0B){ segment = SegmentB;  }
	else if(addresscheck == 0x0C){ segment = SegmentC;  }
	else if(addresscheck == 0x0D){ segment = SegmentD;  }

	//RAM
	else if(addresscheck == 0xA0){ segment = (int *)(0x0200); }

	//Peripherals - 16bit
	else if(addresscheck == 0xFB){ segment = (int *)(0x0100); }

	//Peripherals -  8bit
	else if(addresscheck == 0x8B){ segment = (int *)(0x0010); }

	//Peripherals -  8bit SFR
	else if(addresscheck == 0x88){ segment = (int *)(0x0000); }

	return segment;
}

unsigned int SegmentSize(int *address)
{
	//Correct Segment Address
	address = SegmentAddress(*(unsigned int *)&address);

	volatile unsigned int size = 0;

	//If address belongs to an addressable segment in the Code / Data area, it must be 512 bytes.
	if     ( (unsigned int)Segment31 <= *(unsigned int *)&address && *(unsigned int *)&address <= 0xFFFF ){ size = 512; }

	//If address belongs to an addressable segment in the Information area, it must be 64 bytes.
	else if( (unsigned int)SegmentD  <= *(unsigned int *)&address && *(unsigned int *)&address <= 0x10C0 ){ size =  64; }

	//Maybe its...

	//RAM
	else if( *(unsigned int *)&address == 0x0200 ){ size = 512;  }

	//Peripherals - 16bit
	else if( *(unsigned int *)&address == 0x0100 ){ size = 256;  }

	//Peripherals - 8bit
	else if( *(unsigned int *)&address == 0x0010 ){ size = 240;  }

	//Peripherals - 8bit SFR
	else if( *(unsigned int *)&address == 0x0000 ){ size =  16;  }

	return size;
}

boolie SegmentSizeCompare(int *address_A, int *address_B)
{
	//Correct Segment Address
	address_A = SegmentAddress(*(unsigned int *)&address_A);
	address_B = SegmentAddress(*(unsigned int *)&address_B);

	volatile unsigned int sizeA, sizeB;

	//If address belongs to an addressable segment in the Code / Data area, it must be 512 bytes.
	sizeA = ( (unsigned int)Segment31 <= *(unsigned int *)&address_A &&	*(unsigned int *)&address_A <= 0xFFFF ) ? 512 : 0;
	sizeB = ( (unsigned int)Segment31 <= *(unsigned int *)&address_B &&	*(unsigned int *)&address_B <= 0xFFFF ) ? 512 : 0;

	//If address belongs to an addressable segment in the Information area, it must be 64 bytes.
	if(sizeA == 0){ sizeA = ( (unsigned int)SegmentD <= *(unsigned int *)&address_A && *(unsigned int *)&address_A <= 0x10C0 ) ? 64 : 0; }
	if(sizeB == 0){ sizeB = ( (unsigned int)SegmentD <= *(unsigned int *)&address_B && *(unsigned int *)&address_B <= 0x10C0 ) ? 64 : 0; }

	//Return false if one or neither address belongs to an addressable segment.
	if( sizeA == 0 || sizeB == 0 ){ return false;}

	//If segment sizes of each address is the same, return true.
	if(sizeA == sizeB){ return true; }else{ return false; }
}

void flashRead(volatile int *data, int *address, ...)
{
	//Handle Arguements
	va_list args;
	va_start(args, address);
	volatile dataType readtype = va_arg(args, dataType);
	unsigned int    blocklength = va_arg(args, unsigned int);
	va_end(args);

	//Check and store state of Interrupts if enabled.
	boolie inturrupts_enabled = false;
	if((__get_SR_register() & GIE) == GIE)
	{
		inturrupts_enabled = true;
		_disable_interrupts();
	}

	 while(FCTL3 & BUSY);	//Check BUSY bit
	 FCTL3 = FWKEY;         //Clear Lock bit
	 FCTL1 = FWKEY + WRT;   //Set WRT bit for Write Operation

	switch(readtype)
	{
		case bit:
		case byte:
		{
			volatile char Byte         = *(unsigned char *)address;
			char *datac   = (char *)data;
			*datac        = Byte;
			break;
		}
		case word:
		{
			volatile int  Word         = *(unsigned int  *)address;
			*data  = Word;
			break;
		}
		case block:
		{
			blocklength = (blocklength <=  0) ?  1 : blocklength; //if 0 is msitakenly entered.
			blocklength = (blocklength >  32) ? 32 : blocklength; //if value greter than 32 is msitakenly entered.

			//NOTE: Not a 'true' block write as those can only be done from RAM. Writes in word increments.

			int *d = (int *)data;
			int   i;
			for(i = blocklength; i > 0; i--)
			{
				d[blocklength - i] = *(address++);
				while(FCTL3 & BUSY);							  //Check BUSY bit (check to see if done writing)
			}
		}
	}

	 while(FCTL3 & BUSY);	//Check BUSY bit
	 FCTL3 = FWKEY;         //Clear WRT  bit
	 FCTL3 = FWKEY + LOCK;  //Reset LOCK bit

	//Restore state of Interrupts if they were enabled.
	inturrupts_enabled == true ? _enable_interrupts() : 0;

	return;
}
////////////////////////////////////***************************************************////////////////////////////////////
////////////////////////////////////*		  ...           ...           ... 		  *////////////////////////////////////
////////////////////////////////////*		 (o O)         (O o)         ( oo)     	  *////////////////////////////////////
////////////////////////////////////*   -ooO--(_)--Ooo-ooO--(_)--Ooo-ooO--(_)--Ooo-   *////////////////////////////////////
////////////////////////////////////***************************************************////////////////////////////////////
