%module (docstring="Python SWIG-wrapped module of libsms") pysms
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
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeRes, float* pRes)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeCepstrum, float* pCepstrum)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeEnv, float* pEnv)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeTrack, float* pTrack)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeArray, float* pArray)};
%apply (int DIM1, float* IN_ARRAY1) {(int sizeInArray, float* pInArray)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeOutArray, float* pOutArray)};
%apply (int DIM1, float* INPLACE_ARRAY1) {(int sizeHop, float* pSynthesis)};
// this 2D typemap is for the SMS_File's getFrameDet and getFrameDetP methods
//%apply (int DIM1, int DIM2, float* INPLACE_FARRAY2 ) {(int nTracks, int nComponents, float* pFrame)};

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
%rename (sms_dCepstrum) pysms_dCepstrum;
%rename (sms_synthesize) pysms_synthesize_wrapper;  
//%rename (getFrameDetP) p_getFrameDetP;

%inline %{

typedef struct {
        SMS_Header *header;
        SMS_Data *smsData;
        int allocated;
} SMS_File;

typedef struct {
        SMS_Peak *pSpectralPeaks;
        int nPeaks;
        int nPeaksFound;
} SMS_SpectralPeaks;

void pysms_dCepstrum( int sizeCepstrum, float *pCepstrum, int sizeFreq, float *pFreq, int sizeMag, float *pMag, 
                    float fLambda, int iSamplingRate)
{
        sms_dCepstrum(sizeCepstrum,pCepstrum, sizeFreq, pFreq, pMag, 
                    fLambda, iSamplingRate);
}
void pysms_detectPeaks(int sizeMag, float *pMag, int sizePhase, float *pPhase, 
                      SMS_SpectralPeaks *pPeakStruct, SMS_PeakParams *pPeakParams)
{
        if(sizeMag != sizePhase)
        { 
                sms_error("sizeMag != sizePhase");
                return;
        }
        if(pPeakStruct->nPeaks < pPeakParams->iMaxPeaks)
        { 
                sms_error("nPeaks in SMS_SpectralPeaks is not large enough (less than SMS_PeakParams.iMaxPeaks)");
                return;
        }
        pPeakStruct->nPeaksFound = sms_detectPeaks(sizeMag, pMag, pPhase, pPeakStruct->pSpectralPeaks, pPeakParams);
        
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
void pysms_synthesize_wrapper(SMS_Data *pSmsData, int sizeHop, float *pSynthesis, SMS_SynthParams *pSynthParams) 
{
    if(sizeHop != pSynthParams->sizeHop)
    {
        sms_error("sizeHop != pSynthParams->sizeHop");
        return;
    }
    sms_synthesize(pSmsData, pSynthesis, pSynthParams);
}
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
                   float *pAmp)
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
                        pAmp[i] = $self->smsData[i].pFSinAmp[track];
                }
        }
        // TODO turn into getTrackP - and check if phase exists
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
        void getFrameDet(int i, int sizeFreq, float *pFreq, int sizeAmp, float *pAmp)
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
                
                return;
        }
        void getFrameDetP(int i, int sizeFreq, float *pFreq, int sizeAmp,
                             float *pAmp, int sizePhase, float *pPhase)
        {
                if(!$self->allocated)
                {
                        sms_error("file not yet alloceted");
                        return ;
                }
                if($self->header->iFormat < SMS_FORMAT_HP) 
                {
                        sms_error("file does not contain a phase component in Deterministic (iFormat < SMS_FORMAT_HP)");
                        return;
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
                
                if(sizePhase != sizeFreq || sizePhase != sizeAmp)
                {
                        sms_error("phase array and freq/amp arrays are different in size");
                        return;
                }
                memcpy(pPhase, $self->smsData[i].pFSinPha, sizeof(float) * nTracks);
                
                return;
        }
        void getFrameRes(int i, int sizeRes, float *pRes)
        {
                
                if(!$self->allocated)
                {
                        sms_error("file not yet alloceted");
                        return ;
                }
                if($self->header->iStochasticType < 1) 
                {
                        sms_error("file does not contain a stochastic component");
                        return ;
                }
                int nCoeff = sizeRes;
                if($self->header->nStochasticCoeff > sizeRes) 
                        nCoeff = $self->header->nStochasticCoeff; // return what you can

                memcpy(pRes, $self->smsData[i].pFStocCoeff, sizeof(float) * nCoeff);
                return;
        }
        void getFrameEnv(int i, int sizeEnv, float *pEnv)
        {
                
                if(!$self->allocated)
                {
                        sms_error("file not yet alloceted");
                        return ;
                }
                if($self->header->iEnvType < 1) 
                {
                        sms_error("file does not contain a spectral envelope");
                        return ;
                }
                int nCoeff = sizeEnv;
                if($self->header->nStochasticCoeff > sizeEnv) 
                        nCoeff = $self->header->nEnvCoeff; // return what you can

                memcpy(pEnv, $self->smsData[i].pSpecEnv, sizeof(sfloat) * nCoeff);
                return;
        }
}

