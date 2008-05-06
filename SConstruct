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


sms_flags = '-Wall'



if ARGUMENTS.get(' debug', 1):
    sms_flags += ' -g'

sms_srcpath = 'src'

env = Environment( ENV = os.environ, CCFLAGS=sms_flags, CPPPATH = './src', LIBPATH = './src')
#env.Append(CPPPATH =['./src'] )
# env.Append(LIBPATH =['./src'] )
# print 'ENV: ', env.Dump()

conf = Configure(env)
if not conf.CheckLibWithHeader('m','math.h','c'):
        print 'cannot find libmath'
        Exit(1)
if not conf.CheckLibWithHeader('sndfile','sndfile.h','c'):
        print 'cannot find libsndfile'
        Exit(1)
if not conf.CheckCHeader('sms.h'):
        print 'cannot find sms.h'
        Exit(1)

env = conf.Finish()

#Export('env')

SConscript( ['src/SConscript', 'tools/SConscript'], exports= 'env' )

