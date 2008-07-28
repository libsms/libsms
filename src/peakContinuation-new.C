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
#include "../sms.h"

extern ANAL_FRAME **ppFrames;

/* function to get the next closest peak from a guide
 * float fGuideFreq;		guide's frequency
 * float *pFFreqDistance;	distance of last best peak from guide
 * PEAK *pSpectralPeaks;	array of peaks
 * ANAL_PARAMS analParams;	analysis parameters
 * float fFreqDev;		maximum deviation from guide
 */
static int GetNextClosestPeak (float fGuideFreq, float *pFFreqDistance, 
                               PEAK *pSpectralPeaks, ANAL_PARAMS analParams,
                               float fFreqDev)
{
	int iInitialPeak = 
		MAX_NUM_PEAKS * fGuideFreq / (analParams.iSamplingRate * .5),
		iLowPeak, iHighPeak, iChosenPeak = -1;
	float fLowDistance, fHighDistance, fFreq;

	if (pSpectralPeaks[iInitialPeak].fFreq <= 0)
		iInitialPeak = 0;
	  
	/* find a low peak to start */
	fLowDistance = fGuideFreq - pSpectralPeaks[iInitialPeak].fFreq;
	if (floor(fLowDistance) < floor(*pFFreqDistance))
	{
		while (floor(fLowDistance) <= floor(*pFFreqDistance) && 
		       iInitialPeak > 0)
		{
			iInitialPeak--;
			fLowDistance = fGuideFreq - pSpectralPeaks[iInitialPeak].fFreq;
		}
	}
	else
	{
		while (floor(fLowDistance) >= floor(*pFFreqDistance) &&
		       iInitialPeak < MAX_NUM_PEAKS)
		{
			iInitialPeak++;	 
			if ((fFreq = pSpectralPeaks[iInitialPeak].fFreq) == 0)
			{	
				if (*pFFreqDistance != -1)
					return -1;
				else break;
			}
			fLowDistance = fGuideFreq - fFreq;
		}
		iInitialPeak--;
		fLowDistance = fGuideFreq - pSpectralPeaks[iInitialPeak].fFreq;
	}
  
	if (floor(fLowDistance) <= floor(*pFFreqDistance) || 
	    fLowDistance > fFreqDev)
		iLowPeak = -1;
	else
		iLowPeak = iInitialPeak;
    
	/* find a high peak to finish */
	iHighPeak = iInitialPeak;
	fHighDistance = fGuideFreq - pSpectralPeaks[iHighPeak].fFreq;
	while (floor(fHighDistance) >= floor(-*pFFreqDistance) &&
	       iHighPeak < MAX_NUM_PEAKS)
	{
		iHighPeak++;	 
		if ((fFreq = pSpectralPeaks[iHighPeak].fFreq) == 0)
		{
			iHighPeak = -1;
			break;
		}
		fHighDistance = fGuideFreq - fFreq;
	}
	if (fHighDistance > 0 || fabs(fHighDistance) > fFreqDev ||
	    floor(fabs(fHighDistance)) <= floor(*pFFreqDistance))
		iHighPeak = -1;

	/* chose between the two extrema */
	if (iHighPeak >= 0 && iLowPeak >= 0)
	{
		if (fabs(fHighDistance) > fLowDistance)
			iChosenPeak = iLowPeak;
		else
			iChosenPeak = iHighPeak;
	}
	else if (iHighPeak < 0 && iLowPeak >= 0)
		iChosenPeak = iLowPeak;
	else if (iHighPeak >= 0 && iLowPeak < 0)
		iChosenPeak = iHighPeak;
	else
		return (-1);

	*pFFreqDistance = fabs (fGuideFreq - pSpectralPeaks[iChosenPeak].fFreq);
	return (iChosenPeak);
}	

/* choose the best candidate out of all, returns the peak number of 
 * the best candidate
 *
 * CONT_CANDIDATE *pCandidate;  pointer to all the continuation candidates
 * int nCandidates;             number of candidates
 * float fFreqDev;              maximum frequency deviation allowed
 */
