SMS Library - Building on Windows
=================================

Building libsms on windows requires MinGW, MSYS and a number of additional
dependencies. It is recommended in the order that they are listed in below.
This setup was tested on Windows XP, but should work on later versions.


Main Library Dependencies
=========================

* Get MinGW from http://www.mingw.org (tested with version 5.1.4)
Install it using the automated installer.

* Get MSYS from http://www.mingw.org (tested with version 1.0.11)
Install it using the automated installer.

* Get Python from http://www.python.org (tested with version 2.6)
Install it using the automated installer.

* Get SCons from http://www.scons.org (requires at least version 1.2.0)
Install it using the automated installer.

* Get the libsndfile source code from http://www.mega-nerd.com/libsndfile (tested with version 1.0.20)
Build this source using mingw/msys.
Start up the msys prompt. Change directory to the location that libsndfile was extracted to. Run 
    ./configure
    make 
    make install

* Get the GNU Scientific Library (libgsl) source code from http://www.gnu.org/software/gsl/ (tested with version 1.11)
Build this source using mingw/msys.
Start up the msys prompt. Change directory to the location that libgsl was extracted to.
Run: 
    ./configure
    make 
    make install


SMS Tools (Optional)
====================

Additional SMS Tools may be installed, including programs for analyzing and synthesizing 
audio files using libsms. To build them, the popt library is also required.

* Get popt from http://gnuwin32.sourceforge.net/packages/popt.htm (tested with version 1.8).
Download the zips of the binaries, developer files and dependencies.
Extract everything to the MSYS local directory (by default this is c:\msys\1.0\local).


SMS Python Module (pysms) (Optional)
====================================

There is also a libsms Python module, which comes with Python equivalents of the smsAnal and smsSynth tools.
Building it requires, NumPy, SciPy and SWIG.

* Get NumPy from http://scipy.org (tested with version 1.3.0).
Install using the 'superpack' automated installer.

* Get SciPy from http://scipy.org (tested with version 0.7.1).
Install using the 'superpack' automated installer.

* Get the SWIG source from http://www.swig.org (tested with version 1.3.40)
Build this source using mingw/msys.
Start up the msys prompt. Change directory to the location that swig was extracted to. Run: 
    ./configure
    make 
    make install


Building And Installing The Main Library
========================================

It should now be possible to build and install libsms. 
Start up the msys prompt. Change directory to the location that libsms was extracted to.
To install the library and tools run 
    scons
    scons install
If you do not wish to install the tools, run 
    scons tools=no 
    scons install
If you also want to build and install the Python module, run 
    scons pythonmodule=yes 
    scons install

