/*  uart.h
 *  UART iniitialization subroutine by E N Jaquez Created on: Mar 3, 2013 (Modified by Erasmo Morales, MSCp.E, Mar 3, 2015)
 *	UART/Terminal subroutine by Erasmo Morales, MSCp.E Created on: Mar 3, 2015
 *
 *  Send and receive a chracter or strings through Hardware or Software UART.
 */
extern volatile unsigned int UART_RXflag;	// Global UART Receive Flag. Read Only.
extern volatile unsigned int UART_TXflag;	// Global UART Transmit Flag. Set *****

// Initialize the UART
void UARTinit(void);

// Read a character from UART
unsigned char UARTreadc();

// Read a string from UART
void UARTread();

// Send a character to UART
void UARTsendc(unsigned char c);

// Send a string to UART
void UARTsend(char *string);

// Delay Functions
#define delay_ms(x)		__delay_cycles((long) x* 1000)
#define delay_us(x)		__delay_cycles((long) x)

// Clear a String
void clearString(char* string);

// Delay Send
void delaysend(void);