%extend SMS_AnalParams 
{
        SMS_AnalParams()
        {
                SMS_AnalParams *s = (SMS_AnalParams *)malloc(sizeof(SMS_AnalParams));
                sms_initAnalParams(s);
                return s;
        }
}

%extend SMS_SpectralPeaks 
{
        SMS_SpectralPeaks(int n)
        {
                SMS_SpectralPeaks *s = (SMS_SpectralPeaks *)malloc(sizeof(SMS_SpectralPeaks));
                s->nPeaks = n;
                if ((s->pSpectralPeaks =
                      (SMS_Peak *)calloc (s->nPeaks, sizeof(SMS_Peak))) == NULL)
                {
                        sms_error("could not allocate memory for spectral peaks");
                        return(NULL);
                }
                s->nPeaksFound = 0;
                return s;
        }
        void getFreq( int sizeArray, float *pArray )
        {
                if(sizeArray < $self->nPeaksFound)
                {
                        sms_error("numpy array not big enough");
                        return;
                }
                int i;
                for (i = 0; i < $self->nPeaksFound; i++)
                        pArray[i] = $self->pSpectralPeaks[i].fFreq;

        }
        void getMag( int sizeArray, float *pArray )
        {
                if(sizeArray < $self->nPeaksFound)
                {
                        sms_error("numpy array not big enough");
                        return;
                }
                int i;
                for (i = 0; i < $self->nPeaksFound; i++)
                        pArray[i] = $self->pSpectralPeaks[i].fMag;
        }
        void getPhase( int sizeArray, float *pArray )
        {
                if(sizeArray < $self->nPeaksFound)
                {
                        sms_error("numpy array not big enough");
                        return;
                }
                int i;
                for (i = 0; i < $self->nPeaksFound; i++)
                        pArray[i] = $self->pSpectralPeaks[i].fPhase;

        }
}

%extend SMS_Data 
{
        void getSinAmp(int sizeArray, float *pArray)
        {
                if(sizeArray < $self->nTracks)
                {
                        sms_error("numpy array not big enough");
                        return;
                }
                int i;
                for (i = 0; i < $self->nTracks; i++)
                        pArray[i] = $self->pFSinAmp[i];
        }
        void getSinFreq(int sizeArray, float *pArray)
        {
                if(sizeArray < $self->nTracks)
                {
                        sms_error("numpy array not big enough");
                        return;
                }
                int i;
                for (i = 0; i < $self->nTracks; i++)
                        pArray[i] = $self->pFSinFreq[i];
        }
        void getSpecEnv(int sizeArray, float *pArray)
        {
                if(sizeArray < $self->nEnvCoeff)
                {
                        sms_error("numpy array not big enough");
                        return;
                }
                int i;
                for (i = 0; i < $self->nEnvCoeff; i++)
                        pArray[i] = $self->pSpecEnv[i];
        }
        void setSinAmp(int sizeArray, float *pArray)
        {
                if(sizeArray < $self->nTracks)
                {
                        sms_error("numpy array not big enough");
                        return;
                }
                int i;
                for (i = 0; i < $self->nTracks; i++)
                        $self->pFSinAmp[i] = pArray[i];
                                
        }
        void setSinFreq(int sizeArray, float *pArray)
        {
                if(sizeArray < $self->nTracks)
                {
                        sms_error("numpy array not big enough");
                        return;
                }
                int i;
                for (i = 0; i < $self->nTracks; i++)
                        $self->pFSinFreq[i] = pArray[i];
        }
        void setSinPha(int sizeArray, float *pArray)
        {
                
        }
}

%extend SMS_ModifyParams
{
        /* no need to return an error code, if sms_error is called, it will throw an exception in python */
        void setSinEnv(int sizeArray, float *pArray)
        {
                if(!$self->ready)
                {
                        sms_error("modify parameter structure has not been initialized");
                        return;
                }
                if(sizeArray != $self->sizeSinEnv)
                {
                        sms_error("numpy array is not equal to envelope size");
                        return;
                }
                memcpy($self->sinEnv, pArray, sizeof(sfloat) * $self->sizeSinEnv);
        }
}

