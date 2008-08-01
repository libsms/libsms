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


float *sms_window_spec;
float  *sms_tab_sine, *sms_tab_sinc;

int SmsInit( void )
{
	/* prepare sinc and sine tables only the first time */
	if (sms_tab_sine == NULL) PrepSine (2046); //try 4096
	if (sms_tab_sinc == NULL) PrepSinc ();

        return (1);
}


/* initialize analysis data structure
 * - there can be multple SMS_AnalParams at the same time
 *
 * SMS_AnalParams analParams;    analysis paramaters
 *
 */
int SmsInitAnalysis ( SMS_Header *pSmsHeader, SMS_AnalParams *pAnalParams)
{

        SMS_SndBuffer *pSynthBuf = &pAnalParams->synthBuffer;
        //todo: add a pSoundBuf to simplify things

	int sizeBuffer = (pAnalParams->iMaxDelayFrames * pAnalParams->sizeHop) + SMS_MAX_WINDOW;
	int i;

        AllocateSmsRecord (&pAnalParams->prevFrame, pAnalParams->nGuides, 
                           pSmsHeader->nStochasticCoeff, 1, pAnalParams->sizeHop, pAnalParams->iStochasticType);
  
	/* sound buffer */
	if ((pAnalParams->soundBuffer.pFBuffer = (float *) calloc(sizeBuffer, sizeof(float)))
	    == NULL)
		return -1;
	pAnalParams->soundBuffer.iMarker = -sizeBuffer;
	pAnalParams->soundBuffer.iFirstGood = sizeBuffer;
	pAnalParams->soundBuffer.sizeBuffer = sizeBuffer;
  
	/* deterministic synthesis buffer */

	pSynthBuf->sizeBuffer = pAnalParams->sizeHop << 1;
	if ((pSynthBuf->pFBuffer = 
	      (float *) calloc(pSynthBuf->sizeBuffer, sizeof(float))) == NULL)
		return -1;
	pSynthBuf->iMarker = -sizeBuffer;
	pSynthBuf->iMarker = pSynthBuf->sizeBuffer;
  
	/* buffer of analysis frames */
	if ((pAnalParams->pFrames = (SMS_AnalFrame *) calloc(pAnalParams->iMaxDelayFrames, sizeof(SMS_AnalFrame))) 
	    == NULL)
		return -1;
	if ((pAnalParams->ppFrames = 
	     (SMS_AnalFrame **) calloc(pAnalParams->iMaxDelayFrames, sizeof(SMS_AnalFrame *)))
	     == NULL)
		return -1;
  
	/* initialize the frame pointers and allocate memory */
	for (i = 0; i < pAnalParams->iMaxDelayFrames; i++)
	{
		pAnalParams->pFrames[i].iStatus = SMS_FRAME_EMPTY;
		(pAnalParams->pFrames[i].deterministic).nTraj = pAnalParams->nGuides;
		if (((pAnalParams->pFrames[i].deterministic).pFFreqTraj =
		    (float *)calloc (pAnalParams->nGuides, sizeof(float))) == NULL)
			return -1;
		if (((pAnalParams->pFrames[i].deterministic).pFMagTraj =
		    (float *)calloc (pAnalParams->nGuides, sizeof(float))) == NULL)
			return -1;
		if (((pAnalParams->pFrames[i].deterministic).pFPhaTraj =
		    (float *) calloc (pAnalParams->nGuides, sizeof(float))) == NULL)
			return -1;
		pAnalParams->ppFrames[i] = &pAnalParams->pFrames[i];
	}

	/* initialitze window buffer for spectrum */
	if(!sms_window_spec)
                sms_window_spec = (float *) calloc (SMS_MAX_WINDOW, sizeof(float));

        /* allocate memory for FFT */
//        sizeFFT = SMS_MAX_WINDOW;

#ifdef FFTW
        pAnalParams->pWaveform = fftwf_malloc(sizeof(float) * SMS_MAX_WINDOW);
        pAnalParams->pSpectrum = fftwf_malloc(sizeof(fftwf_complex) * (SMS_MAX_WINDOW / 2 + 1));
#endif

/*         if((pAnalParams->fftPlan =  fftwf_plan_dft_r2c_1d( sizeFFT, pAnalParams->pWaveform, */
/*                                                            pAnalParams->pSpectrum, FFTW_ESTIMATE)) == NULL) */

/*         { */
/*                 printf("SmsInitAnalysis: could not make fftw plan \n"); */
/*                 return -1; */
/*         } */

//        fftwf_print_plan(pAnalParams->fftPlan);


	return (1);
}

