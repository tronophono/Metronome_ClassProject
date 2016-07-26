/*
 * common.h
 *
 *  Created on: Apr 2, 2016
 *      Author: Erasmo Morales
 *
 *      Contains common definitions for other headerfiles.
 */

#ifndef COMMON_H_
#define COMMON_H_

//VALUES
typedef enum {disable, enable, disable_all, enable_all}   iState;
typedef enum {bit, byte, word, block, segment} 			  dataType;
typedef enum {false, true} 								  boolie;
typedef enum {single, multi}							  keyCapType;
typedef enum {Port1, Port2}                               Port;
typedef enum {TimerA0, TimerA1}                           TimerA;

#endif /* COMMON_H_ */
