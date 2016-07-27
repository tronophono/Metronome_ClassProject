/*NOTES!
 *
 * You can add your own check list here if you want.
 * Obviously this will be removed when we are done.
 *
 * Needs to be done - Check list
 * 	1.Time signature on/off(X)
 * 	2.Change from Timesignature to normal beats (1 pitch per beat)(X)
 * 	3.Change buzz config with inputs.()
 * 	4.UART Connectivity.()
 * 	5.Able to send Through LED?()
 */

#include <msp430.h> 
#include "msp430g2553.h"
//#include "UartMSP.h"
#include "buzzer.h"

//============================
//PWM period and duty cycles for respective servo
#define PWM_Period 20000-1

//RadioShack Servo
/*
#define MAXDUTY 2865
#define MINDUTY 900
*/
//SG9 Servo

 #define MAXDUTY 3000
 #define MINDUTY 1000
 #define TXD BIT2
 #define RXD BIT1

//====================================================================================================
/*Contains constant speed values for the following BPM choices:
 *
 */
/* SG90
 * Original - It was a bit off for me, but use if it works with SG90
 * const unsigned int BPM_array[39] = {20,21,22,25,27,28,29,30,31,32,33,35,36,38,39,40,42,44,47,49,50,
 51,52,53,55,56,57,60,63,66,69,72,75,78,82,87,89,90,91};*/

//SG90
// * Updated with new formula values, if it doesn't work use other array.
 unsigned int BPM_array[39] = {27,28,30,31,32,33,35,36,37,39,40,42,44,46,48,51,53,56,59,61,64,67,69,
 72,75,77,80,84,88,92,96,101,107,112,117,123,128,133,139};

/*
//RadioShack servo Speed Values
const unsigned int BPM_array[39] = { 26, 28, 29, 30, 31, 33, 34, 35, 38, 39, 41,
		43, 45, 47, 50, 52, 55, 58, 60, 63, 66, 68, 71, 73, 76, 79, 83, 86, 90,
		94, 100, 105, 110, 115, 121, 126, 131, 136 };
*/
const char BPM_Char[37] = {'40','42','44','46','48','50','52','54','56','58','60','63','66','69','72','76','80','84','88','92','96','100','104','108','112','116','120','126','132','138','144','152','160','168','176','184','192','200','208'};

//====================================================================================================

volatile unsigned int incdec_speed = 0;
int z;
short int NPM = 4; //notes per measure (change this to change top number of time signature).
short int i = 0; //counter
short int buzzer = 1;//May be used for buzzer mode (0-off,1-on with time sig, 2 - on with straight beat

//contains time signature :2/2, 3/4, 4/4, 6/8, 9/8, and 6/4
//volatile char timesig[] = {"2/2","3/4","4/4","6/8","9/8","6/4"};

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;
	int bpm_speed2 = BPM_array[37];
	incdec_speed = bpm_speed2;
	//PWM OUTPUT
	P1DIR |= BIT7;
	P1OUT = 0;
	P2DIR |= BIT6;
	P2SEL &= BIT6;

	//PWM TIMER
	TA0CCTL0 = CCIE;
	TA0CCTL1 = OUTMOD_7;
	TA0CCR0 = PWM_Period;
	TA0CCR1 = MINDUTY;
	TA0CTL = TASSEL_2 + MC_1;

	_enable_interrupts();
//	LPM0;					 //Tested to see if it works, can use LPM that turns of CPU, and other clocks but SMCLK.-It works,commented out for now.
	while (1) {
	};

}
/*
void UART_int(void)
{
	if (CALBC1_1MHZ==0xFF)   // If calibration constant erased
	     {
	        while(1);          // do not load, trap CPU!!
	     }

	   DCOCTL  = 0;             // Select lowest DCOx and MODx settings
	   BCSCTL1 = CALBC1_1MHZ;   // Set range
	   DCOCTL  = CALDCO_1MHZ;   // Set DCO step + modulation
	   P1SEL  |=  BIT1 + BIT2;  // P1.1 UCA0RXD input
	   P1SEL2 |=  BIT1 + BIT2;  // P1.2 UCA0TXD output
	   UCA0CTL1 |=  UCSSEL_2 + UCSWRST;  // USCI Clock = SMCLK,USCI_A0 disabled
	   UCA0BR0   =  104;                 // 104 From datasheet table-
	   UCA0BR1   =  0;                   // -selects baudrate =9600,clk = SMCLK
	   UCA0MCTL  =  UCBRS_1;             // Modulation value = 1 from datasheet
	   //UCA0STAT |=  UCLISTEN;            // loop back mode enabled
	   UCA0CTL1 &= ~UCSWRST;             // Clear UCSWRST to enable USCI_A0
	   IE2 |= UCA0TXIE;                  // Enable the Transmit interrupt
	   IE2 |= UCA0RXIE;                  // Enable the Receive  interrupt
	   _BIS_SR(GIE);                     // Enable the global interrupt
}
*/
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_ISR(void) {
	TA0CCR1 += incdec_speed;
	if (TA0CCR1 > MAXDUTY || TA0CCR1 < MINDUTY) {
		incdec_speed = -incdec_speed;

		switch (buzzer) {
		case 1: {
			if (i < (NPM - 1)) {
				beep(LowBuz, 50);
				i++;
			} else if (i == (NPM - 1)) {
				beep(HighBuz, 50);
				i = 0;
			}
			break;
		}
		case 2: {
			beep(LowBuz, 50);
			break;
		}
		case 0: {
			break;
		}

void main(void)

		}
	}
}

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
    switch(UCA0RXBUF)
    {
    case 0{z=0; break;}
    case 1{z=1; break;}
    case 2{z=2; break;}
    case 3{z=3; break;}
    case 4{z=4; break;}
    case 5{z=5; break;}
    case 6{z=6; break;}
    case 7{z=7; break;}
    case 8{z=8; break;}
    case 9{z=9; break;}
    case 10{z=10; break;}
    case 11{z=11; break;}
    case 12{z=12; break;}
    case 13{z=13; break;}
    case 14{z=14; break;}
    case 15{z=15; break;}
    case 16{z=16; break;}
    case 17{z=17; break;}
    case 18{z=18; break;}
    case 19{z=19; break;}
    case 20{z=20; break;}
    case 21{z=21; break;}
    case 22{z=22; break;}
    case 23{z=23; break;}
    case 24{z=24; break;}
    case 25{z=25; break;}
    case 26{z=26; break;}
    case 27{z=27; break;}
    case 28{z=28; break;}
    case 29{z=29; break;}
    case 30{z=30; break;}
    case 31{z=31; break;}
    case 32{z=32; break;}
    case 33{z=33; break;}
    case 34{z=34; break;}
    case 35{z=35; break;}
    case 36{z=36; break;}
    case 37{z=37; break;}
    }
    speed = BPM_array[z];
    lcdSetText(BPM_Char[z],4,0);
}
