/*This source is used for the buzzer's pitch and time delay per beat
 *
 * This will only handle the beep functions but we dont want all this code in the main file for the metronome.
 * buzzer.c
 *
 *  Created on: Jul 25, 2016
 *      Author: Luis Santos
 */




#include "msp430g2553.h"
#include "buzzer.h"

void delay_ms(unsigned int ms )
{
    unsigned int i;
    for (i = 0; i<= ms; i++)
       __delay_cycles(500); //Built-in function that suspends the execution for 500 cicles
}

void delay_us(unsigned int us )
{
    unsigned int i;
    for (i = 0; i<= us/2; i++)
       __delay_cycles(1);
}


void beep(unsigned int note, unsigned int duration)
{
    int i;
    long delay = (long)(10000/note);
    long time = (long)((duration*100)/(delay*2));
    for (i=0;i<time;i++)
    {
        P1OUT |= BIT7;
        delay_us(delay);
        P1OUT &= ~BIT7;
        delay_us(delay);
    }
    delay_ms(20);
}
