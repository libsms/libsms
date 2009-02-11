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
/*! \file peakDetection.c
 * \brief peak detection algorithm and functions
 */

#include "sms.h"

/*! \brief function used for the parabolic interpolation of the spectral peaks
 *
 * it performs the interpolation in a log scale and
 * stores the location in pFDiffFromMax and
 *
 * \param fMaxVal        value of max bin 
 * \param fLeftBinVal    value for left bin
 * \param fRightBinVal   value for right bin
 * \param pFDiffFromMax location of the tip as the difference from the top bin 
 *  \return the peak height 
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

/*! \brief detect the next local maximum in the spectrum
 *
 * stores the value in pFMaxVal 
 *                                      
 * \param pFMagSpectrum   magnitude spectrum 
 * \param iHighBinBound      highest bin to search
 * \param pICurrentLoc      current bin location
 * \param pFMaxVal        value of the maximum found
 * \param pAnalParams	        analysis parameters
 * \return the bin location of the maximum  
*/
/* static int FindNextMax (float *pFMagSpectrum, int iHighBinBound,  */
/*                         int *pICurrentLoc, float *pFMaxVal,  */
/*                         SMS_AnalParams *pAnalParams) */
static int FindNextMax (float *pFMagSpectrum, int iHighBinBound, 
                        int *pICurrentLoc, float *pFMaxVal, float fMinPeakMag)
{
	int iCurrentBin = *pICurrentLoc;
	float fPrevVal = pFMagSpectrum[iCurrentBin - 1],
		fCurrentVal = pFMagSpectrum[iCurrentBin],
		fNextVal = (iCurrentBin >= iHighBinBound) 
			? 0 : pFMagSpectrum[iCurrentBin + 1];
  
	/* try to find a local maximum */
	while (iCurrentBin <= iHighBinBound)
	{
		if (fCurrentVal > fMinPeakMag &&
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

/*! \brief function to detect the next spectral peak 
 *                           
 * \param pFMagSpectrum    magnitude spectrum 
 * \param iHighBinBound       highest bin to search
 * \param pICurrentLoc       current bin location
 * \param pFPeakMag        magnitude value of peak
 * \param pFPeakLoc        location of peak
 * \param pAnalParams	        analysis parameters
 * \return 1 if found, 0 if not  
 */
static int FindNextPeak (float *pFMagSpectrum, int iHighBinBound, 
                         int *pICurrentLoc, float *pFPeakMag, float *pFPeakLoc,
                         float fMinPeakMag)
{
	int iMaxBin = 0;		/* location of the local maximum */
	float fMaxVal = 0;
  
	/* keep trying to find a good peak while inside the freq range */
	while ((iMaxBin = FindNextMax(pFMagSpectrum, iHighBinBound, 
	       pICurrentLoc, &fMaxVal, fMinPeakMag)) 
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

/*! \brief get the corresponding phase value for a given peak
 *
 * performs linear interpolation for a more accurate phase

 * \param pAPhaSpectrum     phase spectrum
 * \param fPeakLoc                 location of peak
 * \return the phase value                             
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

/*! \brief find the prominent spectral peaks
 * 
 * uses a dB spectrum
 *
 * \param sizeSpec              size of magnitude spectrum
 * \param pMag     pointer to power spectrum
 * \param pPhase     pointer to phase spectrum
 * \param pSpectralPeaks	 pointer to array of peaks
 * \param pPeakParams      peak detection parameters
 * \return the number of peaks found
 */
int sms_detectPeaks (int sizeSpec, float *pMag, float *pPhase,
                   SMS_Peak *pSpectralPeaks, SMS_PeakParams *pPeakParams)
{
        static int iFirstBin, iHighestBin, sizeFft;
        static float  fInvSizeFft;
        static int sizeSpecStatic = 0;

        //printf("sizeSpec: %d \n", sizeSpec);

        if(sizeSpecStatic != sizeSpec)
        {
                sizeSpecStatic = sizeSpec;
                //printf("sizeSpecStatic: %d \n", sizeSpecStatic);
                sizeFft = sizeSpec << 1;
                fInvSizeFft = 1.0 / sizeFft;
                /* set iCurrentLoc to starting bin */
                iFirstBin = MAX (2, sizeFft * pPeakParams->fLowestFreq /
	                          pPeakParams->iSamplingRate);
                iHighestBin  = MIN (sizeSpec-1,
                                    sizeFft * pPeakParams->fHighestFreq /
                                    pPeakParams->iSamplingRate);
        }
        int iCurrentLoc = iFirstBin;
	int iPeak = 0;		/* index for spectral search */
  	float fPeakMag = 0.0;		/* magnitude of peak */
	float fPeakLoc = 0.0;		/* location of peak */
        
/*         printf("sizeSpec: %d, sizeFft: %d, iCurrentLoc: %d, iHighestBin: %d \n", sizeSpec, sizeFft, */
/*                iCurrentLoc, iHighestBin); */
	/* clear peak structure */
	memset (pSpectralPeaks, 0, pPeakParams->iMaxPeaks * sizeof(SMS_Peak));
  
	/* find peaks */
	while ((iPeak < pPeakParams->iMaxPeaks) &&
	       (FindNextPeak(pMag, iHighestBin,
	                     &iCurrentLoc, &fPeakMag, &fPeakLoc, pPeakParams->fMinPeakMag)
	        == 1))
	{
		/* store peak values */
		pSpectralPeaks[iPeak].fFreq = pPeakParams->iSamplingRate * fPeakLoc *
			fInvSizeFft;
		pSpectralPeaks[iPeak].fMag = fPeakMag;
		pSpectralPeaks[iPeak].fPhase = GetPhaseVal(pPhase, fPeakLoc);
		iPeak++;
	}

	/* return the number of peaks found */
	return (iPeak);
}
