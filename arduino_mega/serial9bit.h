#ifndef __SERIAL_9_BIT_H__
#define __SERIAL_9_BIT_H__

#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <Arduino.h>
#include "HardwareSerial.h"
#include "print.h"
//#include "forward.h"
#ifndef fosc 
#define fosc 16000000UL
#endif
// register bits

//#define USART0  0
//#define USART1  1
//#define USART2  0
//#define USART3  0
/*
#ifdef USART0
#define ucsra UCSR0A
#define ucsrb UCSR0B
#define ucsrc UCSR0C
#define ubrrh UBRR0H
#define ubrrl  UBRR0L
#define  usart_rx_vect                USART_RX_vect
#define usart_udre_vect             USART_UDRE_vect
#define udr UDR0
#endif
*/
//#ifdef USART1
#define ucsra UCSR1A
#define ucsrb UCSR1B
#define ucsrc UCSR1C
#define ubrrh UBRR1H
#define ubrrl  UBRR1L
#define  usart_rx_vect                USART1_RX_vect
#define usart_udre_vect             USART1_UDRE_vect
#define udr UDR1
//#endif
/*
#ifdef USART2
#define ucsra UCSR2A
#define ucsrb UCSR2B
#define ucsrc UCSR2C
#define ubrrh UBRR2H
#define ubrrl  UBRR2L
#define  usart_rx_vect                USART2_RX_vect
#define usart_udre_vect             USART2_UDRE_vect
#define udr UDR2
#endif

#ifdef USART3
#define ucsra UCSR3A
#define ucsrb UCSR3B
#define ucsrc UCSR3C
#define ubrrh UBRR3H
#define ubrrl  UBRR3L
#define  usart_rx_vect                USART3_RX_vect
#define usart_udre_vect             USART3_UDRE_vect
#define udr UDR3
#endif
*/


// enum rega{mpcm,u2x,upe,dor,fe,udre,txc,rxc};
//enum regb{txb8,rxb8,ucsz2,txen,rxen,udrie,txcie,rxcie};
//enum regc{ucpol,ucsz0,ucsz,usbs,upm0,upm,umsel0,umsel};

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

#ifdef __cplusplus
extern "C" {
#endif
void uart_init(uint8_t bits_frame);
uint8_t uart_transmit(uint16_t *bufdt, uint8_t length);
uint8_t uart_receive(uint16_t *bufdr, uint8_t length);
uint8_t mdb_transmit(uint8_t *bufdt, uint8_t length);
uint8_t mdb_receive(uint8_t *bufdr, uint8_t length);
uint8_t transmit_byte(uint16_t *data);
uint8_t receive_byte(uint16_t *data);
uint8_t numb_bits(uint8_t buf_T_R);
#ifdef __cplusplus
}
#endif
#endif
