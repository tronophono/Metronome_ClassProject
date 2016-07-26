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
#include "msp430g2553.h"
//#include "UartMSP.h"
#include "buzzer.h"


//============================
//PWM period and duty cycles for respective servo
#define PWM_Period 20000-1

//RadioShack Servo
#define MAXDUTY 2865
#define MINDUTY 900

//SG9 Servo
/*
#define MAXDUTY 3000
#define MINDUTY 1000
*/

//====================================================================================================
/*Contains constant speed values for the following BPM choices:
 *
 */
/* SG90
 * Original - It was a bit off for me, but use if it works with SG90
 * const unsigned int BPM_array[39] = {20,21,22,25,27,28,29,30,31,32,33,35,36,38,39,40,42,44,47,49,50,
51,52,53,55,56,57,60,63,66,69,72,75,78,82,87,89,90,91};*/

/*SG90
 * Updated with new formula values, if it doesn't work use other array.
unsigned int BPM_array[39] = {27,28,30,31,32,34,35,36,39,40,42,44,46,48,51,54,56,59,62,64,67,70,
72,75,78,80,84,88,92,96,102,107,112,118,123,128,134,139};
*/

//RadioShack servo Speed Values
const unsigned int BPM_array[39] = {26,28,29,30,31,33,34,35,38,39,41,43,45,47,50,52,
55,58,60,63,66,68,71,73,76,79,83,86,90,94,100,105,110,115,121,126,131,136};
//====================================================================================================

volatile unsigned int incdec_speed = 0;

short int NPM=4;//notes per measure (change this to change top number of time signature).
short int i=0;//counter
short int buzzer = 1;	//May be used for buzzer mode (0-off,1-on with time sig, 2 - on with straight beat


//contains time signature :2/2, 3/4, 4/4, 6/8, 9/8, and 6/4
//volatile char timesig[] = {"2/2","3/4","4/4","6/8","9/8","6/4"};

int main(void)
{
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
	while(1){};

}


#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_ISR(void)
{
	TA0CCR1 += incdec_speed;
	if (TA0CCR1 > MAXDUTY || TA0CCR1 < MINDUTY)
	{
		incdec_speed = -incdec_speed;

		switch(buzzer)
		{
		 case 1 :
		 {
			if (i < (NPM-1))
			{
				beep(LowBuz,50);
				i++;
			}
			else if (i == (NPM-1))
			{
				beep(HighBuz,50);
				i=0;
			}
			break;
		 }
		 case 2:
		 {
			 beep(LowBuz,50);
			 break;
		 }
		 case 0 :
		 {
			 break;
		 }

		}
	}
}


