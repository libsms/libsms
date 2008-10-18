# -*- python -*-
# Top-level scons script for libsms and tools

import os, sys

# ::::::::::::::: Help :::::::::::::::::::
Help("""
---- libsms pre-release ----
Scons will look for python and pd, and if found, build the corresponding
modules. 
The following building commands are available:
'scons' to build the libary
'scons doxygen' to build html documentation in ./doc/html'
""")

# ::::::::::::::: Command-line options :::::::::::::::::::
opts = Options()
opts.AddOptions(
    PathOption('prefix', 'Directory of architecture independant files.', '/usr/local'),
    BoolOption('debug', 'Build with debugging information', False),
    BoolOption('fftw', 'Use FFTW3 library.', False)
)
#env = Environment( ENV = os.environ, options = opts, CCFLAGS='-Wall ')
env = Environment( ENV = os.environ, options = opts, CCFLAGS='-Wall -Winline ')


if int(ARGUMENTS.get('debug', 0 )):
        env.Append(CCFLAGS = '-g')
else:
        env.Append(CCFLAGS = '-O2 ')

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
# if not conf.CheckLibWithHeader('m','math.h','c'):
#         print 'cannot find libmath'
#         Exit(1)

if not conf.CheckLibWithHeader('sndfile','sndfile.h','c'):
        print 'cannot find libsndfile'
        Exit(1)

buildpd = conf.CheckCHeader('m_pd.h')

if int(ARGUMENTS.get('fftw', 0)):
    if not conf.CheckLibWithHeader('fftw3f','fftw3.h','c'):
        print 'cannot find fft3w, using realft()'
    else:
        env.Append(CCFLAGS = ' -DFFTW ')

env = conf.Finish()
#done checking for libraries

prefix = ARGUMENTS.get('prefix', '/usr/local')

Export( ['env','prefix'] )

#print "libraries exporting: ", env.Dump('LIBS')

#SConscript( ['src/SConscript', 'tools/SConscript', 'pd/SConscript'], exports= ['env','prefix'] )
SConscript('src/SConscript')
SConscript('tools/SConscript')
# if buildpd:
#         SConscript('pd/SConscript')

#SConscript('python/SConscript')

