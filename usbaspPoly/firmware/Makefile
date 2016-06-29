#
#   Makefile for usbasp
#   20061119   Thomas Fischl        original
#   20061120   Hanns-Konrad Unger   help: and TARGET=atmega48 added
#   20100620   Jerome Collin        added fifo.o and usart.o to compile and fusesM*
#   20120206   Laurent Tremblay     added fusesM324pa
#

# TARGET=atmega8    HFUSE=0xc9  LFUSE=0xef
# TARGET=atmega48   HFUSE=0xdd  LFUSE=0xff
# TARGET=at90s2313
TARGET=atmega8


# ISP=bsd      PORT=/dev/parport0
# ISP=ponyser  PORT=/dev/ttyS1
# ISP=stk500   PORT=/dev/ttyS1
# ISP=usbasp   PORT=/dev/usb/ttyUSB0
# ISP=stk500v2 PORT=/dev/ttyUSB0
ISP=usbasp
PORT=/dev/usb/ttyS0

help:
	@echo "Usage: make                same as make help"
	@echo "       make help           same as make"
	@echo "       make main.hex       create main.hex"
	@echo "       make clean          remove redundant data"
	@echo "       make disasm         disasm main"
	@echo "       make flash          upload main.hex into flash of ATMega8 (48)"
	@echo "       make fusesM8        program fuses of ATMega8"
	@echo "       make fusesM48       program fuses of ATMega48"
	@echo "       make fusesM16       program fuses of ATMega16"
	@echo "       make fusesM324p     program fuses of ATMega324p"
	@echo "       make fusesM324pa    program fuses of ATMega324pa"
	@echo "       make fusesM644p     program fuses of ATMega644p"
	@echo "       make avrdude        test avrdude"
	@echo "Current values:"
	@echo "       TARGET=${TARGET}"
	@echo "       CLOCK=12000000"
	@echo "       ISP=${ISP}"
	@echo "       PORT=${PORT}"

COMPILE = avr-gcc -Wall -O2 -Iusbdrv -I. -mmcu=$(TARGET) # -DDEBUG_LEVEL=2

OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o fifo.o \
 usart.o isp.o clock.o main.o

.c.o:
	$(COMPILE) -c $< -o $@
#-Wa,-ahlms=$<.lst

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

