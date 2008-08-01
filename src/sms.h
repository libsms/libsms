/* 
 * Copyright (c) 2008 MUSIC TECHNOLOGY GROUP (MTG)
 *                         UNIVERSITAT POMPEU FABRA 
 * 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */
/*! \file sms.h
 * \brief header file to be included in all SMS application
 */
#ifndef _SMS_H
#define _SMS_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <strings.h>
#include <sndfile.h>
#include <fftw3.h>

/*! \struct SMS_Header 
 *  \brief structure for the header of an SMS file 
 *  
 *  This header contains all the information necessary to read an SMS
 *  file, prepare memory and synthesizer parameters.
 *  
 *  The header also contains variable components for additional information
 *  that may be stored along with the analysis, such as descriptors or text.
 *  \todo describe descriptors better.
 */
typedef struct 
{
	int iSmsMagic;         /*!< identification constant */
	int iHeadBSize;        /*!< size in bytes of header */
	int nFrames;	         /*!< number of data records */
	int iRecordBSize;      /*!< size in bytes of each data frame */
	int iFormat;           /*!< type of data format 
                                 \todo reference enum */
	int iFrameRate;        /*!< rate in Hz of data frames */
	int iStochasticType;   /*!< type stochastic representation */
	int nTrajectories;     /*!< number of trajectoires per frame */
	int nStochasticCoeff;  /*!< number of stochastic coefficients per frame  */
	float fAmplitude;      /*!< average amplitude of represented sound
                                \todo currently unused, cleanup */
	float fFrequency;      /*!< average fundamental frequency
                                \todo currently unused, cleanup */
	int iOriginalSRate;    /*!< sampling rate of original sound */
	int iBegSteadyState;   /*!< record number of begining of steady state
                                \todo currently unused, cleanup */
	int iEndSteadyState;   /*!< record number of end of steady state
                                \todo currently unused, cleanup */
	float fResidualPerc;   /*!< percentage of the residual to original */
	int nLoopRecords;      /*< number of loop records specified */
	int nSpecEnvelopePoints; /*< number of breakpoints in spectral envelope */
	int nTextCharacters;   /*< number of text characters */
	/* variable part */
	int *pILoopRecords;    /*!< array of record numbers of loop points */
	float *pFSpectralEnvelope; /*!< spectral envelope of partials */
	char *pChTextCharacters; /*!< Text string relating to the sound */
} SMS_Header;

/*! \struct SMS_SndHeader 
 *  \brief structure including sound header information 
 */
typedef struct {
    int nSamples;       /*!< Number of samples in the sound */
    int iSamplingRate;   /*!< The sampling rate */
    int channelCount;  /*!< The number of channels */
    int sizeHeader;	     /*!< size of sound header in bytes */
} SMS_SndHeader;

/*! \struct SMS_Data
 *  \brief structure with SMS data
 *
 * \todo details..
 */
typedef struct 
{
	float *pSmsData;        /*!< pointer to all SMS data */
	int sizeData;               /*!< size of all the data */
	float *pFFreqTraj;       /*!< frequency of sinusoids */
	float *pFMagTraj;       /*!< magnitude of sinusoids */
	float *pFPhaTraj;        /*!< phase of sinusoids */
	int nTraj;                     /*!< number of sinusoids */
        float *pFStocWave;   /*!< sampled waveform (if stoc type = wave) */
        int nSamples;             /*!< number of samples in StocWave */
	float *pFStocGain;     /*!< gain of stochastic component */
	float *pFStocCoeff;    /*!< filter coefficients for stochastic component */
	int nCoeff;                  /*!< number of filter coefficients */
} SMS_Data;

/*! \struct SMS_SndBuffer
 * \brief buffer for sound data 
 * 
 * This structure is used for holding a buffer of audio data. iMarker is a 
 * sample number of the sound source that corresponds to the first sample 
 * in the buffer.
 *
 * \todo document what iFirst good is for.
 */
typedef struct
{
	float *pFBuffer;          /*!< buffer for sound data*/
	int sizeBuffer;            /*!< size of buffer */
	int iMarker;               /*!< sample marker relating to sound source */
	int iFirstGood;          /*!< first sample in buffer that is a good one */
} SMS_SndBuffer;

