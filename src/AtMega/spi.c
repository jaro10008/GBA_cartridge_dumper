#include<avr/io.h>

/*
PB4 -> SS
PB5 -> MOSI
PB6 -> MISO
PB7 -> SCK
*/

void SPI_init(void)
{
	/* Set MOSI, SCK and SS output, all others input */
	DDRB |= (1<<PB4)|(1<<PB5)|(1<<PB7);
	DDRB &= ~(1<<PB6);

	/* Set SS to high */
	PORTB |= (1<<PB4);

	/* Enable SPI, Master, set clock rate fck/2,
	 * sample on rising edge, SCK low by default */
	SPSR = (1<<SPI2X);
	SPCR = (1<<SPE)|(1<<MSTR);
}

uint8_t SPI_send_byte_raw(uint8_t data)
{
	SPDR = data;
	while(!(SPSR & (1<<SPIF)));

	return SPDR;
}

void SPI_setSS(){
	PORTB &= ~(1<<PB4);
}

void SPI_clearSS(){
	PORTB |= (1<<PB4);
}

uint8_t SPI_send_byte(uint8_t data)
{
	SPI_setSS();
	const uint8_t res = SPI_send_byte_raw(data);
	SPI_clearSS();

	return res;
}

void SPI_send_bytes(uint8_t* dataIn, uint8_t* dataOut, uint16_t s){
	SPI_setSS();

	if(!dataIn){
		for(uint16_t i = 0; i < s; ++i){
			const uint8_t res = SPI_send_byte_raw(0xFF);
			if(dataOut){
				dataOut[i] = res;
			}
		}
	}
	else{
		for(uint16_t i = 0; i < s; ++i){
			const uint8_t res = SPI_send_byte_raw(dataIn[i]);
			if(dataOut){
				dataOut[i] = res;
			}
		}
	}

	SPI_clearSS();
}
