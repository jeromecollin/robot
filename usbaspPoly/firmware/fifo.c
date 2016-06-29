/*
 * fifo.c - addition to USBasp
 *
 * Autor..........: Matthew Khouzam and Jerome Collin (jerome.collin@polymtl.ca)
 * Description....: fifo for handling rs232 between target and atmega8 (88)
 * Licence........: GNU GPL v2 (see Readme.txt)
 * Creation Date..: 2005-11-20
 * Last change....: 2010-06-20
 */

#include "fifo.h"

/*
  * fifo_init : initialiser le fifo 
 * Arguments: 
 *      struct Fifo *fifo   - la stucture du Fifo sans les données
 *      uint8_t *data       - les données du Fifo
 *      uint16_t size       - la nombre de UINT8_T dans data
*/
void fifo_init( struct Fifo *fifo, uint8_t *data, uint16_t size)
{
    if(fifo != 0x00)
    {
        fifo->m_begin = 0;
        fifo->m_end = 0;
        fifo->m_data = data;
        fifo->m_size = size;
    }
}

/*
 * fifo_enqueue : Ajouter un element au fifo
 * Arguments:
 *      struct Fifo *fifo   - la structure du Fifo initialisé
 *      uint8_t data        - la donnée à rajouter au Fifo.
 * Retourne:
 *      uint8_t
 *          0               - Succes
 *          1               - Échec
 *          2               - Échec, Fifo non-initialisé
 */
uint8_t fifo_enqueue( struct Fifo *fifo , uint8_t data)
{
    uint8_t rc=0x02;
    // Vérifie que la queue n'est pas vide
    if(fifo != 0x00)
    {
        // Si le Fifo est plein retourne 1, sinon retourne 0
        if( fifo_full(fifo) == 0x01 )
            rc = 0x01;
        else
        {
            fifo->m_data[fifo->m_end]=data; 
            fifo->m_end=( ( fifo->m_end + 1 ) & 
                ( fifo->m_size - 1 ) );     
            rc = 0x00;
        }
    }
    return rc;
}

/*
 * fifo_dequeue : Enlever un element du fifo
 * Arguments:
 *      struct Fifo *fifo   - la structure du Fifo initialisé
 * Retourne:
 *      uint16_t
 *          0x00XX               - Succes + la donnée dans l'octet le plus faible
 *          0X01FF               - Échec
 *          0X02FF               - Échec, Fifo non-initialisé
 */
uint16_t fifo_dequeue( struct Fifo *fifo )
{
    uint16_t rc = 0x02FF;
    if( fifo != 0x00 )
    {
        // si le fifo est vide, retounre 0x01ff, sinon, retourne 0x00XX 
        // où XX est la valeur de 8 bits
        if( fifo_empty(fifo)== 0x01 )
        {
            rc = 0x01FF; 
        }
        else
        {
            rc = (uint16_t)fifo->m_data[fifo->m_begin];
            fifo->m_begin = ( ( fifo->m_begin + 1  )
                             & ( fifo->m_size - 1 ) );
        }
    }
    return rc;
}

/*
 * fifo_empty : Est-ce que le fifo est vide?
 * Arguments:
 *      struct Fifo *fifo   - la structure du Fifo initialisé
 * Retourne:
 *      uint8_t
 *          0               - non-vide
 *          1               - vide
 */
uint8_t fifo_empty( const struct Fifo *fifo)
{
   // si le fifo est vide.
    uint8_t rc  = 
        (uint8_t)( fifo->m_begin == fifo->m_end );
    return rc;
}

/*
 * fifo_empty : Est-ce que le fifo est plein?
 * Arguments:
 *      struct Fifo *fifo   - la structure du Fifo initialisé
 * Retourne:
 *      uint8_t
 *          0               - non-plein
 *          1               - plein
 */
uint8_t fifo_full( const struct Fifo *fifo )
{
    // si le fifo est plein
    uint8_t rc = (uint8_t)
        ( fifo->m_begin == 
        ( ( fifo->m_end + 1 ) 
        & ( fifo->m_size - 1 ) ) );
    return rc;
}
