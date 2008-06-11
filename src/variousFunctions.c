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
#include "sms.h"

float *pFWindowSpec;
double *pFSTab = NULL, *pFSincTab = NULL;
short MaxDelayFrames;

int SmsInit( void )
{
	/* prepare sinc and sine tables only the first time */
	if (pFSincTab == NULL)
		PrepSinc ();
	if (pFSTab == NULL)
		PrepSine (2046); //try 4096
        return (1);
}


/* initialize analysis data structures
 *
 * ANAL_PARAMS analParams;    analysis paramaters
 *
 */
int SmsInitAnalysis (ANAL_PARAMS analParams)
{
	extern short MaxDelayFrames;
	extern SOUND_BUFFER soundBuffer, synthBuffer;
	extern ANAL_FRAME **ppFrames, *pFrames;
	int sizeBuffer = (MaxDelayFrames * analParams.sizeHop) + MAX_SIZE_WINDOW;
	int i;
  
	/* sound buffer */
	if ((soundBuffer.pFBuffer = (float *) calloc(sizeBuffer, sizeof(float)))
	    == NULL)
		return -1;
	soundBuffer.iSoundSample = -sizeBuffer;
	soundBuffer.iFirstSample = sizeBuffer;
	soundBuffer.sizeBuffer = sizeBuffer;
  
	/* deterministic synthesis buffer */
	if ((synthBuffer.pFBuffer = 
	      (float *) calloc(2 * analParams.sizeHop, sizeof(float))) == NULL)
		return -1;
	synthBuffer.iSoundSample = -sizeBuffer;
	synthBuffer.sizeBuffer = analParams.sizeHop << 1;
	synthBuffer.iSoundSample = analParams.sizeHop << 1;
  
	/* buffer of analysis frames */
	if ((pFrames = (ANAL_FRAME *) calloc(MaxDelayFrames, sizeof(ANAL_FRAME))) 
	    == NULL)
		return -1;
	if ((ppFrames = 
	     (ANAL_FRAME **) calloc(MaxDelayFrames, sizeof(ANAL_FRAME *)))
	     == NULL)
		return -1;
  
	/* initialize the frame pointers and allocate memory */
	for (i = 0; i < MaxDelayFrames; i++)
	{
		pFrames[i].iStatus = EMPTY;
		(pFrames[i].deterministic).nTraj = analParams.nGuides;
		if (((pFrames[i].deterministic).pFFreqTraj =
		    (float *)calloc (analParams.nGuides, sizeof(float))) == NULL)
			return -1;
		if (((pFrames[i].deterministic).pFMagTraj =
		    (float *)calloc (analParams.nGuides, sizeof(float))) == NULL)
			return -1;
		if (((pFrames[i].deterministic).pFPhaTraj =
		    (float *) calloc (analParams.nGuides, sizeof(float))) == NULL)
			return -1;
		ppFrames[i] = &pFrames[i];
	}

	/* initialitze window buffer for spectrum */
	pFWindowSpec = (float *) calloc (MAX_SIZE_WINDOW, sizeof(float));

        /* allocate memory for FFT */
/*         int sizeFFT = pAnalParams->sizeHop ;   */
/*         pAnalParams->pCfftIn = fftwf_malloc(sizeof(float) * sizeFFT); */
/*         pAnalParams->pFfftOut = fftwf_malloc(sizeof(fftwf_complex) * (sizeFFT / 2 + 1)); */
/*         pAnalParams->fftPlan =  fftwf_plan_dft_r2c_1d( sizeFFT, pAnalParams->pCfftIn, */
/*                                                       pAnalParams->pFfftOut, FFTW_ESTIMATE); */

	return (1);
}

