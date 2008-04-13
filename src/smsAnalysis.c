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

SOUND_BUFFER soundBuffer, synthBuffer;
ANAL_FRAME **ppFrames, *pFrames;
double *pFSTab;
extern short MaxDelayFrames;

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
                 ANAL_PARAMS analParams, long *pINextSizeRead)
{    


	static int sizeWindow = 0;      /* size of current analysis window */
	static SMS_DATA lastFrame;
	int iCurrentFrame = MaxDelayFrames - 1;  /* frame # of current frame */
	int iExtraSamples;              /* samples used for next analysis frame */
	float fRefFundamental = 0;   /* reference fundamental for current frame */

	/* initialize structures */
	if (ppFrames == NULL) 
	{
                Initialize (analParams);
                AllocateSmsRecord (&lastFrame, analParams.nGuides, pSmsData->nCoeff, 1,
                                   analParams.sizeHop, analParams.iStochasticType);
        }

	/* clear SMS output */
	ClearSmsRecord (pSmsData);
  
	/* set initial analysis-window size */
	if (sizeWindow == 0)
		sizeWindow = analParams.iDefaultSizeWindow;
  
	/* fill the input sound buffer and perform pre-emphasis */
	if (sizeNewData > 0)
		FillBuffer (pSWaveform, sizeNewData, analParams);
    
	/* move analysis data one frame back */
	MoveFrames ();

	/* initialize the current frame */
	InitializeFrame (iCurrentFrame, analParams, sizeWindow);
  
	/* if right data in the sound buffer do analysis */
	if (ppFrames[iCurrentFrame]->iStatus == READY)
	{
		float fAvgDev = GetDeviation(iCurrentFrame - 1);
      
		if (analParams.iDebugMode == DEBUG_SMS_ANAL || 
		    analParams.iDebugMode == DEBUG_ALL)
			fprintf (stdout, "Frame %d: sizeWindow %d, sizeNewData %ld, firstSampleBuffer %d, centerSample %d\n",
			         ppFrames[iCurrentFrame]->iFrameNum,
			         sizeWindow, sizeNewData, soundBuffer.iSoundSample,
		           ppFrames[iCurrentFrame]->iFrameSample);

		/* if single note use the default fundamental as reference */
		if (analParams.iSoundType == TYPE_SINGLE_NOTE)
			fRefFundamental = analParams.fDefaultFundamental;
		/* if sound is stable use the last fundamental as a reference */
		else if (fAvgDev != -1 && fAvgDev <= MAX_DEVIATION)
			fRefFundamental = ppFrames[iCurrentFrame - 1]->fFundamental;
		else
			fRefFundamental = 0;
      
		/* compute spectrum, find peaks, and find fundamental of frame */
		ComputeFrame (iCurrentFrame, analParams, fRefFundamental);
      
		/* set the size of the next analysis window */
		if (ppFrames[iCurrentFrame]->fFundamental > 0 &&
		    analParams.iSoundType != TYPE_SINGLE_NOTE)
			sizeWindow = SetSizeWindow (iCurrentFrame, analParams);
      
		/* figure out how much needs to be read next time */
		iExtraSamples = 
			(soundBuffer.iSoundSample + soundBuffer.sizeBuffer) -
			(ppFrames[iCurrentFrame]->iFrameSample + analParams.sizeHop);
		*pINextSizeRead = MAX (0, (sizeWindow+1)/2 - iExtraSamples);

		/* check again the previous frames and recompute if necessary */
		ReAnalyze (iCurrentFrame, analParams);
	}
  
	/* incorporate the peaks into the corresponding trajectories */
	/* This is done after a DELAY_FRAMES delay  */
	if (ppFrames[iCurrentFrame - DELAY_FRAMES]->fFundamental > 0 ||
	    ((analParams.iFormat == FORMAT_INHARMONIC ||
	      analParams.iFormat == FORMAT_INHARMONIC_WITH_PHASE) &&
	     ppFrames[iCurrentFrame - DELAY_FRAMES]->nPeaks > 0))
		PeakContinuation (iCurrentFrame - DELAY_FRAMES, analParams);
    
	/* fill gaps and delete short trajectories */
	if (analParams.iCleanTraj > 0 &&
	    ppFrames[iCurrentFrame - DELAY_FRAMES]->iStatus != EMPTY)
		CleanTrajectories (iCurrentFrame - DELAY_FRAMES, analParams);

	/* do stochastic analysis */
	if (analParams.iStochasticType != STOC_NONE)
	{
		/* synthesize deterministic signal */
		if (ppFrames[1]->iStatus != EMPTY &&
		    ppFrames[1]->iStatus != END)
		{
			/* allocate sine table */
			if (pFSTab == NULL)
				PrepSine(4096);
 
			/* shift synthesis buffer */
			memcpy ((char *) synthBuffer.pFBuffer, (char *) 
			        (synthBuffer.pFBuffer+analParams.sizeHop), 
			        sizeof(float) * analParams.sizeHop);
			memset ((char *) (synthBuffer.pFBuffer+analParams.sizeHop), 0,
			        sizeof(float) * analParams.sizeHop);
      
			/* get deterministic signal with phase  */
			FrameSineSynth (&ppFrames[1]->deterministic,
			                synthBuffer.pFBuffer+analParams.sizeHop,  
			                analParams.sizeHop, &lastFrame, 
			                analParams.iSamplingRate);
		}
  
		/* perform stochastic analysis after 1 frame of the     */
		/* deterministic synthesis because it needs two frames  */
		if (ppFrames[0]->iStatus != EMPTY &&
		    ppFrames[0]->iStatus != END)
		{
			int sizeResidual = analParams.sizeHop * 2;
			int iSoundLoc = ppFrames[0]->iFrameSample - analParams.sizeHop;
			float *pFData = &(soundBuffer.pFBuffer[iSoundLoc - 
			                                       soundBuffer.iSoundSample]);
			float *pFResidual;
			int sizeData = 
				MIN (soundBuffer.sizeBuffer - 
				      (iSoundLoc - soundBuffer.iSoundSample),
				     sizeResidual);
    
			if ((pFResidual = (float *) calloc (sizeResidual, sizeof(float))) 
			    == NULL)
				return -1;

			/* obtain residual sound from original and synthesized sounds */
			GetResidual (synthBuffer.pFBuffer, pFData, pFResidual, sizeData, 
			             analParams);
                         
                        //::::::::::::::::::::: RTE_DEBUG::::::::::::::::::
                        int i;
                        printf("\n:::::::::: pFData:  ::::::::::: sizeData: %d, sizeHop %d ::::::::::::\n",
                               sizeData, analParams.sizeHop);
                        for(i = 0; i < analParams.sizeHop; i++)
                                //printf("%.3f  ", pFResidual[i]);
                                printf("%d  ", (short) pFData[i]);
                        printf("\n");
                        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

                        /* approximate residual */
			StocAnalysis (pFResidual, sizeData, pSmsData, analParams);
                        
                        //::::::::::::::::::::: RTE_DEBUG::::::::::::::::::
/*                         printf("\n:::::::::: smsAnalysis: AFTER StochAnalysis::::::::::::::::::::\n"); */
/*                         for(i = 0; i < analParams.sizeHop; i++) */
/*                                 printf("%.3f ", pSmsData->pFStocAudio[i]); */
/* 			printf("\n"); */
                        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
                        
			/* get sharper transitions in deterministic representation */
			ScaleDeterministic (synthBuffer.pFBuffer, pFData, 
			                    ppFrames[0]->deterministic.pFMagTraj,
			                    analParams, pSmsData->nTraj);
      
			ppFrames[0]->iStatus = DONE;

			free ((char *) pFResidual);
		}
	}
	else if (ppFrames[0]->iStatus != EMPTY &&
	         ppFrames[0]->iStatus != END)
		ppFrames[0]->iStatus = DONE;

	/* get the result */
	if (ppFrames[0]->iStatus == EMPTY)
		return (0);
	/* return analysis data */
	else if (ppFrames[0]->iStatus == DONE)
	{
		/* put data into output */
		int length = sizeof(float) * pSmsData->nTraj;
		memcpy ((char *) pSmsData->pFFreqTraj, (char *) 
		        ppFrames[0]->deterministic.pFFreqTraj, length);
		memcpy ((char *) pSmsData->pFMagTraj, (char *) 	
		         ppFrames[0]->deterministic.pFMagTraj, length);
		if (analParams.iFormat == FORMAT_HARMONIC_WITH_PHASE ||
		    analParams.iFormat == FORMAT_INHARMONIC_WITH_PHASE)
			memcpy ((char *) pSmsData->pFPhaTraj, (char *) 	
			        ppFrames[0]->deterministic.pFPhaTraj, length);
		return (1);
	}
	/* done, end of sound */
	else if (ppFrames[0]->iStatus == END)
		return (-1);
	else
	{
		fprintf (stderr, "error: wrong status of frame\n");
		exit (1);
	}
	return (1);
}
