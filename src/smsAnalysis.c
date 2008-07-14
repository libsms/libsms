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

/* global variables */

//SOUND_BUFFER soundBuffer;// synthBuffer;
//ANAL_FRAME **ppFrames, *pFrames;
//double *pFSTab;
//extern short MaxDelayFrames;

/* 
 * function to perform the SMS analysis on a single frame 
 * the input is a section of the sound, the output is the SMS data
 *
 * short *pSWaveform;	     input data
 * long sizeNewData;	     the size of input data
 * SMS_DATA *pSmsData;     output SMS data
 * ANAL_PARAMS analParams; analysis parameters
 * long *pINextSizeRead;   size of next data to read
 *
 */

int SmsAnalysis (short *pSWaveform, long sizeNewData, SMS_DATA *pSmsData, 
                 ANAL_PARAMS *pAnalParams, long *pINextSizeRead)
{    

//        SOUND_BUFFER *pSynthBuf = &analParams.synthBuffer;
	static int sizeWindow = 0;      /* size of current analysis window */ //RTE ?: shouldn't this just be initilalized outside?

	int iCurrentFrame = pAnalParams->iMaxDelayFrames - 1;  /* frame # of current frame */
	int iExtraSamples;              /* samples used for next analysis frame */
	float fRefFundamental = 0;   /* reference fundamental for current frame */

	/* clear SMS output */
	ClearSmsRecord (pSmsData);
  
	/* set initial analysis-window size */
	if (sizeWindow == 0)
		sizeWindow = pAnalParams->iDefaultSizeWindow;
  
	/* fill the input sound buffer and perform pre-emphasis */
	if (sizeNewData > 0)
		FillBuffer (pSWaveform, sizeNewData, pAnalParams);
    
	/* move analysis data one frame back */
	MoveFrames ( pAnalParams);

	/* initialize the current frame */
	InitializeFrame (iCurrentFrame, pAnalParams, sizeWindow);
  
	/* if right data in the sound buffer do analysis */
	if (pAnalParams->ppFrames[iCurrentFrame]->iStatus == READY)
	{
		float fAvgDev = GetDeviation( pAnalParams, iCurrentFrame - 1);
      
		if (pAnalParams->iDebugMode == DEBUG_SMS_ANAL || 
		    pAnalParams->iDebugMode == DEBUG_ALL)
			fprintf (stdout, "Frame %d: sizeWindow %d, sizeNewData %ld, firstSampleBuffer %d, centerSample %d\n",
			         pAnalParams->ppFrames[iCurrentFrame]->iFrameNum,
			         sizeWindow, sizeNewData, pAnalParams->soundBuffer.iSoundSample,
		           pAnalParams->ppFrames[iCurrentFrame]->iFrameSample);

		/* if single note use the default fundamental as reference */
		if (pAnalParams->iSoundType == TYPE_SINGLE_NOTE)
			fRefFundamental = pAnalParams->fDefaultFundamental;
		/* if sound is stable use the last fundamental as a reference */
		else if (fAvgDev != -1 && fAvgDev <= MAX_DEVIATION)
			fRefFundamental = pAnalParams->ppFrames[iCurrentFrame - 1]->fFundamental;
		else
			fRefFundamental = 0;
      
		/* compute spectrum, find peaks, and find fundamental of frame */
		ComputeFrame (iCurrentFrame, pAnalParams, fRefFundamental);
      
		/* set the size of the next analysis window */
		if (pAnalParams->ppFrames[iCurrentFrame]->fFundamental > 0 &&
		    pAnalParams->iSoundType != TYPE_SINGLE_NOTE)
			sizeWindow = SetSizeWindow (iCurrentFrame, pAnalParams);
      
		/* figure out how much needs to be read next time */
		iExtraSamples = 
			(pAnalParams->soundBuffer.iSoundSample + pAnalParams->soundBuffer.sizeBuffer) -
			(pAnalParams->ppFrames[iCurrentFrame]->iFrameSample + pAnalParams->sizeHop);
		*pINextSizeRead = MAX (0, (sizeWindow+1)/2 - iExtraSamples);

		/* check again the previous frames and recompute if necessary */
		ReAnalyze (iCurrentFrame, pAnalParams);
	}
  
	/* incorporate the peaks into the corresponding trajectories */
	/* This is done after a DELAY_FRAMES delay  */
	if (pAnalParams->ppFrames[iCurrentFrame - DELAY_FRAMES]->fFundamental > 0 ||
	    ((pAnalParams->iFormat == FORMAT_INHARMONIC ||
	      pAnalParams->iFormat == FORMAT_INHARMONIC_WITH_PHASE) &&
	     pAnalParams->ppFrames[iCurrentFrame - DELAY_FRAMES]->nPeaks > 0))
		PeakContinuation (iCurrentFrame - DELAY_FRAMES, pAnalParams);
    
	/* fill gaps and delete short trajectories */
	if (pAnalParams->iCleanTraj > 0 &&
	    pAnalParams->ppFrames[iCurrentFrame - DELAY_FRAMES]->iStatus != EMPTY)
		CleanTrajectories (iCurrentFrame - DELAY_FRAMES, pAnalParams);

	/* do stochastic analysis */
	if (pAnalParams->iStochasticType != STOC_NONE)
	{
		/* synthesize deterministic signal */
		if (pAnalParams->ppFrames[1]->iStatus != EMPTY &&
		    pAnalParams->ppFrames[1]->iStatus != END)
		{
			/* shift synthesis buffer */
			memcpy ( pAnalParams->synthBuffer.pFBuffer, 
                                 pAnalParams->synthBuffer.pFBuffer+pAnalParams->sizeHop, 
			        sizeof(float) * pAnalParams->sizeHop);
			memset (pAnalParams->synthBuffer.pFBuffer+pAnalParams->sizeHop,
                                0, sizeof(float) * pAnalParams->sizeHop);
      
			/* get deterministic signal with phase  */
			FrameSineSynth (&pAnalParams->ppFrames[1]->deterministic,
			                pAnalParams->synthBuffer.pFBuffer+pAnalParams->sizeHop,  
			                pAnalParams->sizeHop, &pAnalParams->prevFrame, 
			                pAnalParams->iSamplingRate);
		}
  
		/* perform stochastic analysis after 1 frame of the     */
		/* deterministic synthesis because it needs two frames  */
		if (pAnalParams->ppFrames[0]->iStatus != EMPTY &&
		    pAnalParams->ppFrames[0]->iStatus != END)
		{
			int sizeResidual = pAnalParams->sizeHop * 2;
			int iSoundLoc = pAnalParams->ppFrames[0]->iFrameSample - pAnalParams->sizeHop;
			float *pFData = &(pAnalParams->soundBuffer.pFBuffer[iSoundLoc - 
			                                       pAnalParams->soundBuffer.iSoundSample]);
			float *pFResidual;
			int sizeData = 
				MIN (pAnalParams->soundBuffer.sizeBuffer - 
				      (iSoundLoc - pAnalParams->soundBuffer.iSoundSample),
				     sizeResidual);
			if ((pFResidual = (float *) calloc (sizeResidual, sizeof(float))) 
			    == NULL)
			{
                                printf("SmsAnalysis: error allocating memory for pFResidual \n");
                                return -1;
                        }
			/* obtain residual sound from original and synthesized sounds */
			GetResidual (pAnalParams->synthBuffer.pFBuffer, pFData, pFResidual, sizeData, 
			             pAnalParams);
                         
                        //::::::::::::::::::::: RTE_DEBUG::::::::::::::::::
/*                          int i; */
/*                         printf("\n:::::::::: pFData (before  StocAnalysis):  sizeHop: %d, sizeData: %d, frame# %d ::::::::::::\n", */
/*                                pAnalParams->sizeHop, sizeData, pAnalParams->ppFrames[iCurrentFrame]->iFrameNum ); */
/*                         for(i = 0; i < pAnalParams->sizeHop; i++) */
/*                                 printf("%f  ", pFResidual[i]); */
/* //                                printf("%d  ", (short) pFData[i]); */
/*                         printf("\n"); */
                        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

                        /* approximate residual */
			StocAnalysis (pFResidual, sizeData, pSmsData, pAnalParams);
                        
                        //::::::::::::::::::::: RTE_DEBUG::::::::::::::::::

/*                         printf("\n:::::0000000000000::::: smsAnalysis: AFTER StochAnalysis::::::::::::::::::::\n"); */
/*                         for(i = 0; i < pAnalParams->sizeHop; i++) */
/*                                 printf("%.3f ", pSmsData->pFStocWave[i]); */
/* 			printf("\n"); */
                        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
                        
			/* get sharper transitions in deterministic representation */
			ScaleDeterministic (pAnalParams->synthBuffer.pFBuffer, pFData, 
			                    pAnalParams->ppFrames[0]->deterministic.pFMagTraj,
			                    pAnalParams, pSmsData->nTraj);
      
			pAnalParams->ppFrames[0]->iStatus = DONE;

			free ((char *) pFResidual);
		}
	}
	else if (pAnalParams->ppFrames[0]->iStatus != EMPTY &&
	         pAnalParams->ppFrames[0]->iStatus != END)
		pAnalParams->ppFrames[0]->iStatus = DONE;

	/* get the result */
	if (pAnalParams->ppFrames[0]->iStatus == EMPTY)
        {
//                printf("returning EMPTY \n");
		return (0);
        }
	/* return analysis data */
	else if (pAnalParams->ppFrames[0]->iStatus == DONE)
	{
		/* put data into output */
		int length = sizeof(float) * pSmsData->nTraj;
		memcpy ((char *) pSmsData->pFFreqTraj, (char *) 
		        pAnalParams->ppFrames[0]->deterministic.pFFreqTraj, length);
		memcpy ((char *) pSmsData->pFMagTraj, (char *) 	
		         pAnalParams->ppFrames[0]->deterministic.pFMagTraj, length);
		if (pAnalParams->iFormat == FORMAT_HARMONIC_WITH_PHASE ||
		    pAnalParams->iFormat == FORMAT_INHARMONIC_WITH_PHASE)
			memcpy ((char *) pSmsData->pFPhaTraj, (char *) 	
			        pAnalParams->ppFrames[0]->deterministic.pFPhaTraj, length);
		return (1);
	}
	/* done, end of sound */
	else if (pAnalParams->ppFrames[0]->iStatus == END)
		return (-1);
	else
	{
		fprintf (stderr, "error: wrong status of frame\n");
		exit (1);
	}
	return (1);
}

