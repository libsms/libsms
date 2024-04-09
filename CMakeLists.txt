CMAKE_MINIMUM_REQUIRED(VERSION 3.9)
PROJECT(libsms VERSION 1.5.0 DESCRIPTION "Spectral Modeling Synthesis")

INCLUDE(GNUInstallDirs)


#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=incompatible-pointer-types -Werror=discarded-qualifiers")
OPTION (SMS_BUILD_TOOLS "Build tools" ON)
OPTION (SMS_BUILD_DOCUMENTATION "Build doxygen documentation" ON)
OPTION (SMS_BUILD_PYTHONMODULE "Build python module" OFF)

set(CMAKE_VERBOSE_MAKEFILE "OFF" CACHE STRING "verbose builds")

FIND_PACKAGE(PkgConfig REQUIRED)

FIND_LIBRARY(M_LIBRARIES m math)
FIND_PACKAGE(GSL)
PKG_CHECK_MODULES(SNDFILE REQUIRED sndfile)

INCLUDE_DIRECTORIES(src)


ADD_LIBRARY(sms SHARED
  src/sms.c
  src/analysis.c
  src/synthesis.c
  src/sineSynth.c
  src/peakDetection.c
  src/harmDetection.c
  src/peakContinuation.c
  src/stocAnalysis.c
  src/residual.c
  src/spectralApprox.c
  src/spectrum.c
  src/cepstrum.c
  src/fixTracks.c
  src/modify.c
  src/transforms.c
  src/filters.c
  src/tables.c
  src/windows.c
  src/fileIO.c
  src/soundIO.c
  src/OOURA.c
  src/SFMT.c
)

TARGET_INCLUDE_DIRECTORIES(sms PRIVATE ${SNDFILE_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(sms PRIVATE ${GSL_LIBRARIES} ${SNDFILE_LIBRARIES} ${M_LIBRARIES})

SET_TARGET_PROPERTIES(sms PROPERTIES VERSION ${PROJECT_VERSION})
SET_TARGET_PROPERTIES(sms PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
SET_TARGET_PROPERTIES(sms PROPERTIES PUBLIC_HEADER src/sms.h)


MACRO(SMS_ADD_TOOL _name)
  ADD_EXECUTABLE(${_name} tools/${_name}.c)
  TARGET_INCLUDE_DIRECTORIES(${_name} PRIVATE ${POPT_INCLUDE_DIRS})
  TARGET_LINK_LIBRARIES(${_name} PRIVATE sms ${POPT_LIBRARIES} ${M_LIBRARIES})
  INSTALL(FILES man/${_name}.1 DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1)
ENDMACRO()

IF ( SMS_BUILD_DOCUMENTATION )
  FIND_PACKAGE(Doxygen REQUIRED)
  ADD_CUSTOM_TARGET(
    docs ALL
    DEPENDS docs.done
  )

  ADD_CUSTOM_COMMAND (
    OUTPUT docs.done
    COMMAND cp ${CMAKE_SOURCE_DIR}/doc/Doxyfile Doxyfile.cmade
    COMMAND echo "GENERATE_XML = NO" >> Doxyfile.cmade
    COMMAND echo "INPUT = \"${CMAKE_SOURCE_DIR}/src\" \"${CMAKE_SOURCE_DIR}/doc/maindoxypage.txt\"" >> Doxyfile.cmade
    COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile.cmade
    COMMAND ${CMAKE_COMMAND} -E touch docs.done
    COMMENT "Generating SMS documentation"
    VERBATIM
    )
ENDIF()


IF ( SMS_BUILD_TOOLS )
  PKG_CHECK_MODULES(POPT REQUIRED popt)

  SMS_ADD_TOOL(smsAnal)
  SMS_ADD_TOOL(smsSynth)
  SMS_ADD_TOOL(smsClean)
  SMS_ADD_TOOL(smsPrint)
  SMS_ADD_TOOL(smsResample)
ENDIF()


IF ( SMS_BUILD_PYTHONMODULE )
  message(FATAL_ERROR "Python module currently disabled")

## SConstruct
##    # check for python
##
##    # check for swig
##    if not 'swig' in env['TOOLS']:
##        print "The Python module cannot be built because swig was not found.\n"
##        env['pythonmodule'] = False
##
##    # check for numpy
##    try:
##        import numpy
##        try:
##            numpy_include = numpy.get_include()
##        except AttributeError:
##            numpy_include = numpy.get_numpy_include()
##    except ImportError:
##        print "The Python module cannot be built because numpy was not found.\n"
##        env['pythonmodule'] = False
##
##
##    python_install_dir = os.path.join(distutils.sysconfig.get_python_lib(), "pysms")
##    env.Alias('install', python_install_dir)
##    env.InstallAs(os.path.join(python_install_dir, "__init__.py"), "python/pysms/__init__.py")
##    env.InstallAs(os.path.join(python_install_dir, "analysis.py"), "python/pysms/analysis.py")
##    env.InstallAs(os.path.join(python_install_dir, "synthesis.py"), "python/pysms/synthesis.py")
##
##    env.Append(SWIGFLAGS = ['-python'])
##    for lib_path in python_lib_path:
##        env.Append(LIBPATH = lib_path)
##    for inc_path in python_inc_path:
##        env.Append(CPPPATH = inc_path)
##    env.Append(CPPPATH = numpy_include)
##    env.Append(CPPPATH = 'src')
##
##    # create the python wrapper using SWIG
##    python_wrapper = env.SharedObject('python/pysms/pysms.i')
##    sms_sources.append(python_wrapper)
##
##    if get_platform() == "win32":
##        env.Append(LIBS = [python_lib])
##        env.SharedLibrary('python/pysms/pysms', sms_sources, SHLIBPREFIX='_', SHLIBSUFFIX='.pyd')
##        env.InstallAs(os.path.join(python_install_dir, 'pysms.py'), 'python/pysms/pysms.py')
##        env.InstallAs(os.path.join(python_install_dir, '_pysms.pyd'), 'python/pysms/_pysms.pyd')
##    elif get_platform() == "darwin":
##        env.Append(LIBS = ['python' + get_version()])
##        env.Prepend(LINKFLAGS=['-framework', 'python'])
##        env.LoadableModule('python/pysms/_pysms.so', sms_sources)
##        env.InstallAs(os.path.join(python_install_dir, 'pysms.py'), 'python/pysms/pysms.py')
##        env.InstallAs(os.path.join(python_install_dir, '_pysms.so'), 'python/pysms/_pysms.so')
##    else: # linux
##        env.Append(LIBS = ['python' + get_version()])
##        env.SharedLibrary('python/pysms/pysms', sms_sources, SHLIBPREFIX='_')
##        env.InstallAs(os.path.join(python_install_dir, 'pysms.py'), 'python/pysms/pysms.py')
##        env.InstallAs(os.path.join(python_install_dir, '_pysms.so'), 'python/pysms/_pysms.so')
##    env.Alias('install', python_install_dir)
ENDIF()


INSTALL(TARGETS sms
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
