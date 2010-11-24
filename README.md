SMS Library
===========

Version 1.15 -- Nov 16, 2010

About This Release
------------------

This is my development version of libsms. The latest stable relase can always be found at
the [libsms homepage at the MTG](http://mtg.upf.edu/static/libsms).

### Notable differences:

* Most of the memory alloation is handled in sms.c now, and is tied to the SMS_AnalParams/SMS_SynthParams
  structures. There should be no more functions with memory allocated to static variables. The only other
  places that memory is allocated now is in fileIO.c and tables.c.
* Major code tidy up. Fixed lots of whitespace issues, removed blocks of commented or unused code.

The main API functions should be compatible with the 1.1* release on the libsms homepage,
(the code for the examples did not have to be changed) but there may be differences behind the scenes. 

Libsms
------
Libsms in an open source C library that implements SMS techniques for the analysis,
transformation and synthesis of musical sounds based on a sinusoidal plus residual model.
It is derived from the original code of Xavier Serra, as part of his PhD thesis. You can read
about this and many things related to SMS at the 
[SMS Homepage](http://mtg.upf.edu/technologies/sms/).

libsms homepage: [http://mtg.upf.edu/static/libsms](http://mtg.upf.edu/static/libsms).

The main documentation for this library is generated by Doxygen now, which can be generated
with the command "scons doxygen" or found at the following url:
[http://mtg.upf.edu/static/libsms/doc](http://mtg.upf.edu/static/libsms/doc).

Dependencies
------------

### All platforms

* A C compiler.
* [Python](http://www.python.org) - tested with 2.6
* [SCons](http://www.scons.org) - tested with version 2.0

Additionally, windows users will need:

* [MinGW/MSYS](http://www.mingw.org/)

### Optional - All platforms

* [SciPy/NumPy](http://www.scipy.org) - tested with NumPy 1.4.1 and SciPy 0.8

Installation
------------

To install the C library

    $ scons
    $ scons install

To uninstall

    $ scons -c install

Use
---
Have a look at the examples (examples folder), or the SMS Tools applications (tools folder).


Contributing
------------

Send any comments, queries, suggestions or bug reports to 
reakin at iua dot upf dot edu, or
john dot c dot glover at nuim dot ie.