int SmsInitSynth( SMSHeader *pSmsHeader, SYNTH_PARAMS *pSynthParams )
{
        /* set synthesis parameters from arguments and header */
	pSynthParams->iOriginalSRate = pSmsHeader->iOriginalSRate;
	pSynthParams->origSizeHop = pSynthParams->iOriginalSRate / pSmsHeader->iFrameRate;
	pSynthParams->iStochasticType = pSmsHeader->iStochasticType;
        if(pSynthParams->iSamplingRate <= 0)  pSynthParams->iSamplingRate = pSynthParams->iOriginalSRate;

        //RTE TODO: round sizeHop to power of 2, in synthParams too
        int sizeHop = pSynthParams->sizeHop;

        pSynthParams->pFStocWindow = 
		(float *) calloc(sizeHop * 2, sizeof(float));
	Hanning (sizeHop * 2, pSynthParams->pFStocWindow);
	pSynthParams->pFDetWindow =
		(float *) calloc(sizeHop * 2, sizeof(float));
	IFFTwindow (sizeHop * 2, pSynthParams->pFDetWindow);


        /* allocate memory for analysis data - size of original hopsize */
	AllocateSmsRecord (&pSynthParams->previousFrame, pSmsHeader->nTrajectories, 
	                   1 + pSmsHeader->nStochasticCoeff, 1,
                           pSynthParams->origSizeHop, pSmsHeader->iStochasticType);

        /* allocate memory for FFT - big enough for output buffer (new hopsize)*/
        int sizeFft = sizeHop << 1;
        printf("sizeof pComplexSpec: %d, sizeof pRealWave: %d \n\n", (sizeFft / 2 + 1), sizeFft);
        pSynthParams->pComplexSpec =  fftwf_malloc(sizeof(fftwf_complex) * (sizeFft / 2 + 1));
        pSynthParams->pRealWave = fftwf_malloc(sizeof(float) * sizeFft);
        if((pSynthParams->fftPlan =  
            fftwf_plan_dft_c2r_1d( sizeFft, pSynthParams->pComplexSpec,
                                   pSynthParams->pRealWave, FFTW_ESTIMATE)) == NULL)
        {
                printf("SmsInitSynth: could not make fftw plan \n");
                return -1;
        }
        
        /*debugging realft */
        pSynthParams->realftOut = (float *) calloc(sizeFft+1, sizeof(float));


        return 1;
}

int SmsFreeSynth( SYNTH_PARAMS *pSynthParams )
{

        free (pSynthParams->realftOut);
        fftwf_free(pSynthParams->pComplexSpec);
        fftwf_free(pSynthParams->pRealWave);
	fftwf_destroy_plan(pSynthParams->fftPlan);

        return 1;
}


/* fill the sound buffer
 *
 * short *pSWaveform           input data
 * int sizeNewData             size of input data
 * int sizeHop                 analysis hop size
 */
void FillBuffer (short *pSWaveform, long sizeNewData, ANAL_PARAMS analParams)
{
	extern SOUND_BUFFER soundBuffer;
	int i;
  
	/* leave space for new data */
	memcpy ((char *) soundBuffer.pFBuffer, 
	       (char *) (soundBuffer.pFBuffer+sizeNewData), 
	       sizeof(float) * (soundBuffer.sizeBuffer - sizeNewData));
  
	soundBuffer.iFirstSample = 
		MAX (0, soundBuffer.iFirstSample - sizeNewData);
	soundBuffer.iSoundSample += sizeNewData;   
  
	/* put the new data in, and do some pre-emphasis */
	if (analParams.iAnalysisDirection == REVERSE)
		for (i=0; i<sizeNewData; i++)
			soundBuffer.pFBuffer[soundBuffer.sizeBuffer - sizeNewData + i] = 
				PreEmphasis((float) pSWaveform[sizeNewData - (1+ i)]);
	else
		for (i=0; i<sizeNewData; i++)
			soundBuffer.pFBuffer[soundBuffer.sizeBuffer - sizeNewData + i] = 
				PreEmphasis((float) pSWaveform[i]);
}

/* shift the buffer of analysis frames to the left */
void MoveFrames ()
{
	extern short MaxDelayFrames;
	extern ANAL_FRAME **ppFrames;
	int i;
	ANAL_FRAME *tmp;
  
	/* shift the frame pointers */
	tmp = ppFrames[0];
	for(i = 1; i < MaxDelayFrames; i++)
		ppFrames[i-1] = ppFrames[i];
  
	ppFrames[MaxDelayFrames-1] = tmp;
}

/* initialize the current frame
 *
 * int iCurrentFrame;            frame number of current frame in buffer
 * ANAL_PARAMS analParams;       analysis parameters
 * int sizeWindow;                  size of analysis window 
 */
