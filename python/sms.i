%module sms
%{
#define SWIG_FILE_WITH_INIT
#include "../src/sms.h"
/* #include "../src/sms.c"  */
/* #include "../src/smsAnalysis.c"  */
/* #include "../src/smsSynthesis.c"  */
/* #include "../src/sineSynth.c"  */
/* #include "../src/peakDetection.c"  */
/* #include "../src/harmDetection.c"  */
/* #include "../src/peakContinuation.c"  */
/* #include "../src/stocAnalysis.c"  */
/* #include "../src/getResidual.c"  */
/* #include "../src/spectralApprox.c"  */
/* #include "../src/spectrum.c"  */
/* #include "../src/fixTracks.c"  */
/* #include "../src/fourier.c"  */
/* #include "../src/filters.c"  */
/* #include "../src/tables.c"  */
/* #include "../src/windows.c" */
/* #include "../src/smsIO.c"  */
/* #include "../src/soundIO.c"  */
/* #include "../src/hybridize.c" */
int sms_init( void );  
%}

%include "../src/sms.h"
