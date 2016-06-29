/*
 * USBasp - USB in-circuit programmer for Atmel AVR controllers
 *
 * Thomas Fischl <tfischl@gmx.de>
 * Matthew Khouzam, Ecole Polytechnique de Montreal
 * Jerome Collin, Ecole Polytechnique de Montreal
  
 * License........: GNU GPL v2 (see Readme.txt)
 * Target.........: ATMega8 at 12 MHz
 * Creation Date..: 2005-02-20
 * Last change....: 2010-06-21
 *
 * PC2 SCK speed option.
 * GND  -> slow (8khz SCK),
 * open -> software set speed (default is 375kHz SCK)
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

#include "usbasp.h"
#include "usbdrv.h"
#include "isp.h"
#include "clock.h"
#include "usart.h"

static uchar replyBuffer[8];

static uchar prog_state = PROG_STATE_IDLE;
static uchar prog_sck = USBASP_ISP_SCK_AUTO;

static uchar prog_address_newmode = 0;
static unsigned long prog_address;
static unsigned int prog_nbytes = 0;
static unsigned int prog_pagesize;
static uchar prog_blockflags;
static uchar prog_pagecounter;

static struct Fifo TxFifo;
static struct Fifo RxFifo;

static uchar TxData[USBASPTXLEN];
static uchar RxData[USBASPRXLEN];

uchar usbFunctionSetup(uchar data[8]) {

    uchar len = 0;

    switch (data[1]) {
    case USBASP_FUNC_CONNECT:

        /* set SCK speed */
        /*if ((PINC & (1 << PC2)) == 0) {
            ispSetSCKOption(USBASP_ISP_SCK_8);
        } else {
            ispSetSCKOption(prog_sck);
        }*/
        /*
           selon http://innovelectronique.fr/2009/04/29/utilisation-davrdude-avec-usbasp/:
           "Lorsque vous achetez un ATMega celui-ci est configuré pour utiliser l’ocillateur
           interne à 1MHz. Vous ne pouvez donc pas le programmer en vitesse rapide mais 
           en vitesse lente uniquement la première fois. Si vous changez les fusibles de 
           configuration afin d’utiliser un quartz externe (>= à 4MHz) ou si vous choisissez 
           une fréquence plus élevé pour l’oscillateur interne alors vous pouvez passer 
           le programmateur en vitesse rapide. Le choix de la vitesse de programmation 
           s’effectue avec l’interrupteur à glissière en haut du programmateur. A droite 
           vous êtes en vitesse lente, à gauche en vitesse rapide."

           L'horloge est donc ajustee à une vitesse lente pour tous les cas

           Jerome Collin - aout 2010
        */
        ispSetSCKOption(USBASP_ISP_SCK_187_5);

        /* set compatibility mode of address delivering */
        prog_address_newmode = 0;

        ledGreenOff();
        ledRedOn();
        ispConnect();
        break;

    case USBASP_FUNC_DISCONNECT:
        ispDisconnect();
        ledRedOff();
        ledGreenOn();
        break;

    case USBASP_FUNC_TRANSMIT:
        replyBuffer[0] = ispTransmit(data[2]);
        replyBuffer[1] = ispTransmit(data[3]);
        replyBuffer[2] = ispTransmit(data[4]);
        replyBuffer[3] = ispTransmit(data[5]);
        len = 4;
        break;

    case USBASP_FUNC_READFLASH:

        if (!prog_address_newmode)
            prog_address = (data[3] << 8) | data[2];

        prog_nbytes = (data[7] << 8) | data[6];
        prog_state = PROG_STATE_READFLASH;
        len = 0xff; /* multiple in */
        break;

    case USBASP_FUNC_READEEPROM:

        if (!prog_address_newmode)
            prog_address = (data[3] << 8) | data[2];

        prog_nbytes = (data[7] << 8) | data[6];
        prog_state = PROG_STATE_READEEPROM;
        len = 0xff; /* multiple in */
        break;

    case USBASP_FUNC_ENABLEPROG:
        replyBuffer[0] = ispEnterProgrammingMode();
        len = 1;
        break;

    case USBASP_FUNC_WRITEFLASH:

        if (!prog_address_newmode)
            prog_address = (data[3] << 8) | data[2];

        prog_pagesize = data[4];
        prog_blockflags = data[5] & 0x0F;
        prog_pagesize += (((unsigned int) data[5] & 0xF0) << 4);
        if (prog_blockflags & PROG_BLOCKFLAG_FIRST) {
            prog_pagecounter = prog_pagesize;
        }
        prog_nbytes = (data[7] << 8) | data[6];
        prog_state = PROG_STATE_WRITEFLASH;
        len = 0xff; /* multiple out */
        break;

    case USBASP_FUNC_WRITEEEPROM:

        if (!prog_address_newmode)
            prog_address = (data[3] << 8) | data[2];

        prog_pagesize = 0;
        prog_blockflags = 0;
        prog_nbytes = (data[7] << 8) | data[6];
        prog_state = PROG_STATE_WRITEEEPROM;
        len = 0xff; /* multiple out */
        break;

    case USBASP_FUNC_SETLONGADDRESS:

        /* set new mode of address delivering (ignore address delivered in commands) */
        prog_address_newmode = 1;
        /* set new address */
        prog_address = *((unsigned long*) &data[2]);
        break;

    case USBASP_FUNC_SETISPSCK:
    
        /* set sck option */
        prog_sck = data[2];
        replyBuffer[0] = 0;
        len = 1;
        break;

    case USBASP_FUNC_SETSERIOS:

        replyBuffer[0] = usart_setbaud(data[2]);
        replyBuffer[1] = usart_setbits(data[3]);
        replyBuffer[2] = usart_setparity(data[4]);
        replyBuffer[3] = 0x00;
        
        usart_init( &TxFifo, TxData, USBASPTXLEN, 
                    &RxFifo, RxData, USBASPRXLEN);
        len = 4;
        break;

    case USBASP_FUNC_READSER:

        if (!prog_address_newmode)
            prog_address = (data[3] << 8) | data[2];
        prog_nbytes = (data[7] << 8) | data[6];
        prog_state = PROG_STATE_READSER;
        len = 0xff; /* multiple in */
        break;

    case USBASP_FUNC_WRITESER:
    
        if (!prog_address_newmode)
            prog_address = (data[3] << 8) | data[2];
        prog_pagesize = 0;
        prog_blockflags = 0;
        prog_nbytes = (data[7] << 8) | data[6];
        prog_state = PROG_STATE_WRITESER;
        len = 0xff; /* multiple out */
        break;
        
    default: 
        // do nothing
        break;
    };

    usbMsgPtr = replyBuffer;

    return len;
}

