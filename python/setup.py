#!/usr/bin/env python

from distutils.core import setup, Extension


pysms_module = Extension('_pysms',
                       sources=['pysms_wrap.c'],
                       include_dirs=['../src'],
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