/*! \struct SMS_Peak 
 * \brief structure for sinusodial peak   
 */

/* information attached to a spectral peak */
typedef struct 
{
	float fFreq;           /*!< frequency of peak */
	float fMag;           /*!< magnitude of peak */
	float fPhase;        /*!< phase of peak */
} SMS_Peak;

#define SMS_MIN_MAG     .3      /*!< \brief minimum magnitude to be searched */
#define SMS_MAX_NPEAKS      200    /*!< \brief maximum number of peaks  */
                                    
/*! \struct SMS_HarmCandidate
 * \brief structure to hold information about a harmonic candidate 
 *
 * \todo details, and maybe move to harmDetection.c
 */
typedef struct 
{
	float fFreq;                   /*!< frequency of harmonic */
	float fMag;                   /*!< magnitude of harmonic */
	float fMagPerc;           /*!< percentage of magnitude */
	float fFreqDev;            /*!< deviation from perfect harmonic */
	float fHarmRatio;         /*!< percentage of harmonics found */
} SMS_HarmCandidate;

/*! \struct SMS_ContCandidate
 * \brief structure to hold information about a continuation candidate 
 *
 * \todo details, and maybe move to harmDetection.c
 */
typedef struct
{
	float fFreqDev;       /*!< frequency deviation from guide */
	float fMagDev;        /*!< magnitude deviation from guide */
	int iPeak;                /* peak number */
} SMS_ContCandidate;        

/*! \struct SMS_Guide
 * \brief information attached to a guide
 *
 * used in peak continuation    
 * \todo details, possibly move
 */
typedef struct
{
	float fFreq;          /*!< frequency of guide */
	float fMag;           /*!< magnitude of guide */
	int iStatus;          /*!< status of guide: DEAD, SLEEPING, ACTIVE */
	int iPeakChosen;    /*!< peak number chosen by the guide (was a short) */
} SMS_Guide;

/*! \struct SMS_AnalFrame
 *  \brief structure to hold an analysis frame
 *
 *  \todo details..
 */
typedef struct 
{
	int iFrameSample;         /*!< sample number of the sound file that 
                               corresponds to the middle of the frame */
	int iFrameSize;           /*!< number of samples used in the frame */
	int iFrameNum;            /*!< frame number */
	SMS_Peak pSpectralPeaks[SMS_MAX_NPEAKS];  /*!< spectral peaks found in frame
                                                   \see SMS_Peak */
	int nPeaks;               /*!< number of peaks found */
	float fFundamental;       /*!< fundamental frequency in frame */
	SMS_Data deterministic;   /*!< deterministic data \see SMS_Data */
	int iStatus; /*!< status of frame enumerated by SMS_FRAME_STATUS
                       \see SMS_FRAME_STATUS */
} SMS_AnalFrame;

/*! \struct SMS_AnalParams
 * \brief structure with useful information for analysis functions
 *
 * \todo details
 */
