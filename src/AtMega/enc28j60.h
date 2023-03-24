/*
 * enc28j60.h
 *
 *  Created on: 10 mar 2021
 *      Author: jaro10008
 */

#ifndef ENC28J60_H_
#define ENC28J60_H_

#include<stdint.h>

void ENC_transmit_data(const uint8_t* data, uint16_t s);

uint8_t getMAC(uint8_t i);

void ENC_init(uint8_t m5, uint8_t m4, uint8_t m3, uint8_t m2, uint8_t m1, uint8_t m0);

#endif /* ENC28J60_H_ */
