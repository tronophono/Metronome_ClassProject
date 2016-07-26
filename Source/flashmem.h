/*  flashmem.h
 *
 *  Created on : Mar 19, 2016
 *  Author     : Erasmo Morales, MSCp.E
 *
 *  Description: Header File for use with the MSP430G3553 Flash Memory Controller.
 *
 *
 */
/**********************************************************************************************************************
***********************************************************************************************************************
**************************   _____  _   _  _   _  _____  _____  _____  _____  _   _  _____   **************************
**************************  |  ___|| | | || \ | |/  __ \|_   _||_   _||  _  || \ | |/  ___|  **************************
**************************  | |_   | | | ||  \| || /  \/  | |    | |  | | | ||  \| |\ `--.   **************************
**************************  |  _|  | | | || . ` || |      | |    | |  | | | || . ` | `--. \  **************************
**************************  | |    | |_| || |\  || \__/\  | |   _| |_ \ \_/ /| |\  |/\__/ /  **************************
**************************  \_|     \___/ \_| \_/ \____/  \_/   \___/  \___/ \_| \_/\____/   **************************
************************** 															         **************************
***********************************************************************************************************************
**********************************************************************************************************************/
/*
 >>> Flash Configure Function: flashConfig(clockSource, clockDiv);
 *
 *		Choose a clock source and divisor.
 *
 *		clockSource: FSSEL_0, FSSEL_1, FSSEL_2, FSSEL_3 <==> ACLK, MCLK, SMCLK, SMCLK
 *		clockDiv   : FN0, .., FN63 <==> /1, ..., /64
 *
 *		Returns: Nothing
 *
 *		Example: flashConfig(FSSEL_3, FN0); // SMCLK, /1
 *
 *
 >>> Flash Interrupts enable: flashInterrupts(emergencyExitInterrupt, [eraseInterrupt, accessViolation]); [optional]
 *
 *		Choose to enable various flash interrupts.
 *
 *		emergencyExitInterrupt: enable_all, disable_all, (disable), enable 								 (default)
 *			Halt any flash operation. Must reenable if interrupt is triggered.
 *
 *		eraseInterrupt		  : (disable), enable 								 						 (default)
 *			Allow an erase operation to be interrupted by any other interrupt.
 *			Will resume when interrupting interrupt is serviced.
 *
 *		accessViolation		  : (disable), enable 														 (default)
 *			Sets the ACCVIFG bit in FTCL3 when an access violation occurs.
 *			Sources the NMI interrupt vector. See the userguide.
 *
 *		Returns: Nothing
 *
 *		Example:  flashInterrupts(enable, disable, disable);
 *
 *
 >>> Invoke Flash Operation Emergency Exit: _emergencyEXIT;
 *
 *		Halt any flash operation. Must reenable interrupt using flashInterrupts().
 *
 *		Returns: Nothing
 *
 *		Example: _emergencyEXIT; //Any flash operation halts here.
 *
 *		Note: Used with other interrupts. Write/Erase results are unpredictable; 0 will
 *			  most likely be written to address.
 *
 *
 >>> Flash Write: flashWrite(data, address, [writetype, blocklength]); [optional]
 *
 *		Write data to a flash memory address.
 *
 *		data        : <any bit pattern (only 0's can be written), byte, word or block (array)>
 *		address     : <any 16bit address (see MISC Section for details)>
 *		writetype   : (bit), byte, word, block (for arrays)										(default)
 *		blocklength : (1), ..., 64 (ignored for anything other than block write type)           (default)
 *
 *		Returns: Nothing
 *
 *		Example: flashWrite(0x11, 0xEC00, byte, 0); //Write a byte to beginning of Segment 9
 *
 *		Note: Blocks are written in word increments. If you wish to move bytes, group your data
 *			  into words in an array. Example: 0x01 and 0x02 -> 0x0201
 *
 *
 >>> Flash Erase: flashErase(address);
 *
 *		Erase the segment in which the address resides. When erasing data, the whole segment must be
 *		erased; erased data is indicated by all 1's.
 *
 *		address: <address to check>
 *
 *		Returns: Nothing
 *
 *		Example: flashErase(0XEC50); //Erases Segment 9
 *
 *
 >>> Flash Copy: flashCopy(address_S, address_D, [copytype, blocklength]); [optional]
 *
 *		Copy data from one address to another.
 *
 *		address_S	: <Source address>
 *		address_D 	: <Destination address>
 *		copytype	: (byte), word, block, segment                                          (default)
 *		blocklength : (1), ..., 64 (ignored for anything other than block copy type)        (default)
 *
 *		Returns: Nothing but exits without doing anything if Segment sizes do not match,...
 *				 ...(see MISC Section for details)
 *
 *		Example: flashCopy(0xEC00, 0xD000, segment, 0); //Copy all data from Segment 9 to Segment 23
 *
 *		Note: Blocks are written in word increments. If you wish to move bytes, group your data
 *			  into words in an array. Example: 0x01 and 0x02 -> 0x0201
 *
 *
 >>> Flash Move: flashMove(address_S, address_D, [copytype, blocklength]); [optional]
 *
 *		Move data from one address to another. Source is erased.
 *
 *		address_S	: <Source address>
 *		address_D 	: <Destination address>
 *		copytype	: (byte), word, block, segment											(default)
 *		blocklength : (1), ..., 64 (ignored for anything other than block move type)        (default)
 *
 *		Returns: Nothing but exits without doing anything if Segment sizes do not match,...
 *				 ...(see MISC Section for details)
 *
 *		Example: flashMove(0xD000, 0xD200, segment, 0); //Move all data from Segment 23 to Segment 22
 *
 *		Note: Blocks are written in word increments. If you wish to move bytes, group your data
 *			  into words in an array. Example: 0x01 and 0x02 -> 0x0201
 *
 *
 >>> Check If Empty: isItEmpty(address, [emptytype]); [optional]
 *
 *		Check if a byte or word located at an address is empty.
 *
 *		address	  : <address to check>
 *		emptytype : (byte), word															(default)
 *
 *		Returns: true if address is empty; false if address contains data
 *
 *		Example: isItEmpty(0XF000, word); //Check if a word exists at address 0XF000
 *
 *		Note: Checks against 0xFF and 0xFFFF for byte and word respectively.
 *			  If set to check for a word using an odd numbered address, the even numbered address below
 *			  it will automatically be selected the to check the word; if checking a byte using an
 *			  odd numbered address, the address will be checked as expected.
 *
 *
 >>> Check If Mass Addresses Are Empty: areTheyEmpty(address, [emptytype, blocklength]); [optional]
 *
 *		Check to see if a block of addresses or a segment is empty.
 *
 *		address		: <address to start checking>
 *		emptytype	: (block), segment														(default)
 *		blocklength : (1), ..., 64 (ignored for anything other than block empty type)       (default)
 *
 *		Returns: Address of first non empty word located in the selected block or segment.
 *
 *		Example: areTheyEmpty(0XF000, block, 10); //Check 10 words starting at 0xF000 for any data
 *
 *		Note: Checks against 0xFF and 0xFFFF for byte and word respectively.
 *			  Only even addresses are return since blocks and segments are check per word.
 *			  Bytes will still be detected. Example: 0xFF11 in 0XF000 would return 0xF000, not 0xF001.
 *
 *
 >>> Return Home Segment: whereAreU(address);
 *
 *		Find the current Segment in which an address resides.
 *
 *		address: <address to check>
 *
 *		Returns: A hex byte code indicating the home segment of the address. See below:
 *
 *							    <Main Memory - Code / Data>
 *	 					Segment0 	 					   = 0x00;
 *	 					Segment1 	 					   = 0x01;
 *	 					Segment2  	 					   = 0x02;
 *	 					Segment3 	 					   = 0x03;
 *	 					Segment4 	 				       = 0x04;
 *	 					Segment5 	 					   = 0x05;
 *	 					Segment6 	 					   = 0x06;
 *	 					Segment7  	 					   = 0x07;
 *	 					Segment8 	 					   = 0x08;
 *	 					Segment9	 					   = 0x09;
 *	 					Segment10	 					   = 0x10;
 *	 					Segment11	 					   = 0x11;
 *	 					Segment12	 					   = 0x12;
 *	 					Segment13	 					   = 0x13;
 *	 					Segment14	 					   = 0x14;
 *	 					Segment15	 					   = 0x15;
 *	 					Segment16	 					   = 0x16;
 *	 					Segment17	 					   = 0x17;
 *	 					Segment18	 					   = 0x18;
 *	 					Segment19	 					   = 0x19;
 *	 					Segment20	 					   = 0x20;
 *	 					Segment21	 					   = 0x21;
 *	 					Segment22	 					   = 0x22;
 *	 					Segment23	 					   = 0x23;
 *	 					Segment24	 					   = 0x24;
 *	 					Segment25	 					   = 0x25;
 *	 					Segment26	 					   = 0x26;
 *	 					Segment27	 					   = 0x27;
 *	 					Segment28	 					   = 0x28;
 *	 					Segment29	 					   = 0x29;
 *	 					Segment30	 					   = 0x30;
 *	 					Segment31	 					   = 0x31;
 *
 *									    <Information>
 *						SegmentA	 					   = 0x0A;
 *						SegmentB	 					   = 0x0B;
 *						SegmentC	 					   = 0x0C;
 *						SegmentD	 					   = 0x0D;
 *
 *										    <RAM>
 *						RAM 							   = 0xA0;
 *
 *								    <Peripherals - 16bit>
 *						Peripherals - 16bit				   = 0xFB;
 *
 *								    <Peripherals -  8bit>
 *						Peripherals -  8bit				   = 0x8B;
 *
 *							      <Peripherals -  8bit SFR>
 *						Peripherals -  8bit SFR			   = 0x88;
 *
 *		Example: whereAreU(0xDC50); //Returns 0x17 - Segment 17
 *
 *
 >>> Find Segment Address: SegmentAddress(address);
 *
 *		Find the start address of the Segment in which an address resides.
 *
 *		address: <address to check>
 *
 *		Returns: The start address of the Segment in which an address resides. See below:
 *
 *							    <Main Memory - Code / Data>
 *	 					Segment0 	 					 = 0xFE00;
 *	 					Segment1 	 					 = 0xFC00;
 *	 					Segment2  	 					 = 0xFA00;
 *	 					Segment3 	 					 = 0xF800;
 *	 					Segment4 	 				     = 0xF600;
 *	 					Segment5 	 					 = 0xF400;
 *	 					Segment6 	 					 = 0xF200;
 *	 					Segment7  	 					 = 0xF000;
 *	 					Segment8 	 					 = 0xEE00;
 *	 					Segment9	 					 = 0xEC00;
 *	 					Segment10	 					 = 0xEA00;
 *	 					Segment11	 					 = 0xE800;
 *	 					Segment12	 					 = 0xE600;
 *	 					Segment13	 					 = 0xE400;
 *	 					Segment14	 					 = 0xE200;
 *	 					Segment15	 					 = 0xE000;
 *	 					Segment16	 					 = 0xDE00;
 *	 					Segment17	 					 = 0xDC00;
 *	 					Segment18	 					 = 0xDA00;
 *	 					Segment19	 					 = 0xD800;
 *	 					Segment20	 					 = 0xD600;
 *	 					Segment21	 					 = 0xD400;
 *	 					Segment22	 					 = 0xD200;
 *	 					Segment23	 					 = 0xD000;
 *	 					Segment24	 					 = 0xCE00;
 *	 					Segment25	 					 = 0xCC00;
 *	 					Segment26	 					 = 0xCA00;
 *	 					Segment27	 					 = 0xC800;
 *	 					Segment28	 					 = 0xC600;
 *	 					Segment29	 					 = 0xC400;
 *	 					Segment30	 					 = 0xC200;
 *	 					Segment31	 					 = 0xC000;
 *
 *									    <Information>
 *						SegmentA	 					 = 0x10C0;
 *						SegmentB	 					 = 0x1080;
 *						SegmentC	 					 = 0x1040;
 *						SegmentD	 					 = 0x1000;
 *
 *										    <RAM>
 *						RAM 							 = 0x0200;
 *
 *								    <Peripherals - 16bit>
 *						Peripherals - 16bit				 = 0x0100;
 *
 *								    <Peripherals -  8bit>
 *						Peripherals -  8bit				 = 0x0010;
 *
 *							      <Peripherals -  8bit SFR>
 *						Peripherals -  8bit SFR		     = 0x0000;
 *
 *		Example: SegmentAddress(0xDA57); //Returns 0xDA00 - Segment 18
 *
 *		Note: If entered address does not belong to the memory map, 0x00FF is returned.
 *
 *
 >>> Find Segment Size: SegmentSize(address);
 *
 *		Find the size of the Segment in which an address resides.
 *
 *		address: <address to check>
 *
 *		Returns: The size in bytes (integer) of the Segment in which an address resides. See below:
 *
 *							    <Main Memory - Code / Data>
 *	 					Segments 0 - 31					   = 512;
 *
 *									    <Information>
 *						Segments A - D 					   =  64;
 *
 *										    <RAM>
 *						RAM 							   = 512;
 *
 *								    <Peripherals - 16bit>
 *						Peripherals - 16bit				   = 256;
 *
 *								    <Peripherals -  8bit>
 *						Peripherals -  8bit				   = 240;
 *
 *							      <Peripherals -  8bit SFR>
 *						Peripherals -  8bit SFR			   =  16;
 *
 *		Example: whereAreU(0xF123); //Returns 512 - Segment 7
 *
 *		Note: If entered address does not belong to the memory map, 0 is returned.
 *
 *
 >>> Compare Segments: SegmentSizeCompare(address_A, address_B);
 *
 *		Compare two segments to see if they are the same size. Will only compare Code / Data and
 *		Information segments.
 *
 *		address_A : <Address residing in a Segment.>
 *		address_B : <Address residing in a different (or same) Segment than address_A>.
 *
 *		Returns: False if addresses belong to Segments of differing sizes; true if they belong to
 *				 Segments of equal sizes.
 *
 *		Example: SegmentSizeCompare(SegmentA, Segment0); //Returns false; 64 != 512
 *
 *		Note: Only Code / Data and Information segments will be compared; used to determine if data copy
 *		      or move between two Segments is possible, which can only occur if the Segments are the
 *			  same size.
 *
 *
 >>> Flash Read: flashRead(data, address, [readtype, blocklength]); [optional]
 *
 *		Write data to a flash memory address.
 *
 *		data        : <byte, word or block (array)>
 *		address     : <any 16bit address (see MISC Section for details)>
 *		readtype   : (byte), word, block (for arrays)										    (default)
 *		blocklength : (1), ..., 64 (ignored for anything other than block write type)           (default)
 *
 *		Returns: Nothing. Stores data from an address and stores into a variable
 *
 *		Example: flashRead(&data, 0xEC00, byte, 0); //Read a byte of data from 0xEC00 - Segment 9 in to variable 'data'
 *
 *		Note: You must use the '&' dereference (AKA: 'the address of pointer') operator when passing a variable name.
 *
 *
 */
