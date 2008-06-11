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
/*
 * header file for all the SMS programs
 *
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

/* structure for header of SMS file */
typedef struct 
{
	/* fix part */
	int iSmsMagic;         /* magic number for SMS data file */
	int iHeadBSize;        /* size in bytes of header */
	int nRecords;	         /* number of data records */
	int iRecordBSize;      /* size in bytes of data record */
	int iFormat;           /* type of data format */
	int iFrameRate;        /* rate in Hz of data records */
	int iStochasticType;   /* representation of stochastic coefficients */
	int nTrajectories;     /* number of trajectoires in each record */
	int nStochasticCoeff;  /* number of stochastic coefficients in each	
 	                          record */
//        int sizeHop;            /* size of hop used in sinusoidal resynthesis */
	float fAmplitude;      /* average amplitude of represented sound */
	float fFrequency;      /* average fundamental frequency */
	int iOriginalSRate;    /* sampling rate of original sound */
	int iBegSteadyState;   /* record number of begining of steady state */
	int iEndSteadyState;   /* record number of end of steady state */
	float fResidualPerc;   /* percentage of the residual with respect to the 
	                          original */
	int nLoopRecords;      /* number of loop records specified */
	int nSpecEnvelopePoints; /* number of breakpoints in spectral envelope */
	int nTextCharacters;   /* number of text characters */
	/* variable part */
	int *pILoopRecords;    /* array of record numbers of loop points */
	float *pFSpectralEnvelope; /* spectral envelope of partials */
	char *pChTextCharacters; /* Textual information relating to the sound */
} SMSHeader;

/* structure  including sound header information */
typedef struct {
    long nSamples;       /* Number of samples in the sound */
    int iSamplingRate;   /* The sampling rate */
    short channelCount;  /* The number of channels */
    long sizeHeader;	     /* size of sound header in bytes */
} SNDHeader;



#define SMS_MAGIC 767

/* for iFormat */
#define FORMAT_HARMONIC 1
#define FORMAT_INHARMONIC 2
#define FORMAT_HARMONIC_WITH_PHASE 3
#define FORMAT_INHARMONIC_WITH_PHASE 4

/* Synthesis method for Deterministic */
#define DET_IFFT 1
#define DET_OSC 2


/* for iStochasticType */
#define STOC_WAVEFORM 0
#define STOC_IFFT 1
#define STOC_APPROX 2
#define STOC_NONE 3

/* Synthesis Types */
#define STYPE_ALL 1
#define STYPE_DET 2
#define STYPE_STOC 3

/* structure with SMS data */
typedef struct 
{
	float *pSmsData;           /* pointer to all SMS data */
	int sizeData;              /* size of all the data */
	float *pFFreqTraj;         /* frequency of sinusoids */
	float *pFMagTraj;          /* magnitude of sinusoids */
	float *pFPhaTraj;          /* phase of sinusoids */
	int nTraj;                 /* number of sinusoids */
        float *pFStocWave;
        int nSamples;        /* number of samples in StocWave */
	float *pFStocGain;         /* gain of stochastic component */
	float *pFStocCoeff;        /* filter coefficients for stochastic component */
	int nCoeff;                /* number of filter coefficients */
} SMS_DATA;

/* useful macros */

/* Error codes returned by SMS file functions */
#define SMS_OK      0  /* no error*/
#define SMS_NOPEN  -1  /* couldn't open file */
#define SMS_NSMS   -2  /* not a SMS file */
#define SMS_MALLOC -3  /* couldn't allocate memory */
#define SMS_RDERR  -4  /* read error */
#define SMS_WRERR	 -5  /* write error */


/* debug modes */
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

