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

/* function used for the parabolic interpolation of the spectral peaks
 * it performs the interpolation in a log scale                       
 * stores the location in pFDiffFromMax and returns the peak height 
 *
 * float fMaxVal;        value of max bin 
 * float fLeftBinVal;    value for left bin
 * float fRightBinVal;   value for right bin
 * float *pFDiffFromMax; location of the tip as the difference from the
 *                         top bin 
 */
static float PeakInterpolation (float fMaxVal, float fLeftBinVal, 
                                float fRightBinVal, float *pFDiffFromMax)
{
	/* get the location of the tip of the parabola */
	*pFDiffFromMax = (.5 * (fLeftBinVal - fRightBinVal) /
		(fLeftBinVal - (2*fMaxVal) + fRightBinVal));
	/* return the value at the tip */
	return(fMaxVal - (.25 * (fLeftBinVal - fRightBinVal) * 
	                  *pFDiffFromMax));
}

/* function to detect the next local maximum in the spectrum
 * store the value in pFMaxVal and returns the bin 
 * location of the maximum 
 *                                      
 * float *pFMagSpectrum;   magnitude spectrum 
 * int iHighBinBound;      highest bin to search
 * int *pICurrentLoc;      current bin location
 * float *pFMaxVal;        value of the maximum found
 */
static int FindNextMax (float *pFMagSpectrum, int iHighBinBound, 
                        int *pICurrentLoc, float *pFMaxVal, 
                        ANAL_PARAMS analParams)
{
	int iCurrentBin = *pICurrentLoc;
	float fPrevVal = pFMagSpectrum[iCurrentBin - 1],
		fCurrentVal = pFMagSpectrum[iCurrentBin],
		fNextVal = (iCurrentBin >= iHighBinBound) 
			? 0 : pFMagSpectrum[iCurrentBin + 1];
  
	/* try to find a local maximum */
	while (iCurrentBin <= iHighBinBound)
	{
		if (fCurrentVal > analParams.fMinPeakMag &&
		   fCurrentVal >= fPrevVal &&
		   fCurrentVal >= fNextVal)
			break;
		iCurrentBin++;
		fPrevVal = fCurrentVal;
		fCurrentVal = fNextVal;
		fNextVal = pFMagSpectrum [1+iCurrentBin];
	}
	/* save the current location, value of maximum and return */
	/*    location of max */
	*pICurrentLoc = iCurrentBin + 1;
	*pFMaxVal = fCurrentVal;
	return(iCurrentBin);
}

/* function to detect the next spectral peak, returns 1 if found, 0 if not  
 *                           
 * float *pFMagSpectrum;    magnitude spectrum 
 * int iHighBinBound;       highest bin to search
 * int *pICurrentLoc;       current bin location
 * float *pFPeakMag;        magnitude value of peak
 * float *pFPeakLoc;        location of peak
 */
static int FindNextPeak (float *pFMagSpectrum, int iHighBinBound, 
                         int *pICurrentLoc, float *pFPeakMag, float *pFPeakLoc,
                         ANAL_PARAMS analParams)
{
	int iMaxBin = 0;		/* location of the local maximum */
	float fMaxVal = 0;
  
	/* keep trying to find a good peak while inside the freq range */
	while ((iMaxBin = FindNextMax(pFMagSpectrum, iHighBinBound, 
	       pICurrentLoc, &fMaxVal, analParams)) 
	       <= iHighBinBound)
	{
		/* get the neighboring samples */
		float fDiffFromMax = 0;
		float fLeftBinVal = pFMagSpectrum[iMaxBin - 1];
		float fRightBinVal = pFMagSpectrum[iMaxBin + 1];
		if (fLeftBinVal <= 0 || fRightBinVal <= 0)
			continue;
		/* interpolate the spectral samples to obtain
		   a more accurate magnitude and freq */
		*pFPeakMag = PeakInterpolation (fMaxVal, fLeftBinVal,
		                                fRightBinVal, &fDiffFromMax);
		*pFPeakLoc = iMaxBin + fDiffFromMax;
		return (1);
	}
	/* if it does not find a peak return 0 */
	return (0);
}

/* get the corresponding phase value for a given peak
 * performs linear interpolation for a more accurate phase
 *    returns the phase value                             
 * float *pAPhaSpectrum;     phase spectrum
 * float fPeakLoc;           location of peak
 */
static float GetPhaseVal (float *pAPhaSpectrum, float fPeakLoc)
{
	int bin = (int) fPeakLoc;
	float fFraction = fPeakLoc - bin,
		fLeftPha = pAPhaSpectrum[bin],
		fRightPha = pAPhaSpectrum[bin+1];
  
	/* check for phase wrapping */
	if (fLeftPha - fRightPha > 1.5 * PI)
		fRightPha += TWO_PI;
	else if (fRightPha - fLeftPha > 1.5 * PI)
		fLeftPha += TWO_PI;
  
	/* return interpolated phase */
	return (fLeftPha + fFraction * (fRightPha - fLeftPha));
}


/* function to find the prominent spectral peaks on a dB spectrum
 * returns the number of peaks found
 *
 * float *pFMagSpectrum;     pointer to power spectrum
 * float *pAPhaSpectrum;     pointer to phase spectrum
 * int sizeMag;              size of magnitude spectrum
 * PEAK *pSpectralPeaks;	 pointer to array of peaks
 * int iSamplingRate;      sampling rate of sound
 */
int PeakDetection (float *pFMagSpectrum, float *pAPhaSpectrum, int sizeMag, 
                   int sizeWindow, PEAK *pSpectralPeaks, 
                   ANAL_PARAMS analParams)
{
	int iPeak = 0;		/* index for spectral search */
	int sizeFft = sizeMag * 2;
	int iCurrentLoc = MAX (2, sizeFft * analParams.fLowestFundamental / 
	                          analParams.iSamplingRate); 
	int iHighestBin  = MIN (sizeMag-1, 
	                        sizeFft * analParams.fHighestFreq / 
	                        analParams.iSamplingRate);
  	float fPeakMag = 0;		/* magnitude of peak */
	float fPeakLoc = 0;		/* location of peak */

	/* clear peak structure */
	memset (pSpectralPeaks, 0, MAX_NUM_PEAKS * sizeof(PEAK));
  
	/* find peaks */
	iPeak = 0;
	while ((iPeak < MAX_NUM_PEAKS) &&
	       (FindNextPeak(pFMagSpectrum, iHighestBin,
	                     &iCurrentLoc, &fPeakMag, &fPeakLoc, analParams)
	        == 1))
	{
		/* store peak values */
		pSpectralPeaks[iPeak].fFreq = analParams.iSamplingRate * fPeakLoc / 
			sizeFft;
		pSpectralPeaks[iPeak].fMag = fPeakMag;
		pSpectralPeaks[iPeak].fPhase = GetPhaseVal(pAPhaSpectrum, fPeakLoc);
		iPeak++;
	}
	/* return the number of peaks found */
	return (iPeak);
}
