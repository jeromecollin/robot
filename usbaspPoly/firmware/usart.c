/*
 * usart.c - addition to USBasp
 *
 * Autor..........: Matthew Khouzam and Jerome Collin (jerome.collin@polymtl.ca)
 * Description....: usart setup to redirect data from target back to atmega8 (88)
 * Licence........: GNU GPL v2 (see Readme.txt)
 * Creation Date..: 2005-11-20
 * Last change....: 2010-06-20
 */
 
#include "usart.h"
#include "usbasp.h"

/*
 *  To ajuste the baud rate of the programmer
 */

uint8_t usart_setbaud( uint8_t speed)
{
    if( speed < (_N_BAUD_+ USBASP_MODE_SETBAUD300) && 
            (speed > USBASP_MODE_SETBAUD300) )
    {
        /* set baudrate */
        UBRRL = (baud[speed-0x10]) & 0x00FF;
        UBRRH = (baud[speed-0x10]) >> 8;
        return speed;
    }
    return 0xFE;
}

/*
 * Ajust the some other bits to control the serial transmission
 */
 
uint8_t usart_setbits( uint8_t bits)
{
    uint8_t rc ;
    switch(bits)
    {   
        case USBASP_MODE_UART5BIT:
        {
            UCSRC = ( 1 << URSEL ); 
            rc = bits;
            break;
        }
        
        case USBASP_MODE_UART6BIT:
        {
            UCSRC = ( 1 << URSEL ) | ( 1 << UCSZ0 );
            rc=bits;
            break;
        }
        
        case USBASP_MODE_UART7BIT:
        {
            UCSRC = ( 1 << URSEL ) | ( 1 << UCSZ1 );
            rc=bits;
            break;
        }
        
        case USBASP_MODE_UART8BIT:
        {
            UCSRC = ( 1 << URSEL ) | ( 1 << UCSZ1 ) | ( 1 << UCSZ0 );
            rc=bits;
            break;
        }
        
        default: 
        {
            rc = 1; 
        }
        
    }
    return rc;
}

/*
 *  Ajust the parity as well
 */ 

uint8_t usart_setparity(uint8_t bits)
{
    uint8_t rc; 
    switch(bits)
    {
        case ( USBASP_MODE_PARITYN ) :
        {
            // ne fais rien
            rc=1;
            break;
        }
        case ( USBASP_MODE_PARITYE ) :
        {
            UCSRC |= ( 1 <<  URSEL ) | ( 1 << UPM1 );
            rc=2;
            break;
        }
        case ( USBASP_MODE_PARITYO ) :
        {
            UCSRC |= ( 1 <<  URSEL ) | ( 1 << UPM1 ) | ( 1 << UPM0 );
            rc=3;
            break;
        }
        default:
        {
            rc=0;
        }
    }
    return rc; 
}

/*
 *  First fonction to call, obviously
 */

uint8_t usart_init(struct Fifo *TxQueue, uint8_t *TxData, uint16_t TxLen,
        struct Fifo *RxQueue, uint8_t *RxData, uint16_t RxLen  )
{
    /* activate the queues */
    fifo_init(TxQueue, TxData, TxLen);
    fifo_init(RxQueue, RxData, RxLen);
    /* active usart */
    DDRD |= ( 1 << PD1 );
    PORTD |= ( 1 << 3); 
    UCSRB |= ( 1 << TXEN ) | ( 1 << RXEN );
    return 0;
}

void usart_stop()
{
    UCSRB = 0;
}

/*
 *  Function to call to send a byte from programmer to target
 */

void usart_tx(struct Fifo *TxQueue)
 {
    if( !fifo_empty( TxQueue ) )
    {
        UDR = fifo_dequeue( TxQueue );
    }
 }
 
/*
 *  Function to call to send a byte from target to programmer
 */
 
void usart_rx(struct Fifo *RxQueue)
 {
    if(!fifo_full(RxQueue))
    {
        fifo_enqueue( RxQueue, UDR );
    }
 }
