/*NOTES!
 *
 * You can add your own check list here if you want.
 * Obviously this will be removed when we are done.
 *
 * Needs to be done - Check list
 * 	1.Time signature on/off(X)
 * 	2.Change from Timesignature to normal beats (1 pitch per beat)(X)
 * 	3.Change buzz config with inputs.()
 */

#include <msp430.h> 
#include "lcdLib.h"
#include "buzzer.h"
//#include "uart.h"

//============================
//PWM period and duty cycles for respective servo
#define PWM_Period 20000-1

//RadioShack Servo
//#define MAXDUTY 2865
//#define MINDUTY 900

//SG9 Servo

#define MAXDUTY 3000
#define MINDUTY 1000


//====================================================================================================
/*Contains constant speed values for the following BPM choices:
 *
 */
/* SG90
 * Original - It was a bit off for me, but use if it works with SG90
 * const unsigned int BPM_array[39] = {20,21,22,25,27,28,29,30,31,32,33,35,36,38,39,40,42,44,47,49,50,
 51,52,53,55,56,57,60,63,66,69,72,75,78,82,87,89,90,91};*/

//SG90
//Updated with new formula values, if it doesn't work use other array.
unsigned int BPM_array[40] = {27,28,30,31,32,33,35,36,37,39,40,42,44,46,48
,51,53,56,59,61,64,67,69,72,75,77,80,84,88,92,96,101,107,112,117,123,128,133,139};


//RadioShack servo Speed Values
/*const unsigned int BPM_array[39] = { 26, 28, 29, 30, 31, 33, 34, 35, 38, 39, 41,
		43, 45, 47, 50, 52, 55, 58, 60, 63, 66, 68, 71, 73, 76, 79, 83, 86, 90,
		94, 100, 105, 110, 115, 121, 126, 131, 136 };
*/
//====================================================================================================

volatile unsigned int incdec_speed = 0;

short int NPM = 4; //notes per measure (change this to change top number of time signature).
short int i = 0; //counter
short int buzzer = 1;   //May be used for buzzer mode (0-off,1-on with time sig, 2 - on with straight beat
char *TS = "4/4";
//contains time signature :2/2, 3/4, 4/4, 6/8, 9/8, and 6/4
//Will be handled using switch-case in UART when said setting is to be changed.
//volatile char timesig[] = {"2/2","3/4","4/4","6/8","9/8","6/4"};

unsigned int z=20; //selects BPM based on space numbers

//BPM_Char used to display BPM to LCD.
volatile int BPM_Char[40] = {40,42,44,46,48,50,52,54,56,58,60,63,66,69,72,76,80,84,88,92,96,100,104,108,112,116,120,126,132,138,144,152,160,168,176,184,192,200,208};

char *status = "ON-TS";

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;
	incdec_speed = BPM_array[z];
	//meh = BPM_Char[z];
	//PWM OUTPUT
	P1DIR |= BIT4;
	P1OUT = 0;

	lcdInit();
	lcdSetText("BPM:",0,0);
	lcdSetText("TS:",0,1);
	lcdSetInt(BPM_Char[z],4,0);
	lcdSetText(TS,3,1);
	lcdSetText(status,10,0);


	//PWM TIMER
	TA0CCTL0 = CCIE;
	TA0CCTL1 = OUTMOD_7;
	TA0CCR0 = PWM_Period;
	TA0CCR1 = MINDUTY;
	TA0CTL = TASSEL_2 + MC_1;
	P2DIR |= BIT6;
	P2SEL &= BIT6;
//	UART_int();
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
//	UCA0STAT |=  UCLISTEN;            // loop back mode enabled
	UCA0CTL1 &= ~UCSWRST;             // Clear UCSWRST to enable USCI_A0
//	IE2 |= UCA0TXIE;                  // Enable the Transmit interrupt
	IE2 |= UCA0RXIE;                  // Enable the Receive  interrupt


				 //Tested to see if it works, can use LPM that turns of CPU, and other clocks but SMCLK.-It works,commented out for now.
    while (1)
    {


    	_enable_interrupts();
    	LPM0;

    };

}

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

		}
	}

}


//UART Reveive ISR
/* Have not initialized UART in main function, so this ISR wont do anything.*/
char r;
#pragma vector = USCIAB0RX_VECTOR
__interrupt void ReceiveInterrupt(void)
{
	r = UCA0RXBUF;
    switch(r)
    {
    case '0':{z=0; break;}
    case '1':{z=1; break;}
    case '2':{z=2; break;}
    case '3':{z=3; break;}
    case '4':{z=4; break;}
    case '5':{z=5; break;}
    case '6':{z=6; break;}
    case '7':{z=7; break;}
    case '8':{z=8; break;}
    case '9':{z=9; break;}
    case 'A':{z=10; break;}
    case 'B':{z=11; break;}
    case 'C':{z=12; break;}
    case 'D':{z=13; break;}
    case 'E':{z=14; break;}
    case 'F':{z=15; break;}
    case 'G':{z=16; break;}
    case 'H':{z=17; break;}
    case 'I':{z=18; break;}
    case 'J':{z=19; break;}
    case 'K':{z=20; break;}
    case 'L':{z=21; break;}
    case 'M':{z=22; break;}
    case 'N':{z=23; break;}
    case 'O':{z=24; break;}
    case 'P':{z=25; break;}
    case 'Q':{z=26; break;}
    case 'R':{z=27; break;}
    case 'S':{z=28; break;}
    case 'T':{z=29; break;}
    case 'U':{z=30; break;}
    case 'V':{z=31; break;}
    case 'W':{z=32; break;}
    case 'X':{z=33; break;}
    case 'Y':{z=34; break;}
    case 'Z':{z=35; break;}
    case 'a':{z=36; break;}
    case 'b':{z=37; break;}
    case 'c':{z=38;break;}
    case '.':{buzzer = 0;break;}
    case ',':{buzzer = 2;break;}
    case '/':{buzzer = 1;break;}		//	"2/2","3/4","4/4","6/8","9/8","6/4"
    case '@':{NPM = 2;TS = "2/2";break;}
    case '#':{NPM = 3;TS = "3/4";break;}
    case '$':{NPM = 4;TS = "4/4";break;}
    case '%':{NPM = 6;TS = "6/8";break;}
    case '^':{NPM = 9;TS = "9/8";break;}
    case '&':{NPM = 6;TS = "6/4";break;}
    }
    switch(buzzer){
    case 0: {status="OFF";break;}
    case 2: {status="ON-SB";break;}
    case 1: {status="ON-TS";break;}
    }

    incdec_speed = BPM_array[z];
    lcdClear();
	lcdSetText("BPM:",0,0);
	lcdSetText("TS:",0,1);
	lcdSetInt(BPM_Char[z],4,0);
	if (status == "ON-TS"){
		lcdSetText(TS,3,1);
	}
	lcdSetText(status,10,0);
}
