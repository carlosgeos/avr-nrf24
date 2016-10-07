#define F_CPU 8000000UL
#include "u8g.h"
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nRF24L01P.h"


#define LED       PB1
#define SPI_MOSI  PB3
#define SPI_MISO  PB4
#define SPI_CLK   PB5
#define NRF_CS    PB2
#define _NRF_CS_L cbi(PORTB, NRF_CS);
#define _NRF_CS_H sbi(PORTB, NRF_CS);
#define NRF_CE    PD0
#define _NRF_CE_L cbi(PORTD, NRF_CE);
#define _NRF_CE_H sbi(PORTD, NRF_CE);
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit)) // clear bit
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))  // set bit
#define NRF_RX_CONF (NRF_PWR_UP + NRF_EN_CRC + NRF_PRIM_RX)

void setup(void);
void loop(void);
int main(void);
uint8_t spi_transfer(uint8_t data);

u8g_t u8g;

void u8g_setup(void)
{
  u8g_InitI2C(&u8g, &u8g_dev_ssd1306_128x64_i2c, U8G_I2C_OPT_NONE);
}

void sys_init(void)
{
#if defined(__AVR__)
  /* select minimal prescaler (max system speed) */
  CLKPR = 0x80;
  CLKPR = 0x00;
#endif
}

void draw()
{
    u8g_SetFont(&u8g, u8g_font_10x20);

    uint8_t nrfStatus;
    uint8_t rx_payload;
    _NRF_CS_L;
    nrfStatus = spi_transfer(NRF_NOP);
    _NRF_CS_H;


    char buffer[16];
    /* char buffer[8 + 1]; */
    sprintf(buffer, "REG = 0x%0.2x", nrfStatus);
    /* buffer[8] = "\0"; */
    u8g_DrawStr(&u8g, 5, 20, buffer);

    if (nrfStatus == 0x40) {
	_NRF_CS_L;
	//spi_transfer(NRF_R_REGISTER | NRF_RF_SETUP);
	rx_payload = spi_transfer(NRF_R_RX_PAYLOAD);
	_NRF_CS_H;

	char buffer2[16];
	sprintf(buffer2, "PL = %c", rx_payload);
	u8g_DrawStr(&u8g, 5, 40, buffer2);

	_delay_ms(1000);
	/* Clear receive bit */
	_NRF_CS_L;
	spi_transfer(NRF_W_REGISTER | NRF_STATUS);
	spi_transfer(0x00);
	_NRF_CS_H;



    }
}


uint8_t spi_transfer(uint8_t data) {
    SPDR = data;
    while (!(SPSR & _BV(SPIF)));
    return SPDR;
}

uint8_t addr[5];

void setup(void) {
    sys_init();
    u8g_setup();

    // LED IO
    sbi(DDRB,  LED); // set LED pin as output
    sbi(PORTB, LED); // turn the LED on
    // SPI bus setup

    cbi(DDRB,  SPI_MISO); // MISO = input
    sbi(PORTB, SPI_MISO); // MISO = input with pullup on (DI)
    sbi(DDRB,  SPI_MOSI); // MOSI = output
    sbi(DDRB,  SPI_CLK);  // SCK  = output
    // nRF24L01+ (Chip enable activates Rx or Tx mode)
    sbi(DDRB,  NRF_CS);   // CS   = output
    sbi(DDRD,  NRF_CE);   // CE   = output
    sbi(PORTB, NRF_CS);

    // set as master
    sbi(SPCR, MSTR);
    // enable SPI
    sbi(SPCR, SPE);
    /* Module setup */
    _NRF_CS_L;
    spi_transfer(NRF_W_REGISTER | NRF_RF_SETUP);
    spi_transfer(0x06);		/* 1Mbps, 0db */
    _NRF_CS_H;

    //DDRB ^= 1<<LED;
    _NRF_CS_L;
    spi_transfer(NRF_W_REGISTER | NRF_RF_CH);
    spi_transfer(0x40);		/* 2.4 GHz + 64 MHz */
    _NRF_CS_H;


    // Address width -> 5 bytes by default. In this case: 0xB3B3B3B3B3

    /* _NRF_CS_L; */
    /* spi_transfer(NRF_W_REGISTER | NRF_SETUP_RETR); */
    /* spi_transfer(0x03);		/\* Number of retransmissions *\/ */
    /* _NRF_CS_H; */


    // SPI : write NRF Tx Addr
    /* _NRF_CS_L; */
    /* spi_transfer(NRF_W_REGISTER | NRF_TX_ADDR); */
    /* spi_transfer(0xB3); */
    /* spi_transfer(0xB3); */
    /* spi_transfer(0xB3); */
    /* spi_transfer(0xB3); */
    /* spi_transfer(0xB3); */
    /* _NRF_CS_H; */


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
    //DDRB ^= 1<<LED;

    // SPI : write NRF default config : POWER_UP and EN_CRC / PWR_RX disable
    _NRF_CS_L;
    spi_transfer(NRF_W_REGISTER | NRF_CONFIG);
    spi_transfer(NRF_RX_CONF);
    _NRF_CS_H;

    // SPI : write NRF default config : POWER_UP and EN_CRC / PWR_RX disable
    _NRF_CS_L;
    spi_transfer(NRF_W_REGISTER | NRF_RX_PW_P0);
    spi_transfer(0x01);
    _NRF_CS_H;

    /* Enter RX Mode */
    // NRF CE pulse (minimum 10us)
    _NRF_CE_H;

    DDRB ^= 1<<LED;
    u8g_FirstPage(&u8g);
    do
    {
	draw();
    } while ( u8g_NextPage(&u8g) );
    u8g_Delay(100);
}

int main(void) {
    setup();
    for(;;) {
	loop();
    }
};
