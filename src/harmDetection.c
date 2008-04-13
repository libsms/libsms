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

/* get closest peak to a given harmonic of the possible fundamental
 *    return the number of the closest peak or -1 if not found  
 *  
 * int iPeakCandidate     peak number of possible fundamental
 * int nHarm              number of harmonic
 * PEAK *pSpectralPeaks   all the peaks
 * int *pICurrentPeak     last peak taken
 */
static int GetClosestPeak (int iPeakCandidate, int nHarm, PEAK *pSpectralPeaks,
                           int *pICurrentPeak, ANAL_PARAMS analParams)
{
	int iBestPeak = *pICurrentPeak + 1, iNextPeak;
	float fBestPeakFreq = pSpectralPeaks[iBestPeak].fFreq,
		fHarmFreq = (1 + nHarm) * pSpectralPeaks[iPeakCandidate].fFreq / 
			analParams.iRefHarmonic, 
		fMinDistance = fabs(fHarmFreq - fBestPeakFreq),
		fMaxPeakDev = .5 * fHarmFreq / (nHarm + 1), fDistance;
  
	iNextPeak = iBestPeak + 1;
	fDistance = fabs(fHarmFreq - pSpectralPeaks[iNextPeak].fFreq);
	while (fDistance < fMinDistance)
	{
		iBestPeak = iNextPeak;
		fMinDistance = fDistance;
		iNextPeak++;
		fDistance = fabs (fHarmFreq - pSpectralPeaks[iNextPeak].fFreq);
	}
  
	/* make sure the chosen peak is good */
	fBestPeakFreq = pSpectralPeaks[iBestPeak].fFreq;
	/* if best peak is not in the range */
	if (fabs (fBestPeakFreq - fHarmFreq) > fMaxPeakDev)
		return (-1);
  
	*pICurrentPeak = iBestPeak;
	return (iBestPeak);
}

/* check if peak is larger enough to be considered a fundamental
 *     without any further testing or too small to be considered
 *
 * return 1 if big peak   -1 if too small , otherwise return 0 
 * float fRefHarmMag      magnitude of possible fundamental
 * PEAK *pSpectralPeaks   all the peaks
 * int nCand              number of existing candidates
 */
static int ComparePeak (float fRefHarmMag, PEAK *pSpectralPeaks, int nCand, 
                        ANAL_PARAMS analParams)
{
	int iPeak;
	float fMag = 0;
  
	/* if peak is very large take it as possible fundamental */
	if (nCand == 0 &&	
	    fRefHarmMag > 80.)
		return (1);
  
	/* compare the peak with the first N_FUND_HARM peaks */
	/* if too small forget it */
	for (iPeak = 0; iPeak < N_FUND_HARM; iPeak++)
		if (pSpectralPeaks[iPeak].fMag > 0 &&
		    fRefHarmMag - pSpectralPeaks[iPeak].fMag < 
		    -analParams.fRefHarmMagDiffFromMax)
			return (-1);
  
	/* if it is much bigger than rest take it */
	for (iPeak = 0; iPeak < N_FUND_HARM; iPeak++)
	{
		fMag = pSpectralPeaks[iPeak].fMag;
		if (fMag <= 0 ||
		    ((fMag != fRefHarmMag) &&
		    (nCand > 0) && (fRefHarmMag - fMag < 30.0)) ||
		    ((nCand == 0) && (fRefHarmMag - fMag < 15.0)))
			return (0);
	}
	return (1);
}


/* check if the current peak is a harmonic of one of the candidates
 *   return 1 if it is a harmonic, 0 if it is not    
 *               
 * float fFundFreq;      frequency of peak to be tested
 * HARM_CANDIDATE *pCHarmonic;  all candidates accepted
 * int nCand;                location of las candidate
 */
int CheckIfHarmonic (float fFundFreq, HARM_CANDIDATE *pCHarmonic, int nCand)
{
	int iPeak;
  
	/* go through all the candidates checking if they are fundamentals */
	/*   of the peak to be considered */
	for (iPeak = 0; iPeak < nCand; iPeak++)
		if (fabs(floor((double)(fFundFreq 
		                        / pCHarmonic[iPeak].fFreq) + .5) -
		         (fFundFreq / pCHarmonic[iPeak].fFreq))
		     <= .1)
			return (1);
	return (0);
}


/* consider a peak as a possible candidate and give it a weight value, 
 * return -1 if not good enough for a candidate, return 0 if reached
 * the top frequency boundary, return -2 if stop checking because it 
 * found a really good one, return 1 if the peak is a good candidate 
 *
 * int iPeak;                iPeak number to be considered
 * PEAK *pSpectralPeaks;     all the peaks
 * FUND_CANDIDATE *pCFundamental;  all the candidates
 * int nCand;                 candidate number that is to be filled
 * ANAL_PARAMS analParams;    analysis parameters
 * float fRefFundamental;     previous fundamental
 */
