/*
 * uart_msp.h
 *
 *  Created on: Mar 3, 2013
 *      Author: E N Jaquez
 */

#ifndef SERIAL_MSP_H_
#define SERIAL_MSP_H_

#include <msp430g2553.h>
#include <msp430.h>
#include <stdio.h>

// Function prototypes
void serial_init(void);
void serial_init_inter(void);
void serial_charTX(char c);
char serial_charRX(void);
void clock_init(void);

#endif /* SERIAL_MSP_H_ */
