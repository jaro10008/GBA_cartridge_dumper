#include<util/delay.h>
#include<avr/io.h>

/*
PORTA -> A0-A7
PORTC -> A8-A15
PORTD -> A16-A23
PB3 -> CS2
PB2 -> CS
PB1 -> RD
PB0 -> WR
*/


void GBA_init(){
	PORTB |= 0x0f;
	DDRB |= 0x0f;

	DDRA = 0;
	DDRC = 0;
	DDRD = 0;

	PORTA = 0;
	PORTC = 0;
	PORTD = 0;
}

void ROM_latch(uint32_t addr){
	PORTA = addr&0xFF;
	PORTC = (addr>>8)&0xFF;
	PORTD = (addr>>16)&0xFF;

	DDRA = 0xff;
	DDRC = 0xff;
	DDRD = 0xff;

	_delay_us(10);

	PORTB &=~(1<<PB2);

	_delay_us(10);

	DDRA = 0;
	DDRC = 0;
	PORTA = 0;
	PORTC = 0;
}

uint16_t ROM_read(){
	uint16_t res;
	PORTB &=~(1<<PB1);
	_delay_us(10);
	res = PINC;
	res <<= 8;
	res|= PINA;
	PORTB |= (1<<PB1);
	_delay_us(10);

	return res;
}

void ROM_clear(){
	DDRD = 0;
	PORTD = 0;
	PORTB |= (1<<PB2);
}

uint8_t RAM_read_byte(uint16_t addr){
	PORTA = addr & 0xff;
	PORTC = (addr >> 8) & 0xff;
	DDRA = 0xff;
	DDRC = 0xff;
	DDRD = 0;
	_delay_us(10);
	PORTB&=~(1<<PB3);
	_delay_us(20);
	PORTB&=~(1<<PB1);
	_delay_us(20);

	uint8_t res = PIND;

	PORTB |= (1<<PB1);
	PORTB |= (1<<PB3);

	DDRA = 0;
	DDRC = 0;
	PORTA = 0;
	PORTC = 0;

	return res;
}

void RAM_sendCommand(uint16_t addr, uint8_t data){
	PORTA = addr & 0xff;
	PORTC = (addr >> 8) & 0xff;
	PORTD = data;
	DDRA = 0xff;
	DDRC = 0xff;
	DDRD = 0xff;

	_delay_us(10);
	PORTB&=~(1<<PB3);
	_delay_us(10);
	PORTB&=~(1<<PB0);
	_delay_us(10);

	PORTB |= (1<<PB0);
	PORTB |= (1<<PB3);

	DDRA = 0;
	DDRC = 0;
	DDRD = 0;
	PORTA = 0;
	PORTC = 0;
	PORTD = 0;
}
