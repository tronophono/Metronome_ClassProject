/*
 * buzzer.h
 *
 *  Created on: Jul 25, 2016
 *      Author: trono_000
 */

#ifndef BUZZER_H_
#define BUZZER_H_

#define HighBuz 440
#define LowBuz	294

void delay_ms(unsigned int ms);
void delay_us(unsigned int us);
void beep(unsigned int note, unsigned int duration);


#endif /* BUZZER_H_ */
