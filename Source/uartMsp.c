/*
 * uart_msp.c
 *
 *  Created on: Mar 3, 2013
 *      Author: E N Jaquez
 */

#include "uartMsp.h"

void serial_init(void) //Initialize TX
{
//initialize P1SEL and P1SEL2 based on datasheet
    P1SEL  |= 0x04; // ****** mask based on pin (same process we used in Lab 6) ****** //
    P1SEL2 |= 0x04;// ****** mask based on pin (same process we used in Lab 6) ****** //

    UCA0CTL1 |= UCSWRST; // Disable UART module for configuration

//initialize UART control and timing registers
    UCA0CTL0 = 0x00;// ****** No parity, Least Sig. Bit first, 8-bit data, 1 stop bit, UART, Asynchronous p. 440 User's Guide ****** //
    UCA0CTL1 |= 0xC0;// ****** SMCLK source, keep in reset state p. 440 User's Guide ****** //
    UCA0BR0 = 104;// ****** 9600 Baud rate	- Assumes 1 MHz clock pg 441 & 435 User's Guide ****** //
    UCA0MCTL = 0x02;// ****** 2nd Stage modulation = 1, Oversampling off p. 442 & 435 User's Guide ****** //
    IE2 = 0x00; // Interrupts disabled

    UCA0CTL1 &= ~UCSWRST; // Enable UART module
}

void serial_init_inter(void) //Initialize RX
{
//initialize P1SEL and P1SEL2 based on datasheet
    P1SEL  |= 0x02; // ****** mask based on pin (same process we used in Lab 6) ****** //
    P1SEL2 |= 0x02;// ****** mask based on pin (same process we used in Lab 6) ****** //

    UCA0CTL1 |= UCSWRST; // Disable UART module for configuration

//initialize UART control and timing registers
    UCA0CTL0 = 0x00;// ****** No parity, Least Sig. Bit first, 8-bit data, 1 stop bit, UART, Asynchronous p. 440 User's Guide ****** //
    UCA0CTL1 |= 0xC0;// ****** SMCLK source, keep in reset state p. 440 User's Guide ****** //
    UCA0BR0 = 104;// ****** 9600 Baud rate	- Assumes 1 MHz clock pg 441 & 435 User's Guide ****** //
    UCA0MCTL = 0x02;// ****** 2nd Stage modulation = 1, Oversampling off p. 442 & 435 User's Guide ****** //

    UCA0CTL1 &= ~UCSWRST; // Enable UART module
}

void serial_charTX(char c)
{
    while( !(IFG2 & UCA0TXIFG) ); // Wait until the transmit buffer is empty
    UCA0TXBUF = c;			// Send the character through the Xmit buffer
}

char serial_charRX(void)
{
    while( !(IFG2 & UCA0RXIFG) ); // Wait until a character has been received
    return UCA0RXBUF; // Return received character
}

void clock_init(void)
{
    DCOCTL = 0x00;
    BCSCTL1 = CALBC1_1MHZ; // Calibrate to 1 MHz
    DCOCTL = CALDCO_1MHZ;

//    BCSCTL1 |= XT2OFF + DIVA_0; //uncomment for RX only if errors arise
//    BCSCTL3 = XT2S_0 + LFXT1S_2 + XCAP_1; //uncomment for RX only if errors arise
}
