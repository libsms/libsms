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

%exception
{
        $action
                if (sms_errorCheck())
                {
                        PyErr_SetString(PyExc_IndexError,sms_errorString());
                        return NULL;
                }
}

 /* apply all numpy typemaps to various names in sms.h */
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeWindow, float* pWindow)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeWaveform, float* pWaveform)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(long sizeSound, float* pSound)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeFft, float* pArray)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeFft, float* pFftBuffer)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeFreq, float* pFreq)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeAmp, float* pAmp)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeMag, float* pMag)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizePhase, float* pPhase)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeTrack, float* pTrack)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeArray, float* pArray)};
%apply (int DIM1, float* IN_ARRAY1) {(int sizeInArray, float* pInArray)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeOutArray, float* pOutArray)};
%apply (int DIM1, int DIM2, float* INPLACE_FARRAY2 ) {(int nPeaks, int peakDim, float* pSpectralPeaks)};

//%include "sms_doxy.i" /*  doxygen 'autodoc's (takes much longer to compile) */
%feature("autodoc","1");
%feature("autodoc", """
sms_spectrum(NPY_FLOAT waveform, NPY_FLOAT window, NPY_FLOAT mag,
            NPY_FLOAT phase) -> err

Computes the real part of the frequency spectrum from a waveform in polar coordinates.
The size of mag and phase should be a power of 2, or it will be rounded up to one.
The waveform and window should be the same size, as well as mag and phase.  
A mag/phase larger than half the waveform/window will cause zero padding.
The mag/phase array must be greater than half the size of waveform/window.

The function definition below was automatically created by swig, but as this function is
typemapped to use numpy arrays, it should be ignored.
""") sms_spectrum;
%include "../src/sms.h" /* all globally declared libsms stuff */


/* overload the functions that will be wrapped to fit numpy typmaps (defined below)
by renaming the wrapped names back to originals */
%rename (sms_detectPeaks) pysms_detectPeaks; 
%rename (sms_spectrum) pysms_spectrum; 
%rename (sms_spectrumMag) pysms_spectrumMag; 
%rename (sms_windowCentered) pysms_windowCentered; 
%rename (sms_invSpectrum) pysms_invSpectrum; 

%inline %{

typedef struct {
        SMS_Header *header;
        SMS_Data *smsData;
        int allocated;
} SMS_File;


int pysms_detectPeaks(int sizeMag, float *pMag, int sizePhase, float *pPhase, int nPeaks,
                      int peakDim, float *pSpectralPeaks, SMS_PeakParams *pPeakParams)
{
        if(sizeMag != sizePhase)
        { 
                sms_error("sizeMag != sizePhase");
                return(0);
        }
        if(peakDim != 3)
        { 
                sms_error("pSpectralPeaks is not a two dimensional array with 3 columns");
                return(0);
        }
        
        return(sms_detectPeaks(sizeMag, pMag, pPhase, (SMS_Peak *)pSpectralPeaks, pPeakParams));
        
}

int pysms_spectrum( int sizeWaveform, float *pWaveform, int sizeWindow, float *pWindow,
                    int sizeMag, float *pMag, int sizePhase, float *pPhase)
{
        return(sms_spectrum(sizeWindow, pWaveform, pWindow, sizeMag, pMag, pPhase));
}
int pysms_spectrumMag( int sizeWaveform, float *pWaveform, int sizeWindow, float *pWindow,
                       int sizeMag, float *pMag)
{
        return(sms_spectrumMag(sizeWindow, pWaveform, pWindow, sizeMag, pMag));
}
int pysms_invSpectrum( int sizeWaveform, float *pWaveform, int sizeWindow, float *pWindow,
                       int sizeMag, float *pMag, int sizePhase, float *pPhase)
{
        return(sms_invSpectrum(sizeWaveform, pWaveform, pWindow, sizeMag, pMag, pPhase));
}
void pysms_windowCentered(int sizeWaveform, float *pWaveform, int sizeWindow,
                          float *pWindow, int sizeFft, float *pFftBuffer)
{
        if (sizeWaveform != sizeWindow)
        { 
                sms_error("sizeWaveform != sizeWindow");
                return;
        }
        sms_windowCentered(sizeWindow, pWaveform, pWindow, sizeFft, pFftBuffer);
}

/*         void pysms_openSF (char *pChInputSoundFile, SMS_SndHeader *pSoundHeader) */
/*         { */
/*                 sms_openSF (pChInputSoundFile, pSoundHeader); */
/*         } */
        
%}

