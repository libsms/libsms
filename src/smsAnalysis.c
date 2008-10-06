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
/*! \file smsAnalysis.c
 * \brief main sms analysis routine
 * 
 * the analysis routine here calls all necessary functions to perform the complete
 * SMS analysis, once the desired analysis parameters are set in SMS_AnalParams. 
 */
#include "sms.h"

/*! \brief  maximum size for magnitude spectrum */
#define SMS_MAX_SPEC 8192  

/*! \brief compute spectrum, find peaks, and fundamental of given frame
 *
 * This is the main core of analysis calls
 * \todo write a clearer description of why this is seperated
 *
 * \param iCurrentFrame          frame number to be computed
 * \param pAnalParams     structure of analysis parameters
 * \param fRefFundamental      reference fundamental 
 */
static void AnalyzeFrame (int iCurrentFrame, SMS_AnalParams *pAnalParams, 
                   float fRefFundamental)
{
	static float pFMagSpectrum[SMS_MAX_SPEC];
	static float pFPhaSpectrum[SMS_MAX_SPEC];
	int sizeMag, i;
	int iSoundLoc = pAnalParams->ppFrames[iCurrentFrame]->iFrameSample - 
		((pAnalParams->ppFrames[iCurrentFrame]->iFrameSize + 1) >> 1) + 1;
	float *pFData = 
		&(pAnalParams->soundBuffer.pFBuffer[iSoundLoc - pAnalParams->soundBuffer.iMarker]);
  
	/* compute the magnitude and phase spectra */
	sizeMag = sms_spectrum(pFData, pAnalParams->ppFrames[iCurrentFrame]->iFrameSize,
	                   pFMagSpectrum, pFPhaSpectrum, pAnalParams);

	/* find the prominent peaks */
	pAnalParams->ppFrames[iCurrentFrame]->nPeaks = 
		sms_detectPeaks (pFMagSpectrum, pFPhaSpectrum, sizeMag, 
                                 pAnalParams->ppFrames[iCurrentFrame]->pSpectralPeaks,
                                 pAnalParams);

	if (pAnalParams->iDebugMode == SMS_DBG_PEAK_DET || 
	    pAnalParams->iDebugMode == SMS_DBG_ALL)
	{
		fprintf(stdout, "Frame %d peaks: ", 
		        pAnalParams->ppFrames[iCurrentFrame]->iFrameNum);
		/* print only the first 10 peaks */
		for(i=0; i<10; i++)
			fprintf(stdout, " %.2f[%.2f], ", 
			        pAnalParams->ppFrames[iCurrentFrame]->pSpectralPeaks[i].fFreq,
			        pAnalParams->ppFrames[iCurrentFrame]->pSpectralPeaks[i].fMag);
		fprintf(stdout, "\n");
	}
  
	/* find a reference harmonic */
	if (pAnalParams->ppFrames[iCurrentFrame]->nPeaks > 0 && 
	    (pAnalParams->iFormat == SMS_FORMAT_H ||
	    pAnalParams->iFormat == SMS_FORMAT_HP))
		sms_harmDetection (pAnalParams->ppFrames[iCurrentFrame], fRefFundamental, pAnalParams);
  
	if (pAnalParams->iDebugMode == SMS_DBG_HARM_DET || 
	    pAnalParams->iDebugMode == SMS_DBG_ALL)
		fprintf(stdout, "Frame %d: fundamental %f\n", 
		        pAnalParams->ppFrames[iCurrentFrame]->iFrameNum,
		        pAnalParams->ppFrames[iCurrentFrame]->fFundamental);
}

/*! \brief re-analyze the previous frames if necessary  
 *
 * \todo explain when this is necessary 
 *
 * \param iCurrentFrame             current frame number
 * \param pAnalParams              structure with analysis parameters
 * \return 1 if frames are good, -1 if analysis is necessary
 * \todo is the return value info correct? Why isn't it used in sms_analyze?
 */
static int ReAnalyzeFrame (int iCurrentFrame, SMS_AnalParams *pAnalParams)
{
        float fFund, fLastFund, fDev;
	int iNewFrameSize, i;
	float fAvgDeviation = sms_fundDeviation(pAnalParams, iCurrentFrame);
        int iFirstFrame = iCurrentFrame - SMS_MIN_GOOD_FRAMES;

	if (pAnalParams->iDebugMode == SMS_DBG_SMS_ANAL || 
	    pAnalParams->iDebugMode == SMS_DBG_ALL)
		fprintf(stdout, "Frame %d reAnalyze: Freq. deviation %f\n", 
		       pAnalParams->ppFrames[iCurrentFrame]->iFrameNum, fAvgDeviation);
  
	if (fAvgDeviation == -1)
		return (-1);
  
	/* if the last SMS_MIN_GOOD_FRAMES are stable look before them */
	/*  and recompute the frames that are not stable */
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
				AnalyzeFrame (iFirstFrame - i, pAnalParams, fLastFund);
				pAnalParams->ppFrames[iFirstFrame - i]->iStatus = SMS_FRAME_RECOMPUTED;
	    
				if (fabs(pAnalParams->ppFrames[iFirstFrame - i]->fFundamental - fLastFund) / 
				    fLastFund >= .2)
				return(-1);
			}
		}
	return (1);
}

