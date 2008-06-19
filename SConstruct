# -*- python -*-
# Top-level scons script for libsms and tools

import os, sys

EnsureSConsVersion(0, 96)


# ::::::::::::::: Command-line options :::::::::::::::::::
opts = Options()
opts.AddOptions(
    BoolOption('debug', 'set to build with debugging information', False),
    BoolOption('fftw', 'use FFTW3 library', False)
)


env = Environment( ENV = os.environ, CCFLAGS='-Wall ')


#print 'ENV: ', env.Dump()

conf = Configure(env)
if not conf.CheckLibWithHeader('m','math.h','c'):
        print 'cannot find libmath'
        Exit(1)
if not conf.CheckLibWithHeader('sndfile','sndfile.h','c'):
        print 'cannot find libsndfile'
        Exit(1)


use_fftw = ARGUMENTS.get('fftw', 0)
if int(use_fftw):

    if not conf.CheckLibWithHeader('fftw3f','fftw3.h','c'):
        print 'cannot find fft3w, using realft()'
    else:
        env.Append(CCFLAGS = '-DFFTW ')

env = conf.Finish()

debug_mode = ARGUMENTS.get('debug',0 )
if int(debug_mode):
    env.Append(CCFLAGS = '-g ')








#Export('env')

SConscript( ['src/SConscript', 'tools/SConscript'], exports= 'env' )