void InitializeFrame (int iCurrentFrame, ANAL_PARAMS analParams, 
                      int sizeWindow)
{
	extern SOUND_BUFFER soundBuffer;
	extern ANAL_FRAME **ppFrames;

	/* clear deterministic data */
	memset ((float *) ppFrames[iCurrentFrame]->deterministic.pFFreqTraj, 0, 
	        sizeof(float) * analParams.nGuides);
	memset ((float *) ppFrames[iCurrentFrame]->deterministic.pFMagTraj, 0, 
	        sizeof(float) * analParams.nGuides);
	memset ((float *) ppFrames[iCurrentFrame]->deterministic.pFPhaTraj, 0, 
	        sizeof(float) * analParams.nGuides);
	/* clear peaks */
	memset ((void *) ppFrames[iCurrentFrame]->pSpectralPeaks, 0,
	        sizeof (PEAK) * MAX_NUM_PEAKS);

	ppFrames[iCurrentFrame]->nPeaks = 0;
	ppFrames[iCurrentFrame]->fFundamental = 0;
  
	ppFrames[iCurrentFrame]->iFrameNum =  
		ppFrames[iCurrentFrame - 1]->iFrameNum + 1;
	ppFrames[iCurrentFrame]->iFrameSize =  sizeWindow;
  
	/* if first frame set center of data around 0 */
	if(ppFrames[iCurrentFrame]->iFrameNum == 1)
		ppFrames[iCurrentFrame]->iFrameSample = 0;
	/* increment center of data by sizeHop */
	else
		ppFrames[iCurrentFrame]->iFrameSample = 
			ppFrames[iCurrentFrame-1]->iFrameSample + analParams.sizeHop;
  	 
	/* check for error */
	if (soundBuffer.iSoundSample >
	         ppFrames[iCurrentFrame]->iFrameSample - (sizeWindow+1)/2)
	{
		fprintf(stderr, "error: runoff on the sound buffer\n");
		exit(1);
	} 
	/* check for end of sound */
	if ((ppFrames[iCurrentFrame]->iFrameSample + (sizeWindow+1)/2) >=
	    analParams.iSizeSound)
	{
		ppFrames[iCurrentFrame]->iFrameNum =  -1;
		ppFrames[iCurrentFrame]->iFrameSize =  0;
		ppFrames[iCurrentFrame]->iStatus =  END;
	}
	else
		/* good status, ready to start computing */
		ppFrames[iCurrentFrame]->iStatus = READY;
}

/* compute spectrum, find peaks, and fundamental of given frame
 *
 * int iCurrentFrame          frame number to be computed
 * ANAL_PARAMS analParams     analysis parameters
 * float fRefFundamental      reference fundamental 
 */
void ComputeFrame (int iCurrentFrame, ANAL_PARAMS analParams, 
                   float fRefFundamental)
{
	extern SOUND_BUFFER soundBuffer;
	extern ANAL_FRAME **ppFrames;

	static float pFMagSpectrum[MAX_SIZE_MAG];
	static float pFPhaSpectrum[MAX_SIZE_MAG];
	int sizeMag, i;
	int iSoundLoc = ppFrames[iCurrentFrame]->iFrameSample - 
		((ppFrames[iCurrentFrame]->iFrameSize + 1) >> 1) + 1;
	float *pFData = 
		&(soundBuffer.pFBuffer[iSoundLoc - soundBuffer.iSoundSample]);
  
	/* compute the magnitude and phase spectra */
	sizeMag = Spectrum(pFData, ppFrames[iCurrentFrame]->iFrameSize,
	                   pFMagSpectrum, pFPhaSpectrum, analParams);
  
	/* find the prominent peaks */
	ppFrames[iCurrentFrame]->nPeaks = 
		PeakDetection (pFMagSpectrum, pFPhaSpectrum, sizeMag, 
		               ppFrames[iCurrentFrame]->iFrameSize,
		               ppFrames[iCurrentFrame]->pSpectralPeaks,
		               analParams);
  
	if (analParams.iDebugMode == DEBUG_PEAK_DET || 
	    analParams.iDebugMode == DEBUG_ALL)
	{
		fprintf(stdout, "Frame %d peaks: ", 
		        ppFrames[iCurrentFrame]->iFrameNum);
		/* print only the first 10 peaks */
		for(i=0; i<10; i++)
			fprintf(stdout, " %.0f[%.1f], ", 
			        ppFrames[iCurrentFrame]->pSpectralPeaks[i].fFreq,
			        ppFrames[iCurrentFrame]->pSpectralPeaks[i].fMag);
		fprintf(stdout, "\n");
	}
  
	/* find a reference harmonic */
	if (ppFrames[iCurrentFrame]->nPeaks > 0 && 
	    (analParams.iFormat == FORMAT_HARMONIC ||
	    analParams.iFormat == FORMAT_HARMONIC_WITH_PHASE))
		HarmDetection (ppFrames[iCurrentFrame], fRefFundamental, analParams);
  
	if (analParams.iDebugMode == DEBUG_HARM_DET || 
	    analParams.iDebugMode == DEBUG_ALL)
		fprintf(stdout, "Frame %d: fundamental %f\n", 
		        ppFrames[iCurrentFrame]->iFrameNum,
		        ppFrames[iCurrentFrame]->fFundamental);
}

/* set window size for next frame 
 *
 * int iCurrentFrame;         number of current frame
 * ANAL_PARAMS analParams;    analysis parameters
 */