typedef struct 
{
	int iDebugMode; /*!< debug codes enumerated by SMS_DBG \see SMS_DBG */
	int iFormat;          /*!< analysis format code defined by SMS_Format \see SMS_Format */
	int iStochasticType;      /*!<  type of stochastic model defined by SMS_StocSynthType 
                                                     \see SMS_StocSynthType */
	float fLowestFundamental; /*!< lowest fundamental frequency in Hz */
	float fHighestFundamental;/*!< highest fundamental frequency in Hz */
	float fDefaultFundamental;/*!< default fundamental in Hz */
	float fPeakContToGuide;   /*!< contribution of previous peak to current guide (between 0 and 1) */
	float fFundContToGuide;   /*!< contribution of current fundamental to current guide (between 0 and 1) */
	float fFreqDeviation;     /*!< maximum deviation from peak to peak */				     
	int iSamplingRate;        /*! sampling rate of sound to be analyzed */
	int iDefaultSizeWindow;   /*!< default size of analysis window in samples */
	int sizeHop;              /*!< hop size of analysis window in samples */
	float fSizeWindow;       /*!< size of analysis window in number of periods */
	int nGuides;              /*!< number of guides used \todo explain */
	int iCleanTraj;           /*!< whether or not to clean trajectories */
	float fMinRefHarmMag;     /*!< minimum magnitude in dB for reference peak */
	float fRefHarmMagDiffFromMax; /*!< maximum magnitude difference from reference peak to highest peak */
	int iRefHarmonic;	       /*!< reference harmonic to use in the fundamental detection */
	int iMinTrajLength;	       /*!< minimum length in samples of a given trajectory */
	int iMaxSleepingTime;	   /*!< maximum sleeping time for a trajectory */
	float fHighestFreq;        /*!< highest frequency to be searched */
	float fMinPeakMag;         /*!< minimum magnitude in dB for a good peak */	
	int iSoundType;            /*!< type of sound to be analyzed emumerated by SMS_SOUND_TYPE 
                                                   \see SMS_SOUND_TYPE */	
	int iAnalysisDirection;    /*!< analysis direction, direct or reverse */	
	int iSizeSound;             /*!< total size of sound to be analyzed in samples */	 	
	int iWindowType;            /*!< type of analysis window enumerated by SMS_WINDOWS 
                                                       \see SMS_WINDOWS */			  	 			 
        int iMaxDelayFrames;     /*!< maximum number of frames to delay before peak continuation */
        SMS_Data prevFrame;   /*!< the previous analysis frame  */
        SMS_SndBuffer soundBuffer; /*!< samples to be analyzed */
        SMS_SndBuffer synthBuffer; /*!< resynthesized samples needed to get the residual */
        SMS_AnalFrame *pFrames;  /*!< \todo explain why AnalFrame is necessary here */
        SMS_AnalFrame **ppFrames; /*!< \todo explain why this double pointer is necessary */
        float fResidualPercentage; /*!< accumalitive residual percentage */
#ifdef FFTW
        fftwf_plan  fftPlan; /*!< plan for FFTW's fourier transform functions, floating point */
        float *pWaveform; /*< array of samples to be passed to fftwf_execute 
                           \todo why isn't the sound buffer above used here, why both? */
        fftwf_complex *pSpectrum; /*< complex array of spectra produced by fftwf_execute */
#endif
} SMS_AnalParams;

/*! \struct SMS_SynthParams
 * \brief structure with useful information for synthesis functions
 *
 * \todo details
 */
typedef struct
{
	int iStochasticType;       /*!<  type of stochastic model defined by SMS_StocSynthType 
                                                     \see SMS_StocSynthType */
	int iSynthesisType;        /*!< type of synthesis to perform \see SMS_SynthType */
        int iDetSynthType;         /*!< method for synthesizing deterministic component
                                                 \see SMS_DetSynthType */
	int iOriginalSRate;  /*!< samplerate of the sound model source \todo this should not be necessary, 
                                             it is only used to create other parameters... should just use that param
                                             here instead */
	int iSamplingRate;         /*!< synthesis samplerate */
	SMS_Data prevFrame; /*!< previous data frame, used for smooth interpolation between frames */
	int sizeHop;                   /*!< number of samples to synthesis for each frame */
        int origSizeHop;            /*!< original number of samples used to create each analysis frame */
	float *pFDetWindow;    /*!< array to hold the window used for deterministic synthesis
                                                \todo explain which window this is */
        float *pFStocWindow; /*!< array to hold the window used for stochastic synthesis
                                                \todo explain which window this is */
#ifdef FFTW
        fftwf_plan  fftPlan;         /*!< plan for FFTW's inverse fourier transform functions, floating point */
        fftwf_complex *pSpectrum; /*!< complex array of spectra used to create synthesis */
        float *pWaveform;       /*!< synthesis samples produced by fftwf_execute */
#else
        float *realftOut; /*!< RTE_DEBUG : comparing realft and fftw \todo remove this */
#endif
} SMS_SynthParams;

/*!  \brief analysis format
 *
 *   \todo explain how this is important for the analysis
 */