/* structure with useful information for the analysis program */
typedef struct 
{
	int iDebugMode;     /* 0 no debug
                           1 debug initialitzation functions,
                           2 debug peak detection function,
                           3 debug harmonic detection function,
                           4 debug peak continuation function,
                           5 debug clean trajectories function,
                           6 debug sine synthesis function,
                           7 debug stochastic analysis function,
                           8 debug stochastic synthesis function,
                           9 debug top level analysis function,
                           10 debug everything */
	int iFormat;          /* 1 the sound is analized as harmonic,
                           2 is inharmonic 
                           3 is harmonic and phases are kept
                           4 is inharmonic and phases are kept */
	int iStochasticType;      /*  TODO: update 1: filter, 2: lines, 3: none */
	float fLowestFundamental; /* lowest fundamental frequency in Hz */
	float fHighestFundamental;/* highest fundamental frequency in Hz */
	float fDefaultFundamental;/* default fundamental in Hz */
	float fPeakContToGuide;   /* contribution of previous peak to current
                                guide (between 0 and 1) */
	float fFundContToGuide;   /* contribution of current fundamental to
                                current guide (between 0 and 1) */
	float fFreqDeviation;     /* maximum deviation from peak to peak */				     
	int iSamplingRate;        /* sampling rate of input sound */
	int iDefaultSizeWindow;   /* default size of analysis window in samples */
	int sizeHop;              /* hop size of analysis window in samples */
	float fSizeWindow;       /* size of analysis window in number of periods */
	int nGuides;              /* number of guides used */
	int iCleanTraj;           /* whether or not to clean trajectories */
	float fMinRefHarmMag;     /* minimum magnitude in dB for reference peak */
	float fRefHarmMagDiffFromMax; /* maximum magnitude difference from 
	                                 reference peak to highest peak */
	int iRefHarmonic;	       /* reference harmonic to use in the fundamental 
	                              detection */
	int iMinTrajLength;	       /* minimum length in samples of a given
	                            trajectory */
	int iMaxSleepingTime;	   /* maximum sleeping time for a trajectory */
	float fHighestFreq;        /* highest frequency to be searched */
	float fMinPeakMag;         /* minimum magnitude in dB for a good peak */	
	int iSoundType;            /* type of sound to be analyzed */	
	int iAnalysisDirection;    /* analysis direction, direct or reverse */	
	int iSizeSound;             /* total size of input sound */	 	
	int iWindowType;            /* type of analysis window */			  	 			 
        fftwf_plan  fftPlan;
        float fResidualPercentage; /* accumalitive residual percentage */
        float *pCfftIn;
        fftwf_complex *pFfftOut;
} ANAL_PARAMS;

/* structure with useful information for synthesis */
typedef struct
{
	int iStochasticType; 
	int iSynthesisType;        /* globally defined above */
        int iDetSynthType;         /* globally defined above */
	int iOriginalSRate;
	int iSamplingRate;
	SMS_DATA previousFrame;
	int sizeHop;
        int origSizeHop;
	float *pFDetWindow;
        float *pFStocWindow;
        fftwf_plan  fftPlan;
        fftwf_complex *pComplexSpec;
        float *pRealWave;
        float *realftOut; // RTE_DEBUG : comparing realft and fftw
} SYNTH_PARAMS;

#define SIZE_SYNTH_FRAME  128   /* size of synthesis frame */

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
	SMS_DATA deterministic;   /* deterministic data */
	enum frameStatus iStatus; /* status of frame */
} ANAL_FRAME;

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

/* function declarations */ 

int SmsAnalysis (short *pSWaveform, long sizeNewData, SMS_DATA *pSmsRecord,
                 ANAL_PARAMS analParams, long *pINextSizeRead);

int SmsInit( void );  

int SmsInitAnalysis (ANAL_PARAMS analParams);

int SmsInitSynth( SMSHeader *pSmsHeader, SYNTH_PARAMS *pSynthParams );

int SmsFreeSynth( SYNTH_PARAMS *pSynthParams );

void FillBuffer (short *pSWaveform, long sizeNewData, ANAL_PARAMS analParams);

void moveFrames();

void InitializeFrame (int iCurrentFrame, ANAL_PARAMS analParams, 
                      int sizeWindow);
		     
void ComputeFrame (int iCurrentFrame, ANAL_PARAMS analParams, 
                   float fRefFundamental);

void GetWindow (int sizeWindow, float *pFWindow, int iWindowType);

void BlackmanHarris (int sizeWindow, float *pFWindow);

void Hamming (int sizeWindow, float *pFWindow);

void Hanning (int sizeWindow, float *pFWindow);

void realft (float *data, int n, int isign);

//int initFFTW( ANAL_PARAMS *pAnalParams);

//int initInverseFFTW( SYNTH_PARAMS *pSynthParams);

int Spectrum (float *pFWaveform, int sizeWindow, float *pFMagSpectrum, 
             float *pFPhaseSpectrum, ANAL_PARAMS analParams);

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
		  
int SetSizeWindow (int iCurrentFrame, ANAL_PARAMS analParams);

float GetDeviation (int iCurrentFrame);

int ReAnalyze (int iCurrentFrame, ANAL_PARAMS analParams);

int PeakDetection (float *pFMagSpectrum, float *pAPhaSpectrum, int sizeMag, 
                   int sizeWindow, PEAK *pSpectralPeaks, 
                   ANAL_PARAMS analParams);

void HarmDetection (ANAL_FRAME *pFrame, float fRefFundamental,
                    ANAL_PARAMS analParams);

