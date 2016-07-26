/*  uart.c
 *  UART iniitialization subroutine by E N Jaquez Created on: Mar 3, 2013 (Modified by Erasmo Morales, MSCp.E, Mar 3, 2015)
 *	UART/Terminal subroutine by Erasmo Morales, MSCp.E Created on: Mar 3, 2015
 *
 *  Send and receive a chracter or strings through Hardware or Software UART.
 */
#include "msp430g2553.h"
#include "uart.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define DLAY 30								//Delay Padding between messages being sent by micro.
#define RLED BIT0
#define GLED BIT6
#define RXD  BIT1
#define TXD  BIT2
#define BFL   20 							//Default Buffer length for storage of messages;
#define  LT   68							//Detects capitols A,B, C and D as arrows from keyboard; maybe different for other systems.
#define  RT   67
#define  DN   66
#define  UP   65
#define  BS  127							//Backspace
#define  DL  126							//Delete

//UART Variables
volatile unsigned int  UART_TXflag;			//Flag for the tx_char.
volatile unsigned char tx_c;				//This char is the most current char to go into the UART
volatile unsigned int  UART_RXflag;			//Flag for the rx_char.
volatile unsigned char rx_c;				//This char is the most current char to come out of the UART

//Terminal Variables
#define TERMINAL true						//Set to false for use with App or Micro communication; true to use with a terminal program.
											//When set to true, allows for basic terminal editing

#define CLABEL true 						//Console Label: Set to true to add a label preceeding your input text. (TERMINAL must also be true)
char *string_cl = {"YOU: "};

#define AUTOR false 						//Auto Return: If TERMINAL is true, automatically return/send message when input string reaches it's length.


void UARTinit(void)
{
	P1SEL  |= RXD + TXD;						//Setup the I/O
	P1SEL2 |= RXD + TXD;

    P1DIR  |= RLED + GLED; 						//P1.0 red LED. Toggle when char received.
    P1OUT  |= RLED + GLED; 						//LED off

	DCOCTL  = 0x00;
    BCSCTL1 = CALBC1_1MHZ; 						//Calibrate to 1 MHz
    DCOCTL  = CALDCO_1MHZ;

    UCA0CTL1 |= UCSWRST; 						//Disable UART module for configuration

    //Initialize UART control and timing registers according to Class specifications (Lab 8).
	UCA0CTL0  = 0x00;					//****** No parity, Least Sig. Bit first, 8-bit data, 1 stop bit, UART, Asynchronous p. 440 User's Guide ****** //
	UCA0CTL1 |= UCSSEL_3 + UCSWRST;		//****** SMCLK source, keep in reset state p. 440 User's Guide ****** //
	UCA0BR0   = 0x68;					//****** 9600 Baud rate	- Assumes 1 MHz clock pg 441 & 435 User's Guide ****** //
	UCA0MCTL  = UCBRS_2 + UCBRF_0 + ~UCOS16; //****** 2nd Stage modulation = 1, Oversampling off p. 442 & 435 User's Guide ****** //
	IE2       = 0x00; 						 //Interrupts disabled

	UCA0CTL1 &= ~UCSWRST; 					 //Enable UART module

	IFG2 &= ~UCA0RXIFG; 					 //Clear the Receive interrupt flag
	IE2  |=  UCA0RXIE; 						 //Enable UART RX interrupt

	UART_RXflag = 0;						 //Set rx_flag to 0
	UART_TXflag = 0;						 //Set tx_flag to 0

	return;
}

/*UARTreadc
* Get a char from the UART. Waits till it gets one.
* INPUT : None
* RETURN: Char from UART
*/
unsigned char UARTreadc()					//Waits for a valid char from the UART
{
	while (UART_RXflag == 0){};		 		//Wait for rx_flag to be set
	UART_RXflag = 0;						//ACK rx_flag
    return rx_c;
}