enum SMS_Format
{
        SMS_FORMAT_H, /*!< format harmonic */
        SMS_FORMAT_IH,      /*!< format inharmonic */
        SMS_FORMAT_HP,     /*!< format harmonic with phase */
        SMS_FORMAT_IHP    /*!< format inharmonic with phase */
};

/*! \brief synthesis types
 * 
 * These values are used to determine whether to synthesize
 * both deterministic and stochastic components together,
 * the deterministic component alone, or the stochastic 
 * component alone.
 */
enum SMS_SynthType
{
        SMS_STYPE_ALL, /*!< both components combined */
        SMS_STYPE_DET,      /*!< deterministic component alone */
        SMS_STYPE_STOC    /*!< stochastic component alone */
};

/*! \brief synthesis method for deterministic component
 * 
 * There are two options for deterministic synthesis available to the 
 * SMS synthesizer.  The Inverse Fast Fourier Transform method
 * (IFFT) is more effecient for models with lots of partial trajectories, but can
 * possibly smear transients.  The Sinusoidal Table Lookup (SIN) can
 * theoritically support faster moving trajectories at a higher fidelity, but
 * can consume lots of cpu at varying rates.  
 */
enum SMS_DetSynthType
{
        SMS_DET_IFFT,        /*!< Inverse Fast Fourier Transform (IFFT) */
        SMS_DET_SIN          /*!< Sinusoidal Table Lookup (SIN) */
};

/*! \brief synthesis method for stochastic component
 *
 * Currently, Stochastic Approximation is the only reasonable choice 
 * for stochastic synthesis: this method approximates the spectrum of
 * the stochastic component by a specified number of coefficients during
 * analyses, and then approximates another set of coefficients during
 * synthesis in order to fit the specified hopsize. The phases of the
 * coefficients are randomly generated, according to the theory that a
 * stochastic spectrum consists of random phases.
 * 
 * Waveform samples simply stores the residual component to file. During
 * resynthesis, the samples are looped in order to fit the specified 
 * hopsize. 
 *
 * The Inverse FFT method is not implemented, but holds the idea of storing
 * the exact spectrum and phases of the residual component to file.  During
 * synthesis, it may be possible to achieve higher fidelity by interpolating this
 * data, instead of approximating the phases
 *
 * No stochastic component can also be specified in order to skip the this
 * time consuming process altogether.  This is especially useful when 
 * performing multiple analyses to fine tune parameters pertaining to the 
 * determistic component; once that is achieved, the stochastic component
 * will be much better as well.
 *  
 * \todo review the various options for stochastic synthesis and determine
 * which ones should stay
 */
enum SMS_StocSynthType
{
        SMS_STOC_WAVE,            /*!< waveform samples */
        SMS_STOC_IFFT,               /*!< inverse FFT (not used) */
        SMS_STOC_APPROX,        /*!< spectral approximation */
        SMS_STOC_NONE              /*!< no stochastistic component */
};


/*! \brief Error codes returned by SMS file functions */
enum SMS_ERRORS
{
        SMS_OK,              /*!< no error*/
        SMS_NOPEN,       /*!< couldn't open file */
        SMS_NSMS ,        /*!< not a SMS file */
        SMS_MALLOC,    /*!< couldn't allocate memory */
        SMS_RDERR,        /*!< read error */
        SMS_WRERR	      /*!< write error */
};

/*! \brief debug modes 
 *
 * \todo write details about debug files
 */
enum SMS_DBG
{
        SMS_DBG_NONE,                    /*!< no debugging */
        SMS_DBG_INIT,                       /*!< debug initialitzation functions */
        SMS_DBG_PEAK_DET,	          /*!< debug peak detection function */
        SMS_DBG_HARM_DET,	  /*!< debug harmonic detection function */
        SMS_DBG_PEAK_CONT,        /*!< debug peak continuation function */
        SMS_DBG_CLEAN_TRAJ,	  /*!< debug clean trajectories function */
        SMS_DBG_SINE_SYNTH,	  /*!< debug sine synthesis function */
        SMS_DBG_STOC_ANAL,        /*!< debug stochastic analysis function */
        SMS_DBG_STOC_SYNTH,      /*!< debug stochastic synthesis function */
        SMS_DBG_SMS_ANAL,          /*!< debug top level analysis function */
        SMS_DBG_ALL,                       /*!< debug everything */
        SMS_DBG_RESIDUAL,            /*!< write residual to file */
        SMS_DBG_SYNC,                    /*!< write original, synthesis and residual 
                                                                to a text file */
 };