int SmsInitSynth( SMS_Header *pSmsHeader, SMS_SynthParams *pSynthParams )
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
	AllocateSmsRecord (&pSynthParams->prevFrame, pSmsHeader->nTrajectories, 
	                   1 + pSmsHeader->nStochasticCoeff, 1,
                           pSynthParams->origSizeHop, pSmsHeader->iStochasticType);

        /* allocate memory for FFT - big enough for output buffer (new hopsize)*/
        int sizeFft = sizeHop << 1;
#ifdef FFTW
        pSynthParams->pSpectrum =  fftwf_malloc(sizeof(fftwf_complex) * (sizeFft / 2 + 1));
        pSynthParams->pWaveform = fftwf_malloc(sizeof(float) * sizeFft);
        if((pSynthParams->fftPlan =
            fftwf_plan_dft_c2r_1d( sizeFft, pSynthParams->pSpectrum,
                                   pSynthParams->pWaveform, FFTW_ESTIMATE)) == NULL)
        {
                printf("SmsInitSynth: could not make fftw plan \n");
                return -1;
        }
#else        
        /*debugging realft */
        pSynthParams->realftOut = (float *) calloc(sizeFft+1, sizeof(float));
#endif

        return 1;
}

int SmsFreeAnalysis( SMS_AnalParams *pAnalParams )
{
#ifdef FFTW
        fftwf_free(pAnalParams->pWaveform);
        fftwf_free(pAnalParams->pSpectrum);

        fftwf_destroy_plan(pAnalParams->fftPlan);
#endif
        return 1;
}


int SmsFreeSynth( SMS_SynthParams *pSynthParams )
{

#ifdef FFTW
        fftwf_free(pSynthParams->pSpectrum);
        fftwf_free(pSynthParams->pWaveform);
	fftwf_destroy_plan(pSynthParams->fftPlan);
#else
        free (pSynthParams->realftOut);
#endif
        return 1;
}


/* fill the sound buffer
 *
 * short *pSWaveform           input data
 * int sizeNewData             size of input data
 * int sizeHop                 analysis hop size
 */
void FillBuffer (short *pSWaveform, long sizeNewData, SMS_AnalParams *pAnalParams)
{
//	extern SMS_SndBuffer soundBuffer;
	int i;
  
	/* leave space for new data */
	memcpy ( pAnalParams->soundBuffer.pFBuffer,  pAnalParams->soundBuffer.pFBuffer+sizeNewData, 
                 sizeof(float) * (pAnalParams->soundBuffer.sizeBuffer - sizeNewData));
  
	pAnalParams->soundBuffer.iFirstGood = 
		MAX (0, pAnalParams->soundBuffer.iFirstGood - sizeNewData);
	pAnalParams->soundBuffer.iMarker += sizeNewData;   
  
	/* put the new data in, and do some pre-emphasis */
	if (pAnalParams->iAnalysisDirection == SMS_DIR_REV)
		for (i=0; i<sizeNewData; i++)
			pAnalParams->soundBuffer.pFBuffer[pAnalParams->soundBuffer.sizeBuffer - sizeNewData + i] = 
				PreEmphasis((float) pSWaveform[sizeNewData - (1+ i)]);
	else
		for (i=0; i<sizeNewData; i++)
			pAnalParams->soundBuffer.pFBuffer[pAnalParams->soundBuffer.sizeBuffer - sizeNewData + i] = 
				PreEmphasis((float) pSWaveform[i]);
}

/* shift the buffer of analysis frames to the left */
void MoveFrames (SMS_AnalParams *pAnalParams)
{
//	extern short MaxDelayFrames;
//	extern SMS_AnalFrame **ppFrames;
	int i;
	SMS_AnalFrame *tmp;
  
	/* shift the frame pointers */
	tmp = pAnalParams->ppFrames[0];
	for(i = 1; i < pAnalParams->iMaxDelayFrames; i++)
		pAnalParams->ppFrames[i-1] = pAnalParams->ppFrames[i];
  
	pAnalParams->ppFrames[pAnalParams->iMaxDelayFrames-1] = tmp;
}