/*UARTread
* Get a string of known length from the UART. Strings terminate when enter is pressed or string buffer fills.
* Will return when all the chars are received or a carriage return (\r) is received. Waits for the data.
* INPUT : Array pointer
* RETURN: None
*/
void UARTread(char* string)
{
	volatile int length  			 = BFL;
	volatile unsigned int i 	     = 0;
	volatile unsigned int temp       = 0;
	char ctest;
	bool ischar                      = false;
	static char prev_string[BFL]     = "";	 //Hold a string for recall.

	if(TERMINAL && CLABEL){		UARTsend(string_cl);	}

	//Heavily modified to provide basic/useful editing in terminal programs such as PuTTY and KiTTY.
	//Depending on the system, the key values of the Left, Right, Up and Down Arrow keys May Need to be modified at the top.
	while(((i < length - 1)))					//Grab data until the array fills (Auto returns after length is filled and TERMINAL is false).
	{											//If TERMINAL is false, characters will stored until full.
		ctest = UARTreadc();

		if(TERMINAL)
		{
			//Test for special editing characters.
			//Print Characters if they are normal characters.
			 if(ctest == '\r')					   //If '\r' is received, the user wants to send.
			{
				for(; i < length ; i++)			   //Fill the rest of the string with \0 nul. Overwrites the \r with \0
				{
					if(!(string[i] != '\0')){ string[i] = '\0'; }
				}
				break;
			}
			else if(ctest != LT && ctest != RT && ctest != DN && ctest != UP && ctest != BS && ctest != DL && ctest != '\r')
			{
				string[i] = ctest;
				UARTsendc(string[i]);
				i++;
				//ischar    = true;
			}
			else if(ctest == BS && (i > 0))			    //If we receive a 'backspace', delete previous character and shift the rest left.
			{
				i--;							    //Go back twice to start storing characters from index of deleted character.
				temp = i;
				UARTsendc(8);

				//Shift characters after deleted characters in to place.
				//Clear remaining charactgers off screen.
				for(; i < length - 1; i++)
				{
					if(string[i + 1] != '\0')
					{
						string[i] = string[i + 1];
						UARTsendc(string[i]);
					}
					else
					{
						string[i] = '\0';
						UARTsendc(32);
						UARTsendc(127);
						break;
					}
				}

				for(; i > temp; i--){ UARTsendc(8); } //Backspace into position.
			}
			else if(ctest == DL && i != length - 1)					//If we receive a 'delete', delete current character, remove remaining characters left.
			{
				temp = i;
				for(; i < length - 1; i++)
				{
					if(string[i + 1] != '\0')
					{
						string[i] = string[i + 1];
						UARTsendc(string[i + 1]);
					}
					else
					{
						string[i] = '\0';
						UARTsendc(32);
						UARTsendc(127);
						break;
					}
				}

				for(; i > temp; i--){ UARTsendc(8); } //Backspace into position.
			}
			else if(ctest == LT && (i > 0))			  //If we receive a 'Left Arrow', Move cursor to the left.
			{
				i--; 						          //Stay in current index and start storing characters from here.
				UARTsendc(8);
			}
			else if(ctest == RT && i < length - 1  && i >= 0 && string[i] != '\0')	//If we receive a 'Right Arrow', Move cursor to the right.
			{
				UARTsendc(string[i]);
				i++;							    								//Stay in current index and start storing characters from here.
			}
			else if(ctest == DN && strlen(string) > 0 )								//If we receive and 'Down Arrow', jump to end of current message.
			{
				for(; i < strlen(string); i++){ UARTsendc(string[i]); }
			}
			else if(ctest == UP && (strlen(prev_string) > 0))						//If we receive and 'Up Arrow', recall last message.
			{
				for(; i < length - 1; i++){ UARTsendc(32); }
				i = length - 1;
				for(; i > 0; i--){ UARTsendc(127); }
				for(; i < length - 1; i++){ string[i] = '\0'; }
				UARTsend(prev_string);
				strncpy(string, &prev_string[0], length);							//Replace current input with previously entered/saved message.
				i = strlen(string);
			}

			//if(ischar){ ischar = !ischar; UARTsendc(string[i]); i++; }
			if(!AUTOR && i == length - 1){ i--; UARTsendc(8); }
		}
//		if(ischar){ i++; ischar = !ischar; }
//		if(!AUTOR && i == length - 1){ i--; UARTsendc(8); }
		else{ string[i] = ctest; i++; }


	}//end while
	//else{ string[i] = ctest; i++; }

	strncpy(prev_string, &string[0], length);
}

/*UARTsendc
* Sends a char to the UART. Will wait if the UART is busy.
* INPUT : Char to send
* RETURN: None
*/
void UARTsendc(unsigned char c)
{
	tx_c = c;							//Put the char into the tx_char
	IE2 |= UCA0TXIE; 					//Enable USCI_A0 TX interrupt
	while(UART_TXflag == 1){};			//Have to wait for the TX buffer
	UART_TXflag = 1;					//Reset the tx_flag
	return;
}

/*UARTsend
* Sends a string to the UART. Will wait if the UART is busy
* INPUT: Pointer to String to send
* RETURN: None
*/
void UARTsend(char *string)							//Sends a String to the UART.
{
     while(*string){ UARTsendc(*string++); }		//Advance though string till end
     return;
}

/*Clears a string*/
void clearString(char* string)
{
	int l = strlen(string);
	int i = 0;
	for(; i < l ; i++)	//Clear string
	{
		string[i] = '\0';
	}
}

/*Print string*/
void printString(char* string)
{
	int l = strlen(string);
	int i = 0;
	for(; i < l ; i++)	//Clear string
	{
		UARTsendc(string[i]);
	}
}

/*Delay Send Message*/
void delaysend(void)
{
	UARTsendc(13);					//Send a 'Return' character (\r).
	UARTsendc(10);					//Send a 'Newline' character (\n).
	delay_ms(DLAY);
}

#pragma vector = USCIAB0TX_VECTOR					//UART TX USCI Interrupt
__interrupt void USCI0TX_ISR(void)
{
	UCA0TXBUF   = tx_c;								//Copy char to the TX Buffer
	UART_TXflag = 0;								//ACK the tx_flag
	P1OUT |=  GLED;									//Notify that we send a char by toggling LED
	delay_us(1000);									//Delay so we can see.
	P1OUT &= ~GLED;
	IE2   &= ~UCA0TXIE; 							//Turn off the interrupt to save CPU
}

#pragma vector = USCIAB0RX_VECTOR					//UART RX USCI Interrupt. This triggers when the USCI receives a char.
__interrupt void USCI0RX_ISR(void)
{
	rx_c         = UCA0RXBUF;						//Copy from RX buffer, in doing so we ACK the interrupt as well
	UART_RXflag  = 1;								//Set the rx_flag to 1
	P1OUT       |= RLED;							//Notify that we received a char by toggling LED
	delay_ms(50);
	P1OUT      &= ~RLED;
}