#define SMS_MAX_WINDOW 8190    /*!< \brief maximum size for analysis window */

/* \brief type of sound to be analyzed
 *
 * \todo explain the differences between these two 
 */
enum SMS_SOUND_TYPE
{
        SMS_SOUND_TYPE_MELODY,    /*!< sound composed of several notes */
        SMS_SOUND_TYPE_NOTE          /*!< sound composed of a single note */
};

/* \brief direction of analysis
 *
 * \todo explain when to use reverse 
 */
enum SMS_DIRECTION
{
        SMS_DIR_FWD,           /*!< analysis from left to right */
        SMS_DIR_REV           /*!< analysis from right to left */
};

/* \brief window selection
 *
 * \todo verify terminolgy and usage 
 */
enum SMS_WINDOWS
{
        SMS_WIN_HAMMING,     /*!< hamming */ 		
        SMS_WIN_BH_62,            /*!< blackman-harris, 62dB cutoff */ 		
        SMS_WIN_BH_70,            /*!< blackman-harris, 70dB cutoff */ 	
        SMS_WIN_BH_74,            /*!< blackman-harris, 74dB cutoff */ 
        SMS_WIN_BH_92             /*!< blackman-harris, 92dB cutoff */ 
};

/*    re-analyze and clean trajectories */

#define SMS_MIN_GOOD_FRAMES 3  /*!< minimum number of stable frames for backward search */

#define SMS_MAX_DEVIATION .01   /*!< maximum deviation allowed */
/*! number of frames in the past to be looked in possible re-analyze */
#define SMS_ANAL_DELAY     10   
/*! total number of delay frames */
#define SMS_DELAY_FRAMES (SMS_MIN_GOOD_FRAMES + SMS_ANAL_DELAY)

/*!
 *  \brief frame status
 */
enum SMS_FRAME_STATUS 
{
        SMS_FRAME_EMPTY,
        SMS_FRAME_READY,
        SMS_FRAME_PEAKS_FOUND,
        SMS_FRAME_FUND_FOUND,
        SMS_FRAME_TRAJ_FOUND,
        SMS_FRAME_CLEANED, 
        SMS_FRAME_RECOMPUTED,
        SMS_FRAME_DETER_SYNTH,
        SMS_FRAME_STOC_COMPUTED, 
        SMS_FRAME_DONE,
        SMS_FRAME_END
};


#define SMS_MIN_SIZE_FRAME  128   /* size of synthesis frame */

extern float *sms_tab_sine; /*!< global table to hold a sine function */
extern float *sms_tab_sinc; /*!< global table to hold a sinc function */

extern float *sms_window_spec;


/*! \defgroup Math_Macros 
 *  \brief mathematical operations and values needed for functions within
 *   this library 
 * \{
 */
#define PI 3.141592653589793238462643    /*!< pi */
#define TWO_PI 6.28318530717958647692 /*!< pi * 2 */
#define PI_2 1.57079632679489661923        /*< pi / 2 */
#define HALF_MAX 1073741823.5  /*!< half the max of a 32-bit word */
#define LOG2 0.69314718 /*!< \todo write this in mathematical terms */

#define TO_DB(x)    	((x > SMS_MIN_MAG) ? 20 * log10(x/SMS_MIN_MAG) : 0)
#define TO_MAG(x)     ((x <= 0) ? 0 : SMS_MIN_MAG * pow(10.0, x/20.0))

#ifndef MAX
/*! \brief returns the maximum of a and b */
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
/*! \brief returns the minimum of a and b */
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

#define SHORT_TO_FLOAT ( 2.0f / pow(2.0,16)) /*!< \todo should not be necessary, if all shorts are removed */
/*! \} */

