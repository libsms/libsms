CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
IF(POLICY CMP0086)
  CMAKE_POLICY (SET CMP0086 NEW)
ENDIF()

PROJECT(libsms VERSION 1.5.0 DESCRIPTION "Spectral Modeling Synthesis")

INCLUDE(GNUInstallDirs)
INCLUDE(GenerateExportHeader)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=incompatible-pointer-types -Werror=discarded-qualifiers")

OPTION (SMS_BUILD_TOOLS "Build SMS tools" ON)
OPTION (SMS_BUILD_DOCUMENTATION "Build SMS documentation using doxygen" ON)
OPTION (SMS_BUILD_PYTHONMODULE "Build the SMS Python module" ON)
OPTION (SMS_ENABLE_TWISTER "Enable SIMD-oriented Fast Mersenne Twister algorithm for random number generation" ON)

FIND_PACKAGE(PkgConfig REQUIRED)

FIND_LIBRARY(M_LIBRARIES m math)
FIND_PACKAGE(GSL)
PKG_CHECK_MODULES(SNDFILE REQUIRED sndfile)

INCLUDE_DIRECTORIES(
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  src
)

SET(sms_HEADERS
  src/sms.h
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>/sms_export.h
)

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


IF ( SMS_ENABLE_TWISTER )
  TARGET_COMPILE_DEFINITIONS(sms PUBLIC MERSENNE_TWISTER=1)
ENDIF()


SET_TARGET_PROPERTIES(sms PROPERTIES VERSION ${PROJECT_VERSION})
SET_TARGET_PROPERTIES(sms PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

SET_TARGET_PROPERTIES(sms PROPERTIES
  C_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON
)

SET_TARGET_PROPERTIES(sms PROPERTIES PUBLIC_HEADER "${sms_HEADERS}")
GENERATE_EXPORT_HEADER(sms)

MACRO(SMS_ADD_TOOL _name)
  ADD_EXECUTABLE(${_name} tools/${_name}.c)
  TARGET_INCLUDE_DIRECTORIES(${_name} PRIVATE ${POPT_INCLUDE_DIRS})
  TARGET_LINK_LIBRARIES(${_name} PRIVATE sms ${POPT_LIBRARIES} ${M_LIBRARIES})
  INSTALL(FILES man/${_name}.1 DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1)
ENDMACRO()

IF ( SMS_BUILD_DOCUMENTATION )
  FIND_PACKAGE(Doxygen REQUIRED dot)
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

  INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/API
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
  MESSAGE(WARNING "Python module experimental")

  FIND_PACKAGE(SWIG REQUIRED)
  FIND_PACKAGE(Python COMPONENTS Interpreter Development NumPy)
  INCLUDE(${SWIG_USE_FILE})
  SWIG_ADD_LIBRARY(pysms
    LANGUAGE python
    SOURCES python/pysms/pysms.i
  )
  SET_PROPERTY(TARGET pysms PROPERTY SWIG_COMPILE_DEFINITIONS SMS_EXPORT="")
  TARGET_INCLUDE_DIRECTORIES(pysms PRIVATE ${Python_INCLUDE_DIRS} ${Python_NumPy_INCLUDE_DIRS})
  TARGET_INCLUDE_DIRECTORIES(pysms PRIVATE ${CMAKE_SOURCE_DIR}/python/pysms)
  TARGET_LINK_LIBRARIES(pysms PRIVATE sms ${Python_LIBRARIES})

  INSTALL(TARGETS pysms
    DESTINATION ${Python_SITEARCH}/pysms
  )
  INSTALL(FILES
      python/pysms/__init__.py
      python/pysms/analysis.py
      python/pysms/synthesis.py
    DESTINATION ${Python_SITEARCH}/pysms
  )
ENDIF()


INSTALL(TARGETS sms
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
