#ifndef __SERIAL_9_BIT_H__
#define __SERIAL_9_BIT_H__

#define Tx     0
#define Rx     1

#define BUF_SIZE      128

#ifndef VARIANT_MCK
#define VARIANT_MCK			84000000
#endif

typedef struct{
	uint8_t buf[BUF_SIZE];
	volatile uint8_t read;
	volatile uint8_t write;
}buf_t;

#ifdef __cplusplus
extern "C" {
#endif
	void uart_init();
	uint8_t uart_transmit(uint16_t *bufdt, uint8_t length);
	uint8_t uart_receive(uint16_t *bufdr, uint8_t length);
	uint8_t mdb_transmit(uint8_t *bufdt, uint8_t length);
	uint8_t mdb_receive(uint8_t *bufdr, uint8_t length);
	uint8_t transmit_byte(uint16_t *data);
	uint8_t receive_byte(uint16_t *data);
	void data_flush();
	void USART1_Handler();
	//uint8_t numb_bits(uint8_t buf_T_R);
#ifdef __cplusplus
}
#endif
#endif