void GenPeakContinuation (int iFrame, ANAL_PARAMS analParams);

int DeleteCandidate (CONT_CANDIDATE *pCandidate, int nCand, int iBestPeak);

int PeakContinuation (int iFrame, ANAL_PARAMS analParams);

float PreEmphasis (float fInput);

float DeEmphasis (float fInput);

void Covariance (float *pFSig, int nSig, int nStage, float *pFPhi, int nMax);

void CovLatticeHarm (float *pFPhi, int nMax, int m, float *pFPredCoeff, 
                     float *pFReflexCoeff, float *pFError, float *pFScr);

int CleanTrajectories (int iCurrentFrame, ANAL_PARAMS analParams);

void ScaleDeterministic (float *pFSynthBuffer, float *pFOriginalBuffer,
                         float *pFMagTraj, ANAL_PARAMS analParams, int nTraj);
			
int PrepSine (int nTableSize);

double SinTab (double fTheta);

int PrepSinc ();

double SincTab (double fTheta);

int SmsSynthesis (SMS_DATA *pSmsRecord, float*pFSynthesis, 
                  SYNTH_PARAMS *pSynthParams);
                

int FrameSineSynth (SMS_DATA *pSmsRecord, float *pFBuffer, 
                    int sizeBuffer, SMS_DATA *pLastFrame,
                    int iSamplingRate);

long random ();

int WriteSmsHeader (char *pChFileName, SMSHeader *pSmsHeader, 
                    FILE **ppOutSmsFile);

int WriteSmsFile (FILE *pSmsFile, SMSHeader *pSmsHeader);

int WriteSmsRecord (FILE *pSmsFile, SMSHeader *pSmsHeader, 
                    SMS_DATA *pSmsRecord);

int InitSmsHeader (SMSHeader *pSmsHeader);

int AllocSmsRecord (SMSHeader *pSmsHeader, SMS_DATA *pSmsRecord);

int AllocateSmsRecord (SMS_DATA *pSmsRecord, int nTraj, int nCoeff, 
                       int iPhase, int sizeHop, int stochType);

int GetSmsRecord (FILE *pInputFile, SMSHeader *pSmsHeader, int iRecord,
                  SMS_DATA *pSmsRecord);

int GetSmsHeader (char *pChFileName, SMSHeader **ppSmsHeader,
                  	FILE **ppInputFile);

int GetRecordBSize (SMSHeader *pSmsHeader);

const char* SmsReadErrorStr( int iError);

int quit (char *pChText);

void InitSmsRecord (SMS_DATA *pSmsRecord);

void FreeSmsRecord (SMS_DATA *pSmsRecord);

void ClearSmsRecord (SMS_DATA *pSmsRecord);

int CopySmsRecord (SMS_DATA *pCopySmsRecord, SMS_DATA *pOriginalSmsRecord);

void MoveFrames ();

int GetResidual (float *pFSynthesis, float *pFOriginal,  
                 float *pFResidual, int sizeWindow, ANAL_PARAMS analParams);

int StocAnalysis (float *pFResidual, int sizeWindow, 
                  SMS_DATA *pSmsRecord, ANAL_PARAMS analParams);

int CreateResidualFile (ANAL_PARAMS analParams);

int WriteToResidualFile (float *pFBuffer, int sizeBuffer);

int WriteResidualFile ();

int CreateDebugFile (ANAL_PARAMS analParams);

int WriteDebugFile ();

int InterpolateArrays (float *pFArray1, int sizeArray1, float *pFArray2,
                       int sizeArray2, float *pFArrayOut, int sizeArrayOut,
                       float fInterpFactor);

int InterpolateSmsRecords (SMS_DATA *pSmsRecord1, SMS_DATA *pSmsRecord2,
                           SMS_DATA *pSmsRecordOut, float fInterpFactor);

int FilterArray (float *pFArray, int size1, int size2, float *pFOutArray);

void ClearSine();

void IFFTwindow (int sizeWindow, float *pFWindow);

int GetSoundData (SNDHeader *pSoundHeader, short *pSoundData, long sizeSound,
                  long offset);

int OpenSound (char *pChInputSoundFile, SNDHeader *pSoundHeader);

int CreateOutputSoundFile (SYNTH_PARAMS synthParams, char *pChOutputSoundFile);

int WriteToOutputFile (float *pFBuffer, int sizeBuffer);

int WriteOutputFile ();

int Hybridize (short *pIWaveform1, int sizeWave1, short *pIWaveform2, 
               int sizeWave2, float *pFWaveform, HYB_PARAMS params);

int freeBuffers ();

#endif