static int ChooseBestCand (CONT_CANDIDATE *pCandidate, int nCandidates, 
                           float fFreqDev)
{
	int i, iHighestCand, iClosestCand, iBestCand = 0;
	float fMaxMag, fClosestFreq;
  
	/* intial guess */
	iClosestCand = 0;
	fClosestFreq = pCandidate[iClosestCand].fFreqDev;
	iHighestCand = 0;
	fMaxMag = pCandidate[iHighestCand].fMagDev;
  
	/* get the best candidate */
	for (i = 1; i < nCandidates; i++)
	{
		/* look for the one with highest magnitude */
		if (pCandidate[i].fMagDev > fMaxMag)
		{
			fMaxMag = pCandidate[i].fMagDev;
			iHighestCand = i;
		}
		/* look for the closest one to the guide */
		if (pCandidate[i].fFreqDev < fClosestFreq)
		{
			fClosestFreq = pCandidate[i].fFreqDev;
			iClosestCand = i;
		}
	}
	iBestCand = iHighestCand;
  
	/* reconcile the two results */
	if (iBestCand != iClosestCand &&
	    fabs(pCandidate[iHighestCand].fFreqDev - fClosestFreq) > fFreqDev / 2)
		iBestCand = iClosestCand; 
  
	return(pCandidate[iBestCand].iPeak);
}

/* check for one guide that has choosen iBestPeak
 * int iBestPeak;		choosen peak for a guide
 * GUIDE *pGuides;		array of guides
 * int nGuides;			total number of guides
 */
static int CheckForConflict (int iBestPeak, GUIDE *pGuides, int nGuides)
{
	int iGuide;
  
	for (iGuide = 0; iGuide < nGuides; iGuide++)
		if (pGuides[iGuide].iPeakChosen == iBestPeak)
			return iGuide;
   
	return -1;
}

/* chose the best of the two guides for the conflicting peak 
 * int iConflictingGuide;	conflicting guide number
 * int iGuide;			guide number
 * GUIDE *pGuides;		array of guides
 * PEAK *pSpectralPeaks;	array of peaks
 */
static int BestGuide (int iConflictingGuide, int iGuide, GUIDE *pGuides,
                      PEAK *pSpectralPeaks)
{
	int iConflictingPeak = pGuides[iConflictingGuide].iPeakChosen;
	float fGuideDistance = fabs (pSpectralPeaks[iConflictingPeak].fFreq -
	                             pGuides[iGuide].fFreq);
	float fConfGuideDistance = fabs (pSpectralPeaks[iConflictingPeak].fFreq -
	                                 pGuides[iConflictingGuide].fFreq);

	if (fGuideDistance > fConfGuideDistance)
		return (iConflictingGuide);
	else
		return (iGuide);
}

/* function to find the best continuation peak for a given guide
 *	returns the peak number
 * GUIDE *pGuideVal;		guide attributes
 * PEAK *pSpectralPeaks;	peak values at the current frame
 * ANAL_PARAMS analParams;	analysis parameters
 * float fFreqDev;              frequency deviation allowed
 */
static int GetBestPeak (GUIDE *pGuides, int iGuide, PEAK *pSpectralPeaks, 
                        ANAL_PARAMS analParams, float fFreqDev)
{
	int iCand = 0, iPeak, iBestPeak, iConflictingGuide, iWinnerGuide;
	float fGuideFreq = pGuides[iGuide].fFreq,
		fGuideMag = pGuides[iGuide].fMag,
		fFreqDistance = -1, fMagDistance = 0;
	CONT_CANDIDATE pCandidate[MAX_CONT_CANDIDATES];

	/* find all possible candidates */
	while (iCand < MAX_CONT_CANDIDATES)
	{
		/* find the next best peak */
		if ((iPeak = GetNextClosestPeak (fGuideFreq, &fFreqDistance,
		                                 pSpectralPeaks, analParams, 
		                                 fFreqDev)) < 0)
			break;
	
		/* if the peak's magnitude is not too small accept it as */
		/* possible candidate        */
		if ((fMagDistance = pSpectralPeaks[iPeak].fMag  - fGuideMag) > -20.0)
		{
			pCandidate[iCand].fFreqDev = fabs(fFreqDistance);
			pCandidate[iCand].fMagDev = fMagDistance;
			pCandidate[iCand].iPeak = iPeak;
      	      
			if(analParams.iDebugMode == DEBUG_PEAK_CONT ||
			   analParams.iDebugMode == DEBUG_ALL)
				fprintf (stdout, "candidate %d: freq %f mag %f\n", 
				         iCand, pSpectralPeaks[iPeak].fFreq, 	
				         pSpectralPeaks[iPeak].fMag);
			iCand++;
		}
	}
	/* get best candidate */
	if (iCand < 1)
		return (0);
	else if (iCand == 1)
		iBestPeak = pCandidate[0].iPeak;
	else
		iBestPeak = ChooseBestCand (pCandidate, iCand, 
		                            analParams.fFreqDeviation);
      
	if(analParams.iDebugMode == DEBUG_PEAK_CONT ||
	   analParams.iDebugMode == DEBUG_ALL)
		fprintf (stdout, "BestCandidate: freq %f\n",
		         pSpectralPeaks[iBestPeak].fFreq);

	/* if peak taken by another guide resolve conflict */
	if ((iConflictingGuide = CheckForConflict (iBestPeak, pGuides, 
	                                           analParams.nGuides)) >= 0)
	{
		iWinnerGuide = BestGuide (iConflictingGuide, iGuide, pGuides, 
		                          pSpectralPeaks);
		if(analParams.iDebugMode == DEBUG_PEAK_CONT ||
		   analParams.iDebugMode == DEBUG_ALL)
			fprintf (stdout, 
			         "Conflict: guide: %d (%f), and guide: %d (%f). best: %d\n", 
			         iGuide, pGuides[iGuide].fFreq, 
			         iConflictingGuide, pGuides[iConflictingGuide].fFreq, 	    
			         iWinnerGuide);

		if (iGuide == iWinnerGuide)				     
		{
			pGuides[iGuide].iPeakChosen = iBestPeak;
			pGuides[iConflictingGuide].iPeakChosen = -1;
		}
	}
	else
		pGuides[iGuide].iPeakChosen = iBestPeak;

	return (iBestPeak);
}

