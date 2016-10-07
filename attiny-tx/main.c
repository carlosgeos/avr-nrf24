#define F_CPU 8000000UL
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nRF24L01P.h"

/* ATTiny84 IO pins

            ___^___
+3,3v      -|VCC GND|- 0v
           -|PB0 PA0|-
   LED     -|PB1 PA1|-
   RESET   -|PB3 PA2|- CE NRF (Tx/Rx)
           -|PB2 PA3|- CS NRF (SPI)
           -|PA7 PA4|- CLK (SPI)
MISO (SPI) -|PA6 PA5|- MOSI (SPI)
             -------
*/

#define LED       PB1
#define SPI_MISO  PA6
#define SPI_MOSI  PA5
#define SPI_CLK   PA4
#define NRF_CS    PA3
#define _NRF_CS_L cbi(PORTA, NRF_CS);
#define _NRF_CS_H sbi(PORTA, NRF_CS);
#define NRF_CE    PA2
#define _NRF_CE_L cbi(PORTA, NRF_CE);
#define _NRF_CE_H sbi(PORTA, NRF_CE);
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit)) // clear bit
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))  // set bit
#define NRF_DEF_CONF (NRF_PWR_UP + NRF_EN_CRC)

void setup(void);
void loop(void);
int main(void);
uint8_t spi_transfer(uint8_t data);

uint8_t spi_transfer(uint8_t data) {
    // data in transmit buffer
    USIDR = data;
    // clear overflow flag
    sbi(USISR, USIOIF);
    // while no overflow
    while ((USISR & _BV(USIOIF)) == 0) {
	// 3 wire mode | software clock via USITC
	USICR = (1<<USIWM0) | (1<<USICS1) | (1<<USICLK) | (1<<USITC);
    }
    return USIDR;
}

uint8_t addr[5];

void setup(void) {

    // LED IO
    sbi(DDRB,  LED); // set LED pin as output
    sbi(PORTB, LED); // turn the LED on
    // SPI bus setup
    cbi(DDRA,  SPI_MISO); // MISO = input
    sbi(PORTA, SPI_MISO); // MISO = input with pullup on (DI)
    sbi(DDRA,  SPI_MOSI); // MOSI = output
    sbi(DDRA,  SPI_CLK);  // SCK  = output
    // nRF24L01+ (Chip enable activates Rx or Tx mode)
    sbi(DDRA,  NRF_CS);   // CS   = output
    sbi(DDRA,  NRF_CE);   // CE   = output


    /* Module setup */
    _NRF_CS_L;
    spi_transfer(NRF_W_REGISTER | NRF_RF_SETUP);
    spi_transfer(0x06);		/* 1Mbps, 0db */
    _NRF_CS_H;

    _NRF_CS_L;
    spi_transfer(NRF_W_REGISTER | NRF_RF_CH);
    spi_transfer(0x40);		/* 2.4 GHz + 64 MHz */
    _NRF_CS_H;

    // Address width -> 5 bytes by default. In this case: 0xB3B3B3B3B3

    _NRF_CS_L;
    spi_transfer(NRF_W_REGISTER | NRF_SETUP_RETR);
    spi_transfer(0x03);		/* Number of retransmissions */
    _NRF_CS_H;


    // SPI : write NRF Tx Addr
    _NRF_CS_L;
    spi_transfer(NRF_W_REGISTER | NRF_TX_ADDR);
    spi_transfer(0xB3);
    spi_transfer(0xB3);
    spi_transfer(0xB3);
    spi_transfer(0xB3);
    spi_transfer(0xB3);
    _NRF_CS_H;


    // SPI : write NRF Rx Addr P0 for auto ACK
    _NRF_CS_L;
    spi_transfer(NRF_W_REGISTER | NRF_RX_ADDR_P0);
    spi_transfer(0xB3);
    spi_transfer(0xB3);
    spi_transfer(0xB3);
    spi_transfer(0xB3);
    spi_transfer(0xB3);
    _NRF_CS_H;

}

void loop(void) {
    // SPI
    uint16_t i;
    uint8_t nrfStatus;
    //DDRB ^= 1<<LED;

    // SPI : clear MAX_RT flag in STATUS register
    _NRF_CS_L;
    spi_transfer(NRF_W_REGISTER | NRF_STATUS);
    spi_transfer(0x10);
    _NRF_CS_H;

    // SPI : flush Tx buffer
    _NRF_CS_L;
    nrfStatus = spi_transfer(NRF_FLUSH_TX);
    _NRF_CS_H;

    // SPI : read NRF Addr P0
    _NRF_CS_L;
    nrfStatus = spi_transfer(NRF_NOP);
    _NRF_CS_H;

    // SPI : read NRF Tx Addr and display it
    _NRF_CS_L;
    spi_transfer(NRF_R_REGISTER | NRF_TX_ADDR);
    for (i=0; i < 5; i++)
      addr[i] = spi_transfer(NRF_NOP);
    _NRF_CS_H;


    // SPI : write NRF default config : POWER_UP and EN_CRC / PWR_RX disable
    _NRF_CS_L;
    spi_transfer(NRF_W_REGISTER | NRF_CONFIG);
    spi_transfer(NRF_DEF_CONF);
    _NRF_CS_H;


    // SPI : write NRF Tx payload
    _NRF_CS_L;
    spi_transfer(NRF_W_REGISTER | NRF_W_TX_PAYLOAD);
    /* char msg[] = ""; */
    /* size_t msg_size = sizeof(msg)/sizeof(msg[0]); */
    /* int j; */
    /* for (j = 0; j < msg_size; ++j) */
    spi_transfer('C');
    _NRF_CS_H;

    /* Transmitting data */
    // NRF CE pulse (minimum 10us)
    _NRF_CE_H;
    // wait data is transmit (loop until TX_DS and MAX_RT == 0)
    // TODO here it could be better to use IRQ from the chip
    _delay_us(15);
    /* i = 0; */
    /* do { */
    /* 	i++; */
    /* 	_delay_us(15); */
    /* 	_NRF_CS_L; */
    /* 	nrfStatus = spi_transfer(NRF_NOP); */
    /* 	_NRF_CS_H; */
    /* } while ((nrfStatus & 0x30) == 0); */
    _NRF_CE_L;

    _delay_ms(1000);

    DDRB ^= 1<<LED;
}

int main(void) {
    setup();
    for(;;) {
	loop();
    }
};
