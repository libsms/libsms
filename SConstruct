# -*- python -*-
# Top-level scons script for libsms and tools

import os, sys

EnsureSConsVersion(0, 96)


# ::::::::::::::: Command-line options :::::::::::::::::::
opts = Options()
opts.AddOptions(
    PathOption('prefix', 'Directory of architecture independant files.', '/usr/local'),
    BoolOption('debug', 'Build with debugging information', False),
    BoolOption('fftw', 'Use FFTW3 library.', False)
)


env = Environment( ENV = os.environ, options = opts, CCFLAGS='-Wall ')

Help(opts.GenerateHelpText(env))

#print 'ENV: ', env.Dump()

conf = Configure(env)
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

#debug_mode = ARGUMENTS.get('debug',0 )
if int(ARGUMENTS.get('debug', 0 )):
    env.Append(CCFLAGS = '-g ')

prefix = ARGUMENTS.get('prefix', '/usr/local')






#Export('env')

SConscript( ['src/SConscript', 'tools/SConscript'], exports= ['env','prefix'] )