/* initialize the current frame
 *
 * int iCurrentFrame;            frame number of current frame in buffer
 * SMS_AnalParams analParams;       analysis parameters
 * int sizeWindow;                  size of analysis window 
 */
void InitializeFrame (int iCurrentFrame, SMS_AnalParams *pAnalParams, 
                      int sizeWindow)
{
//	extern SMS_SndBuffer soundBuffer;
//	extern SMS_AnalFrame **ppFrames;

	/* clear deterministic data */
	memset ((float *) pAnalParams->ppFrames[iCurrentFrame]->deterministic.pFFreqTraj, 0, 
	        sizeof(float) * pAnalParams->nGuides);
	memset ((float *) pAnalParams->ppFrames[iCurrentFrame]->deterministic.pFMagTraj, 0, 
	        sizeof(float) * pAnalParams->nGuides);
	memset ((float *) pAnalParams->ppFrames[iCurrentFrame]->deterministic.pFPhaTraj, 0, 
	        sizeof(float) * pAnalParams->nGuides);
	/* clear peaks */
	memset ((void *) pAnalParams->ppFrames[iCurrentFrame]->pSpectralPeaks, 0,
	        sizeof (SMS_Peak) * SMS_MAX_NPEAKS);

	pAnalParams->ppFrames[iCurrentFrame]->nPeaks = 0;
	pAnalParams->ppFrames[iCurrentFrame]->fFundamental = 0;
  
	pAnalParams->ppFrames[iCurrentFrame]->iFrameNum =  
		pAnalParams->ppFrames[iCurrentFrame - 1]->iFrameNum + 1;
	pAnalParams->ppFrames[iCurrentFrame]->iFrameSize =  sizeWindow;
  
	/* if first frame set center of data around 0 */
	if(pAnalParams->ppFrames[iCurrentFrame]->iFrameNum == 1)
		pAnalParams->ppFrames[iCurrentFrame]->iFrameSample = 0;
	/* increment center of data by sizeHop */
	else
		pAnalParams->ppFrames[iCurrentFrame]->iFrameSample = 
			pAnalParams->ppFrames[iCurrentFrame-1]->iFrameSample + pAnalParams->sizeHop;
  	 
	/* check for error */
	if (pAnalParams->soundBuffer.iMarker >
	         pAnalParams->ppFrames[iCurrentFrame]->iFrameSample - (sizeWindow+1)/2)
	{
		fprintf(stderr, "error: runoff on the sound buffer\n");
		exit(1);
	} 
	/* check for end of sound */
	if ((pAnalParams->ppFrames[iCurrentFrame]->iFrameSample + (sizeWindow+1)/2) >=
	    pAnalParams->iSizeSound)
	{
		pAnalParams->ppFrames[iCurrentFrame]->iFrameNum =  -1;
		pAnalParams->ppFrames[iCurrentFrame]->iFrameSize =  0;
		pAnalParams->ppFrames[iCurrentFrame]->iStatus =  SMS_FRAME_END;
	}
	else
		/* good status, ready to start computing */
		pAnalParams->ppFrames[iCurrentFrame]->iStatus = SMS_FRAME_READY;
}


/* set window size for next frame 
 *
 * int iCurrentFrame;         number of current frame
 * SMS_AnalParams analParams;    analysis parameters
 */
int SetSizeWindow (int iCurrentFrame, SMS_AnalParams *pAnalParams)
{
//	extern SMS_AnalFrame **ppFrames;
	float fFund = pAnalParams->ppFrames[iCurrentFrame]->fFundamental,
        fPrevFund = pAnalParams->ppFrames[iCurrentFrame-1]->fFundamental;
	int sizeWindow;
  
	/* if the previous fundamental was stable use it to set the window size */
	if (fPrevFund > 0 &&
	    fabs(fPrevFund - fFund) / fFund <= .2)
		sizeWindow = (int) ((pAnalParams->iSamplingRate / fFund) *
			pAnalParams->fSizeWindow * .5) * 2 + 1;
	/* otherwise use the default size window */
	else
		sizeWindow = pAnalParams->iDefaultSizeWindow;
  
	if (sizeWindow > SMS_MAX_WINDOW)
	{
		fprintf (stderr, "sizeWindow (%d) too big, set to %d\n", sizeWindow, 
		         SMS_MAX_WINDOW);
		sizeWindow = SMS_MAX_WINDOW;
	}
  
	return (sizeWindow);
}

