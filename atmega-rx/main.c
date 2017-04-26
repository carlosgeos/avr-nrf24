#define F_CPU 8000000UL

#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nRF24L01P.h"

#define LED       PB1
#define RELAY     PD7
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

void setup(void);
void loop(void);
int main(void);
uint8_t spi_transfer(uint8_t data);

char rx_payload[6];
const char password[6] = "carlos";

void activate_relay() {
    sbi(PORTD, RELAY);
    _delay_ms(100);
    cbi(PORTD, RELAY);
}


void flash() {			/* Some feedback to test */
    sbi(PORTB, LED);
    _delay_ms(20);
    cbi(PORTB, LED);
    _delay_ms(20);
    sbi(PORTB, LED);
    _delay_ms(20);
    cbi(PORTB, LED);
    _delay_ms(20);
    sbi(PORTB, LED);
    _delay_ms(20);
    cbi(PORTB, LED);
    _delay_ms(20);
}

void clear_recv() {
    _NRF_CS_L;
    spi_transfer(NRF_W_REGISTER | NRF_STATUS);
    spi_transfer(NRF_RX_DR);
    _NRF_CS_H;
}


/* This Interrupt Service Routine runs with global interrupts initally
 * disabled */
ISR(INT0_vect) {
    flash();
    /* Quit RX mode */
    _NRF_CE_L;

    /* read payload */
    _NRF_CS_L;
    spi_transfer(NRF_R_RX_PAYLOAD);
    size_t i;
    for (i = 0; i < 6; ++i) {
	rx_payload[i] = spi_transfer(NRF_NOP);
    }
    _NRF_CS_H;

    /* Clear receive bit */
    clear_recv();

    //if (strcmp(rx_payload, password) != 0) activate_relay();
    if (rx_payload[0] == 'c') {
	activate_relay();
    }

}


uint8_t spi_transfer(uint8_t data) {
    SPDR = data;
    while (!(SPSR & _BV(SPIF)));
    return SPDR;
}

uint8_t addr[5];

void config_module() {
    /* Module setup */
    _NRF_CS_L;
    spi_transfer(NRF_W_REGISTER | NRF_RF_SETUP);
    spi_transfer(NRF_DR_1MBPS | NRF_TX_PWR_0DB);
    _NRF_CS_H;


    _NRF_CS_L;
    spi_transfer(NRF_W_REGISTER | NRF_RF_CH);
    spi_transfer(0x40);		/* 2.4 GHz + 64 MHz */
    _NRF_CS_H;


    /* write NRF RX Addr P0 */
    _NRF_CS_L;
    spi_transfer(NRF_W_REGISTER | NRF_RX_ADDR_P0);
    spi_transfer(0xB3);
    spi_transfer(0xB3);
    spi_transfer(0xB3);
    spi_transfer(0xB3);
    spi_transfer(0xB3);
    _NRF_CS_H;

    // SPI : write NRF default config : POWER_UP and EN_CRC / PWR_RX disable
    _NRF_CS_L;
    spi_transfer(NRF_W_REGISTER | NRF_CONFIG);
    spi_transfer(NRF_PWR_UP | NRF_EN_CRC | NRF_PRIM_RX);
    _NRF_CS_H;


    _NRF_CS_L;
    spi_transfer(NRF_W_REGISTER | NRF_RX_PW_P0);
    spi_transfer(0x06);
    _NRF_CS_H;

}

void config_spi() {
    // nRF24L01+ (Chip enable activates Rx or Tx mode)
    sbi(DDRB,  NRF_CS);   // CS   = output
    sbi(DDRD,  NRF_CE);   // CE   = output
    sbi(PORTB, NRF_CS);

    // SPI bus setup
    cbi(DDRB,  SPI_MISO); // MISO = input
    //sbi(PORTB, SPI_MISO); // MISO = input with pullup on (DI)
    sbi(DDRB,  SPI_MOSI); // MOSI = output
    sbi(DDRB,  SPI_CLK);  // SCK  = output

    // set as master
    sbi(SPCR, MSTR);
    // set clock as f_osc / 16 = 500kHz if 8MHz clock
    sbi(SPCR, SPR0);
    // enable SPI
    sbi(SPCR, SPE);
}

void setup(void) {
    sbi(DDRB, LED);
    sbi(DDRD, RELAY);

    /* Sleep setup */
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    /* Interrupts setup (low level on INT0, since the IRQ pin is
     * active low). Default EICRA is good */
    sbi(EIMSK, INT0);		/* External Interrupt Mask Register */
    /* SPI setup */
    config_spi();
    /* Module setup */
    config_module();
}

void loop() {
    /* Enter RX Mode */
    _NRF_CE_H;
    _delay_us(130);		/* Time the chip takes to start listening */


    sei();
    sleep_mode();
}

int main(void) {
    cli();
    _delay_ms(100);		/* Wait for module to power up */
    setup();
    for (; ; ) {
    	loop();
    }
};
