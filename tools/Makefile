#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# Makefile for SMS command line tools
#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

TOOLS = \
	smsAnal smsSynth smsClean \
	smsMod smsPrint smsResample \
	smsSynthDet #smsUndB

CFLAGS = -g -Wall
CC = cc
PREFIX=/usr/local
INCLUDES= -I../src
SMSLIB=-L../lib -lsms 
LIBS=  $(SMSLIB) -lsndfile -lm 

all: $(TOOLS)

smsAnal: smsAnal.o
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

smsAnal.o: smsAnal.c
	$(CC) -c smsAnal.c -o smsAnal.o $(CFLAGS) $(INCLUDES)

smsSynthDet: smsSynthDet.o
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@  $(LIBS)

smsSynthDet.o: smsSynthDet.c
	$(CC) -c smsSynthDet.c -o smsSynthDet.o $(CFLAGS) $(INCLUDES)

smsSynth: smsSynth.o
	$(CC) $(CFLAGS) $(INCLUDES) smsSynth.o -o smsSynth  $(LIBS) 

smsSynth.o: smsSynth.c
	$(CC) -c smsSynth.c -o smsSynth.o $(CFLAGS) $(INCLUDES)

smsClean: smsClean.o
	$(CC) $(CFLAGS) $(INCLUDES) smsClean.o -o smsClean  $(LIBS)

smsClean.o: smsClean.c
	$(CC) -c smsClean.c -o smsClean.o $(CFLAGS) $(INCLUDES)

smsMod: smsMod.o
	$(CC) $(CFLAGS) $(INCLUDES) smsMod.o -o smsMod  $(LIBS) 

smsMod.o: smsMod.c
	$(CC) -c smsMod.c -o smsMod.o $(CFLAGS) $(INCLUDES)

smsPrint: smsPrint.o
	$(CC) $(CFLAGS) $(INCLUDES) smsPrint.o -o smsPrint  $(LIBS)

smsPrint.o: smsPrint.c
	$(CC) -c smsPrint.c -o smsPrint.o $(CFLAGS) $(INCLUDES)

smsResample: smsResample.o
	$(CC) $(CFLAGS) $(INCLUDES) smsResample.o -o smsResample  $(LIBS) 

smsResample.o: smsResample.c
	$(CC) -c smsResample.c -o smsResample.o $(CFLAGS) $(INCLUDES)

clean: 
	rm -f *.o $(TOOLS)

install:
	@for i in $(TOOLS); do \
	install -m755 $$i $(PREFIX)/bin; \
	echo "install -m755 $$i /usr/local/bin"; \
	done

uninstall::
	@for i in $(TOOLS); do \
	echo "rm $(PREFIX)/bin/$$i"; \
	rm $(PREFIX)/bin/$$i; done
