# -*- python -*-
# Top-level scons script for libsms and tools

import os, sys

# ::::::::::::::: Command-line options :::::::::::::::::::
opts = Options()
opts.AddOptions(
    PathOption('prefix', 'Directory of architecture independant files.', '/usr/local'),
    BoolOption('debug', 'Build with debugging information', False),
    BoolOption('fftw', 'Use FFTW3 library.', False)
)
env = Environment( ENV = os.environ, options = opts, CCFLAGS='-Wall ')

#env = Environment( CCFLAGS='-Wall ',
 #                  LIBPATH = ['/usr/local/lib', '/usr/lib'])

Help(opts.GenerateHelpText(env))

#print 'LIBPATH: ', env.Dump('LIBPATH')

conf = Configure(env)
#if not conf.CheckLib('m'):
#        print 'Did not find libm.a or m.lib, exiting!'
#        Exit(1)
#if not conf.CheckCHeader('math.h'):
#        print 'Math.h must be installed!'
#        Exit(1)
if not conf.CheckLibWithHeader('m','math.h','c'):
        print 'cannot find libmath'
        Exit(1)

if not conf.CheckLibWithHeader('sndfile','sndfile.h','c'):
        print 'cannot find libsndfile'
        Exit(1)

if int(ARGUMENTS.get('fftw', 0)):
    if not conf.CheckLibWithHeader('fftw3f','fftw3.h','c'):
        print 'cannot find fft3w, using realft()'
    else:
        env.Append(CCFLAGS = '-DFFTW ')

env = conf.Finish()
#done checking for libraries

if int(ARGUMENTS.get('debug', 0 )):
    env.Append(CCFLAGS = '-g ')

prefix = ARGUMENTS.get('prefix', '/usr/local')

SConscript( ['src/SConscript', 'tools/SConscript'], exports= ['env','prefix'] )
