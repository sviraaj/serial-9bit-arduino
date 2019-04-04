
#ifndef F_CPU
#define F_CPU       16000000UL
#endif

#include <avr/io.h>
#include <inttypes.h>
//#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include "HardwareSerial.h"
#include "mdbex.h"
#include "print.h"
//#include "forward.h"

/*
#define fosc 16000000UL
// register bits
#define ucsra UCSR1A
#define ucsrb UCSR1B
#define ucsrc UCSR1C
#define ubrrh UBRR1H
#define ubrrl  UBRR1L
#define  usart_rx_vect                USART1_RX_vect
#define usart_udre_vect             USART1_UDRE_vect
#define udr UDR1


#define rxc             7          // RXC0
#define txc             6         //TXC0
#define udre           5      //UDRE0
#define fe                4           //FE0
#define dor             3         // DOR0
#define upe            2        //UPE0
#define u2x             1          //U2X0
#define mpcm        0    //MPCM0

#define rxcie             7        //RXCIE0
#define txcie              6        //TXCIE0
#define udrie              5       // UDRIE0
#define rxen               4       //RXEN0
#define txen               3       //TXEN0
#define ucsz2            2     // UCSZ02
#define rxb8               1      //RXB80
#define txb8               0     //TXB80

#define  umsel          7     //UMSEL01
#define  upm             5     // UPM01
#define   usbs            3     // USBS0
#define   ucsz             2    //UCSZ01
#define   ucpol           0    //UCPOL0



//
#define BUF_SIZE      128

#define Tx     0
#define Rx     1

#define baudr 9600


	typedef struct{
		uint8_t buf[BUF_SIZE];
		volatile uint8_t read;
		volatile uint8_t write;
	}buf_t;

	void uart_init(uint8_t bits_frame);
	uint8_t uart_transmit(uint16_t *bufdt, uint8_t length);
	uint8_t uart_receive(uint16_t *bufdr, uint8_t length);
	uint8_t transmit_byte(uint16_t *data);
	uint8_t receive_byte(uint16_t *data);
	uint8_t numb_bits(uint8_t buf_T_R);

*/

buf_t buffer[2] = { { {}, 0, 0 }, { {}, 0, 0 } };
//static FILE mystdout = FDEV_SETUP_STREAM(, NULL, _FDEV_SETUP_WRITE);

void uart_init(uint8_t bits_frame){

	// setting the baud rate 
	uint16_t br = (fosc / 8 / baudr - 1)/2;
	ubrrh = (uint8_t)(br >> 8);
	ubrrl = (uint8_t)br;
	//initialise the number of data bits in a frame
	switch (bits_frame){
	case 5:
		ucsrc = (0 << ucsz);
	case 6:
		ucsrc = (1 << ucsz);
	case 7:
		ucsrc = (2 << ucsz);
	case 8:
		ucsrc = (3 << ucsz);
	case 9:
		ucsrc = (3 << ucsz);
		ucsrb = (1 << ucsz2);
	}
	//mode select
	//ucsrc |= (1 << umsel);

	// interrupt enable
	ucsrb |= (1 << rxcie);
	//enable trnasmitter and receiver
	ucsrb |= (1 << rxen) | (1 << txen);

}

uint8_t numb_bits(uint8_t buf_T_R){
	return buffer[buf_T_R].write - buffer[buf_T_R].read;
}
uint8_t uart_transmit(uint16_t *bufdt, uint8_t length){
	uint8_t i = length;
	while (length){

		//while (buffer[Tx].write - buffer[Tx].read >= BUF_SIZE - 1);
		buffer[Tx].buf[buffer[Tx].write] = (uint8_t)(bufdt[i - length] >> 8);
		buffer[Tx].write = (buffer[Tx].write + 1) % BUF_SIZE;
		buffer[Tx].buf[buffer[Tx].write] = (uint8_t)(bufdt[i - length] & 0xFF);
		buffer[Tx].write = (buffer[Tx].write + 1) % BUF_SIZE;
		//enable interrupt when new data available in buffer
		ucsrb |= (1 << udrie);
		length--;
	}
	return 0;
}

