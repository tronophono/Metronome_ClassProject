//Metronome - Final Project

/*Members - Luis Santos, Esteban Acosta, Marc Villarreal*/



//Headers
#include <msp430.h> 
#include "lcdLib.h"
#include "buzzer.h"		//Used to control pitch of buzzer
#include "flashmem.h"
//============================
//PWM period and duty cycles for respective servo
#define PWM_Period 20000-1

//SG9 Servo
#define MAXDUTY 3000
#define MINDUTY 1000


//====================================================================================================
//SG90 servo
//contains rounded values from formula in Metronome_Data_Graph spread sheet for BPM
unsigned int BPM_array[39] = {27,28,30,31,32,33,35,36,37,39,40,42,44,46,48,51,53,56,59,61,64,67,69,72,75,77,80,84,88,92,96,101,107,112,117,123,128,133,139};

//BPM_Char used to display BPM to LCD.
volatile int BPM_Char[39] = {40,42,44,46,48,50,52,54,56,58,60,63,66,69,72,76,80,84,88,92,96,100,104,108,112,116,120,126,132,138,144,152,160,168,176,184,192,200,208};

const char *timesig[6] = {"2/2","3/4","4/4","6/8","9/8","6/4"};
//====================================================================================================

volatile unsigned int incdec_speed =0; //Used to increment duty cycle for servo

volatile int NPM=2; //notes per measure (change this to change top number of time signature).
volatile int i = 0; //counter
volatile int buzzer=0;   //May be used for buzzer mode (0-off,1-on with time sig, 2 - on with straight beat
volatile int TS;	//Used for index for time signature array (timesig[])


//Will be handled using switch-case in UART when said setting is to be changed.
unsigned int value = 0; //Stores value of ADC10MEM, which is also the voltage level

volatile int z=38; //Index value for BPM_array and BPM_Char
volatile int wait = 0; //Increases wait time for sample, 25 is 10 seconds

char *status; //Used to display buzzer mode
//Used to store receiving buffer character


int main(void) {
	WDTCTL = WDTPW | WDTHOLD;
	lcdInit();


	flashConfig(FSSEL_3, FN0);
	flashInterrupts(enable_all);
	flashInterrupts(disable, disable, disable);

	//Check if previous session was saved, if there is a save session, load values to variables
	volatile boolie empty00 = isItEmpty(Segment10,byte);
	if (empty00 == false)
	{
		flashRead(&z,Segment10,byte);
		flashRead(&NPM,Segment10+1,byte);
		flashRead(&buzzer,Segment10+2,byte);
		flashRead(&TS,Segment10+3,byte);
	}

	incdec_speed = BPM_array[z]; //change speed of servo with loaded settings

	//Used for Buzzer and battery indications led(for testing)
	P1DIR |= BIT4;
	P1OUT = 0;

	//PWM TIMER
	TA0CCTL0 = CCIE;
	TA0CCTL1 = OUTMOD_7;
	TA0CCR0 = PWM_Period;
	TA0CCR1 = MINDUTY;
	TA0CTL = TASSEL_2 + MC_1;
	P2DIR |= BIT6;
	P2SEL &= BIT6;

	if (CALBC1_1MHZ==0xFF)   // If calibration constant erased
	{
		while(1);          // do not load, trap CPU!!
	}
	DCOCTL  = 0;             // Select lowest DCOx and MODx settings
	BCSCTL1 = CALBC1_1MHZ;   // Set range
	DCOCTL  = CALDCO_1MHZ;   // Set DCO step + modulation


	//ADC10 Set up
	ADC10CTL1 = INCH_11 + ADC10DIV_0 + SHS_0 + ADC10SSEL_0;
	ADC10CTL0 = SREF_1 + REFON + ADC10SHT_2 + ADC10SR + ADC10ON +ADC10IE;	//set 1.5v as ref for now

	//Used to time ADC10 sampling
	TA1CCTL0 = CCIE;
	TA1CCTL1 = OUTMOD_4;
	TA1CCR0 = 50000-1;
	TA1CTL = TASSEL_2 + MC_1 + ID_3;





	//UART SETUP
	P1SEL  |=  BIT1 + BIT2;  // P1.1 UCA0RXD input
	P1SEL2 |=  BIT1 + BIT2;  // P1.2 UCA0TXD output
	UCA0CTL1 |=  UCSSEL_2 + UCSWRST;  // USCI Clock = SMCLK,USCI_A0 disabled
	UCA0BR0   =  104;                 // 104 From datasheet table-
	UCA0BR1   =  0;                   // -selects baudrate =9600,clk = SMCLK
	UCA0MCTL  =  UCBRS_1;             // Modulation value = 1 from datasheet
	UCA0CTL1 &= ~UCSWRST;             // Clear UCSWRST to enable USCI_A0
	IE2 |= UCA0RXIE;                  // Enable the Receive  interrupt

	lcdClear();
	lcdSetText("BPM:",0,0);
	lcdSetText("BZ:",8,0);
	lcdSetText("TS:",0,1);
	lcdSetInt(BPM_Char[z],4,0);
	switch(buzzer){
	case 0: {status="OFF";break;}
	case 2: {status="ON-SB";break;}
	case 1: {status="ON-TS";break;}
	default:{break;}
	}
	lcdSetText(status,11,0);
	if (buzzer == 1)
	{
		lcdSetText(timesig[TS],3,1);
	}
	//Displays current settings and status
	while (1)
	{
		switch(buzzer){
		case 0: {status="OFF";break;}
		case 2: {status="ON-SB";break;}
		case 1: {status="ON-TS";break;}
		default:{break;}
		}
		_enable_interrupts();
		LPM0;
	};

}