/* get deviation from average fundamental
 *
 * return -1 if really off
 * int iCurrentFrame;        number of current frame 
 */
float GetDeviation ( SMS_AnalParams *pAnalParams, int iCurrentFrame)
{
//	extern SMS_AnalFrame **ppFrames;
	float fFund, fSum = 0, fAverage, fDeviation = 0;
  int i;

	/* get the sum of the past few fundamentals */
	for (i = 0; i < SMS_MIN_GOOD_FRAMES; i++)
	{
		fFund = pAnalParams->ppFrames[iCurrentFrame-i]->fFundamental;
		if(fFund <= 0)
			return(-1);
		else
			fSum += fFund;
	}
  
	/* find the average */
	fAverage = fSum / SMS_MIN_GOOD_FRAMES;
  
	/* get the deviation from the average */
	for (i = 0; i < SMS_MIN_GOOD_FRAMES; i++)
		fDeviation += fabs(pAnalParams->ppFrames[iCurrentFrame-i]->fFundamental - fAverage);
  
	/* return the deviation from the average */
	return (fDeviation / (SMS_MIN_GOOD_FRAMES * fAverage));
}

/* re-analyze the previous frames if necessary  
 *
 * int iCurrentFrame;             current frame number
 * SMS_AnalParams analParams;           analysis parameters
 */
int ReAnalyze (int iCurrentFrame, SMS_AnalParams *pAnalParams)
{
//	extern SMS_AnalFrame **ppFrames;
	float fAvgDeviation = GetDeviation(pAnalParams, iCurrentFrame),
		fFund, fLastFund, fDev;
	int iNewFrameSize, i,
		iFirstFrame = iCurrentFrame - SMS_MIN_GOOD_FRAMES;

	if (pAnalParams->iDebugMode == SMS_DBG_SMS_ANAL || 
	    pAnalParams->iDebugMode == SMS_DBG_ALL)
		fprintf(stdout, "Frame %d reAnalyze: Freq. deviation %f\n", 
		       pAnalParams->ppFrames[iCurrentFrame]->iFrameNum, fAvgDeviation);
  
	if (fAvgDeviation == -1)
		return (-1);
  
	/* if the last SMS_MIN_GOOD_FRAMES are stable look before them */
	/*  and recompute the frames that are not stable           */
	if (fAvgDeviation <= SMS_MAX_DEVIATION)
		for (i = 0; i < SMS_ANAL_DELAY; i++)
		{
			if (pAnalParams->ppFrames[iFirstFrame - i]->iFrameNum <= 0 ||
			    pAnalParams->ppFrames[iFirstFrame - i]->iStatus == SMS_FRAME_RECOMPUTED)
				return(-1);
			fFund = pAnalParams->ppFrames[iFirstFrame - i]->fFundamental;
			fLastFund = pAnalParams->ppFrames[iFirstFrame - i + 1]->fFundamental;
			fDev = fabs (fFund - fLastFund) / fLastFund;
			iNewFrameSize = ((pAnalParams->iSamplingRate / fLastFund) *
				pAnalParams->fSizeWindow/2) * 2 + 1;
	
			if (fFund <= 0 || fDev > .2 ||
			    fabs ((double)(pAnalParams->ppFrames[iFirstFrame - i]->iFrameSize - 
			          iNewFrameSize)) / 
			    iNewFrameSize >= .2)
			{
				pAnalParams->ppFrames[iFirstFrame - i]->iFrameSize = iNewFrameSize;
				pAnalParams->ppFrames[iFirstFrame - i]->iStatus = SMS_FRAME_READY;
	    
				if (pAnalParams->iDebugMode == SMS_DBG_SMS_ANAL || 
				    pAnalParams->iDebugMode == SMS_DBG_ALL)
					fprintf(stdout, "re-analyzing frame %d\n", 
					        pAnalParams->ppFrames[iFirstFrame - i]->iFrameNum);
	    
				/* recompute frame */
				ComputeFrame (iFirstFrame - i, pAnalParams, fLastFund);
				pAnalParams->ppFrames[iFirstFrame - i]->iStatus = SMS_FRAME_RECOMPUTED;
	    
				if (fabs(pAnalParams->ppFrames[iFirstFrame - i]->fFundamental - fLastFund) / 
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

