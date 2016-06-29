
# name of the final exacutable to be built goes here
BIN = progmem

# name of the source files without any extension
SRCNAME = $(BIN)

CC = g++

# CFLAGS = -g
# CCFLAGS = -DCPLUSPLUS -g  # for use with C++ if file ext is .cc
CFLAGS = -DCPLUSPLUS -g  # for use with C++ if file ext is .c

SRCS = $(SRCNAME).y $(SRCNAME).l
OBJS = $(SRCNAME).yy.o $(SRCNAME).tab.o
LIBS = -lfl -lm 

$(BIN): $(OBJS)
	$(CC) $(CCFLAGS) $(OBJS) $(LIBS) -o $(BIN)

$(SRCNAME).tab.h $(SRCNAME).tab.c: $(SRCNAME).y
	bison -v -t -d $(SRCNAME).y  

$(SRCNAME).yy.c: $(SRCNAME).l $(SRCNAME).tab.h
	flex -o$(SRCNAME).yy.c $(SRCNAME).l
	sed -e 's/\"syntax error\"/\"erreur de syntaxe\"/g' $(SRCNAME).tab.c  > tmp
	mv tmp $(SRCNAME).tab.c
	rm -f tmp

all:    
	touch $(SRCS)
	make

clean:
	rm -f $(OBJS) $(BIN) $(SRCNAME).yy.c \
		$(SRCNAME).tab.h $(SRCNAME).tab.c $(SRCNAME).output

