#
# Makefile simple pour le programme serieViaUSB
# Quelques regles simples.
#
# Jerome Collin
# Fevrier 2007
# 
PROG = serieViaUSB
BINNAME = serieViaUSB

CC = g++

CCFLAGS = -DCPLUSPLUS -g -I . -Wall -O3 -Wformat=0

OBJS = serieViaUSB.o
LIBS = -l usb

$(PROG): $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $(BINNAME)

.cc.o:
	$(CC) $(CCFLAGS) -c $*.cc

all: $(PROG)
	
clean:
	rm -f $(OBJS) $(PROG) *~

