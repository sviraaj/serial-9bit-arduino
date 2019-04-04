/*
 * GccLibrary1.cpp
 *
 * Created: 6/3/2016 5:48:01 PM
 * Author : viraaj
 */ 

#include "sam.h"
#include "library.h"

#include "Arduino.h"

/* Replace with your library code */

buf_t buffer[2] = { { {}, 0, 0 }, { {}, 0, 0 } };

void uart_init() 
{
	////PIOA config
	
	//disable interrupts from PIOA
	PIOA->PIO_IDR = PIO_IDR_P12|PIO_IDR_P13;
	
	//configure PIOA to be assigned to peripherals
	PIOA->PIO_PDR = PIO_PDR_P12|PIO_PDR_P13;
	
	//Now assign peripheral A to the pins
	PIOA->PIO_ABSR &= ~(PIO_ABSR_P12|PIO_ABSR_P13);
	
	//pull up enable TX and RX
	 PIOA->PIO_PUER = PIO_PA12A_RXD1 | PIO_PA13A_TXD1;
	 
	 /////UART config
	 
	 //Enable the peripheral usart controller
	 PMC->PMC_PCER0 = 1 << ID_USART1;
	 
	 //reset receiver and transmitter and disable them
    USART1->US_CR = US_CR_RSTRX|US_CR_RSTTX|US_CR_RXDIS|US_CR_TXDIS;
	
	//mode selection
	USART1->US_MR = US_MR_PAR_NO|US_MR_MODE9; //9 bit, no parity, sync mode
	
	//baud rate set 9600

	USART1->US_BRGR = (SystemCoreClock / 9600) >> 4; //84000000/16*9600=546.875
	
	// Disable PDC channel requests
	USART1->US_PTCR = US_PTCR_RXTDIS | US_PTCR_TXTDIS;
	
	//interrupt disable
	USART1->US_IDR = 0xFFFFFFFF;
	
	//enable interrupt in NVIC 
	NVIC_EnableIRQ((IRQn_Type) ID_USART1);
	
	//enable receiver interrupt
	USART1->US_IER = US_IER_RXRDY; //enable bit set
	
	//enable receiver and transmitter
	//USART1->US_CR &= ~(US_CR_RXDIS|US_CR_TXDIS);
	USART1->US_CR = US_CR_RXEN|US_CR_TXEN;
	buffer[Tx].write = 0; 
	buffer[Tx].read = 0;
	return ;
}


void USART1_Handler(void)
{
	
	uint32_t status = USART1->US_CSR;
	
	if((status & US_CSR_TXRDY) == US_CSR_TXRDY) //US_THR reg empty?
	{
		if (buffer[Tx].write == buffer[Tx].read) 	USART1->US_IDR = (US_IDR_TXRDY); //disable transmitter interrupt
					
		else{
			uint16_t thr = 0x000;
			if (buffer[Tx].buf[buffer[Tx].read] & 0x01) {
				thr = 1<<8;
				buffer[Tx].read = (buffer[Tx].read + 1) % BUF_SIZE;
			}
			else{
				thr = 0x000;
				buffer[Tx].read = (buffer[Tx].read + 1) % BUF_SIZE;
			}

			if (buffer[Tx].write == buffer[Tx].read)  USART1->US_IDR = (US_IDR_TXRDY); //disable transmitter interrupt
			else{
				thr |= buffer[Tx].buf[buffer[Tx].read];
				buffer[Tx].read = (buffer[Tx].read + 1) % BUF_SIZE;
				//write thr to US_THR reg
				USART1->US_THR = US_THR_TXCHR(thr);
					
						
			}
		}	
		
	}

    if((status & US_CSR_RXRDY) == US_CSR_RXRDY) //US_RHR reg ready?
     {
		 	uint16_t rhr = 0x000;
			rhr = (USART1->US_RHR) & US_RHR_RXCHR_Msk;
			buffer[Rx].buf[buffer[Rx].write] = (rhr >> 8) & 0x01;
		 	buffer[Rx].write = (buffer[Rx].write + 1) % BUF_SIZE;
		 	buffer[Rx].buf[buffer[Rx].write] = rhr & 0xFF;
		 	buffer[Rx].write = (buffer[Rx].write + 1) % BUF_SIZE;
		 	
		 	
	 }


}


uint8_t uart_transmit(uint16_t *bufdt, uint8_t length)
{
	uint8_t i = length;
	while (length){
		//while (buffer[Tx].write - buffer[Tx].read >= BUF_SIZE - 1);
		buffer[Tx].buf[buffer[Tx].write] = (uint8_t)(bufdt[i - length] >> 8);	
		buffer[Tx].write = (buffer[Tx].write + 1) % BUF_SIZE;
		buffer[Tx].buf[buffer[Tx].write] = (uint8_t)(bufdt[i - length] & 0xFF);
		buffer[Tx].write = (buffer[Tx].write + 1) % BUF_SIZE;
		
		//enable interrupt when new data available in buffer
		USART1->US_IER = US_IER_TXRDY; //interrupt enable bit set
		//length inc
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
		USART1->US_IER = US_IER_TXRDY; //interrupt enable bit set
		//length inc
		length--;
	}
	return 0;
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

uint8_t transmit_byte(uint16_t *data){
	return (uart_transmit(data, 1));
}

uint8_t receive_byte(uint16_t *data){
	return (uart_receive(data, 1));
}

void data_flush(){
	buffer[Rx].read = buffer[Rx].write;
}