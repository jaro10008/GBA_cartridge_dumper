/*
 * gba.h
 *
 *  Created on: 30 mar 2021
 *      Author: jaro10008
 */

#ifndef GBA_H_
#define GBA_H_

#include<util/delay.h>
#include<avr/io.h>

void GBA_init();

void ROM_latch(uint32_t addr);

uint16_t ROM_read();

void ROM_clear();

uint8_t RAM_read_byte(uint16_t addr);

void RAM_sendCommand(uint16_t addr, uint8_t data);


#endif /* GBA_H_ */