//Turns green LED off if voltage low, on when high. If ADC10MEM does not change
//the voltage does not change
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{

	if (ADC10MEM != value)
	{
		value = ADC10MEM;
		if (value == 0x3ff)
		{
			ADC10CTL0 = SREF_1 + REF2_5V + REFON + ADC10SHT_2 + ADC10SR + ADC10ON; //changes Ref to 2.5v
			ADC10CTL0 |= ENC + ADC10SC;
			while (ADC10CTL1 & ADC10BUSY);
			value = ADC10MEM;
			ADC10CTL0 &= ~ENC;
			ADC10CTL0 &= ~(ADC10ON);
			P1OUT |= BIT6;
		}
		else{
			P1OUT &= ~BIT6;}
	}
}



//Used to take a sample of the power supply, done every 10 seconds
#pragma vector = TIMER1_A0_VECTOR
__interrupt void TIMER1_ISR(void)
{
	wait++;

	if (wait == 25)
	{
		ADC10CTL0 &= ~ENC;
		while(ADC10CTL1 & BUSY);
		ADC10CTL0 |= ADC10ON + ENC + ADC10SC;
		wait = 0;
	}
	LPM0_EXIT;
}


//Handles the speed of the servo arm, and the buzzer intervals
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_ISR(void)
{

	TA0CCR1 += incdec_speed;

	//Toggles arm back and forth
	if (TA0CCR1 > MAXDUTY || TA0CCR1 < MINDUTY) {
		incdec_speed = -incdec_speed;

		//Checks buzzer value
		switch (buzzer) {
		case 1: {
			if (i < (NPM - 1)) {
				beep(LowBuz, 50);	//Gives a low pitch sound
				i++;
			} else if (i == (NPM - 1)) {
				beep(HighBuz, 50); //Gives higher pitch sound
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


//Receives users input, in app if user choses "208bpm" sends a 'c' character
//ISR translates 'c' to 208bpm for servo and buzzer
char r;
char s;
#pragma vector = USCIAB0RX_VECTOR
__interrupt void ReceiveInterrupt(void)
{
	volatile boolie empty01;
	volatile boolie empty02;
	volatile boolie empty03;
	volatile boolie empty04;

	while( !(IFG2 & UCA0RXIFG) );
	r = UCA0RXBUF;

	//Switch case translates received characters from the app into values for metronome
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
	case '/':{buzzer = 1;break;}
	case '@':{NPM = 2;TS = 0;break;}
	case '#':{NPM = 3;TS = 1;break;}
	case '$':{NPM = 4;TS = 2;break;}
	case '%':{NPM = 6;TS = 3;break;}
	case '^':{NPM = 9;TS = 4;break;}
	case '&':{NPM = 6;TS = 5;break;}
	case '?':
	{
		empty01 = isItEmpty(Segment3,byte);
		empty02 = isItEmpty(Segment3+1,byte);
		empty03 = isItEmpty(Segment3+2,byte);
		empty04 = isItEmpty(Segment3+3,byte);
		if ((empty01 && empty02 && empty03 && empty04) == false)
		{
			flashErase(Segment3);
			s = 'E';
			IE2 |= UCA0TXIE;
		}
		else
		{

			flashWrite(z,Segment3,byte,0);
			flashWrite(NPM,Segment3+1,byte,0);
			flashWrite(buzzer,Segment3+2,byte,0);
			flashWrite(TS,Segment3+3,byte,0);
			s = 'S';
			IE2 |= UCA0TXIE;

		}
		break;
	}
	case '*':
	{
		empty01 = isItEmpty(Segment4,byte);
		empty02 = isItEmpty(Segment4+1,byte);
		empty03 = isItEmpty(Segment4+2,byte);
		empty04 = isItEmpty(Segment4+3,byte);
		if ((empty01 && empty02 && empty03 && empty04) == false)
		{
			flashErase(Segment4);
			s = 'E';
			IE2 |= UCA0TXIE;
		}
		else
		{

			flashWrite(z,Segment4,byte,0);
			flashWrite(NPM,Segment4+1,byte,0);
			flashWrite(buzzer,Segment4+2,byte,0);
			flashWrite(TS,Segment4+3,byte,0);
			s = 'S';
			IE2 |= UCA0TXIE;

		}
		break;
	}
	case '-':
	{
		empty01 = isItEmpty(Segment5,byte);
		empty02 = isItEmpty(Segment5+1,byte);
		empty03 = isItEmpty(Segment5+2,byte);
		empty04 = isItEmpty(Segment5+3,byte);
		if ((empty01 && empty02 && empty03 && empty04) == false)
		{
			flashErase(Segment5);
			s = 'E';
			IE2 |= UCA0TXIE;
		}

		flashWrite(z,Segment5,byte,0);
		flashWrite(NPM,Segment5+1,byte,0);
		flashWrite(buzzer,Segment5+2,byte,0);
		flashWrite(TS,Segment5+3,byte,0);
		s = 'S';
		IE2 |= UCA0TXIE;
		break;
	}

	case '(':
	{
		empty01 = isItEmpty(Segment3,byte);
		empty02 = isItEmpty(Segment3+1,byte);
		empty03 = isItEmpty(Segment3+2,byte);
		empty04 = isItEmpty(Segment3+3,byte);
		if ((empty01 && empty02 && empty03 && empty04) == false)
		{
			flashRead(&z,Segment3,byte);
			flashRead(&NPM,Segment3+1,byte);
			flashRead(&buzzer,Segment3+2,byte);
			flashRead(&TS,Segment3+3,byte);
			s = 'L';
			IE2 |= UCA0TXIE;
		}
		break;
	}
	case ')':
	{
		empty01 = isItEmpty(Segment4,byte);
		empty02 = isItEmpty(Segment4+1,byte);
		empty03 = isItEmpty(Segment4+2,byte);
		empty04 = isItEmpty(Segment4+3,byte);
		if ((empty01 && empty02 && empty03 && empty04) == false)
		{
			flashRead(&z,Segment4,byte);
			flashRead(&NPM,Segment4+1,byte);
			flashRead(&buzzer,Segment4+2,byte);
			flashRead(&TS,Segment4+3,byte);
			s = 'L';
			IE2 |= UCA0TXIE;
		}
		break;
	}
	case '+':
	{
		empty01 = isItEmpty(Segment5,byte);
		empty02 = isItEmpty(Segment5+1,byte);
		empty03 = isItEmpty(Segment5+2,byte);
		empty04 = isItEmpty(Segment5+3,byte);
		if ((empty01 && empty02 && empty03 && empty04) == false)
		{
			flashRead(&z,Segment5,byte);
			flashRead(&NPM,Segment5+1,byte);
			flashRead(&buzzer,Segment5+2,byte);
			flashRead(&TS,Segment5+3,byte);
			s = 'L';
			IE2 |= UCA0TXIE;
		}
		break;
	}
	}

	//Changes BPM to new value
	incdec_speed = BPM_array[z];
	lcdClear();
	lcdSetText("BPM:",0,0);
	lcdSetText("BZ:",8,0);
	lcdSetText("TS:",0,1);
	lcdSetInt(BPM_Char[z],4,0);

	switch(buzzer){
	case 0: {status="OFF";break;}
	case 2: {status="ON-SB";break;}
	case 1: {status="ON-TS";break;}
	default:{break;}
	}
	lcdSetText(status,11,0);
	if (buzzer == 1)
	{
		lcdSetText(timesig[TS],3,1);
	}

	//Erases previous session, and saves new settings.
	volatile boolie empty00 = isItEmpty(Segment10,block);
	if (empty00 == false)
	{
		flashErase(Segment10);
	}
	flashWrite(z,Segment10,byte,0);
	flashWrite(NPM,Segment10+1,byte,0);
	flashWrite(buzzer,Segment10+2,byte,0);
	flashWrite(TS,Segment10+3,byte,0);
}


//Sends 'R' character for "Changes Done"
#pragma vector = USCIAB0TX_VECTOR		//UART TX USCI Interrupt
__interrupt void USCI0TX_ISR(void)
{
	while( !(IFG2 & UCA0TXIFG) );
	UCA0TXBUF   = s;
	while( !(IFG2 & UCA0TXIFG) );
	UCA0TXBUF = '\r';
	while( !(IFG2 & UCA0TXIFG) );
	UCA0TXBUF = '\n';
	IE2   &= ~UCA0TXIE;
}