/**********************************************************************************************************************
***********************************************************************************************************************
*******************************************  ___  ___ _____  _____  _____      ****************************************
*******************************************  |  \/  ||_   _|/  ___|/  __ \     ****************************************
*******************************************  | .  . |  | |  \ `--. | /  \/     ****************************************
*******************************************  | |\/| |  | |   `--. \| |         ****************************************
*******************************************  | |  | | _| |_ /\__/ /| \__/\ _   ****************************************
*******************************************  \_|  |_/ \___/ \____/  \____/(_)  ****************************************
*******************************************  								   ****************************************
***********************************************************************************************************************
**********************************************************************************************************************/
/*
 *	>>> Trying to write incorrect data to an address will corrupt it, resulting in all zeros (0x0000).
 *
 *	>>> The Interrupt Vectors and Code/Data sections overlap.
 *
 *	>>> When writing word data into an address, it is stored LSB first.
 *		Example: data = 0x0201 into address = 0xF000
 *				 [0xF000](0x01) [0xF001](0x02)
 *				 		  LSB             MSB
 *
 *	>>> The lower the Segment Designation, the higher the memory address:
 *		Example: Segment0 > Segment31 : 0xFE00 > 0xC000
 *
 *	>>> Only Segments of the same size can be moved or copied to:
 *		Example: Code / Data Segments (Segment0 - 31) = 512 and Information Segments (SegmentsA - D) = 64
 *
 *		Note that Segment A is locked. It can be unlocked but it contains calibration data that you will miss
 *		if you delete it. It CANNOT be recovered.
 *
 */
