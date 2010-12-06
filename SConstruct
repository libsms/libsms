# -*- python -*-
# Copyright (c) 2008 MUSIC TECHNOLOGY GROUP (MTG)
#                    UNIVERSITAT POMPEU FABRA 
#  
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

import os, sys
import distutils.sysconfig

# location of msys (windows only)
# by default, it installs to C:/msys/1.0
msys_path = "C:/msys/1.0"

def get_platform():
    if sys.platform[:5] == 'linux':
        return 'linux'
    elif sys.platform[:3] == 'win':
        return 'win32'
    elif sys.platform[:6] == 'darwin':
        return 'darwin'
    else:
        return 'unsupported'

def get_version():
    return sys.version[:3]    

# check that the current platform is supported
if get_platform() == "unsupported":
    print "Error: Cannot build on this platform. "
    print "       Only Linux, Mac OS X and Windows are currently supported."
    exit(1)

# environment
if get_platform() == 'win32':
    # can only build with mingw on windows
    env = Environment(ENV=os.environ, tools=['mingw'])
else:
    env = Environment(ENV=os.environ)

# set default installation directories
default_install_dir = ""
if get_platform() == 'win32':
    default_install_dir = "C:/msys/1.0/local"
    man_prefix = "C:/msys/1.0/local/man/man1"
else:
    default_install_dir = "/usr/local"
    man_prefix = "/usr/share/man/man1"

# command-line options
vars = Variables(['variables.cache'])
vars.AddVariables(
    ('prefix', 'Directory of architecture independant files (where to install).', default_install_dir),
    ('libpath', 'Directory to look for libraries (adds to defaults).', '/usr/local/lib'),
    ('cpath', 'Directory to look for c headers (adds to defaults).', '/usr/local/include'),
    BoolVariable('debug', 'Build with debugging information', False),
    BoolVariable('twister', 'Use SIMD-oriented Fast Mersenne Twister algorithm for random number generation.', True),
    BoolVariable('verbose','print verbose environment information', False),
    BoolVariable('tools', 'Build SMS Tools', True),
    BoolVariable('pythonmodule', 'Build the SMS Python Module', False),
    BoolVariable('doxygen', 'Create the SMS documentation using doxygen', False),
    BoolVariable('universal', 'Compile library/python module for both i386 and x86_64 (only works on OS X 10.6)', False)
)
vars.Update(env)
vars.Save('variables.cache', env)
Help(vars.GenerateHelpText(env))

if env['debug']:
    sms_cflags = '-Wall -g -O0 -Wshadow'
else:
    sms_cflags = ' -O2 -funroll-loops -fomit-frame-pointer -Wall -W -Wno-unused -Wno-parentheses -Wno-switch -fno-strict-aliasing'
env.Append(CCFLAGS = sms_cflags)

# set default library and include directories
if get_platform() == 'linux':
    env.Append(LIBPATH=['/usr/local/lib', '/usr/lib'])
    env.Append(CPPPATH=['/usr/local/include', '/usr/include'])
elif get_platform() == 'darwin':
    env.Append(LIBPATH=['/usr/lib', '/opt/local/lib', '/usr/local/lib' ])
    env.Append(CPPPATH=['/usr/include', '/opt/local/include', '/usr/local/include'])
elif get_platform() == 'win32':
    env.Append(LIBPATH=['/usr/local/lib', '/usr/lib', 'C:/msys/1.0/local/lib', 
                        'C:/msys/1.0/lib', 'C:/Python26/libs'])    
    env.Append(CPPPATH=['/usr/local/include', '/usr/include', 'C:/msys/1.0/local/include',
                        'C:/msys/1.0/local/include/gsl', 'C:/msys/1.0/include', 
                        'C:/Python26/include'])
else:
    print "problem: do not know how to build for architecture", os.platform
    exit(1)

# add whatever paths are specified at the command line    
env.Append(LIBPATH = env['libpath'])
env.Append(CPPPATH = env['cpath'])

# Mac Architecture settings
if env['universal'] and get_platform() == 'darwin':
	mac_arch = '-arch i386 -arch x86_64'
	env.Append(CCFLAGS = mac_arch, LINKFLAGS = mac_arch)

# print environment settings
if env['verbose']:
    print "++ ENVIRONMENT SETTINGS ++"
    print "PATH:", os.environ['PATH']
    print "LIBPATH:", env.Dump('LIBPATH')
    print "CPPPATH:", env.Dump('CPPPATH')

conf = Configure(env)

# check for libmath
if not conf.CheckLibWithHeader('m','math.h','c'):
    print "The required library libmath could not be found"
    exit(1)

# check for libsndfile
if not conf.CheckLibWithHeader('sndfile', 'sndfile.h', 'c'):
    print "The required library libsndfile could not be found"
    print "Get it from http://www.mega-nerd.com/libsndfile"
    exit(1)

# if using windows, assume default gsl paths
# this is because env.ParseConfig calls gsl-config using the 
# windows shell rather than the msys shell, and gsl-config
# is a shell script so it will not run using the windows shell
# TODO: is there a way to get env.ParseConfig to call the msys
# shell instead? Might be useful, although would introduce 
# another dependency, msys.
if get_platform() == 'win32':
    # check for libgsl
    if not conf.CheckLibWithHeader('gsl', 'gsl_sys.h', 'c'):
        print "The required library gsl (GNU Scientific Library) could not be found"
        print "Get it from http://www.gnu.org/software/gsl/"
        exit(1)
    if not conf.CheckLibWithHeader('gslcblas', 'gsl_cblas.h', 'c'):
        print "The required library gsl (GNU Scientific Library) could not be found"
        print "Get it from http://www.gnu.org/software/gsl/"
        exit(1)
