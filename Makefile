CC=g++
CFLAGS=-c -Wall -O2 #-O3 -funroll-loops -c
LDFLAGS=-lm #-O2
SOURCES=MyBot.cc bot.cc state.cc
HEADERS=bot.h state.h
OBJECTS=$(addsuffix .o, $(basename ${SOURCES}))
EXECUTABLE=MyBot

TESTLDFLAGS=-lm -lgtest -lpthread -O2
TESTSOURCES=unittest.cc bot.cc state.cc
TESTOBJECTS=$(addsuffix .o, $(basename ${TESTSOURCES}))
TESTEXECUTABLE=UnitTest

VIZLDFLAGS=-O2 -lm -lGL -lGLU -lglut
VIZSOURCES=VizBot.cc bot.cc state.cc
VIZOBJECTS=$(addsuffix .o, $(basename ${VIZSOURCES}))
VIZEXECUTABLE=VizBot

UTILLDFLAGS=-lm -lGL -lGLU -lglut -O2
UTILSOURCES=t3.cc bot.cc state.cc
UTILOBJECTS=$(addsuffix .o, $(basename ${UTILSOURCES}))
UTILEXECUTABLE=t3

#Uncomment the following to enable debugging
CFLAGS+=-g -DDEBUG 
#Comment the following to enable assertions 
#CFLAGS+=-DNDEBUG

all: $(OBJECTS) $(EXECUTABLE) $(VIZEXECUTABLE) $(UTILEXECUTABLE) test

test: $(TESTOBJECTS) $(TESTEXECUTABLE)
	tools/test_bot.sh ./MyBot
	./UnitTest

zip:
	rm MyBot.zip
	zip MyBot *.h $(SOURCES)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(TESTEXECUTABLE): $(TESTOBJECTS)
	$(CC) $(TESTOBJECTS) -o $@ $(TESTLDFLAGS) 

$(VIZEXECUTABLE): $(VIZOBJECTS)
	$(CC) $(VIZOBJECTS) -o $@ $(VIZLDFLAGS) 

$(UTILEXECUTABLE): $(UTILOBJECTS)
	$(CC) $(UTILOBJECTS) -o $@ $(UTILLDFLAGS) 

.cc.o: *.h
	$(CC) $(CFLAGS) $< -o $@

clean: 
	-rm -f ${EXECUTABLE} ${OBJECTS} *.d
	-rm -f ${TESTEXECUTABLE} ${TESTOBJECTS} *.d
	-rm -f ${VIZEXECUTABLE} ${VIZOBJECTS} *.d
	-rm -f ${UTILEXECUTABLE} ${UTILOBJECTS} *.d
	-rm -f debug.txt

.PHONY: all clean

