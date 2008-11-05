# -*- python -*-
# Top-level scons script for libsms and tools

import os, sys

# ::::::::::::::: Help :::::::::::::::::::
Help("""
---- libsms pre-release ----
Scons will look for python and pd, and if found, build the corresponding
modules. 
The following building commands are available:
'scons' to build libsms and the command line tools
'scons pd' to build the pd externals
'scons doxygen' to build html documentation in ./doc/html'
""")

# ::::::::::::::: Command-line options :::::::::::::::::::
opts = Options()
opts.AddOptions(
    PathOption('prefix', 'Directory of architecture independant files.', '/usr/local'),
    PathOption('pdfolder', 'Directory to where main pd folders are (doc,extra,include,etc).', '/usr/local'),
    BoolOption('debug', 'Build with debugging information', False),
    BoolOption('fftw', 'Use FFTW3 library.', False)
)

if int(ARGUMENTS.get('debug', 0 )):
    sms_cflags = '-Wall -g -Wshadow'
else:
    sms_cflags = ' -O2 -funroll-loops -fomit-frame-pointer \
        -Wall -W\
        -Wno-unused -Wno-parentheses -Wno-switch '

env = Environment( ENV = os.environ, options = opts, CCFLAGS=sms_cflags)

# default action is to build
commands = COMMAND_LINE_TARGETS or 'build'


if 'doxygen' in commands:
        if os.path.exists('/usr/bin/doxygen') or os.path.exists('/usr/local/bin/doxygen'):
                os.system('cd ./doc && doxygen Doxyfile')
                Exit(1)
        else:
                print "cannot create doxygen documents because doxygen is not installed"
                Exit(1)

Help(opts.GenerateHelpText(env))

conf = Configure(env)
if not conf.CheckLibWithHeader('m','math.h','c'):
        print 'cannot find libmath'
        Exit(1)

if not conf.CheckLibWithHeader('sndfile','sndfile.h','c'):
        print 'cannot find libsndfile'
        Exit(1)

# buildpd = conf.CheckCHeader('m_pd.h')

if int(ARGUMENTS.get('fftw', 0)):
    if not conf.CheckLibWithHeader('fftw3f','fftw3.h','c'):
        print 'cannot find fft3w, using realft()'
    else:
        env.Append(CCFLAGS = ' -DFFTW ')

env = conf.Finish()

prefix = ARGUMENTS.get('prefix', '/usr/local')

Export( ['env','prefix', 'commands'] )

SConscript('src/SConscript')
SConscript('tools/SConscript')
if 'pd' in commands:
    SConscript('pd/SConscript')