/* compute spectrum, find peaks, and fundamental of given frame
 *
 * int iCurrentFrame          frame number to be computed
 * ANAL_PARAMS analParams     analysis parameters
 * float fRefFundamental      reference fundamental 
 */
void ComputeFrame (int iCurrentFrame, ANAL_PARAMS *pAnalParams, 
                   float fRefFundamental)
{
//	extern SOUND_BUFFER soundBuffer;
//	extern ANAL_FRAME **ppFrames;

	static float pFMagSpectrum[MAX_SIZE_MAG];
	static float pFPhaSpectrum[MAX_SIZE_MAG];
	int sizeMag, i;
	int iSoundLoc = pAnalParams->ppFrames[iCurrentFrame]->iFrameSample - 
		((pAnalParams->ppFrames[iCurrentFrame]->iFrameSize + 1) >> 1) + 1;
	float *pFData = 
		&(pAnalParams->soundBuffer.pFBuffer[iSoundLoc - pAnalParams->soundBuffer.iSoundSample]);
  
	/* compute the magnitude and phase spectra */
	sizeMag = Spectrum(pFData, pAnalParams->ppFrames[iCurrentFrame]->iFrameSize,
	                   pFMagSpectrum, pFPhaSpectrum, pAnalParams);



	/* find the prominent peaks */
	pAnalParams->ppFrames[iCurrentFrame]->nPeaks = 
		PeakDetection (pFMagSpectrum, pFPhaSpectrum, sizeMag, 
		               pAnalParams->ppFrames[iCurrentFrame]->iFrameSize,
		               pAnalParams->ppFrames[iCurrentFrame]->pSpectralPeaks,
		               pAnalParams);

        /// RTE DEBUG //////////////
//        printf(" # %d , nPeaks: %d \n",iCurrentFrame, pAnalParams->ppFrames[iCurrentFrame]->nPeaks);
        ///////////////////////////////////////

  
	if (pAnalParams->iDebugMode == DEBUG_PEAK_DET || 
	    pAnalParams->iDebugMode == DEBUG_ALL)
	{
		fprintf(stdout, "Frame %d peaks: ", 
		        pAnalParams->ppFrames[iCurrentFrame]->iFrameNum);
		/* print only the first 10 peaks */
		for(i=0; i<10; i++)
			fprintf(stdout, " %.0f[%.1f], ", 
			        pAnalParams->ppFrames[iCurrentFrame]->pSpectralPeaks[i].fFreq,
			        pAnalParams->ppFrames[iCurrentFrame]->pSpectralPeaks[i].fMag);
		fprintf(stdout, "\n");
	}
  
	/* find a reference harmonic */
	if (pAnalParams->ppFrames[iCurrentFrame]->nPeaks > 0 && 
	    (pAnalParams->iFormat == FORMAT_HARMONIC ||
	    pAnalParams->iFormat == FORMAT_HARMONIC_WITH_PHASE))
		HarmDetection (pAnalParams->ppFrames[iCurrentFrame], fRefFundamental, pAnalParams);
  
	if (pAnalParams->iDebugMode == DEBUG_HARM_DET || 
	    pAnalParams->iDebugMode == DEBUG_ALL)
		fprintf(stdout, "Frame %d: fundamental %f\n", 
		        pAnalParams->ppFrames[iCurrentFrame]->iFrameNum,
		        pAnalParams->ppFrames[iCurrentFrame]->fFundamental);
}