clean:
	rm -f main.hex main.lst main.obj main.cof main.list main.map main.eep.hex main.bin *.o main.s usbdrv/*.o

# file targets:
main.bin:	$(OBJECTS)
	$(COMPILE) -o main.bin $(OBJECTS) -Wl,-Map,main.map

main.hex:	main.bin
	rm -f main.hex main.eep.hex
	avr-objcopy -j .text -j .data -O ihex main.bin main.hex
#	./checksize main.bin
# do the checksize script as our last action to allow successful compilation
# on Windows with WinAVR where the Unix commands will fail.

disasm:	main.bin
	avr-objdump -d main.bin

cpp:
	$(COMPILE) -E main.c

flash:
	avrdude -c ${ISP} -p ${TARGET} -P ${PORT} -U flash:w:main.hex

# Fuse atmega8 and atmega16 high byte HFUSE:
# 0xc9 = 1 1 0 0   1 0 0 1 <-- BOOTRST (boot reset vector at 0x0000)
#        ^ ^ ^ ^   ^ ^ ^------ BOOTSZ0
#        | | | |   | +-------- BOOTSZ1
#        | | | |   + --------- EESAVE (don't preserve EEPROM over chip erase)
#        | | | +-------------- CKOPT (full output swing)
#        | | +---------------- SPIEN (allow serial programming)
#        | +------------------ WDTON (WDT not always on)
#        +-------------------- RSTDISBL (reset pin is enabled)
# Fuse atmega8 and atmega16 low byte LFUSE:
# 0x9f = 1 0 0 1   1 1 1 1
#        ^ ^ \ /   \--+--/
#        | |  |       +------- CKSEL 3..0 (external >8M crystal)
#        | |  +--------------- SUT 1..0 (crystal osc, BOD enabled)
#        | +------------------ BODEN (BrownOut Detector enabled)
#        +-------------------- BODLEVEL (2.7V)
#
fusesM8:
	avrdude -c ${ISP} -p ${TARGET} -P ${PORT} -u \
    -U hfuse:w:0xC9:m -U lfuse:w:0x9F:m

fusesM16:
	avrdude -c ${ISP} -p atmega16 -P ${PORT} -u \
    -U hfuse:w:0xC9:m -U lfuse:w:0x9F:m    

#
# Fuse atmega48 
#
# high byte hfuse:
# 0xdf = 1 1 0 1   1 1 1 1     factory setting
#        ^ ^ ^ ^   ^ \-+-/
#        | | | |   |   +------ BODLEVEL (Brown out disabled)
#        | | | |   + --------- EESAVE (don't preserve EEPROM over chip erase)
#        | | | +-------------- WDTON (WDT not always on)
#        | | +---------------- SPIEN (allow serial programming)
#        | +------------------ DWEN (debug wire is disabled)
#        +-------------------- RSTDISBL (reset pin is enabled)
# 0xdd = ext.reset, no DW, SPI, no watchdog, no save eeprom, BOD 2.7V
# 
# low byte lfuse:
# 0x62 = 0 1 1 0   0 0 1 0     factory setting
#        ^ ^ \ /   \--+--/
#        | |  |       +------- CKSEL 3..0 (internal 8Mhz Oszillator)
#        | |  +--------------- SUT 1..0 (start-up time)
#        | +------------------ CKOUT (no clock output)
#        +-------------------- CKDIV8 (divide clock by 8)
# 0xdc = divide/1,no clock output,fast raising power,low Pw Oszil. 3..8 Mhz
# 0xe0 = divide/1,no clock output,fast raising power,external Oszil.
# 0xff = divide/1,no clock output,slow raising power,low Pw Oszil 8..  Mhz

fusesM48:
	avrdude -c ${ISP} -p ${TARGET} -P ${PORT} -u \
    -U hfuse:w:0xDF:m -U lfuse:w:0x62:m

#
# Fuses for ATMega164p, ATMega324p, ATMega324pa and ATMega644p
#
# Extended fuse byte: 0xfd
# 0xfd = 1 1 1 1   1 1 0 1
#        \         / \-+-/
#             |        +------ BODLEVEL 2..0 (2.7 Volt for brown out detection)`
#             +--------------- Reserved
# Fuse high byte:
# 0xD9 = 1 1 0 1   1 0 0 1 <-- BOOTRST (boot reset vector at 0x0000)
#        ^ ^ ^ ^   ^ ^ ^------ BOOTSZ0
#        | | | |   | +-------- BOOTSZ1
#        | | | |   + --------- EESAVE (don't preserve EEPROM over chip erase)
#        | | | +-------------- WDTON (WDT not always on)
#        | | +---------------- SPIEN (allow serial programming)
#        | +------------------ JTAG (Enable JTAG)
#        +-------------------- OCDEN (On Chip Debug Enabled, 0 = enabled)
#
# Fuse low byte:
# 0xd7 = 1 1 0 1   0 1 1 1
#        ^ ^ \ /         +---- CKSEL 0 (crystal, BOD enabled) 
#        | |  |    \-+-/------ CKSEL 3..1 (full swing crystal Oscillator)
#        | |  +--------------- SUT 1..0 (crystal osc, BOD enabled)
#        | +------------------ CKOUT (clock output on port B1, 1 = unprogrammed)
#        +-------------------- CKDIV (input clock divided by 8, 1 = unprogrammed)
fusesM164p:
	avrdude -c ${ISP} -p atmega164p -P ${PORT} \
      -U efuse:w:0xFD:m -U hfuse:w:0xD9:m -U lfuse:w:0xD7:m

fusesM324p:
	avrdude -c ${ISP} -p atmega324p -P ${PORT} \
      -U efuse:w:0xFD:m -U hfuse:w:0xD9:m -U lfuse:w:0xD7:m

fusesM324pa:
	avrdude -c ${ISP} -p atmega324pa -P ${PORT} \
      -U efuse:w:0xFD:m -U hfuse:w:0xD9:m -U lfuse:w:0xD7:m

fusesM644p:
	avrdude -c ${ISP} -p atmega644p -P ${PORT} \
      -U efuse:w:0xFD:m -U hfuse:w:0xD9:m -U lfuse:w:0xD7:m

#
# Very useful just to test the software side on PC
#
avrdude:
	avrdude -c ${ISP} -p ${TARGET} -P ${PORT} -v

SERIAL = `echo /dev/tty.USA19QI*`
UISP = uisp -dprog=$S -dserial=$(SERIAL) -dpart=auto
# The two lines above are for "uisp" and the AVR910 serial programmer connected
# to a Keyspan USB to serial converter to a Mac running Mac OS X.
# Choose your favorite programmer and interface.

uisp:	all
	$(UISP) --erase
	$(UISP) --upload --verify if=main.hex

