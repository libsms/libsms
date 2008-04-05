#
# Master Makefile for all SMS files
#


all::
	cd src && make;
	cd tools && make;
	cd tools && cp smsAnal smsSynth smsPrint ../

clean::
	cd src && make clean;
	cd tools && make clean;

install::
	cd src && make install;
	cd tools && make install;

uninstall::
	cd src && make uninstall;
	cd tools && make uninstall;

tags:
	(cd src; etags -t *.[ch])
