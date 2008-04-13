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

#define ENV_THRESHOLD     .01

/* get the stochastic representation from the residual waveform
 * by approximating its spectrum with an IIR filter
 * return -1 if no representation, 1 if got a representation
 *
 * float *pFResidual;        residual signal
 * int sizeWindow;           size of buffer
 * SMS_DATA *pSmsData;       pointer to output SMS data
 * ANAL_PARAMS analParams;   analysis parameters
 */
static int StocAnalysisIIR (float *pFResidual, int sizeWindow, 
                            SMS_DATA *pSmsData, ANAL_PARAMS analParams)
{
	int nStage = pSmsData->nCoeff, nMax = pSmsData->nCoeff+1, i, j;
	float *pFCovMatrix, *pFOldCoeff, *pFOtherCoeff, *pFScratch,
		fError = 0;
	static float fOldError = 0;
    
	/* allocate temporary arrays */
	if ((pFCovMatrix = 
		(float *) calloc ((pSmsData->nCoeff+1) * (pSmsData->nCoeff+1), 
		                  sizeof(float))) == NULL)
		return -1;			
	if ((pFOldCoeff = (float *) calloc(pSmsData->nCoeff, sizeof(float))) 
		== NULL)
		return -1;
	if ((pFOtherCoeff = (float *) calloc(pSmsData->nCoeff, sizeof(float)))
		== NULL)
		return -1;
	if ((pFScratch = (float *) calloc(pSmsData->nCoeff+1, sizeof(float)))
		== NULL)
		return -1;
    
	/* compute the covariance matrix */
	Covariance (pFResidual-1, sizeWindow, nStage, pFCovMatrix, nMax);
  
	/* get the lattice coefficients */
	for (i = 1; i <= pSmsData->nCoeff; i++)
	{
		CovLatticeHarm (pFCovMatrix, nMax, i, pFOtherCoeff, 
		                &pSmsData->pFStocCoeff[i-1], &fError, pFScratch);
		/* check for unstable filter */
		if (fabs (pSmsData->pFStocCoeff[i-1]) >= 1)
		{
			fprintf (stderr, 
			         "stocAnalysis: unstable filter, using previous one\n");
			for (j = 0; j < pSmsData->nCoeff; j++)
				pSmsData->pFStocCoeff[j] = pFOldCoeff[j];
			fError = fOldError;
			break;
		}
	}
	/* check for wrong error */
	if (fError > 0 && fError < 3.40282346638528860e+38) 
		/* get the gain from the error */
		*(pSmsData->pFStocGain) = TO_DB(sqrt(fError / sizeWindow));
	else
	{
		fprintf (stderr, "stocAnalysis: unstable filter, setting to 0\n");
		for (j = 0; j < pSmsData->nCoeff; j++)
			pSmsData->pFStocCoeff[j] = 0;
		*(pSmsData->pFStocGain) = 0;
		fError = 0;
	}
  
	/* save result in case next filter is unstable */
	for (i = 0; i < pSmsData->nCoeff; i++)
		pFOldCoeff[i] = pSmsData->pFStocCoeff[i];
	fOldError = fError;
  
	free ((char *) pFCovMatrix);
	free ((char *) pFOldCoeff);
	free ((char *) pFOtherCoeff);
	free ((char *) pFScratch);
	return (1);
}


/* get the stochastic representation from the residual waveform
 * by fitting a line segments to its magnitude spectrum
 * return -1 if no representation, 1 if got a representation
 *
 * float *pFResidual;        residual signal
 * int sizeWindow;           size of buffer
 * SMS_DATA *pSmsData;       pointer to output SMS data
 * ANAL_PARAMS analParams;   analysis parameters
 */
static int StocAnalysisFFT (float *pFResidual, int sizeWindow, 
                            SMS_DATA *pSmsData, ANAL_PARAMS analParams)
{
	int i, sizeFft = (int) pow(2.0, (float)(1+(floor(log((float) sizeWindow)
		/ LOG2)))), sizeMag = sizeFft >> 1;
	float  *pFMagSpectrum, fMag = 0;
	static float *pFWindow = NULL;

	/* allocate buffers */    
	if ((pFMagSpectrum = (float *) calloc(sizeMag, sizeof(float))) == NULL)
		return -1;

	if (pFWindow == NULL)
	{
		if ((pFWindow = (float *) calloc(sizeWindow, sizeof(float))) == NULL)
			return -1;
		Hamming (sizeWindow, pFWindow);
	}

	QuickSpectrumF (pFResidual, pFWindow, sizeWindow, pFMagSpectrum, 
	                (float *) NULL, sizeFft);

 
	SpectralApprox (pFMagSpectrum, sizeMag, sizeMag, pSmsData->pFStocCoeff, 
	                pSmsData->nCoeff, pSmsData->nCoeff);
  
	/* get energy of spectrum  */
	for (i = 0; i < sizeMag; i++)
		fMag += pFMagSpectrum[i];
 
	*pSmsData->pFStocGain = MAX (fMag / sizeMag, ENV_THRESHOLD);

	/* normalize envelope */
	for (i = 0; i <  pSmsData->nCoeff; i++)
		pSmsData->pFStocCoeff[i] /= *pSmsData->pFStocGain;
    
	*pSmsData->pFStocGain = TO_DB (*pSmsData->pFStocGain);
 
	free ((char *) pFMagSpectrum);
	return (1);
}


/* main function for the stochastic analysis
 * return -1 if no representation, 1 if got a representation
 * float *pFResidual;        residual signal
 * int sizeWindow;           size of buffer
 * SMS_DATA *pSmsData;       pointer to output SMS data
 * ANAL_PARAMS analParams;   analysis parameters
 */
int StocAnalysis (float *pFResidual, int sizeWindow, 
                  SMS_DATA *pSmsData, ANAL_PARAMS analParams)
{
	int iError = 1;
	if (analParams.iStochasticType == STOC_WAVEFORM)
        {
                memcpy( pSmsData->pFStocAudio, pFResidual, sizeof(float) * analParams.sizeHop);
                //TODO: compare input/output window sizes and add samples if necessary
        }
        else if (analParams.iStochasticType == STOC_STFT) 
        {
                //TODO: realft and store real/imag pairs in sms file
        }
	else if (analParams.iStochasticType == STOC_APPROX)
		iError = 
			StocAnalysisFFT (pFResidual, sizeWindow, pSmsData, analParams);
	else
		return -1;

	return iError;
}