# if not using windows, call gsl-config
else:
    env.ParseConfig("gsl-config --cflags --libs")

# look for python
if env['pythonmodule']:	
    python_lib_path = []
    python_inc_path = []

    # linux
    if get_platform() == "linux":
        python_inc_path = ['/usr/include/python' + get_version()]
    # os x
    elif get_platform() == "darwin":
        python_inc_path = ['/Library/Frameworks/Python.framework/Headers', 
                           '/System/Library/Frameworks/Python.framework/Headers']
    # windows
    elif get_platform() == "win32":
        python_lib = 'python%c%c'% (get_version()[0], get_version()[2])
        python_inc_path = ['c:\\Python%c%c\include' % (get_version()[0], get_version()[2])]
        python_lib_path.append('c:\\Python%c%c\libs' % (get_version()[0], get_version()[2]))

    if not conf.CheckHeader("Python.h", language = "C"):
        for i in python_inc_path:
            pythonh = conf.CheckHeader("%s/Python.h" % i, language = "C")
            if pythonh: 
                print "Python version is " + get_version()
                break

	if not pythonh:
	    print "Python headers are missing. Cannot build python module."

    # check for swig
    if not 'swig' in env['TOOLS']:
        print "The Python module cannot be built because swig was not found.\n"
        env['pythonmodule'] = False

    # check for numpy
    try:
        import numpy
        try:
            numpy_include = numpy.get_include()
        except AttributeError:
            numpy_include = numpy.get_numpy_include()
    except ImportError:
        print "The Python module cannot be built because numpy was not found.\n"
        env['pythonmodule'] = False

# check for popt
if env['tools']:
    # check for popt
    if not conf.CheckLibWithHeader('popt', 'popt.h', 'c'):
        print "Warning: The popt library could not be found."
        print "         Will not build SMS Tools."
        env['tools'] = False

if env['twister']:
    env.Append(CCFLAGS = '-DMERSENNE_TWISTER')

env = conf.Finish()

prefix = env['prefix']
Export(['env', 'prefix', 'man_prefix'])

# build the library
sms_sources = SConscript('src/SConscript')
# prepend the 'src' directory onto each source file
for i in range(len(sms_sources)):
    sms_sources[i] = os.path.join("src", sms_sources[i])

# build SMS tools
if env['tools']:
    # append the src path for static library and header path
    SConscript('tools/SConscript')

# build the python module
if env['pythonmodule']:
    python_install_dir = os.path.join(distutils.sysconfig.get_python_lib(), "pysms")
    env.Alias('install', python_install_dir)
    env.InstallAs(os.path.join(python_install_dir, "__init__.py"), "python/__init__.py")
    env.InstallAs(os.path.join(python_install_dir, "analysis.py"), "python/analysis.py")
    env.InstallAs(os.path.join(python_install_dir, "synthesis.py"), "python/synthesis.py")

    env.Append(SWIGFLAGS = ['-python'])
    for lib_path in python_lib_path:
        env.Append(LIBPATH = lib_path) 
    for inc_path in python_inc_path:
        env.Append(CPPPATH = inc_path)
    env.Append(CPPPATH = numpy_include)
    env.Append(CPPPATH = 'src')

    # create the python wrapper using SWIG
    python_wrapper = env.SharedObject('python/pysms.i')
    sms_sources.append(python_wrapper)

    if get_platform() == "win32":
        env.Append(LIBS = [python_lib])
        env.SharedLibrary('python/pysms', sms_sources, SHLIBPREFIX='_', SHLIBSUFFIX='.pyd')
        env.InstallAs(os.path.join(python_install_dir, 'pysms.py'), 'python/pysms.py')
        env.InstallAs(os.path.join(python_install_dir, '_pysms.pyd'), 'python/_pysms.pyd')
    elif get_platform() == "darwin":
        env.Append(LIBS = ['python' + get_version()])
        env.Prepend(LINKFLAGS=['-framework', 'python'])
        env.LoadableModule('python/_pysms.so', sms_sources) 
        env.InstallAs(os.path.join(python_install_dir, 'pysms.py'), 'python/pysms.py')
        env.InstallAs(os.path.join(python_install_dir, '_pysms.so'), 'python/_pysms.so')        
    else: # linux
        env.Append(LIBS = ['python' + get_version()])
        env.SharedLibrary('python/pysms', sms_sources, SHLIBPREFIX='_')
        env.InstallAs(os.path.join(python_install_dir, 'pysms.py'), 'python/pysms.py')
        env.InstallAs(os.path.join(python_install_dir, '_pysms.so'), 'python/_pysms.so')
    env.Alias('install', python_install_dir)

# build doxygen (doesn't work on windows yet)
if env['doxygen']:
    if not get_platform() == "windows":
        if not os.system('doxygen --version'): #make sure it can be found
            os.system('cd ./doc && doxygen Doxyfile')
            exit(1)
        else:
            print "cannot create doxygen documents because doxygen is not installed"
            exit(1)
    else:
        print "Cannot build doxygen documents on windows yet"
        exit(1)