%pythoncode %{
from numpy import array as np_array
def array (n, type='float32'):
        return(np_array(n, type))

from numpy import zeros as np_zeros
def zeros (n, type='float32'):
        return(np_zeros(n, type))

from scipy import floor, hstack
from pysms import * 

def pysms_analyze(audio_file, frame_rate=300, window_size=1001, window_type=SMS_WIN_HAMMING, num_stoc_coeffs=128, default_fundamental=100, highest_freq=12000, env_type=0, env_order=0):
        "todo: add documentation"
        sms_init()
        snd_header = SMS_SndHeader()
        sms_header = SMS_Header()
        analysis_params = SMS_AnalParams()

        sms_header.nStochasticCoeff = num_stoc_coeffs
        analysis_params.fDefaultFundamental = default_fundamental
        analysis_params.fHighestFreq = highest_freq 
        analysis_params.specEnvParams.iType = env_type 
        analysis_params.specEnvParams.iOrder = env_order

        window = zeros(window_size)
        sms_getWindow(window, window_type)

        # Try to open the input file
        if(sms_openSF(audio_file, snd_header)):
                raise NameError("error opening sound file: " + sms_errorString())

        sms_initAnalysis(analysis_params, snd_header)
        sms_fillHeader(sms_header, analysis_params, "analysis_synthesis.py")

        analysis_frames = []
        num_samples = snd_header.nSamples
        num_frames = 0
        sample_offset = 0
        size_new_data = 0
        do_analysis = True

        # Analysis loop
        while do_analysis:
                sample_offset += size_new_data
                if((sample_offset + analysis_params.sizeNextRead) < num_samples):
                        size_new_data = analysis_params.sizeNextRead
                else:
                        size_new_data = num_samples - sample_offset
                frame = zeros(size_new_data).astype('float32')
                if sms_getSound(snd_header, frame, sample_offset):
                        raise NameError("error opening sound file: " + sms_errorString())
                data = SMS_Data()
                sms_allocFrameH(sms_header, data)

                status = sms_analyze(frame, data, analysis_params)  
                if status == 1:
                        analysis_frames.append(data)
                        num_frames += 1
                elif status == -1:
                        do_analysis = False
                        sms_header.nFrames = num_frames

        sms_freeAnalysis(analysis_params)
        sms_free()
        return analysis_frames, sms_header, snd_header

# ----------------------------------------------------------------------------

def pysms_synthesize(frames, sms_header, synth_type=0, det_synth_type=SMS_DET_IFFT, hop_size=SMS_MIN_SIZE_FRAME): 
        "todo: add documentation" 
        sms_init() 
        interp_frame = SMS_Data() 
        synth_params = SMS_SynthParams() 
        synth_params.iSynthesisType = synth_type
        synth_params.iDetSynthType = det_synth_type 
        synth_params.sizeHop = hop_size 
        synth_params.iSamplingRate = 0

        sms_initSynth(sms_header, synth_params)
        sms_allocFrame(interp_frame, sms_header.nTracks, sms_header.nStochasticCoeff, 0, sms_header.iStochasticType, sms_header.nEnvCoeff)

        synth_samples = zeros(synth_params.sizeHop)
        num_synth_samples = 0
        target_synth_samples = sms_header.nFrames * synth_params.origSizeHop
        loc_incr = 1.0 / synth_params.origSizeHop
        audio_output = array([])

# Synthesis loop
        while num_synth_samples < target_synth_samples:
                interp_factor = loc_incr * num_synth_samples
                left_frame_loc = int(min(sms_header.nFrames - 1, floor(interp_factor)))
                right_frame_loc = left_frame_loc + 1 if (left_frame_loc < sms_header.nFrames - 2) else left_frame_loc
                left_frame = frames[left_frame_loc]
                right_frame = frames[right_frame_loc]
                sms_interpolateFrames(left_frame, right_frame, interp_frame, interp_factor - left_frame_loc)
                sms_synthesize(interp_frame, synth_samples, synth_params)
                audio_output = hstack((audio_output, synth_samples))
                num_synth_samples += synth_params.sizeHop
              
        sms_freeFrame(interp_frame)
        sms_freeSynth(synth_params)
        sms_free()
        return audio_output
%}