/* function to get the next maximum peak
 * PEAK *pSpectralPeaks;	array of peaks
 * float *pFCurrentMax;		last peak maximum
 */
static int GetNextMax (PEAK *pSpectralPeaks, float *pFCurrentMax)
{
	float fPeakMag, fMaxMag = MAG_THRESHOLD;
	int iPeak, iMaxPeak = -1;
  
	for (iPeak = 0; iPeak < MAX_NUM_PEAKS; iPeak++)
	{
		fPeakMag = pSpectralPeaks[iPeak].fMag;
    
		if (fPeakMag == 0)
			break;
      
		if (fPeakMag > fMaxMag && fPeakMag < *pFCurrentMax)
		{
			iMaxPeak = iPeak;
			fMaxMag = fPeakMag;
		}
	}
	*pFCurrentMax = fMaxMag;
	return (iMaxPeak);
}

/* function to get a good starting peak for a trajectory
 * int iGuide;      		current guide
 * GUIDE *pGuides;  		array of guides
 * int nGuides;			    total number of guides
 * PEAK *pSpectralPeaks;	array of peaks
 * float *pFCurrentMax;		current peak maximum
 */
static int GetStartingPeak (int iGuide, GUIDE *pGuides, int nGuides,
                            PEAK *pSpectralPeaks, float *pFCurrentMax)
{
	int iPeak = -1;
	short peakNotFound = 1;
  
	while (peakNotFound == 1 && *pFCurrentMax > 0)
	{
		if ((iPeak = GetNextMax (pSpectralPeaks, pFCurrentMax)) < 0)
			return (-1);
  
		if (CheckForConflict (iPeak, pGuides, nGuides) < 0)
		{
			pGuides[iGuide].iPeakChosen = iPeak;
			pGuides[iGuide].iStatus = BEGINNING;
			pGuides[iGuide].fFreq = pSpectralPeaks[iPeak].fFreq;
			peakNotFound = 0;
		}
	}
	return (1);
}

/* function to advance the guides through the next frame
 * the output is the freq., mag., and phase trajectories
 *
 * int iFrame;	 current frame number
 * ANAL_PARAMS analParams; analysis parameters
 */