int GoodCandidate (int iPeak, PEAK *pSpectralPeaks, HARM_CANDIDATE *pCHarmonic,
                   int nCand, ANAL_PARAMS analParams, float fRefFundamental)
{
	float fHarmFreq, fRefHarmFreq, fRefHarmMag, fTotalMag = 0, fTotalDev = 0, 
		fTotalMaxMag = 0, fAvgMag = 0, fAvgDev = 0, fHarmRatio = 0;
	int iHarm = 0, iChosenPeak = 0, iPeakComp, iCurrentPeak, nGoodHarm = 0, i;
  
	fRefHarmFreq = fHarmFreq = pSpectralPeaks[iPeak].fFreq;
  
	fTotalDev = 0;
	fRefHarmMag = pSpectralPeaks[iPeak].fMag;
	fTotalMag = fRefHarmMag;
  
	/* check if magnitude is big enough */
	if (((fRefFundamental > 0) && 
        (fRefHarmMag < analParams.fMinRefHarmMag - 10)) ||
        ((fRefFundamental <= 0) && 
        (fRefHarmMag < analParams.fMinRefHarmMag)))
		return (-1);
  
	/* check that is not a harmonic of a previous candidate */
	if (nCand > 0 &&
		CheckIfHarmonic (fRefHarmFreq / analParams.iRefHarmonic, pCHarmonic, 
			             nCand))
		return (-1);
  
	/* check if it is very big or very small */
	iPeakComp = ComparePeak (fRefHarmMag, pSpectralPeaks, nCand, analParams);
	/* too small */
	if (iPeakComp == -1)
		return (-1);
	/* very big */
	else if (iPeakComp == 1)
	{	
		pCHarmonic[nCand].fFreq = fRefHarmFreq;
		pCHarmonic[nCand].fMag = fRefHarmMag;
		pCHarmonic[nCand].fMagPerc = 1;
		pCHarmonic[nCand].fFreqDev = 0;
		pCHarmonic[nCand].fHarmRatio = 1;
		return (-2);
	}
  
	/* get a weight on the peak by comparing its harmonic series   */
	/* with the existing peaks                                     */
	if (analParams.iSoundType != TYPE_SINGLE_NOTE)
	{
		fHarmFreq = fRefHarmFreq;
		iCurrentPeak = iPeak;
		nGoodHarm = 0;
		for (iHarm = analParams.iRefHarmonic; iHarm < N_FUND_HARM; iHarm++)
		{
			fHarmFreq += fRefHarmFreq / analParams.iRefHarmonic;
			iChosenPeak = GetClosestPeak(iPeak, iHarm, pSpectralPeaks, 
			                             &iCurrentPeak, analParams); 
			if (iChosenPeak > 0)
			{
				fTotalDev += 
					fabs(fHarmFreq - pSpectralPeaks[iChosenPeak].fFreq) /
					fHarmFreq;
				fTotalMag += pSpectralPeaks[iChosenPeak].fMag;
				nGoodHarm++;
			}
		}
  
		for (i = 0; i <= iCurrentPeak; i++)
			fTotalMaxMag +=  pSpectralPeaks[i].fMag;
  
		fAvgDev = fTotalDev / (iHarm + 1);
		fAvgMag = fTotalMag / fTotalMaxMag;
		fHarmRatio = (float) nGoodHarm / (N_FUND_HARM - 1);
	}

	if (analParams.iDebugMode == DEBUG_HARM_DET ||
	    analParams.iDebugMode == DEBUG_ALL)
		fprintf(stdout, 
		        "Harmonic Candidate: frq: %f mag: %f frqDev: %f magPrc: %f harmDev %f\n",
		        fRefHarmFreq, fRefHarmMag, fAvgDev, fAvgMag, fHarmRatio);
  
	if (analParams.iSoundType != TYPE_SINGLE_NOTE)
	{
		if (fRefFundamental > 0)
		{
			if(fAvgDev > FREQ_DEV_THRES || fAvgMag < MAG_PERC_THRES - .1 ||
			   fHarmRatio < HARM_RATIO_THRES - .1)
				return (-1);
		}
		else
		{
			if (fAvgDev > FREQ_DEV_THRES || fAvgMag < MAG_PERC_THRES ||
			    fHarmRatio < HARM_RATIO_THRES)
				return (-1);
		}
	}
	pCHarmonic[nCand].fFreq = fRefHarmFreq;
	pCHarmonic[nCand].fMag = fRefHarmMag;
	pCHarmonic[nCand].fMagPerc = fAvgMag;
	pCHarmonic[nCand].fFreqDev = fAvgDev;
	pCHarmonic[nCand].fHarmRatio = fHarmRatio;
  
	return (1);
}

/* choose the best fundamental out of all the candidates
 * HARM_CANDIDATE *pCFundamental;      array of candidates
 * ANAL_PARAMS analParams;             analysis parameters
 * int nGoodPeaks;                     number of candiates
 * float fPrevFund;                    reference fundamental
 */
