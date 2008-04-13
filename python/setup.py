#!/usr/bin/env python

from distutils.core import setup, Extension


sms_module = Extension('_sms',
                       sources=['sms_wrap.c'],
                       include_dirs=['../src'],
                       libraries = ["sndfile", "sms"] #should libsms be in here?
                       )

setup (name = 'sms',
       version = '0.1',
       author      = "MTG",
       description = """TODO""",
       ext_modules = [sms_module],
       py_modules = ["sms"],
       )
