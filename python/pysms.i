%module (docstring="Python SWIG-wrapped module of libsms") sms
%{
#define SWIG_FILE_WITH_INIT
#include "../src/sms.h"
%}

%include "numpy.i" /* numpy typemaps */

%init %{
        import_array(); /* numpy-specific */
        sms_init(); /* initialize the library (makes some tables, seeds the random number generator, etc */
%}



 /* apply all numpy typemaps to various names in sms.h */
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeWindow, float* pWindow)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeWaveform, float* pWaveform)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(long sizeSound, float* pSound)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeFft, float* pArray)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeFft, float* pFftBuffer)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeMag, float* pMag)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizePhase, float* pPhase)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeArray, float* pArray)};
/*void sms_spectrumMag( int sizeInArray, float *pInArray, int sizeOutArray, float *pOutArray) */
%apply (int DIM1, float* IN_ARRAY1) {(int sizeInArray, float* pInArray)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeOutArray, float* pOutArray)};

//%include "sms_doxy.i" /*  doxygen 'autodoc's */
//%feature("autodoc","1");
%include "../src/sms.h" /* all globally declared definitions/functions/structures

/* the following declarations need to be %ignored because they do not work well with 
numpy.  Wrapper functions for these functions are defined below, then %renamed 
back to the original name */

%ignore sms_spectrum;
%ignore sms_spectrumMag;
%ignore sms_windowCentered;
/* rename the following functions (defined below) back to the original names, which
   were ignored */
%rename (sms_spectrum) pysms_spectrum; 
%rename (sms_spectrumMag) pysms_spectrumMag; 
%rename (sms_windowCentered) pysms_windowCentered; 

%inline %{

        void pysms_spectrum( int sizeWaveform, float *pWaveform, int sizeWindow, float *pWindow,
                             int sizeMag, float *pMag, int sizePhase, float *pPhase)
        {
                sms_spectrum(sizeWindow, pWaveform, pWindow, sizeMag, pMag, pPhase);
        }
        void pysms_spectrumMag( int sizeWaveform, float *pWaveform, int sizeWindow, float *pWindow,
                                int sizeMag, float *pMag)
        {
                sms_spectrumMag(sizeWindow, pWaveform, pWindow, sizeMag, pMag);
        }
        void pysms_windowCentered(int sizeWaveform, float *pWaveform, int sizeWindow,
                                  float *pWindow, int sizeFft, float *pFftBuffer)
        {
                sms_windowCentered(sizeWindow, pWaveform, pWindow, sizeFft, pFftBuffer);
        }

        
%}