uint8_t mdb_transmit(uint8_t *bufdt, uint8_t length){
	uint8_t i = length;
	while (length){
		//while (buffer[Tx].write - buffer[Tx].read >= BUF_SIZE - 1);
		buffer[Tx].buf[buffer[Tx].write] = 0x00;
		buffer[Tx].write = (buffer[Tx].write + 1) % BUF_SIZE;
		buffer[Tx].buf[buffer[Tx].write] = (bufdt[i - length] & 0xFF);
		buffer[Tx].write = (buffer[Tx].write + 1) % BUF_SIZE;
		//enable interrupt when new data available in buffer
		ucsrb |= (1 << udrie);
		length--;
	}
	return 0;
}


ISR(usart_udre_vect){
	if (buffer[Tx].write == buffer[Tx].read) ucsrb &= ~(1 << udrie);
	else{
		if (buffer[Tx].buf[buffer[Tx].read] & 0x01) {
			ucsrb |= (1 << txb8);
			buffer[Tx].read = (buffer[Tx].read + 1) % BUF_SIZE;
		}
		else{
			ucsrb &= ~(1 << txb8);
			buffer[Tx].read = (buffer[Tx].read + 1) % BUF_SIZE;
		}

		if (buffer[Tx].write == buffer[Tx].read) ucsrb &= ~(1 << udrie);
		else{
			udr = buffer[Tx].buf[buffer[Tx].read];
			buffer[Tx].read = (buffer[Tx].read + 1) % BUF_SIZE;
		}
	}
	return;
}

uint8_t uart_receive(uint16_t *bufdr, uint8_t length){
	uint8_t i = length;
	//null transaction
	while (buffer[Rx].read == buffer[Rx].write) ;
	while (length){
		//incomplete transaction
		while (buffer[Rx].read == buffer[Rx].write) ;
		
			uint8_t hb = buffer[Rx].buf[buffer[Rx].read];
			buffer[Rx].read = (buffer[Rx].read + 1) % BUF_SIZE;
			while (buffer[Rx].read == buffer[Rx].write) ;
			
				uint8_t lb = buffer[Rx].buf[buffer[Rx].read];
				bufdr[i - length] = (hb << 8) | lb;
				buffer[Rx].read = (buffer[Rx].read + 1) % BUF_SIZE;
	
		length--;
	}
	return 0;
}

uint8_t mdb_receive(uint8_t *bufdr, uint8_t length){
	uint8_t i = length;
	//null transaction
	while (buffer[Rx].read == buffer[Rx].write) ;
	while (length){
		//incomplete transaction
		while (buffer[Rx].read == buffer[Rx].write) ;
		
			uint8_t hb = buffer[Rx].buf[buffer[Rx].read];
			buffer[Rx].read = (buffer[Rx].read + 1) % BUF_SIZE;
			while (buffer[Rx].read == buffer[Rx].write);
			
				uint8_t lb = buffer[Rx].buf[buffer[Rx].read];
				bufdr[i - length] = lb;
				buffer[Rx].read = (buffer[Rx].read + 1) % BUF_SIZE;
				length--;
	}
	return 0;
}

ISR(usart_rx_vect){
	buffer[Rx].buf[buffer[Rx].write] = (ucsrb >> rxb8) & 0x01;
	buffer[Rx].write = (buffer[Rx].write + 1) % BUF_SIZE;
	buffer[Rx].buf[buffer[Rx].write] = udr;
	buffer[Rx].write = (buffer[Rx].write + 1) % BUF_SIZE;
	return;
}

uint8_t transmit_byte(uint16_t *data){
	return (uart_transmit(data, 1));
}

uint8_t receive_byte(uint16_t *data){
	return (uart_receive(data, 1));
}

