/*
 * usart.h - addition to USBasp
 *
 * Autor..........: Matthew Khouzam and Jerome Collin (jerome.collin@polymtl.ca)
 * Description....: usart setup to redirect data from target back to atmega8 (88)
 * Licence........: GNU GPL v2 (see Readme.txt)
 * Creation Date..: 2005-11-20
 * Last change....: 2010-06-20
 */

#ifndef __usart_h_included__
#define __usart_h_included__

#include <avr/io.h>

#include "fifo.h"

/* 
 * take 512 bytes for both fifos. 
 *  Jerome: modification: 256 only
 */

#ifndef USBASPTXLEN
    #define USBASPTXLEN             (1 << 7)
#endif

#ifndef USBASPRXLEN
    #define USBASPRXLEN             (1 << 7)
#endif

#define _N_BAUD_ 10

const static uint16_t baud[_N_BAUD_] = {
     2499   /* 300 */
    ,1249   /* 600 */
    ,624    /* 1200 */
    ,311    /* 2400 */
    ,155    /* 4800 */
    ,77     /* 9600 */
    ,38     /* 19200 */
    ,18     /* 38400 */
    ,12     /* 57600 */
    ,6      /* 115200 */
};

uint8_t usart_setbaud( uint8_t speed);

uint8_t usart_setbaud( uint8_t speed);
 
uint8_t usart_setbits( uint8_t bits);

uint8_t usart_setparity(uint8_t bits);

uint8_t usart_init(struct Fifo *TxQueue, uint8_t *TxData, uint16_t TxLen, 
                   struct Fifo *RxQueue, uint8_t *RxData, uint16_t RxLen);

void usart_stop( void );

void usart_tx(struct Fifo *fifo );

void usart_rx(struct Fifo *fifo );

#endif /* __usart_h_included__ */
