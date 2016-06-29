/*
 * usbasp.c - part of USBasp
 *
 * Autor..........: Thomas Fischl <tfischl@gmx.de>
 * Description....: Definitions and macros for usbasp
 * Licence........: GNU GPL v2 (see Readme.txt)
 * Creation Date..: 2009-02-28
 * Last change....: 2009-02-28
 */

#ifndef USBASP_H_
#define USBASP_H_

/* USB function call identifiers */
#define USBASP_FUNC_CONNECT     1
#define USBASP_FUNC_DISCONNECT  2
#define USBASP_FUNC_TRANSMIT    3
#define USBASP_FUNC_READFLASH   4
#define USBASP_FUNC_ENABLEPROG  5
#define USBASP_FUNC_WRITEFLASH  6
#define USBASP_FUNC_READEEPROM  7
#define USBASP_FUNC_WRITEEEPROM 8
#define USBASP_FUNC_SETLONGADDRESS 9
#define USBASP_FUNC_SETISPSCK  10
#define USBASP_FUNC_SETSERIOS  11
#define USBASP_FUNC_READSER    12
#define USBASP_FUNC_WRITESER   13

/* programming state */
#define PROG_STATE_IDLE         0
#define PROG_STATE_WRITEFLASH   1
#define PROG_STATE_READFLASH    2
#define PROG_STATE_READEEPROM   3
#define PROG_STATE_WRITEEEPROM  4
#define PROG_STATE_READSER      5
#define PROG_STATE_WRITESER     6

/* Block mode flags */
#define PROG_BLOCKFLAG_FIRST    1
#define PROG_BLOCKFLAG_LAST     2

/* ISP SCK speed identifiers */
#define USBASP_ISP_SCK_AUTO   0
#define USBASP_ISP_SCK_0_5    1   /* 500 Hz */
#define USBASP_ISP_SCK_1      2   /*   1 kHz */
#define USBASP_ISP_SCK_2      3   /*   2 kHz */
#define USBASP_ISP_SCK_4      4   /*   4 kHz */
#define USBASP_ISP_SCK_8      5   /*   8 kHz */
#define USBASP_ISP_SCK_16     6   /*  16 kHz */
#define USBASP_ISP_SCK_32     7   /*  32 kHz */
#define USBASP_ISP_SCK_93_75  8   /*  93.75 kHz */
#define USBASP_ISP_SCK_187_5  9   /* 187.5  kHz */
#define USBASP_ISP_SCK_375    10  /* 375 kHz   */
#define USBASP_ISP_SCK_750    11  /* 750 kHz   */
#define USBASP_ISP_SCK_1500   12  /* 1.5 MHz   */

/* for serial handling */
#define USBASP_MODE_SETBAUD300      0x10
#define USBASP_MODE_SETBAUD600      0x11
#define USBASP_MODE_SETBAUD1200     0x12
#define USBASP_MODE_SETBAUD2400     0x13
#define USBASP_MODE_SETBAUD4800     0x14
#define USBASP_MODE_SETBAUD9600     0x15
#define USBASP_MODE_SETBAUD19200    0x16
#define USBASP_MODE_SETBAUD38400    0x17
#define USBASP_MODE_SETBAUD57600    0x18
#define USBASP_MODE_SETBAUD115200   0x19

#define USBASP_MODE_UART5BIT        0x05
#define USBASP_MODE_UART6BIT        0x06
#define USBASP_MODE_UART7BIT        0x07
#define USBASP_MODE_UART8BIT        0x08

#define USBASP_MODE_PARITYN         0x01
#define USBASP_MODE_PARITYE         0x02
#define USBASP_MODE_PARITYO         0x03

/* macros for gpio functions */
#define ledRedOn()    PORTC &= ~(1<< PC0);PORTC |= (1 << PC1)
#define ledRedOff()   PORTC |= (1 << PC0) | (1 << PC1)
#define ledGreenOn()  PORTC &= ~(1<< PC1);PORTC |= (1 << PC0)
#define ledGreenOff() PORTC |= (1 << PC1) | (1 << PC0)

#endif /* USBASP_H_ */
