CC=g++
CFLAGS=-O3 -funroll-loops -c
LDFLAGS=-O2 -lm
SOURCES=MyBot.cc bot.cc state.cc
OBJECTS=$(addsuffix .o, $(basename ${SOURCES}))
EXECUTABLE=MyBot

TESTLDFLAGS=-O2 -lm -lgtest -lpthread
TESTSOURCES=unittest.cc bot.cc state.cc
TESTOBJECTS=$(addsuffix .o, $(basename ${TESTSOURCES}))
TESTEXECUTABLE=UnitTest

#Uncomment the following to enable debugging
CFLAGS+=-g -DDEBUG

all: $(OBJECTS) $(EXECUTABLE) test

test: $(TESTOBJECTS) $(TESTEXECUTABLE)
	./UnitTest

zip:
	rm MyBot.zip
	zip MyBot *.h $(SOURCES)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

$(TESTEXECUTABLE): $(TESTOBJECTS)
	$(CC) $(TESTOBJECTS) -o $@ $(TESTLDFLAGS) 

.cc.o: *.h
	$(CC) $(CFLAGS) $< -o $@

clean: 
	-rm -f ${EXECUTABLE} ${OBJECTS} *.d
	-rm -f ${TESTEXECUTABLE} ${TESTOBJECTS} *.d
	-rm -f debug.txt

.PHONY: all clean

