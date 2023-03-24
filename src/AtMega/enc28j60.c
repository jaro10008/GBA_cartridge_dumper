#include"spi.h"
#include<util/delay.h>
#include<avr/io.h>

static uint8_t bank = 0;
static uint8_t mac[6];

static void ENC_change_bank(uint8_t new_bank);

static uint8_t ENC_prepare_reg(uint8_t reg){
	const uint8_t new_bank = reg >> 5;

	reg &= 0x1F;

	if((reg < 0x1B) && (new_bank != bank)){
		ENC_change_bank(new_bank);
	}

	return reg;
}

static void ENC_bit_field_set(uint8_t reg, uint8_t value){
	if(reg > 0x2F){ //this command can't be used on MAC/MII/PHY registers
		return;
	}
	reg = ENC_prepare_reg(reg);

	SPI_setSS();

	SPI_send_byte_raw(0x80 | reg);
	SPI_send_byte_raw(value);

	SPI_clearSS();
}

static void ENC_bit_field_clear(uint8_t reg, uint8_t value){
	if(reg > 0x2F){ //this command can't be used on MAC/MII/PHY registers
		return;
	}

	reg = ENC_prepare_reg(reg);

	SPI_setSS();

	SPI_send_byte_raw(0xA0 | reg);
	SPI_send_byte_raw(value);

	SPI_clearSS();
}

static void ENC_write_control_register(uint8_t reg, uint8_t value){

	reg = ENC_prepare_reg(reg);

	SPI_setSS();

	SPI_send_byte_raw(0x40 | reg);
	SPI_send_byte_raw(value);

	SPI_clearSS();
}

static uint8_t ENC_read_control_register(uint8_t reg){
	reg = ENC_prepare_reg(reg);

	SPI_setSS();

	SPI_send_byte_raw(reg);
	if(bank > 1){
		SPI_send_byte_raw(0xFF); //dummy byte
	}

	const uint8_t res = SPI_send_byte_raw(0xFF);

	SPI_clearSS();

	return res;
}

static void ENC_change_bank(uint8_t new_bank){
	if((bank | new_bank) != new_bank){
		ENC_bit_field_clear(0x1F, 0x03);
	}

	if(new_bank){
		ENC_bit_field_set(0x1F, new_bank);
	}

	bank = new_bank;
}

static void ENC_write_buffer(const uint8_t* data, uint16_t s){
	SPI_setSS();

	SPI_send_byte_raw(0x7A);

	for(uint16_t i = 0; i < s; ++i){
		SPI_send_byte_raw(data[i]);
	}

	SPI_clearSS();
}

static void ENC_read_buffer(uint8_t* data, uint16_t s){
	SPI_setSS();

	SPI_send_byte_raw(0x3A);

	for(uint16_t i = 0; i < s; ++i){
		data[i] = SPI_send_byte_raw(0xFF);
	}

	SPI_clearSS();
}

static uint8_t buffer = 0;

void ENC_transmit_data(const uint8_t* data, uint16_t s){
	const uint8_t hs = 20 + 8;

	//D6-BE-7B-F2-D1-10
	const uint8_t header[15] = {
			0x0E,
			0x90,0xe6,0xba,0xf1,0x0a,0x4d,
			0xd6,0xbe,0x7b,0xf2,0xd1,0x10,
			0x08, 0x00
	};
	uint8_t ipHeader[5*32] = {
			0x45, 0x00, (s + hs) >> 8, (s+hs) & 0xFF,
			0x00, 0x00,	0x00, 0x00,
			0x40, 0x11, 0x00, 0x00,
			0x0a, 0x23, 0x00, 0x02,
			0x0a, 0x23, 0x00, 0x01
	};

	const uint8_t udpHeader[8] = {
			0x13, 0x88, 0x13, 0x88,
			(s + 8) >> 8, (s+8) & 0xFF, 0x00, 0x00
	};

	uint32_t sum = 0;
	for(uint8_t i = 0; i < 20; i += 2){
		uint16_t add = ipHeader[i];
		add = (add << 8) | ipHeader[i+1];
		sum += add;
	}
	while(sum >> 16){
		sum = (sum&0xffff) + (sum >> 16);
	}
	sum = sum ^ 0xffff;
	ipHeader[10] = sum >> 8;
	ipHeader[11] = sum & 0xff;

	uint16_t start_address = 0;

	if(buffer){
		start_address = 2048;
	}

	buffer = 1 - buffer;

	ENC_write_control_register(0x02, start_address & 0xFF);
	ENC_write_control_register(0x03, start_address >> 8);

	ENC_write_buffer(header, 15);
	ENC_write_buffer(ipHeader, 20);
	ENC_write_buffer(udpHeader, 8);
	ENC_write_buffer(data, s);

	while(ENC_read_control_register(0x1F) & 0x08);

	ENC_write_control_register(0x04, start_address & 0xFF);
	ENC_write_control_register(0x05, start_address >> 8);

	ENC_write_control_register(0x06, (s+14 + hs + start_address) & 0xFF);
	ENC_write_control_register(0x07, (s+14 + hs + start_address) >> 8);
	ENC_bit_field_set(0x1F, 0x08);
}

uint8_t getMAC(uint8_t i){
	return mac[i];
}

void ENC_init(uint8_t m5, uint8_t m4, uint8_t m3, uint8_t m2, uint8_t m1, uint8_t m0){
	SPI_init();

	_delay_us(300); //time for the device to stabilize its clock

	SPI_send_byte(0xFF); //soft reset
	_delay_us(100);		 //time for the device to reset

	/*MACON3 - append CRC, padding on
	 * 	frame length check, FULLDPX
	 */
	ENC_write_control_register(0x42, 0x33);

	//MACON1 - pause frames
	ENC_write_control_register(0x40, 0x0C);

	// write to MIREGADR the address of PHCON1
	ENC_write_control_register(0x54, 0x00);//0x56, 57

	//write to PHCON1 - Full-duplex
	ENC_write_control_register(0x56, 0x00);
	ENC_write_control_register(0x57, 0x01);

	_delay_us(20); //wait for PHCON1 to update

	//Back-to-Back Inter-Packet Gap register, MABBIPG
	ENC_write_control_register(0x44, 0x15);

	//Non-Back-to-Back Inter-Packet Gap register low byte, MAIPGL
	ENC_write_control_register(0x46, 0x12);

	//init MAC address
	ENC_write_control_register(0x64, m5);
	ENC_write_control_register(0x65, m4);
	ENC_write_control_register(0x62, m3);
	ENC_write_control_register(0x63, m2);
	ENC_write_control_register(0x60, m1);
	ENC_write_control_register(0x61, m0);

	mac[0] = m0;
	mac[1] = m1;
	mac[2] = m2;
	mac[3] = m3;
	mac[4] = m4;
	mac[5] = m5;
}

