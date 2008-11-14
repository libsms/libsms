# -*- python -*-
# Top-level scons script for libsms and tools

import os, sys

# ::::::::::::::: Help :::::::::::::::::::
Help("""
---- libsms ----
The following building commands are available:
'scons' to build libsms and the command line tools
'scons pd' to build the pd externals
'scons doxygen' to build html documentation in ./doc/html'
""")

# ::::::::::::::: Command-line options :::::::::::::::::::
opts = Options()
opts.AddOptions(
    PathOption('prefix', 'Directory of architecture independant files (where to install).', '/usr/local'),
    PathOption('libpath', 'Directory to look for libraries (adds to defaults).', '/usr/local/lib'),
    PathOption('cpath', 'Directory to look for c headers (adds to defaults).', '/usr/local/include'),
    BoolOption('debug', 'Build with debugging information', False),
    BoolOption('fftw', 'Use FFTW3 library.', False),
    BoolOption('verbose','print verbose environment information', False)
)

if int(ARGUMENTS.get('debug', 0 )):
    sms_cflags = '-Wall -g -Wshadow'
else:
    sms_cflags = ' -O2 -funroll-loops -fomit-frame-pointer -Wall -W -Wno-unused -Wno-parentheses -Wno-switch '

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

if sys.platform == 'linux2':
    env.Append(LIBPATH=['/usr/local/lib','/usr/lib'])
    env.Append(CPPPATH=['/usr/local/include','/usr/include'])
elif sys.platform == 'darwin':
    env.Append(LIBPATH=['/opt/local/lib','/usr/local/lib','/usr/lib'])
    env.Append(CPPPATH=['/opt/local/include','/usr/local/include','/usr/include'])
else:
    print "problem: do not know how to build for architecture", os.platform

#add whatever paths are specified at the command line    
env.Append(LIBPATH = ARGUMENTS.get('libpath', '' ))
env.Append(CPPPATH = ARGUMENTS.get('cpath', '' ))

if int(ARGUMENTS.get('verbose', 0 )):
    print "++ ENVIRONMENT SETTINGS++"
    print "PATH:", os.environ['PATH']
    print "LIBPATH:", env.Dump('LIBPATH')
    print "CPPPATH:", env.Dump('CPPPATH')

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
        env.Append(CCFLAGS = ' -DFFTW ')

env = conf.Finish()

prefix = ARGUMENTS.get('prefix', '/usr/local')

Export( ['env','prefix', 'commands'] )

SConscript('src/SConscript')
SConscript('tools/SConscript')