int SetSizeWindow (int iCurrentFrame, ANAL_PARAMS analParams)
{
	extern ANAL_FRAME **ppFrames;
	float fFund = ppFrames[iCurrentFrame]->fFundamental,
        fPrevFund = ppFrames[iCurrentFrame-1]->fFundamental;
	int sizeWindow;
  
	/* if the previous fundamental was stable use it to set the window size */
	if (fPrevFund > 0 &&
	    fabs(fPrevFund - fFund) / fFund <= .2)
		sizeWindow = (int) ((analParams.iSamplingRate / fFund) *
			analParams.fSizeWindow * .5) * 2 + 1;
	/* otherwise use the default size window */
	else
		sizeWindow = analParams.iDefaultSizeWindow;
  
	if (sizeWindow > MAX_SIZE_WINDOW)
	{
		fprintf (stderr, "sizeWindow (%d) too big, set to %d\n", sizeWindow, 
		         MAX_SIZE_WINDOW);
		sizeWindow = MAX_SIZE_WINDOW;
	}
  
	return (sizeWindow);
}

/* get deviation from average fundamental
 *
 * return -1 if really off
 * int iCurrentFrame;        number of current frame 
 */
float GetDeviation (int iCurrentFrame)
{
	extern ANAL_FRAME **ppFrames;
	float fFund, fSum = 0, fAverage, fDeviation = 0;
  int i;

	/* get the sum of the past few fundamentals */
	for (i = 0; i < MIN_GOOD_FRAMES; i++)
	{
		fFund = ppFrames[iCurrentFrame-i]->fFundamental;
		if(fFund <= 0)
			return(-1);
		else
			fSum += fFund;
	}
  
	/* find the average */
	fAverage = fSum / MIN_GOOD_FRAMES;
  
	/* get the deviation from the average */
	for (i = 0; i < MIN_GOOD_FRAMES; i++)
		fDeviation += fabs(ppFrames[iCurrentFrame-i]->fFundamental - fAverage);
  
	/* return the deviation from the average */
	return (fDeviation / (MIN_GOOD_FRAMES * fAverage));
}

/* re-analyze the previous frames if necessary  
 *
 * int iCurrentFrame;             current frame number
 * ANAL_PARAMS analParams;           analysis parameters
 */
int ReAnalyze (int iCurrentFrame, ANAL_PARAMS analParams)
{
	extern ANAL_FRAME **ppFrames;
	float fAvgDeviation = GetDeviation(iCurrentFrame),
		fFund, fLastFund, fDev;
	int iNewFrameSize, i,
		iFirstFrame = iCurrentFrame - MIN_GOOD_FRAMES;

	if (analParams.iDebugMode == DEBUG_SMS_ANAL || 
	    analParams.iDebugMode == DEBUG_ALL)
		fprintf(stdout, "Frame %d reAnalyze: Freq. deviation %f\n", 
		       ppFrames[iCurrentFrame]->iFrameNum, fAvgDeviation);
  
	if (fAvgDeviation == -1)
		return (-1);
  
	/* if the last MIN_GOOD_FRAMES are stable look before them */
	/*  and recompute the frames that are not stable           */
	if (fAvgDeviation <= MAX_DEVIATION)
		for (i = 0; i < ANAL_DELAY; i++)
		{
			if (ppFrames[iFirstFrame - i]->iFrameNum <= 0 ||
			    ppFrames[iFirstFrame - i]->iStatus == RECOMPUTED)
				return(-1);
			fFund = ppFrames[iFirstFrame - i]->fFundamental;
			fLastFund = ppFrames[iFirstFrame - i + 1]->fFundamental;
			fDev = fabs (fFund - fLastFund) / fLastFund;
			iNewFrameSize = ((analParams.iSamplingRate / fLastFund) *
				analParams.fSizeWindow/2) * 2 + 1;
	
			if (fFund <= 0 || fDev > .2 ||
			    fabs ((double)(ppFrames[iFirstFrame - i]->iFrameSize - 
			          iNewFrameSize)) / 
			    iNewFrameSize >= .2)
			{
				ppFrames[iFirstFrame - i]->iFrameSize = iNewFrameSize;
				ppFrames[iFirstFrame - i]->iStatus = READY;
	    
				if (analParams.iDebugMode == DEBUG_SMS_ANAL || 
				    analParams.iDebugMode == DEBUG_ALL)
					fprintf(stdout, "re-analyzing frame %d\n", 
					        ppFrames[iFirstFrame - i]->iFrameNum);
	    
				/* recompute frame */
				ComputeFrame (iFirstFrame - i, analParams, fLastFund);
				ppFrames[iFirstFrame - i]->iStatus = RECOMPUTED;
	    
				if (fabs(ppFrames[iFirstFrame - i]->fFundamental - fLastFund) / 
				    fLastFund >= .2)
				return(-1);
			}
		}
	return (1);
}
/* this should only be used in the case of a horrible problem.
   otherwise, keep sailing and let the parent app decide what 
   to do with an error code. */
int quit (char *pChText)
{
	fprintf (stderr, pChText);
	fprintf (stderr, "\n");
	exit (1);
	return (1);
}