static int GetBestCandidate (HARM_CANDIDATE *pCHarmonic, 
                             ANAL_PARAMS analParams,
                             int nGoodPeaks, float fPrevFund)
{
	int iBestCandidate = 0, iPeak;
	float fBestFreq, fHarmFreq, fDev;
  
	/* if a fundamental existed in previous frame take the closest candidate */
	if (fPrevFund > 0)
		for (iPeak = 1; iPeak < nGoodPeaks; iPeak++)
		{
			if (fabs (fPrevFund - pCHarmonic[iPeak].fFreq /
				      analParams.iRefHarmonic) <
			    fabs(fPrevFund - pCHarmonic[iBestCandidate].fFreq / 
			         analParams.iRefHarmonic))
				iBestCandidate = iPeak;
		}
	else
		/* try to find the best candidate */
		for (iPeak = 1; iPeak < nGoodPeaks; iPeak++)
		{
			fBestFreq = pCHarmonic[iBestCandidate].fFreq / 
				analParams.iRefHarmonic;
			fHarmFreq = fBestFreq * 
				floor (.5 + 
				       (pCHarmonic[iPeak].fFreq / analParams.iRefHarmonic) / 
			           fBestFreq);
			fDev = fabs (fHarmFreq - (pCHarmonic[iPeak].fFreq / 
			             analParams.iRefHarmonic)) / fHarmFreq;
	
			/* if candidate is far from harmonic from best candidate and */
			/* bigger, take it */
			if (fDev > .2 &&
			    pCHarmonic[iPeak].fMag >
			    pCHarmonic[iBestCandidate].fMag)
				iBestCandidate = iPeak;
			/* if frequency deviation is much smaller, take it */
			else if (pCHarmonic[iPeak].fFreqDev < 
				       .2 * pCHarmonic[iBestCandidate].fFreqDev)
				iBestCandidate = iPeak;
			/* if freq. deviation is smaller and bigger amplitude, take it */
			else if (pCHarmonic[iPeak].fFreqDev < 
		         pCHarmonic[iBestCandidate].fFreqDev &&
		         pCHarmonic[iPeak].fMagPerc >
		         pCHarmonic[iBestCandidate].fMagPerc &&
		         pCHarmonic[iPeak].fMag >
		         pCHarmonic[iBestCandidate].fMag)
				iBestCandidate = iPeak;
		}
	return (iBestCandidate);
}

/* find a given harmonic peak from a set of spectral peaks,     
 * put the frequency of the fundamental in the current frame
 * ANAL_FRAME *pFrame;              current frame
 * float fRefFundamental;           frequency of previous frame
 * ANAL_PARAMS analParams;          analysis parameters
 */
void HarmDetection (ANAL_FRAME *pFrame, float fRefFundamental,
                    ANAL_PARAMS analParams)
{
	int iPeak = -1, nGoodPeaks = 0, iCandidate, iBestCandidate;
	float fLowestFreq, fHighestFreq, fPeakFreq=0;
	HARM_CANDIDATE pCHarmonic[N_HARM_PEAKS];

	/* find all possible candidates to use as harmonic reference */
	fLowestFreq = analParams.fLowestFundamental * analParams.iRefHarmonic;
	fHighestFreq = analParams.fHighestFundamental * analParams.iRefHarmonic;
	while (fPeakFreq < fHighestFreq)
	{
		iPeak++;
		fPeakFreq = pFrame->pSpectralPeaks[iPeak].fFreq;
		if (fPeakFreq > fHighestFreq)
			break;

		/* no more peaks */
		if (pFrame->pSpectralPeaks[iPeak].fMag <= 0)
			break;
    
		/* peak too low */
		if (fPeakFreq < fLowestFreq)
			continue;

		/* if previous fundamental look only around it */
		if (fRefFundamental > 0 && 
			fabs(fPeakFreq - (analParams.iRefHarmonic * fRefFundamental)) / 
				fRefFundamental > .5)
			continue;
      
		iCandidate = GoodCandidate (iPeak, pFrame->pSpectralPeaks, 
		                            pCHarmonic, nGoodPeaks, analParams,
		                            fRefFundamental);
		/* good candiate found */
		if (iCandidate == 1)
			nGoodPeaks++;
		/* a perfect candiate found */
		else 
			if (iCandidate == -2)
			{
				nGoodPeaks++;
				break;
			}
	}
  
	/* if no candidate for fundamental, continue */
	if (nGoodPeaks == 0)
		pFrame->fFundamental =  -1;
	/* if only 1 candidate for fundamental take it */
	else 
		if (nGoodPeaks == 1)
			pFrame->fFundamental = pCHarmonic[0].fFreq /
				analParams.iRefHarmonic;
	/* if more than one candidate choose the best one */
	else
	{
		iBestCandidate = GetBestCandidate (pCHarmonic, analParams, nGoodPeaks, 
		                                  fRefFundamental);
		pFrame->fFundamental = pCHarmonic[iBestCandidate].fFreq / 
			analParams.iRefHarmonic;
	}
}