/**********************************************************************************************************************
***********************************************************************************************************************
******************  ______  _____ ______  _____  _   _  _____  _____  _____  _____  _   _  _____   ********************
******************  |  _  \|  ___||  ___||_   _|| \ | ||_   _||_   _||_   _||  _  || \ | |/  ___|  ********************
******************  | | | || |__  | |_     | |  |  \| |  | |    | |    | |  | | | ||  \| |\ `--.   ********************
******************  | | | ||  __| |  _|    | |  | . ` |  | |    | |    | |  | | | || . ` | `--. \  ********************
******************  | |/ / | |___ | |     _| |_ | |\  | _| |_   | |   _| |_ \ \_/ /| |\  |/\__/ /  ********************
******************  |___/  \____/ \_|     \___/ \_| \_/ \___/   \_/   \___/  \___/ \_| \_/\____/   ********************
****************** 																			       ********************
***********************************************************************************************************************
**********************************************************************************************************************/

#ifndef FLASHMEM_H_
#define FLASHMEM_H_

#include <msp430g2553.h>
#include "common.h"

//FUNCTIONS
#define _emergencyEXIT  FCTL3 = FWKEY + EMEX
void flashConfig(unsigned int clockSource, unsigned int clockDiv);
void flashInterrupts(iState emergencyExitInterrupt, ...);
void flashWrite(void *data, int *address, ...);
void flashErase(int *address);
void flashCopy(int *address_S, int *address_D, ...);
void flashMove(int *address_S, int *address_D, ...);
boolie            isItEmpty(int *address, ...);
unsigned int      areTheyEmpty(int *address, ...);
unsigned char     whereAreU(int *address);
int *             SegmentAddress(int *address);
unsigned int	  SegmentSize(int *address);
boolie		      SegmentSizeCompare(int *address_A, int *address_B);
void flashRead(volatile int *data, int *address, ...);

