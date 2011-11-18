CC=g++
CFLAGS=-O3 -funroll-loops -c
LDFLAGS=-O2 -lm
SOURCES=main.cc Bot.cc State.cc
OBJECTS=$(addsuffix .o, $(basename ${SOURCES}))
EXECUTABLE=MyBot

#Uncomment the following to enable debugging
CFLAGS+=-g -DDEBUG

all: $(OBJECTS) $(EXECUTABLE)

zip:
	rm MyBot.zip
	zip MyBot Makefile *.h *.cc

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cc.o: *.h
	$(CC) $(CFLAGS) $< -o $@

clean: 
	-rm -f ${EXECUTABLE} ${OBJECTS} *.d
	-rm -f debug.txt

.PHONY: all clean