/*! \brief main function to perform the SMS analysis on a single frame 
 *
 * The input is a section of the sound, the output is the SMS data
 *
 * \param pWaveform	     pointer to input waveform data
 * \param sizeNewData	     the size of input data
 * \param pSmsData          pointer to output SMS data
 * \param pAnalParams   pointer to analysis parameters
 * \param pINextSizeRead size of next data to read
 * \return \todo sort out return meanings
 */
int sms_analyze (float *pWaveform, long sizeNewData, SMS_Data *pSmsData, 
                 SMS_AnalParams *pAnalParams, int *pINextSizeRead)
{    
	static int sizeWindow = 0;      /* size of current analysis window */ //RTE ?: shouldn't this just be initilalized outside?

	int iCurrentFrame = pAnalParams->iMaxDelayFrames - 1;  /* frame # of current frame */
	int i, iError, iExtraSamples;              /* samples used for next analysis frame */
	float fRefFundamental = 0;   /* reference fundamental for current frame */
        SMS_AnalFrame *pTmpAnalFrame;

	/* clear SMS output */
	sms_clearRecord (pSmsData);
  
	/* set initial analysis-window size */
	if (sizeWindow == 0)
		sizeWindow = pAnalParams->iDefaultSizeWindow;
  
	/* fill the input sound buffer and perform pre-emphasis */
	if (sizeNewData > 0)
		sms_fillSndBuffer (pWaveform, sizeNewData, pAnalParams);
    
	/* move analysis data one frame back */
	pTmpAnalFrame = pAnalParams->ppFrames[0];
	for(i = 1; i < pAnalParams->iMaxDelayFrames; i++)
		pAnalParams->ppFrames[i-1] = pAnalParams->ppFrames[i];
	pAnalParams->ppFrames[pAnalParams->iMaxDelayFrames-1] = pTmpAnalFrame;


	/* initialize the current frame */
	iError = sms_initFrame (iCurrentFrame, pAnalParams, sizeWindow);
        if(iError != SMS_OK) return (-1);
  
	/* if right data in the sound buffer do analysis */
	if (pAnalParams->ppFrames[iCurrentFrame]->iStatus == SMS_FRAME_READY)
	{
		float fAvgDev = sms_fundDeviation( pAnalParams, iCurrentFrame - 1);
      
		if (pAnalParams->iDebugMode == SMS_DBG_SMS_ANAL || 
		    pAnalParams->iDebugMode == SMS_DBG_ALL)
			fprintf (stdout, "Frame %d: sizeWindow %d, sizeNewData %ld, firstSampleBuffer %d, centerSample %d\n",
			         pAnalParams->ppFrames[iCurrentFrame]->iFrameNum,
			         sizeWindow, sizeNewData, pAnalParams->soundBuffer.iMarker,
		           pAnalParams->ppFrames[iCurrentFrame]->iFrameSample);

		/* if single note use the default fundamental as reference */
		if (pAnalParams->iSoundType == SMS_SOUND_TYPE_NOTE)
			fRefFundamental = pAnalParams->fDefaultFundamental;
		/* if sound is stable use the last fundamental as a reference */
		else if (fAvgDev != -1 && fAvgDev <= SMS_MAX_DEVIATION)
			fRefFundamental = pAnalParams->ppFrames[iCurrentFrame - 1]->fFundamental;
		else
			fRefFundamental = 0;

		/* compute spectrum, find peaks, and find fundamental of frame */
		AnalyzeFrame (iCurrentFrame, pAnalParams, fRefFundamental);

		/* set the size of the next analysis window */
		if (pAnalParams->ppFrames[iCurrentFrame]->fFundamental > 0 &&
		    pAnalParams->iSoundType != SMS_SOUND_TYPE_NOTE)
			sizeWindow = sms_sizeNextWindow (iCurrentFrame, pAnalParams);
      
		/* figure out how much needs to be read next time */
		iExtraSamples = 
			(pAnalParams->soundBuffer.iMarker + pAnalParams->soundBuffer.sizeBuffer) -
			(pAnalParams->ppFrames[iCurrentFrame]->iFrameSample + pAnalParams->sizeHop);
		*pINextSizeRead = MAX (0, (sizeWindow+1)/2 - iExtraSamples);

		/* check again the previous frames and recompute if necessary */
                /*! \todo when deviation is really off, this function returns -1, yet it 
                  isn't used.. is it being recomputed ?? */
		ReAnalyzeFrame (iCurrentFrame, pAnalParams);
	}
  
	/* incorporate the peaks into the corresponding trajectories */
	/* This is done after a SMS_DELAY_FRAMES delay  */
	if (pAnalParams->ppFrames[iCurrentFrame - SMS_DELAY_FRAMES]->fFundamental > 0 ||
	    ((pAnalParams->iFormat == SMS_FORMAT_IH ||
	      pAnalParams->iFormat == SMS_FORMAT_IHP) &&
	     pAnalParams->ppFrames[iCurrentFrame - SMS_DELAY_FRAMES]->nPeaks > 0))
		sms_peakContinuation (iCurrentFrame - SMS_DELAY_FRAMES, pAnalParams);
    
	/* fill gaps and delete short trajectories */
	if (pAnalParams->iCleanTraj > 0 &&
	    pAnalParams->ppFrames[iCurrentFrame - SMS_DELAY_FRAMES]->iStatus != SMS_FRAME_EMPTY)
		sms_cleanTrajectories (iCurrentFrame - SMS_DELAY_FRAMES, pAnalParams);

	/* do stochastic analysis */
	if (pAnalParams->iStochasticType != SMS_STOC_NONE)
	{
		/* synthesize deterministic signal */
		if (pAnalParams->ppFrames[1]->iStatus != SMS_FRAME_EMPTY &&
		    pAnalParams->ppFrames[1]->iStatus != SMS_FRAME_END)
		{
			/* shift synthesis buffer */
			memcpy ( pAnalParams->synthBuffer.pFBuffer, 
                                 pAnalParams->synthBuffer.pFBuffer+pAnalParams->sizeHop, 
			        sizeof(float) * pAnalParams->sizeHop);
			memset (pAnalParams->synthBuffer.pFBuffer+pAnalParams->sizeHop,
                                0, sizeof(float) * pAnalParams->sizeHop);
      
			/* get deterministic signal with phase  */
			sms_sineSynthFrame (&pAnalParams->ppFrames[1]->deterministic,
			                pAnalParams->synthBuffer.pFBuffer+pAnalParams->sizeHop,  
			                pAnalParams->sizeHop, &pAnalParams->prevFrame, 
			                pAnalParams->iSamplingRate);
		}
  
		/* perform stochastic analysis after 1 frame of the     */
		/* deterministic synthesis because it needs two frames  */
		if (pAnalParams->ppFrames[0]->iStatus != SMS_FRAME_EMPTY &&
		    pAnalParams->ppFrames[0]->iStatus != SMS_FRAME_END)
		{
			int sizeResidual = pAnalParams->sizeHop * 2;
			int iSoundLoc = pAnalParams->ppFrames[0]->iFrameSample - pAnalParams->sizeHop;
			float *pFData = &(pAnalParams->soundBuffer.pFBuffer[iSoundLoc - 
			                                       pAnalParams->soundBuffer.iMarker]);
			float *pFResidual;
			int sizeData = 
				MIN (pAnalParams->soundBuffer.sizeBuffer - 
				      (iSoundLoc - pAnalParams->soundBuffer.iMarker),
				     sizeResidual);
			if ((pFResidual = (float *) calloc (sizeResidual, sizeof(float))) 
			    == NULL)
			{
                                printf("sms_analyze: error allocating memory for pFResidual \n");
                                return -1;
                        }

			/* obtain residual sound from original and synthesized sounds */
			sms_residual (pAnalParams->synthBuffer.pFBuffer, pFData, pFResidual, sizeData, 
			             pAnalParams);
                         
                        /* approximate residual */
			sms_stocAnalysis (pFResidual, sizeData, pSmsData, pAnalParams);
                        
			/* get sharper transitions in deterministic representation */
			sms_scaleDet (pAnalParams->synthBuffer.pFBuffer, pFData, 
			                    pAnalParams->ppFrames[0]->deterministic.pFMagTraj,
			                    pAnalParams, pSmsData->nTraj);
      
			pAnalParams->ppFrames[0]->iStatus = SMS_FRAME_DONE;

			free ((char *) pFResidual);
		}
	}
	else if (pAnalParams->ppFrames[0]->iStatus != SMS_FRAME_EMPTY &&
	         pAnalParams->ppFrames[0]->iStatus != SMS_FRAME_END)
		pAnalParams->ppFrames[0]->iStatus = SMS_FRAME_DONE;
	/* get the result */
	if (pAnalParams->ppFrames[0]->iStatus == SMS_FRAME_EMPTY)
        {
		return (0);
        }
	/* return analysis data */
	else if (pAnalParams->ppFrames[0]->iStatus == SMS_FRAME_DONE)
	{
		/* put data into output */
		int length = sizeof(float) * pSmsData->nTraj;
		memcpy ((char *) pSmsData->pFFreqTraj, (char *) 
		        pAnalParams->ppFrames[0]->deterministic.pFFreqTraj, length);
		memcpy ((char *) pSmsData->pFMagTraj, (char *) 	
		         pAnalParams->ppFrames[0]->deterministic.pFMagTraj, length);
		if (pAnalParams->iFormat == SMS_FORMAT_HP ||
		    pAnalParams->iFormat == SMS_FORMAT_IHP)
			memcpy ((char *) pSmsData->pFPhaTraj, (char *) 	
			        pAnalParams->ppFrames[0]->deterministic.pFPhaTraj, length);
		return (1);
	}
	/* done, end of sound */
	else if (pAnalParams->ppFrames[0]->iStatus == SMS_FRAME_END)
		return (-1);
	else
	{
		printf ("sms_analyze error: wrong status of frame\n");
		//exit (1);
                return(-1);
	}
	return (1);
}