uchar usbFunctionRead(uchar *data, uchar len) {

  uchar i;
  uint16_t tmpData;
  uchar tmpLen = len - 1;
  uchar done = 0;

  /* check if programmer is in correct read state */
  if ((prog_state != PROG_STATE_READFLASH) &&
      (prog_state != PROG_STATE_READEEPROM) &&
      (prog_state != PROG_STATE_READSER))
  {
    return 0xff;
  }

  /* fill packet */
  switch(prog_state)
  {
    case PROG_STATE_READFLASH:
        for (i = 0; i < len; i++)
        {
            data[i] = ispReadFlash(prog_address);
            prog_address++;
        }
        break;
    case PROG_STATE_READEEPROM:
        for (i = 0; i < len; i++)
        {
            data[i] = ispReadEEPROM(prog_address);
            prog_address++;
        }
        break;
    case PROG_STATE_READSER:
        data[0] = tmpLen;
        prog_address++;
        for (i = 1; i < tmpLen; i++)
        {
            tmpData = fifo_dequeue(&RxFifo);
            data[i] = tmpData;
            prog_address++;
            if ( done == 0 && (tmpData & 0xFF00) > 0 ) {
                done = 1;
                data[0] = i - 1;
            }
        }
        break;
    default:
        // do nothing
        break;
  };
  
  /* last packet? */
  if (len < 8) {
    prog_state = PROG_STATE_IDLE;
  }

  return len;
}

uchar usbFunctionWrite(uchar *data, uchar len) {

    uchar retVal = 0;
    uchar i;
    uchar serLen = 0;

    /* check if programmer is in correct write state */
    if ((prog_state != PROG_STATE_WRITEFLASH) &&
        (prog_state != PROG_STATE_WRITEEEPROM)&&
        (prog_state != PROG_STATE_WRITESER)) 
    {
        return 0xff;
    }
  

    for (i = 0; i < len; i++) 
    {
        switch (prog_state)
        {
        case PROG_STATE_WRITEFLASH:
            /* Flash */
    
            if (prog_pagesize == 0) 
            {
                /* not paged */
                ispWriteFlash(prog_address, data[i], 1);
            } 
            else 
            {
                /* paged */
                ispWriteFlash(prog_address, data[i], 0);
                prog_pagecounter --;
                if (prog_pagecounter == 0) 
                {
                    ispFlushPage(prog_address, data[i]);
                    prog_pagecounter = prog_pagesize;
                }
            }
            break;
        
        case PROG_STATE_WRITEEEPROM:
            /* EEPROM */
            ispWriteEEPROM(prog_address, data[i]);
            break;

        case PROG_STATE_WRITESER:
            /* serial */
            if ( i == 0 )
            {
               serLen = data[0] + 1;
            }
            else if ( i < serLen )
            {
               fifo_enqueue(&TxFifo, data[i]);
            }
            break;
        
        default:

            //do nothing;
            break;
    };
    prog_nbytes --;

    if (prog_nbytes == 0) 
    {
        prog_state = PROG_STATE_IDLE;
        if ((prog_blockflags & PROG_BLOCKFLAG_LAST) && 
            (prog_pagecounter != prog_pagesize)) 
        {

            /* last block and page flush pending, so flush it now */
            ispFlushPage(prog_address, data[i]);
        }
      
        retVal = 1; // Need to return 1 when no more data is to be received
    }

    prog_address ++;
  }

  return retVal;
}


int main(void) {
    uchar i, j;

    /* no pullups on USB and ISP pins */
    PORTD = 0;
    PORTB = 0;
    
    /* all outputs except PD2 = INT0 */
    DDRD = ~(1 << 2);

    /* output SE0 for USB reset */
    DDRB = ~0;
    j = 0;
    /* USB Reset by device only required on Watchdog Reset */
    while (--j) {
        i = 0;
        /* delay >10ms for USB reset */
        while (--i)
            ;
    }
    /* all USB and ISP pins inputs */
    DDRB = 0;

    /* all inputs except PC0, PC1 */
    DDRC = 0x03;
    PORTC = 0xfe;

    /* init timer */
    clockInit();
    ledGreenOn();

    /* main event loop */
    usbInit();
    sei();
    for (;;) {
        usbPoll();
        if((UCSRA & (1<< UDRE)) != 0 )
        {
            usart_tx( &TxFifo);
        }
        if((UCSRA & (1<< RXC)) != 0)
        {
            usart_rx( &RxFifo);
        }
    }
    return 0;
}

