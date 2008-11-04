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
/*! \file stocAnalysis.c
 * \brief stochastic analysis using spectral analysis and approximation
 */
#include "sms.h"

#define ENV_THRESHOLD     .01

/*! \brief main function for the stochastic analysis
 * \param pFResidual      pointer to residual signal
 * \param sizeBuffer         size of buffer
 * \param pSmsData        pointer to output SMS data
 * \param pAnalParams   pointer to structure of analysis parameters
 * \return -1 if no representation, 1 if got a representation
 */
int sms_stocAnalysis (float *pFResidual, int sizeBuffer, 
                      SMS_Data *pSmsData, SMS_AnalParams *pAnalParams)
{
	int iError = 1;
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
                sms_getWindow(sizeBuffer, pFWindow, SMS_WIN_HAMMING);
#ifdef FFTW
                /* \todo memory leak here.. */
                static SMS_Fourier fftData;
                fftData.pWaveform = fftwf_malloc(sizeof(float) * sizeFft);
                fftData.pSpectrum = fftwf_malloc(sizeof(fftwf_complex) * (sizeFft / 2 + 1));
                fftData.plan =  fftwf_plan_dft_r2c_1d( sizeFft, fftData.pWaveform,
                                                       fftData.pSpectrum, FFTW_ESTIMATE);
#endif // FFTW
        }

        sms_quickSpectrum (pFResidual, pFWindow, sizeBuffer, pFMagSpectrum, 
                           (float *) NULL, sizeFft);
 
        sms_spectralApprox (pFMagSpectrum, sizeMag, sizeMag, pSmsData->pFStocCoeff, 
                            pSmsData->nCoeff, pSmsData->nCoeff);
  
        /* get energy of spectrum  */
        for (i = 0; i < sizeMag; i++)
                fMag += pFMagSpectrum[i];
 
        *pSmsData->pFStocGain = MAX (fMag / sizeMag, ENV_THRESHOLD);

        /* normalize envelope */
        for (i = 0; i <  pSmsData->nCoeff; i++)
                pSmsData->pFStocCoeff[i] /= *pSmsData->pFStocGain;
    
        *pSmsData->pFStocGain = sms_magToDB (*pSmsData->pFStocGain);
        free ((char *) pFMagSpectrum);
        return (1);

	return iError;
}

