# ::::::::::::::
# library/Makefile
# ::::::::::::::

#include ../Makefile

SMSLIB = libsms.a
PREFIX = /usr/local

#
# Source files for this library 
#
OBJS = sineSynth.o sinTab.o variousFunctions.o \
peakDetection.o harmDetection.o peakContinuation.o spectrum.o \
realft.o covLatticeHarm.o windows.o stocAnalysis.o smsIO.o filters.o \
fixTrajectories.o smsSynthesis.o smsAnalysis.o spectralApprox.o \
getResidual.o interpolateArrays.o filterArray.o sincTab.o \
soundIO.o hybridize.o

#all: $(SMSLIB)

$(SMSLIB): $(OBJS) 
	ar q $(SMSLIB) $(OBJS)
	if [ -f /bin/ranlib ] ; then ranlib $(SMSLIB); fi
	mv $(SMSLIB) ../lib

clean:
	-rm -f ../lib/$(SMSLIB)
	-rm -f *.o

install:  $(SMSLIB)
	install -m 755 ../lib/$(SMSLIB) $(PREFIX)/lib/
	install -m 755 sms.h $(PREFIX)/include

uninstall: $(SMSLIB)
	rm $(PREFIX)/lib/$(SMSLIB)
	rm $(PREFIX)/include/sms.h