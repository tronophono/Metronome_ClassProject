//Esteban Acosta
//EE3176- Lab Final Example code

#include "msp430g2553.h"

#define TXLED BIT0
#define RXLED BIT6
#define TXD BIT2
#define RXD BIT1

const char string[7] = { "Hello!!" };
int i; //Counter

void main(void)
{

	WDTCTL = WDTPW + WDTHOLD; // Stop the Watch dog

  //------------------- Configure the Clocks -------------------//

  if (CALBC1_1MHZ==0xFF)   // If calibration constant erased
     {
        while(1);          // do not load, trap CPU!!
     }

   DCOCTL  = 0;             // Select lowest DCOx and MODx settings
   BCSCTL1 = CALBC1_1MHZ;   // Set range
   DCOCTL  = CALDCO_1MHZ;   // Set DCO step + modulation

  //---------------- Configuring the LED's ----------------------//

   //P1DIR  |=  BIT0 + BIT6;  // P1.0 and P1.6 output
   //P1OUT  &= ~BIT0 + BIT6;  // P1.0 and P1.6 = 0

  //--------- Setting the UART function for P1.1 & P1.2 --------//

   P1SEL  |=  BIT1 + BIT2;  // P1.1 UCA0RXD input
   P1SEL2 |=  BIT1 + BIT2;  // P1.2 UCA0TXD output

  //------------ Configuring the UART(USCI_A0) ----------------//

   UCA0CTL1 |=  UCSSEL_2 + UCSWRST;  // USCI Clock = SMCLK,USCI_A0 disabled
   UCA0BR0   =  104;                 // 104 From datasheet table-
   UCA0BR1   =  0;                   // -selects baudrate =9600,clk = SMCLK
   UCA0MCTL  =  UCBRS_1;             // Modulation value = 1 from datasheet
   //UCA0STAT |=  UCLISTEN;            // loop back mode enabled
   UCA0CTL1 &= ~UCSWRST;             // Clear UCSWRST to enable USCI_A0

  //---------------- Enabling the interrupts ------------------//

   IE2 |= UCA0TXIE;                  // Enable the Transmit interrupt
   IE2 |= UCA0RXIE;                  // Enable the Receive  interrupt
   _BIS_SR(GIE);                     // Enable the global interrupt


}

  //-----------------------------------------------------------------------//
  //                Transmit and Receive interrupts                        //
  //-----------------------------------------------------------------------//

  #pragma vector = USCIAB0TX_VECTOR
  __interrupt void TransmitInterrupt(void)
  {
  	P1OUT |= TXLED;
  	     UCA0TXBUF = string[i++]; // TX next character
  	    if (i == sizeof string) // TX over?
  	       UC0IE &= ~UCA0TXIE; // Disable USCI_A0 TX interrupt
  	    P1OUT &= ~TXLED;



  	    P1OUT &= ~TXLED; }


#pragma vector = USCIAB0RX_VECTOR
__interrupt void ReceiveInterrupt(void)
{
   P1OUT |= RXLED;
    if (UCA0RXBUF == '1') // 'a' received?
    {
       i = 0;
       UC0IE |= UCA0TXIE; // Enable USCI_A0 TX interrupt
      UCA0TXBUF = string[i++];
    }
    P1OUT &= ~RXLED;
}