int PeakContinuation (int iFrame, ANAL_PARAMS analParams)
{
	int iGuide, iCurrentPeak = -1, iGoodPeak = -1;
	float fFund = ppFrames[iFrame]->fFundamental,
		fFreqDev = fFund * analParams.fFreqDeviation, fCurrentMax = 1000;
  	static GUIDE *pGuides = NULL;

	if (pGuides == NULL)
	{
		if ((pGuides = (GUIDE *) calloc(analParams.nGuides, sizeof(GUIDE))) 
		   == NULL)
			return -1;
		for (iGuide = 0; iGuide < analParams.nGuides; iGuide++)
		{
			pGuides[iGuide].iPeakChosen = -1;
			pGuides[iGuide].iStatus = DEAD;
		}
		if (analParams.iFormat == SMS_FORMAT_H ||
		    analParams.iFormat == SMS_FORMAT_HP)
		for (iGuide = 0; iGuide < analParams.nGuides; iGuide++)
			pGuides[iGuide].fFreq = analParams.fDefaultFundamental 
			                        * (iGuide + 1);
	}        

	/* update guides with fundamental contribution */
	if (fFund > 0 &&
	    (analParams.iFormat == SMS_FORMAT_H ||
	     analParams.iFormat == SMS_FORMAT_HP))
		for(iGuide = 0; iGuide < analParams.nGuides; iGuide++)
			pGuides[iGuide].fFreq = 
				(1 - analParams.fFundContToGuide) * pGuides[iGuide].fFreq +        
				analParams.fFundContToGuide * fFund * (iGuide + 1);
  
	if (analParams.iDebugMode == DEBUG_PEAK_CONT ||
	    analParams.iDebugMode == DEBUG_ALL)
		fprintf (stdout, 
		         "Frame %d Peak Continuation: \n", 
		         ppFrames[iFrame]->iFrameNum);

	/* continue all guides */
	for (iGuide = 0; iGuide < analParams.nGuides; iGuide++)
	{
		float fPreviousFreq = 
			ppFrames[iFrame-1]->deterministic.pFFreqTraj[iGuide];
   
		/* get the guide value by upgrading the previous guide */
		if (fPreviousFreq > 0)
			pGuides[iGuide].fFreq =
				(1 - analParams.fPeakContToGuide) * pGuides[iGuide].fFreq +
				analParams.fPeakContToGuide * fPreviousFreq;
   
		if (analParams.iDebugMode == DEBUG_PEAK_CONT ||
		    analParams.iDebugMode == DEBUG_ALL)
			fprintf (stdout, "Guide %d:  freq %f, mag %f\n", 
			         iGuide, pGuides[iGuide].fFreq, pGuides[iGuide].fMag);
      
		if (pGuides[iGuide].fFreq <= 0.0 ||
		    pGuides[iGuide].fFreq > analParams.fHighestFreq)
		{
			pGuides[iGuide].iStatus = DEAD;
			pGuides[iGuide].fFreq = 0;
			pGuides[iGuide].iPeakChosen = -1;
			continue;
		}
      
		pGuides[iGuide].iPeakChosen = -1;
    
		if (analParams.iFormat == SMS_FORMAT_IH ||
		    analParams.iFormat == SMS_FORMAT_IHP)
			fFreqDev = pGuides[iGuide].fFreq * analParams.fFreqDeviation;
      
		/* get the best peak for the guide */
		iGoodPeak = 
			GetBestPeak(pGuides, iGuide, ppFrames[iFrame]->pSpectralPeaks, 
			            analParams, fFreqDev);
	}
  
	/* try to find good peaks for the DEAD guides */
	if (analParams.iFormat == SMS_FORMAT_IH ||
	    analParams.iFormat == SMS_FORMAT_IHP)
		for(iGuide = 0; iGuide < analParams.nGuides; iGuide++)
		{
			if (pGuides[iGuide].iStatus != DEAD)
				continue; 
	
			if (GetStartingPeak (iGuide, pGuides, analParams.nGuides, 
			                     ppFrames[iFrame]->pSpectralPeaks,
			                     &fCurrentMax) == -1)
				break;
		}

	/* save all the continuation values,
	 * assume output trajectories are already clear */
	for (iGuide = 0; iGuide < analParams.nGuides; iGuide++)
	{
		if (pGuides[iGuide].iStatus == DEAD)
			continue; 

		if (analParams.iFormat == SMS_FORMAT_IH ||
		    analParams.iFormat == SMS_FORMAT_IHP)
		{
			if (pGuides[iGuide].iStatus > 0 &&
			    pGuides[iGuide].iPeakChosen == -1)
			{ 
				if(pGuides[iGuide].iStatus++ > analParams.iMaxSleepingTime)
				{
					pGuides[iGuide].iStatus = DEAD;
					pGuides[iGuide].fFreq = 0;
					pGuides[iGuide].fMag = 0;
					pGuides[iGuide].iPeakChosen = -1;	  	  
	 			}
				else
					pGuides[iGuide].iStatus++;
				continue;
			}
      
			if (pGuides[iGuide].iStatus == ACTIVE &&
			    pGuides[iGuide].iPeakChosen == -1)
			{
				pGuides[iGuide].iStatus = 1;
				continue;
			}
		}

		/* if good continuation peak found, save it */
		if ((iCurrentPeak = pGuides[iGuide].iPeakChosen) >= 0)
		{
			ppFrames[iFrame]->deterministic.pFFreqTraj[iGuide] = 
				ppFrames[iFrame]->pSpectralPeaks[iCurrentPeak].fFreq;
			ppFrames[iFrame]->deterministic.pFMagTraj[iGuide] = 
				ppFrames[iFrame]->pSpectralPeaks[iCurrentPeak].fMag;
			ppFrames[iFrame]->deterministic.pFPhaTraj[iGuide] = 
				ppFrames[iFrame]->pSpectralPeaks[iCurrentPeak].fPhase;
     
			pGuides[iGuide].iStatus = ACTIVE;
			pGuides[iGuide].iPeakChosen = -1;
		}
	}
	return 1;
}
