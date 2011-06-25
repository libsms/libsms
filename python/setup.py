# Copyright (c) 2008 MUSIC TECHNOLOGY GROUP (MTG)
#                    UNIVERSITAT POMPEU FABRA 
#  
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
"""
libsms in an open source library that implements SMS techniques for analysis,
transformation and synthesis of musical sounds based on a sinusoidal plus 
residual model. It is based on the original code that Xavier Serra completed 
as part of his PhD thesis. The goal of this library is to organize the many 
different components of SMS into an efficient, open, and well-documented 
architecture that can be utilized by various software systems within their own 
programming styles.
"""
from distutils.core import setup, Extension
import os

# detect platform
platform = os.uname()[0] if hasattr(os, 'uname') else 'Windows'

# get numpy include directory
try:
    import numpy
    try:
        numpy_include = numpy.get_include()
    except AttributeError:
        numpy_include = numpy.get_numpy_include()
except ImportError:
    print "Error: Numpy was not found."
    exit(1)


sources = """
    OOURA.c cepstrum.c peakContinuation.c soundIO.c tables.c
    fileIO.c peakDetection.c spectralApprox.c transforms.c
    filters.c residual.c spectrum.c windows.c SFMT.c fixTracks.c
    sineSynth.c stocAnalysis.c harmDetection.c sms.c synthesis.c
    analysis.c modify.c
    """.split()

sources = map(lambda x: '../src/' + x, sources) 
sources.append("pysms/pysms.i")
include_dirs = ['../src', numpy_include, '/usr/local/include'] 

sms = Extension("pysms/_pysms", 
                sources=sources,
                include_dirs=include_dirs,
                libraries=['m', 'fftw3', 'gsl', 'gslcblas', 'sndfile'],
                extra_compile_args=['-DMERSENNE_TWISTER'])

doc_lines = __doc__.split("\n")

setup(name='pysms',
      description=doc_lines[0],
      long_description="\n".join(doc_lines[2:]),
      url='http://www.mtg.upf.edu/static/libsms/',
      download_url='http://www.mtg.upf.edu/static/libsms/',
      license='GPL',
      platforms=["Linux", "Mac OS-X", "Unix", "Windows"],
      version='1.15',
      ext_modules=[sms],
      packages=['pysms'])
