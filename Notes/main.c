#include <msp430.h> 
#define	PWM_Period	20000
#define	MAXDUTY	3000
#define	MINDUTY 1000
/*
 * main.c
 */
unsigned int incdec_speed = 50;
int go = 1;
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	
    P1DIR |= BIT2;
    P1SEL |= BIT2;
    P1SEL2 &= ~BIT2;


    TA0CCTL0 = CCIE;
    TA0CCTL1 = OUTMOD_7;
    TACCR0 = PWM_Period;
    TACCR1 = MINDUTY;
    TA0CTL = TASSEL_2 + MC_1;

    _enable_interrupts();
    while(1)
    {

    }



}
#pragma vector = TIMER0_A0_VECTOR
 __interrupt void TIMER_A0_ISR(void)
 {
 	if (go == 1)
 	{

 		TA0CCR1 += incdec_speed;
 		if(TA0CCR1 > MAXDUTY || TA0CCR1 < MINDUTY)
 		{
 			incdec_speed = -incdec_speed;
 		}

 	}

 }








