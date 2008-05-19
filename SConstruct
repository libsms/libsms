# -*- python -*-
# Top-level scons script for libsms and tools

import os, sys

EnsureSConsVersion(0, 96)

sms_version = '0.1'

# ::::::::::::::: Command-line options :::::::::::::::::::
opts = Options()
opts.AddOptions(
    BoolOption('debug', 'set to build with debugging information', False)
)




    

sms_srcpath = 'src'

env = Environment( ENV = os.environ, CCFLAGS='-Wall ', CPPPATH = './src', LIBPATH = './src')
#env.Append(CPPPATH =['./src'] )
# env.Append(LIBPATH =['./src'] )
# print 'ENV: ', env.Dump()

use_fftw = ARGUMENTS.get('fftw', 0)
debug_mode = ARGUMENTS.get('debug',0 )
if int(use_fftw):
    env.Append(CCFLAGS = '-DFFTW ')

if int(debug_mode):
    env.Append(CCFLAGS = '-g ')




conf = Configure(env)
if not conf.CheckLibWithHeader('m','math.h','c'):
        print 'cannot find libmath'
        Exit(1)
if not conf.CheckLibWithHeader('sndfile','sndfile.h','c'):
        print 'cannot find libsndfile'
        Exit(1)
if not conf.CheckLibWithHeader('fftw3f','fftw3.h','c'):
        print 'cannot find fft3w'
        Exit(1)
if not conf.CheckCHeader('sms.h'):
        print 'cannot find sms.h'
        Exit(1)

env = conf.Finish()

#Export('env')

SConscript( ['src/SConscript', 'tools/SConscript'], exports= 'env' )

