/*
 * fifo.h - addition to USBasp
 *
 * Autor..........: Matthew Khouzam and Jerome Collin (jerome.collin@polymtl.ca)
 * Description....: fifo for handling rs232 between target and atmega8 (88)
 * Licence........: GNU GPL v2 (see Readme.txt)
 * Creation Date..: 2005-11-20
 * Last change....: 2010-06-20
 */

#ifndef __fifo_h_included__
#define __fifo_h_included__
#include <avr/io.h>

struct Fifo
{
    uint16_t m_begin;
    uint16_t m_end;
    uint16_t m_size;
    uint8_t *m_data;
};

void fifo_init( struct Fifo *fifo, uint8_t *data, uint16_t size  );

uint8_t fifo_enqueue( struct Fifo *fifo , uint8_t data);

uint16_t fifo_dequeue( struct Fifo *fifo  );

uint8_t fifo_empty( const struct Fifo *fifo );

uint8_t fifo_full( const struct Fifo *fifo );


#endif /* __fifo_h_included__ */

