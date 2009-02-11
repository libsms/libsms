#!/usr/bin/env python

from distutils.core import setup, Extension

#temporarily including /usr/lib/python2.5... blah to find arrayobject.h, but this probably
#varies from platform to platform.  Should look for it a different way and append the
#directory once found.

# this code was taken from the scipy wiki to add the numpy include directory
# across numpy versions.  I haven't needed it yet, but it is here if it should become
# an issue.
# try:
#     numpy_include = numpy.get_include()
#  except AttributeError:
#     numpy_include = numpy.get_numpy_include()


pysms_module = Extension('_pysms',
                       sources=['pysms_wrap.c'],
                       include_dirs=['../src', '/usr/lib/python2.5/site-packages/numpy/core/include/'],
                       library_dirs=['../src'],
                       libraries = ["sndfile", "sms"] #should libsms be in here?
                       )

setup (name = 'pysms',
       version = '0.2',
       author      = "Rich Eakin, MTG",
       author_email      = "reakin@iua.upf.edu",
       url = "http://mtg.upf.edu/static/libsms",
       description = """TODO: write a description""",
       ext_modules = [pysms_module],
       py_modules = ["pysms"],
       )
