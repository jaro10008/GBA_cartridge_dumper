#ifndef SPI_H_
#define SPI_H_

#include<stdint.h>

void SPI_init(void);

uint8_t SPI_send_byte_raw(uint8_t data);

void SPI_setSS();

void SPI_clearSS();

uint8_t SPI_send_byte(uint8_t data);

void SPI_send_bytes(uint8_t* dataIn, uint8_t* dataOut, uint16_t s);


#endif /* SPI_H_ */
