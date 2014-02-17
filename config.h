/*
 * config.h
 *
 *  Created on: 16.02.2014
 *      Author: Frederick
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <stdlib.h>
#include "uart.h"

// Kalibrierung RTC
#define F_CAL 12800000L         // gemessener Kalibriertakt in 10^-5 Hz
                                // Endung L ist wichtig!
#define UART_BAUD_RATE      9600

// Batteriespannungen
#define _75PERCENT	12450
#define _50PERCENT	12240
#define _25PERCENT	12060
#define _0PERCENT	11900



// PORTS definieren
#define LED_GREEN_1_PORT 	D
#define LED_GREEN_2_PORT 	D
#define LED_YELLOW_PORT 	D
#define LED_RED_PORT		B

#define LED_GREEN_1		(1<<5)
#define	LED_GREEN_2		(1<<6)
#define LED_YELLOW		(1<<7)
#define LED_RED			(1<<0)

#define GLUE(a, b)	a##b
#define PORT(x)		GLUE(PORT, x)
#define PIN(x)		GLUE(PIN, x)
#define DDR(x)		GLUE(DDR, x)

#define setBits(port,mask)	do{ (port) |=  (mask); }while(0)
#define clrBits(port,mask)	do{ (port) &= ~(mask); }while(0)
#define tstBits(port,mask)	((port) & (mask))

#define led_green_1_on() 	setBits(PORT(LED_GREEN_1_PORT), LED_GREEN_1)
#define led_green_2_on() 	setBits(PORT(LED_GREEN_2_PORT), LED_GREEN_2)
#define led_yellow_on()		setBits(PORT(LED_YELLOW_PORT), LED_YELLOW)
#define led_red_on() 		setBits(PORT(LED_RED_PORT), LED_RED)

#define led_green_1_off() 	clrBits(PORT(LED_GREEN_1_PORT), LED_GREEN_1)
#define led_green_2_off() 	clrBits(PORT(LED_GREEN_2_PORT), LED_GREEN_2)
#define led_yellow_off()	clrBits(PORT(LED_YELLOW_PORT), LED_YELLOW)
#define led_red_off() 		clrBits(PORT(LED_RED_PORT), LED_RED)


// 			Access bits like variables:
struct bits {
	unsigned char b0:1;
	unsigned char b1:1;
	unsigned char b2:1;
	unsigned char b3:1;
	unsigned char b4:1;
	unsigned char b5:1;
	unsigned char b6:1;
	unsigned char b7:1;
} __attribute__((__packed__));

#define SBIT_(port,pin) ((*(volatile struct bits*)&port).b##pin)
#define	SBIT(x,y)	SBIT_(x,y)

// 16Bit <--> 8Bit - Makros
#define uniq(LOW,HEIGHT)	((HEIGHT << 8)|LOW)			// 2x 8Bit 	--> 16Bit
#define LOW_BYTE(x)        	(x & 0xff)					// 16Bit 	--> 8Bit
#define HIGH_BYTE(x)       	((x >> 8) & 0xff)			// 16Bit 	--> 8Bit



#endif /* CONFIG_H_ */
