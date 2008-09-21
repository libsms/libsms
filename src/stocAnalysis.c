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
 * by fitting a line segments to its magnitude spectrum
 * return -1 if no representation, 1 if got a representation
 *
 * float *pFResidual;        residual signal
 * int sizeBuffer;           size of buffer
 * SMS_Data *pSmsData;       pointer to output SMS data
 * SMS_AnalParams *pAnalParams;   analysis parameters
 */
static int StocApproxFFT (float *pFResidual, int sizeBuffer, 
                            SMS_Data *pSmsData, SMS_AnalParams *pAnalParams)
{
	int i;
        int sizeFft = (int) pow(2.0, (float)(1+(floor(log((float) sizeBuffer) / LOG2))));
        int sizeMag = sizeFft >> 1;
	float  *pFMagSpectrum;
        float fMag = 0.0;
	static float *pFWindow = NULL;

	/* allocate buffers */    
	if ((pFMagSpectrum = (float *) calloc(sizeMag, sizeof(float))) == NULL)
		return -1;

	if (pFWindow == NULL)
	{
		if ((pFWindow = (float *) calloc(sizeBuffer, sizeof(float))) == NULL)
			return -1;
		//Hamming (sizeBuffer, pFWindow);
                sms_getWindow(sizeBuffer, pFWindow, SMS_WIN_HAMMING);
	}
//        printf("sizeFft: %d, sizeBuffer: %d \n", sizeFft, sizeBuffer);
	sms_quickSpectrum (pFResidual, pFWindow, sizeBuffer, pFMagSpectrum, 
                           (float *) NULL, sizeFft, pAnalParams);

 
	sms_spectralApprox (pFMagSpectrum, sizeMag, sizeMag, pSmsData->pFStocCoeff, 
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
 * int sizeBuffer;           size of buffer
 * SMS_Data *pSmsData;       pointer to output SMS data
 * SMS_AnalParams *pAnalParams;   analysis parameters
 */
int sms_stocAnalysis (float *pFResidual, int sizeBuffer, 
                  SMS_Data *pSmsData, SMS_AnalParams *pAnalParams)
{
	int iError = 1;
	if (pAnalParams->iStochasticType == SMS_STOC_WAVE)
        {
              memcpy( pSmsData->pFStocWave, pFResidual, sizeof(float) * pAnalParams->sizeHop);
        }
        else if (pAnalParams->iStochasticType == SMS_STOC_IFFT) 
        {
                //TODO: computer spectrum and store in pSmsData
                // how large is FFT?
        }
	else if (pAnalParams->iStochasticType == SMS_STOC_APPROX)
		iError = 
			StocApproxFFT (pFResidual, sizeBuffer, pSmsData, pAnalParams);
	else
		return -1;

	return iError;
}

