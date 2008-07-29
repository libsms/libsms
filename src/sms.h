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

/*! \brief identification constant
 * 
 * constant number that is first within SMS_Header, in order to correctly
 * identify an SMS file when read.  */
#define SMS_MAGIC 767  

/*!  \brief analysis format
 *
 *   \todo explain how this is important for the analysis
 */
enum SMS_Format
{
        SMS_FORMAT_H = 1, /*!< format harmonic */
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
        SMS_STYPE_ALL =1, /*!< both components combined */
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
        SMS_DET_IFFT = 1, /*!< Inverse Fast Fourier Transform (IFFT) */
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

/* useful macros */

/* Error codes returned by SMS file functions */
#define SMS_OK      0  /* no error*/
#define SMS_NOPEN  -1  /* couldn't open file */
#define SMS_NSMS   -2  /* not a SMS file */
#define SMS_MALLOC -3  /* couldn't allocate memory */
#define SMS_RDERR  -4  /* read error */
#define SMS_WRERR	 -5  /* write error */


/* debug modes */
#define DEBUG_NONE 0
#define DEBUG_INIT        1     /* debug initialitzation functions */
#define DEBUG_PEAK_DET    2	    /* debug peak detection function */
#define DEBUG_HARM_DET    3	    /* debug harmonic detection function */
#define DEBUG_PEAK_CONT   4     /* debug peak continuation function */
#define DEBUG_CLEAN_TRAJ  5	    /* debug clean trajectories function */
#define DEBUG_SINE_SYNTH  6	    /* debug sine synthesis function */
#define DEBUG_STOC_ANAL   7     /* debug stochastic analysis function */
#define DEBUG_STOC_SYNTH  8     /* debug stochastic synthesis function */
#define DEBUG_SMS_ANAL    9     /* debug top level analysis function */
#define DEBUG_ALL         10    /* debug everything */
#define DEBUG_RESIDUAL    11    /* write residual to file */
#define DEBUG_SYNC	      12    /* write original, synthesis and residual to
                                   a text file */

#ifndef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

#define TWO_PI 6.28318530717958647692
#define PI 3.141592653589793238462643
#define PI_2 1.57079632679489661923
#define HALF_MAX 1073741823.5  /* half the max of a 32-bit word */
#define LOG2 0.69314718

#define EMPHASIS_COEFF    .9      /* coefficient for pre_emphasis filter */

#define TO_DB(x)    	((x > MAG_THRESHOLD) ? 20 * log10(x/MAG_THRESHOLD) : 0)
#define TO_MAG(x)     ((x <= 0) ? 0 : MAG_THRESHOLD * pow(10.0, x/20.0))

#define SHORT_TO_FLOAT ( 2.0f / pow(2.0,16)) 

/* for hybrid program */
#define MAX_BUFF 1000000
#define ENV_THRESHOLD     .01

/*    short-time Fourier analysis   */

#define WINDOWS_IN_FFT 2        /* the number of analysis windows
                                   that fit in one FFT */
#define MAX_SIZE_WINDOW 8190    /* maximum size for analysis window */
#define MAX_SIZE_MAG    8192    /* maximum size for magnitude spectrum */

/*    peak detection   */

/* information attached to a spectral peak */
typedef struct 
{
	float fFreq;                  /* frequency of peak */
	float fMag;                   /* magnitude of peak */
	float fPhase;                 /* phase of peak */
} PEAK;

#define MAG_THRESHOLD     .3      /* minimum magnitude to be searched */
#define MAX_NUM_PEAKS      200    /* maximum number of peaks */

/*   harmonic detection */

/* information attached to a harmonic candidate */
typedef struct 
{
	float fFreq;                  /* frequency */
	float fMag;                   /* magnitude */
	float fMagPerc;               /* percentage of magnitude */
	float fFreqDev;               /* deviation from perfect harmonic */
	float fHarmRatio;             /* percentage of harmonics found */
} HARM_CANDIDATE;


#define N_FUND_HARM      6 /* number of harmonics to use for fundamental 
                              detection */
#define N_HARM_PEAKS     4 /* number of peaks to check as possible ref 
                              harmonics */
#define FREQ_DEV_THRES  .07 /* threshold for deviation from perfect 
                               harmonics */
#define MAG_PERC_THRES   .6 /* threshold for magnitude of harmonics
                               with respect to the total magnitude */
#define HARM_RATIO_THRES .8 /* threshold for percentage of harmonics found */
#define TYPE_MELODY		0    /* sound composed of several notes */
#define TYPE_SINGLE_NOTE	1  /* sound composed of a single note */
#define DIRECT	0           /* analysis from left to right */
#define REVERSE	1           /* analysis from right to left */


#define HAMMING 		   0
#define BLACKMAN_HARRIS_62 1
#define BLACKMAN_HARRIS_70 2
#define BLACKMAN_HARRIS_74 3
#define BLACKMAN_HARRIS_92 4

/*    peak continuation    */

/* diferent status of guide */
#define BEGINNING -2
#define DEAD -1
#define ACTIVE 0

/* information attached to a guide */
typedef struct
{
	float fFreq;          /* frequency of guide */
	float fMag;           /* magnitude of guide */
	int iStatus;          /* status of guide: DEAD, SLEEPING, ACTIVE */
	short iPeakChosen;    /* peak number chosen by the guide */
} GUIDE;

/* information attached to a continuation candidate */
typedef struct
{
	float fFreqDev;       /* frequency deviation from guide */
	float fMagDev;        /* magnitude deviation from guide */
	int iPeak;            /* peak number */
} CONT_CANDIDATE;        

#define MAX_CONT_CANDIDATES 5  /* maximum number of peak continuation
                                  candidates */


/*    re-analyze and clean trajectories */

#define MIN_GOOD_FRAMES 3  /* minimum number of stable frames for backward 
                              search */
#define MAX_DEVIATION .01   /* maximum deviation allowed */
#define ANAL_DELAY     10   /* number of frames in the past to be
                               looked in possible re-analyze */
/* total number of delay frames */
#define DELAY_FRAMES (MIN_GOOD_FRAMES + ANAL_DELAY)


/* buffer for sound data */
typedef struct
{
	int iSoundSample;          /* sample number of the sound file that
                                corresponds to the first sample in buffer */
	float *pFBuffer;           /* buffer for original sound */
	int sizeBuffer;            /* size of buffer */
	int iFirstSample;          /* first sample in buffer that is a good one */
} SOUND_BUFFER;

enum frameStatus {EMPTY, READY, PEAKS_FOUND, FUND_FOUND, TRAJ_FOUND, CLEANED, 
                  RECOMPUTED, DETER_SYNTH, STOC_COMPUTED, DONE, END};

/* analysis frame structure */
typedef struct 
{
	int iFrameSample;         /* sample number of the sound file that 
                               corresponds to the middle of the frame */
	int iFrameSize;           /* number of samples used in the frame */
	int iFrameNum;            /* frame number */
	PEAK pSpectralPeaks[MAX_NUM_PEAKS];  /* spectral peaks found in frame */
	int nPeaks;               /* number of peaks found */
	float fFundamental;       /* fundamental frequency in frame */
	SMS_Data deterministic;   /* deterministic data */
	enum frameStatus iStatus; /* status of frame */
} ANAL_FRAME;

/* structure with useful information for the analysis program */
typedef struct 
{
	int iDebugMode; /* debug codes defined below */
	int iFormat;          /* format code defined below */
	int iStochasticType;      /*  type of stochastic synthesis */
	float fLowestFundamental; /* lowest fundamental frequency in Hz */
	float fHighestFundamental;/* highest fundamental frequency in Hz */
	float fDefaultFundamental;/* default fundamental in Hz */
	float fPeakContToGuide;   /* contribution of previous peak to current guide (between 0 and 1) */
	float fFundContToGuide;   /* contribution of current fundamental to current guide (between 0 and 1) */
	float fFreqDeviation;     /* maximum deviation from peak to peak */				     
	int iSamplingRate;        /* sampling rate of input sound */
	int iDefaultSizeWindow;   /* default size of analysis window in samples */
	int sizeHop;              /* hop size of analysis window in samples */
	float fSizeWindow;       /* size of analysis window in number of periods */
	int nGuides;              /* number of guides used */
	int iCleanTraj;           /* whether or not to clean trajectories */
	float fMinRefHarmMag;     /* minimum magnitude in dB for reference peak */
	float fRefHarmMagDiffFromMax; /* maximum magnitude difference from reference peak to highest peak */
	int iRefHarmonic;	       /* reference harmonic to use in the fundamental detection */
	int iMinTrajLength;	       /* minimum length in samples of a given  trajectory */
	int iMaxSleepingTime;	   /* maximum sleeping time for a trajectory */
	float fHighestFreq;        /* highest frequency to be searched */
	float fMinPeakMag;         /* minimum magnitude in dB for a good peak */	
	int iSoundType;            /* type of sound to be analyzed */	
	int iAnalysisDirection;    /* analysis direction, direct or reverse */	
	int iSizeSound;             /* total size of input sound */	 	
	int iWindowType;            /* type of analysis window */			  	 			 
        int iMaxDelayFrames;
        SMS_Data prevFrame;
        SOUND_BUFFER soundBuffer;
        SOUND_BUFFER synthBuffer;
        ANAL_FRAME *pFrames;
        ANAL_FRAME **ppFrames;
        float fResidualPercentage; /* accumalitive residual percentage */
#ifdef FFTW
        fftwf_plan  fftPlan;
        float *pWaveform;
        fftwf_complex *pSpectrum;
#endif
} ANAL_PARAMS;

/* structure with useful information for synthesis */
typedef struct
{
	int iStochasticType; 
	int iSynthesisType;        /* globally defined above */
        int iDetSynthType;         /* globally defined above */
	int iOriginalSRate;
	int iSamplingRate;
	SMS_Data previousFrame;
	int sizeHop;
        int origSizeHop;
	float *pFDetWindow;
        float *pFStocWindow;
#ifdef FFTW
        fftwf_plan  fftPlan;
        fftwf_complex *pSpectrum;
        float *pWaveform;
#else
        float *realftOut; // RTE_DEBUG : comparing realft and fftw
#endif
} SYNTH_PARAMS;

#define SIZE_SYNTH_FRAME  128   /* size of synthesis frame */



/* structure for hybrid program */
typedef struct
{
  int nCoefficients;
  float fGain;
  float fMagBalance;
  int iSmoothOrder;
  float *pCompressionEnv;
  int sizeCompressionEnv;
} HYB_PARAMS;

/* todo: define sin table size here too */
#define SIN_TABLE_SIZE 4096//was 2046
#define SINC_TABLE_SIZE 4096
extern float *sms_tab_sine;
extern float *sms_tab_sinc;

extern float *sms_window_spec;

/* function declarations */ 

int SmsAnalysis (short *pSWaveform, long sizeNewData, SMS_Data *pSmsRecord,
                 ANAL_PARAMS *pAnalParams, long *pINextSizeRead);

int SmsInit( void );  

int SmsInitAnalysis ( SMS_Header *pSmsHeader, ANAL_PARAMS *pAnalParams);

int SmsInitSynth( SMS_Header *pSmsHeader, SYNTH_PARAMS *pSynthParams );

int SmsFreeAnalysis (ANAL_PARAMS *pAnalParams);

int SmsFreeSynth( SYNTH_PARAMS *pSynthParams );

void FillBuffer (short *pSWaveform, long sizeNewData, ANAL_PARAMS *pAnalParams);

void moveFrames();

void InitializeFrame (int iCurrentFrame, ANAL_PARAMS *pAnalParams, 
                      int sizeWindow);
		     
void ComputeFrame (int iCurrentFrame, ANAL_PARAMS *pAnalParams, 
                   float fRefFundamental);

void GetWindow (int sizeWindow, float *pFWindow, int iWindowType);

void BlackmanHarris (int sizeWindow, float *pFWindow);

void Hamming (int sizeWindow, float *pFWindow);

void Hanning (int sizeWindow, float *pFWindow);

void realft (float *data, int n, int isign);

//int initFFTW( ANAL_PARAMS *pAnalParams);

//int initInverseFFTW( SYNTH_PARAMS *pSynthParams);

int Spectrum (float *pFWaveform, int sizeWindow, float *pFMagSpectrum, 
             float *pFPhaseSpectrum, ANAL_PARAMS *pAnalParams);

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
		  
int SetSizeWindow (int iCurrentFrame, ANAL_PARAMS *pAnalParams);

float GetDeviation (ANAL_PARAMS *pAnalParams, int iCurrentFrame);

int ReAnalyze (int iCurrentFrame, ANAL_PARAMS *pAnalParams);

int PeakDetection (float *pFMagSpectrum, float *pAPhaSpectrum, int sizeMag, 
                   int sizeWindow, PEAK *pSpectralPeaks, 
                   ANAL_PARAMS *pAnalParams);

void HarmDetection (ANAL_FRAME *pFrame, float fRefFundamental,
                    ANAL_PARAMS *pAnalParams);

void GenPeakContinuation (int iFrame, ANAL_PARAMS *pAnalParams);

int DeleteCandidate (CONT_CANDIDATE *pCandidate, int nCand, int iBestPeak);

int PeakContinuation (int iFrame, ANAL_PARAMS *pAnalParams);

float PreEmphasis (float fInput);

float DeEmphasis (float fInput);

void Covariance (float *pFSig, int nSig, int nStage, float *pFPhi, int nMax);

void CovLatticeHarm (float *pFPhi, int nMax, int m, float *pFPredCoeff, 
                     float *pFReflexCoeff, float *pFError, float *pFScr);

int CleanTrajectories (int iCurrentFrame, ANAL_PARAMS *pAnalParams);

void ScaleDeterministic (float *pFSynthBuffer, float *pFOriginalBuffer,
                         float *pFMagTraj, ANAL_PARAMS *pAnalParams, int nTraj);
			
int PrepSine (int nTableSize);

double SinTab (double fTheta);

int PrepSinc ();

double SincTab (double fTheta);

int SmsSynthesis (SMS_Data *pSmsRecord, float*pFSynthesis, 
                  SYNTH_PARAMS *pSynthParams);
                

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

void MoveFrames (ANAL_PARAMS *pAnalParams);

int GetResidual (float *pFSynthesis, float *pFOriginal,  
                 float *pFResidual, int sizeWindow, ANAL_PARAMS *pAnalParams);

int StocAnalysis (float *pFResidual, int sizeWindow, 
                  SMS_Data *pSmsRecord, ANAL_PARAMS *pAnalParams);

int CreateResidualFile (ANAL_PARAMS *pAnalParams);

int WriteToResidualFile (float *pFBuffer, int sizeBuffer);

int WriteResidualFile ();

int CreateDebugFile (ANAL_PARAMS *pAnalParams);

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

int CreateOutputSoundFile (SYNTH_PARAMS synthParams, char *pChOutputSoundFile);

int WriteToOutputFile (float *pFBuffer, int sizeBuffer);

int WriteOutputFile ();

int Hybridize (short *pIWaveform1, int sizeWave1, short *pIWaveform2, 
               int sizeWave2, float *pFWaveform, HYB_PARAMS params);

int freeBuffers ();

#endif
