#define F_CPU 10000000UL

#include "enc28j60.h"
#include "gba.h"
#include<util/delay.h>
#include<avr/io.h>

//FUSES: low - 0xEF,   high - 0xC1

int main(void){

	//init ENC with MAC address: D6-BE-7B-F2-D1-10
	ENC_init(0xd6, 0xbe, 0x7b, 0xf2, 0xd1, 0x10);
	GBA_init();
	_delay_ms(1000);
	uint8_t tab[514];
	
	//first to bytes are the block index, starting from 0
	tab[0] = 0;
	tab[1] = 0;


	//DUMP RAM

	for(uint32_t p = 0; p < 128; ++p){
		//set block index
		tab[0] = p&0xff;
		tab[1] = (p>>8)&0xff;

		//read 512 bytes from RAM
		for(uint16_t i = 0; i < 512; ++i){
			tab[i+2] = RAM_read_byte((uint16_t)(p * 512 + i));
		}

		//send the entire block via ethernet
		ENC_transmit_data(tab, 514);
	}

	//switch memory bank to the second one (128K flash memory only)
	RAM_sendCommand(0x5555, 0xaa);
	RAM_sendCommand(0x2aaa, 0x55);
	RAM_sendCommand(0x5555, 0xb0);
	RAM_sendCommand(0x0000, 0x01);

	for(uint32_t p = 0; p < 128; ++p){
		tab[0] = (p + 128)&0xff;
		tab[1] = ((p+128)>>8)&0xff;

		for(uint16_t i = 0; i < 512; ++i){
			tab[i+2] = RAM_read_byte((uint16_t)(p * 512 + i));
		}

		ENC_transmit_data(tab, 514);
	}

	//Signal the end of operation - blink SS diode
	while(1){
		_delay_ms(1000);
		PORTB^=(1<<PB4);
	}
}