%extend SMS_File 
{
        void load( char *pFilename )
        {
                int i;
                FILE *pSmsFile;
                $self->allocated = 0;
                sms_getHeader (pFilename, &$self->header, &pSmsFile);
                if(sms_errorCheck()) return;
                
                $self->smsData = calloc($self->header->nFrames, sizeof(SMS_Data));
                for( i = 0; i < $self->header->nFrames; i++ )
                {
                        sms_allocFrameH ($self->header,  &$self->smsData[i]);
                        if(sms_errorCheck()) return;
                        sms_getFrame (pSmsFile, $self->header, i, &$self->smsData[i]);
                        if(sms_errorCheck()) return;
                }
                $self->allocated = 1;
                return;
        }
        void close(void)
        {
                int i;
                if(!$self->allocated)
                {
                        sms_error("file not yet alloceted");
                        return;
                }
                $self->allocated = 0;
                for( i = 0; i < $self->header->nFrames; i++)
                        sms_freeFrame(&$self->smsData[i]);
                free($self->smsData);
                return;
        }

        void getTrack(int track, int sizeFreq, float *pFreq, int sizeAmp,
                   float *pAmp, int sizePhase, float *pPhase)
        {
                /* fatal error protection first */
                if(!$self->allocated)
                {
                        sms_error("file not yet alloceted");
                        return ;
                }
                if(track >= $self->header->nTracks)
                {
                        sms_error("desired track is greater than number of tracks in file");
                        return;
                }
                if(sizeFreq != sizeAmp)
                {
                        sms_error("freq and amp arrays are different in size");
                        return;
                }
                /* make sure arrays are big enough, or return less data */
                int nFrames = MIN (sizeFreq, $self->header->nFrames);
                int i;
                for( i=0; i < nFrames; i++)
                {
                        pFreq[i] = $self->smsData[i].pFSinFreq[track];
                        pAmp[i] = $self->smsData[i].pFSinFreq[track];
                }
                if($self->header->iFormat < SMS_FORMAT_HP) return;
                
                if(sizePhase != sizeFreq || sizePhase != sizeAmp)
                {
                        sms_error("phase array and freq/amp arrays are different in size");
                        return;
                }
                for( i=0; i < nFrames; i++)
                        pPhase[i] = $self->smsData[i].pFSinPha[track];
                
                return;
        }
        void sms_getFrameDet(int i, int sizeFreq, float *pFreq, int sizeAmp,
                             float *pAmp, int sizePhase, float *pPhase)
        {
                if(!$self->allocated)
                {
                        sms_error("file not yet alloceted");
                        return ;
                }
                if(i >= $self->header->nFrames)
                {
                        sms_error("index is greater than number of frames in file");
                        return;
                }
                int nTracks = $self->smsData[i].nTracks;
                if(sizeFreq > nTracks)
                {
                        sms_error("index is greater than number of frames in file");
                        return;
                }
                if(sizeFreq != sizeAmp)
                {
                        sms_error("freq and amp arrays are different in size");
                        return;
                }
                memcpy(pFreq, $self->smsData[i].pFSinFreq, sizeof(float) * nTracks);
                memcpy(pAmp, $self->smsData[i].pFSinAmp, sizeof(float) * nTracks);
                
                if($self->header->iFormat < SMS_FORMAT_HP) return;
                
                if(sizePhase != sizeFreq || sizePhase != sizeAmp)
                {
                        sms_error("phase array and freq/amp arrays are different in size");
                        return;
                }
                memcpy(pPhase, $self->smsData[i].pFSinPha, sizeof(float) * nTracks);
                
                return;
        }

}

/* %extend SMS_AnalParams */
/* { */
/*         SMS_AnalParams __init__(void) */
/*         { */
/*                 sms_initAnalParams(&$self); */
/*                 return; */
/*         } */
/* } */
%extend SMS_AnalParams {
 SMS_AnalParams() {
   SMS_AnalParams *s = (SMS_AnalParams *)malloc(sizeof(SMS_AnalParams));
   sms_initAnalParams(s);
   return s;
 }
}

%pythoncode %{

from numpy import array as np_array
def array (n, type='float32'):
        return(np_array(n, type))

from numpy import zeros as np_zeros
def zeros (n, type='float32'):
        return(np_zeros(n, type))

%}