/* function declarations */ 

int SmsAnalysis (short *pSWaveform, long sizeNewData, SMS_Data *pSmsRecord,
                 SMS_AnalParams *pAnalParams, long *pINextSizeRead);

int SmsInit( void );  

int SmsInitAnalysis ( SMS_Header *pSmsHeader, SMS_AnalParams *pAnalParams);

int SmsInitSynth( SMS_Header *pSmsHeader, SMS_SynthParams *pSynthParams );

int SmsFreeAnalysis (SMS_AnalParams *pAnalParams);

int SmsFreeSynth( SMS_SynthParams *pSynthParams );

void FillBuffer (short *pSWaveform, long sizeNewData, SMS_AnalParams *pAnalParams);

void moveFrames();

void InitializeFrame (int iCurrentFrame, SMS_AnalParams *pAnalParams, 
                      int sizeWindow);
		     
void ComputeFrame (int iCurrentFrame, SMS_AnalParams *pAnalParams, 
                   float fRefFundamental);

void GetWindow (int sizeWindow, float *pFWindow, int iWindowType);

void BlackmanHarris (int sizeWindow, float *pFWindow);

void Hamming (int sizeWindow, float *pFWindow);

void Hanning (int sizeWindow, float *pFWindow);

void realft (float *data, int n, int isign);

int Spectrum (float *pFWaveform, int sizeWindow, float *pFMagSpectrum, 
             float *pFPhaseSpectrum, SMS_AnalParams *pAnalParams);

int QuickSpectrum (short *pIWaveform, float *pFWindow, int sizeWindow, 
                   float *pFMagSpectrum, float *pFPhaseSpectrum, int sizeFft);

int QuickSpectrumF (float *pFWaveform, float *pFWindow, int sizeWindow, 
                    float *pFMagSpectrum, float *pFPhaseSpectrum, int sizeFft);

int InverseQuickSpectrum (float *pFMagSpectrum, float *pFPhaseSpectrum, 
                          int sizeFft, float *pFWaveform, int sizeWave);

int InverseQuickSpectrumW (float *pFMagSpectrum, float *pFPhaseSpectrum, 
                           int sizeFft, float *pFWaveform, int sizeWave, 
                           float *pFWindow);

int SpectralApprox (float *pFSpec1, int sizeSpec1, int sizeSpec1Used,
                    float *pFSpec2, int sizeSpec2, int nCoefficients);
		  
int SetSizeWindow (int iCurrentFrame, SMS_AnalParams *pAnalParams);

float GetDeviation (SMS_AnalParams *pAnalParams, int iCurrentFrame);

int ReAnalyze (int iCurrentFrame, SMS_AnalParams *pAnalParams);

int PeakDetection (float *pFMagSpectrum, float *pAPhaSpectrum, int sizeMag, 
                   int sizeWindow, SMS_Peak *pSpectralPeaks, 
                   SMS_AnalParams *pAnalParams);

void HarmDetection (SMS_AnalFrame *pFrame, float fRefFundamental,
                    SMS_AnalParams *pAnalParams);

void GenPeakContinuation (int iFrame, SMS_AnalParams *pAnalParams);

int DeleteCandidate (SMS_ContCandidate *pCandidate, int nCand, int iBestPeak);

int PeakContinuation (int iFrame, SMS_AnalParams *pAnalParams);

float PreEmphasis (float fInput);

float DeEmphasis (float fInput);

void Covariance (float *pFSig, int nSig, int nStage, float *pFPhi, int nMax);

void CovLatticeHarm (float *pFPhi, int nMax, int m, float *pFPredCoeff, 
                     float *pFReflexCoeff, float *pFError, float *pFScr);

int CleanTrajectories (int iCurrentFrame, SMS_AnalParams *pAnalParams);

void ScaleDeterministic (float *pFSynthBuffer, float *pFOriginalBuffer,
                         float *pFMagTraj, SMS_AnalParams *pAnalParams, int nTraj);
			
int PrepSine (int nTableSize);

double SinTab (double fTheta);

int PrepSinc ();

double SincTab (double fTheta);

