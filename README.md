libsms
===========

Version 1.101

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
* [libsndfile](http://www.mega-nerd.com/libsndfile/)
* [libgsl](http://www.gnu.org/software/gsl/)
* [popt](http://freshmeat.net/projects/popt) - only used in building tools, will skip them if it can't be found

Additionally, windows users will need:

* [MinGW/MSYS](http://www.mingw.org/)

### Optional - All platforms

* [SciPy/NumPy](http://www.scipy.org) - tested with NumPy 1.4.1 and SciPy 0.8

Basic Installation Instructions
-------------------------------

To install the C library:

    $ scons
    $ scons install

To uninstall:

    $ scons -c install

To build the python module:

	$ scons pythonmodule=yes
	
To build and install the library/python module and make it universal binary all in one go:

	$ sudo scons pythonmodule=yes universal=yes install
	
Installing on Windows is a bit complicated.  See [INSTALL.windows](./INSTALL.windows)

TODO: instructions for mac os x install


Use
---
Have a look at the examples (examples folder), or the SMS Tools applications (tools folder).  Lots of python tests and examples are included in the python folder, although none very well refined yet.


Contributing
------------

Send any comments, queries, suggestions or bug reports to 
rich dot eakin at gmail dot com, or
john dot c dot glover at nuim dot ie.