//SEGMENT DEFINITIONS

//Main Memory - Code / Data
#define Segment0  (int *)(0xFE00u) //Interrupt Vectors Start at 0xFFC0
#define Segment1  (int *)(0xFC00u)
#define Segment2  (int *)(0xFA00u)
#define Segment3  (int *)(0xF800u)
#define Segment4  (int *)(0xF600u)
#define Segment5  (int *)(0xF400u)
#define Segment6  (int *)(0xF200u)
#define Segment7  (int *)(0xF000u)
#define Segment8  (int *)(0xEE00u)
#define Segment9  (int *)(0xEC00u)
#define Segment10 (int *)(0xEA00u)
#define Segment11 (int *)(0xE800u)
#define Segment12 (int *)(0xE600u)
#define Segment13 (int *)(0xE400u)
#define Segment14 (int *)(0xE200u)
#define Segment15 (int *)(0xE000u)
#define Segment16 (int *)(0xDE00u)
#define Segment17 (int *)(0xDC00u)
#define Segment18 (int *)(0xDA00u)
#define Segment19 (int *)(0xD800u)
#define Segment20 (int *)(0xD600u)
#define Segment21 (int *)(0xD400u)
#define Segment22 (int *)(0xD200u)
#define Segment23 (int *)(0xD000u)
#define Segment24 (int *)(0xCE00u)
#define Segment25 (int *)(0xCC00u)
#define Segment26 (int *)(0xCA00u)
#define Segment27 (int *)(0xC800u)
#define Segment28 (int *)(0xC600u)
#define Segment29 (int *)(0xC400u)
#define Segment30 (int *)(0xC200u)
#define Segment31 (int *)(0xC000u)

//Information Memory
#define SegmentA  (int *)(0x10C0u) //Locked, usually contains device calibration info; caution when altering
#define SegmentB  (int *)(0x1080u)
#define SegmentC  (int *)(0x1040u)
#define SegmentD  (int *)(0x1000u)

//Other-------------------------------------------------------------------------------------------------------

//RAM
#define RAM                (int *)(0x0200u)

//Peripherals - 16bit
#define Peripherals16bit   (int *)(0x0100u)

//Peripherals - 8bit
#define Peripherals8bit    (int *)(0x0010u)

//Peripherals - 8bit SFR
#define Peripherals8bitSFR (int *)(0x0000u)

#endif /* FLASHMEM_H_ */
////////////////////////////////////***************************************************////////////////////////////////////
////////////////////////////////////*		  ...           ...           ... 		  *////////////////////////////////////
////////////////////////////////////*		 (o O)         (O o)         ( oo)    	  *////////////////////////////////////
////////////////////////////////////*   -ooO--(_)--Ooo-ooO--(_)--Ooo-ooO--(_)--Ooo-   *////////////////////////////////////
////////////////////////////////////***************************************************////////////////////////////////////