int SmsSynthesis (SMS_Data *pSmsRecord, float*pFSynthesis, 
                  SMS_SynthParams *pSynthParams);
                

int FrameSineSynth (SMS_Data *pSmsRecord, float *pFBuffer, 
                    int sizeBuffer, SMS_Data *pLastFrame,
                    int iSamplingRate);

long random ();

int WriteSmsHeader (char *pChFileName, SMS_Header *pSmsHeader, 
                    FILE **ppOutSmsFile);

int WriteSmsFile (FILE *pSmsFile, SMS_Header *pSmsHeader);

int WriteSmsRecord (FILE *pSmsFile, SMS_Header *pSmsHeader, 
                    SMS_Data *pSmsRecord);

int InitSmsHeader (SMS_Header *pSmsHeader);

int AllocSmsRecord (SMS_Header *pSmsHeader, SMS_Data *pSmsRecord);

int AllocateSmsRecord (SMS_Data *pSmsRecord, int nTraj, int nCoeff, 
                       int iPhase, int sizeHop, int stochType);

int GetSmsRecord (FILE *pInputFile, SMS_Header *pSmsHeader, int iRecord,
                  SMS_Data *pSmsRecord);

int GetSmsHeader (char *pChFileName, SMS_Header **ppSmsHeader,
                  	FILE **ppInputFile);

int GetRecordBSize (SMS_Header *pSmsHeader);

const char* SmsReadErrorStr( int iError);

int quit (char *pChText);

void InitSmsRecord (SMS_Data *pSmsRecord);

void FreeSmsRecord (SMS_Data *pSmsRecord);

void ClearSmsRecord (SMS_Data *pSmsRecord);

int CopySmsRecord (SMS_Data *pCopySmsRecord, SMS_Data *pOriginalSmsRecord);

void MoveFrames (SMS_AnalParams *pAnalParams);

int GetResidual (float *pFSynthesis, float *pFOriginal,  
                 float *pFResidual, int sizeWindow, SMS_AnalParams *pAnalParams);

int StocAnalysis (float *pFResidual, int sizeWindow, 
                  SMS_Data *pSmsRecord, SMS_AnalParams *pAnalParams);

int CreateResidualFile (SMS_AnalParams *pAnalParams);

int WriteToResidualFile (float *pFBuffer, int sizeBuffer);

int WriteResidualFile ();

int CreateDebugFile (SMS_AnalParams *pAnalParams);

int WriteDebugFile ();

int InterpolateArrays (float *pFArray1, int sizeArray1, float *pFArray2,
                       int sizeArray2, float *pFArrayOut, int sizeArrayOut,
                       float fInterpFactor);

int InterpolateSmsRecords (SMS_Data *pSmsRecord1, SMS_Data *pSmsRecord2,
                           SMS_Data *pSmsRecordOut, float fInterpFactor);

int FilterArray (float *pFArray, int size1, int size2, float *pFOutArray);

void ClearSine();

void IFFTwindow (int sizeWindow, float *pFWindow);

int GetSoundData (SMS_SndHeader *pSoundHeader, short *pSoundData, long sizeSound,
                  long offset);

int OpenSound (char *pChInputSoundFile, SMS_SndHeader *pSoundHeader);

int CreateOutputSoundFile (SMS_SynthParams synthParams, char *pChOutputSoundFile);

int WriteToOutputFile (float *pFBuffer, int sizeBuffer);

int WriteOutputFile ();

int freeBuffers ();

/***********************************************************************************/
/************ things for hybrid program that may not be necessary ***********************/

//#define MAX_BUFF 1000000

/*! \struct SMS_HybParams
 * \brief structure for hybrid program 
 * \todo find out wha this is, hah 
 */
typedef struct
{
  int nCoefficients;
  float fGain;
  float fMagBalance;
  int iSmoothOrder;
  float *pCompressionEnv;
  int sizeCompressionEnv;
} SMS_HybParams;

int Hybridize (short *pIWaveform1, int sizeWave1, short *pIWaveform2, 
               int sizeWave2, float *pFWaveform, SMS_HybParams params);


#endif /* _SMS_H */
