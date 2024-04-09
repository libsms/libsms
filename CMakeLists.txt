CMAKE_MINIMUM_REQUIRED(VERSION 3.9)
PROJECT(libsms VERSION 1.5.0 DESCRIPTION "Spectral Modeling Synthesis")

INCLUDE(GNUInstallDirs)


#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=incompatible-pointer-types -Werror=discarded-qualifiers")
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

INSTALL(TARGETS sms
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
