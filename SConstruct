# -*- python -*-
# top-level scons script for libsms and tools
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

# environment
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
    sms_cflags = '-Wall -g -Wshadow'
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
	mac_arch = ' -arch i386 -arch x86_64 '
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

    # check for swig
    if not 'swig' in env['TOOLS']:
        print "The Python module cannot be built because swig was not found.\n"
        env['pythonmodule'] = False

# check for popt
if env['tools']:
    # check for popt
    if not conf.CheckLibWithHeader('popt', 'popt.h', 'c'):
        print "The required library popt could not be found"
        exit(1)

env = conf.Finish()

if env['twister']:
    env.Append(CCFLAGS = ' -DMERSENNE_TWISTER ')

prefix = env['prefix']
Export(['env','prefix', 'man_prefix'])

# build the library
SConscript('src/SConscript')

# build SMS tools
if env['tools']:
    # append the src path for static library and header path
    SConscript('tools/SConscript')

# build the python module
if env['pythonmodule']:
    python_install_dir = distutils.sysconfig.get_python_lib()
    env.Append(SWIGFLAGS = ['-python'])
    for lib_path in python_lib_path:
        env.Append(LIBPATH = lib_path) 
    for inc_path in python_inc_path:
        env.Append(CPPPATH = inc_path)
    env.Append(CPPPATH = numpy_include)
    env.Append(CPPPATH = 'src')

    if get_platform() == "win32":
        env.Append(LIBS = [python_lib])
        python_wrapper = env.SharedObject('python/pysms.i')
        env.SharedLibrary('python/pysms', python_wrapper, SHLIBPREFIX='_', SHLIBSUFFIX='.pyd')
        env.InstallAs(os.path.join(python_install_dir, 'pysms.py'), 'python/pysms.py')
        env.InstallAs(os.path.join(python_install_dir, '_pysms.pyd'), 'python/_pysms.pyd')
    elif get_platform() == "darwin":
        env.Append(LIBS = ['python' + get_version()])
        python_wrapper = env.SharedObject('python/pysms.i')
        env.Prepend(LINKFLAGS=['-framework', 'python'])
        env.LoadableModule('python/_pysms.so', python_wrapper) 
        env.InstallAs(os.path.join(python_install_dir, 'pysms.py'), 'python/pysms.py')
        env.InstallAs(os.path.join(python_install_dir, '_pysms.so'), 'python/_pysms.so')        
    else: # linux
        env.Append(LIBS = ['python' + get_version()])
        python_wrapper = env.SharedObject('python/pysms.i')
        env.SharedLibrary('python/pysms', python_wrapper, SHLIBPREFIX='_')
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
