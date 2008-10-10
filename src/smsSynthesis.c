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
/*! \file smsSynthesis.c
 * \brief main synthesis routines
 */
#include "sms.h"

/*! \brief synthesis of one frame of the deterministic component using the IFFT
 *
 * \param pSmsData pointer to SMS data structure frame
 * \param pFBuffer pointer to audio buffer
 * \param pSynthParams pointer to structure of synthesis parameters
 */
static void SineSynthIFFT (SMS_Data *pSmsData, float *pFBuffer, 
                          SMS_SynthParams *pSynthParams)
{
        int sizeFft = pSynthParams->sizeHop << 1; 
        int iHalfSamplingRate = pSynthParams->iSamplingRate >> 1;
        int sizeMag = pSynthParams->sizeHop;
        int nBins = 8;
        int nTracks = pSmsData->nTracks;
        int iFirstBin, k, i, l, b;
        float fMag=0.0, fFreq=0.0, fPhase=0.0, fLoc, fSin, fCos, fBinRemainder, 
                fTmp, fNewMag,  fIndex;
        float fSamplingPeriod = 1.0 / pSynthParams->iSamplingRate;


#ifdef FFTW
        memset (pSynthParams->fftw.pSpectrum, 0, (sizeMag + 1) * sizeof(fftwf_complex));

        for (i = 0; i < nTracks; i++)
        {
                if (((fMag = pSmsData->pFSinAmp[i]) > 0) &&
                    ((fFreq = (pSmsData->pFSinFreq[i]) * pSynthParams->fTranspose) < iHalfSamplingRate))
                {
                        if (pSynthParams->prevFrame.pFSinAmp[i] <= 0)
                        {
                                pSynthParams->prevFrame.pFSinPha[i] = 
                                        TWO_PI * sms_random();
                        }
                        /* magnitude is stored in dB because most manipulations of the model need to be
                           performed in dB, not magnitude, and manipulations drastically increase computation time */
                        fMag = sms_dBToMag (fMag);
                        fTmp = pSynthParams->prevFrame.pFSinPha[i] +
                                TWO_PI * fFreq * fSamplingPeriod * sizeMag;
                        fPhase = fTmp - floor(fTmp / TWO_PI) * TWO_PI;
                        fLoc = sizeFft * fFreq  * fSamplingPeriod;
                        iFirstBin = (int) fLoc - 3;
                        fBinRemainder = fLoc - floor (fLoc);
                        fSin = sms_sine (fPhase);
                        fCos = sms_sine (fPhase + PI_2);
                        for (k = 1, l = iFirstBin; k <= nBins; k++, l++)
                        {
                                fIndex = (k - fBinRemainder);
                                if (fIndex > 7.999) fIndex = 0;
                                fNewMag = fMag * sms_sinc (fIndex);
                                if (l > 0 && l < sizeMag)
                                {
                                        pSynthParams->fftw.pSpectrum[l][0] += fNewMag * fCos;
                                        pSynthParams->fftw.pSpectrum[l][1] += fNewMag * fSin;
                                }
                                else if (l == 0) /* DC component - purely real*/
                                        pSynthParams->fftw.pSpectrum[l][0] += 2 * fNewMag * fCos;
                                else if (l < 0)
                                {
                                        b = abs(l);
                                        pSynthParams->fftw.pSpectrum[b][0] -= fNewMag * fCos; /* minus? */
                                        pSynthParams->fftw.pSpectrum[b][1] += fNewMag * fSin;
                                }
                                else if (l > sizeMag)
                                {
                                        b = sizeMag - (l - sizeMag);
                                        pSynthParams->fftw.pSpectrum[b][0] -= fNewMag * fCos; /* minus again..? */
                                        pSynthParams->fftw.pSpectrum[b][1] += fNewMag * fSin;
                                }
                                else if (l == sizeMag) /* Nyquist Component - purely real*/
                                        pSynthParams->fftw.pSpectrum[l][0] += 2 * fNewMag * fCos;
                        }
                }
                pSynthParams->prevFrame.pFSinAmp[i] = fMag;
                pSynthParams->prevFrame.pFSinPha[i] = fPhase;
                pSynthParams->prevFrame.pFSinFreq[i] = fFreq;
        }

        fftwf_execute(pSynthParams->fftw.plan);

        for(i = 0, k = sizeMag; i < sizeMag; i++, k++)
                pFBuffer[i] += pSynthParams->fftw.pWaveform[k] * pSynthParams->pFDetWindow[i] * 0.5;
        for(i= sizeMag, k = 0; i < sizeFft; i++, k++)
                pFBuffer[i] +=  pSynthParams->fftw.pWaveform[k] * pSynthParams->pFDetWindow[i] * 0.5;

#else //using realft (or OOURA)

        memset (pSynthParams->realftOut, 0, (sizeFft +1) * sizeof(float));
        for (i = 0; i < nTracks; i++)
        {
                if (((fMag = pSmsData->pFSinAmp[i]) > 0) &&
                    ((fFreq = (pSmsData->pFSinFreq[i]) * pSynthParams->fTranspose) < iHalfSamplingRate))
                {
                        if (pSynthParams->prevFrame.pFSinAmp[i] <= 0)
                                pSynthParams->prevFrame.pFSinPha[i] = TWO_PI * sms_random();

                        fMag = sms_dBToMag (fMag);
                        fTmp = pSynthParams->prevFrame.pFSinPha[i] +
                                TWO_PI * fFreq * fSamplingPeriod * sizeMag;
                        fPhase = fTmp - floor(fTmp * INV_TWO_PI) * TWO_PI;
                        fLoc = sizeFft * fFreq  * fSamplingPeriod;
                        iFirstBin = (int) fLoc - 3;
                        fBinRemainder = fLoc - floor (fLoc);
                        fSin = sms_sine (fPhase);
                        fCos = sms_sine (fPhase + PI_2);
                        for (k = 1, l = iFirstBin; k <= nBins; k++, l++)
                        {
                                fIndex = (k - fBinRemainder);
                                if (fIndex > 7.999) fIndex = 0;
                                fNewMag = fMag * sms_sinc (fIndex);
                                if (l > 0 && l < sizeMag)
                                {
                                        pSynthParams->realftOut[l*2+1] += fNewMag * fCos;
                                        pSynthParams->realftOut[l*2] += fNewMag * fSin;
                                }
                                else if (l == 0)
                                {
                                        pSynthParams->realftOut[0] += 2 * fNewMag * fSin;
                                }
                                else if (l < 0)
                                {
                                        b = abs(l);
                                        pSynthParams->realftOut[b*2+1] -= fNewMag * fCos;
                                        pSynthParams->realftOut[b*2] += fNewMag * fSin;
                                }
                                else if (l > sizeMag)
                                {
                                        b = sizeMag - (l - sizeMag);
                                        pSynthParams->realftOut[b*2+1] -= fNewMag * fCos;
                                        pSynthParams->realftOut[b*2] += fNewMag * fSin;
                                }
                                else if (l == sizeMag)
                                {
                                        pSynthParams->realftOut[1] += 2 * fNewMag * fSin;
                                }
                        }
                }
                pSynthParams->prevFrame.pFSinAmp[i] = fMag;
                pSynthParams->prevFrame.pFSinPha[i] = fPhase;
                pSynthParams->prevFrame.pFSinFreq[i] = fFreq;
        }

        sms_rdft(sizeFft, pSynthParams->realftOut, -1);

        for(i = 0, k = sizeMag; i < sizeMag; i++, k++) 
                pFBuffer[i] += pSynthParams->realftOut[k] * pSynthParams->pFDetWindow[i];
        for(i= sizeMag, k = 0; i < sizeFft; i++, k++) 
                pFBuffer[i] +=  pSynthParams->realftOut[k] * pSynthParams->pFDetWindow[i];



#endif /* FFTW */
}

/*! \brief synthesis of one frame of the stochastic component by apprimating phases
 * 
 * computes a linearly interpolated spectral envelope to fit the correct number of output
 * audio samples. Phases are generated randomly.  
 *
 * \param pSmsData pointer to the current SMS frame
 * \param pFBuffer pointer to the audio buffer
 * \param pSynthParams pointer to a strucure of synthesis parameters
 * \return 0 if no stochastic gain, -1 if calloc error, 1 if finished successfully
 * \todo cleanup returns and various constant multipliers. check that approximation is ok
 */
static int StocSynthApprox (SMS_Data *pSmsData, float *pFBuffer, 
                          SMS_SynthParams *pSynthParams)
{
        float *pFMagSpectrum, *pFPhaseSpectrum;
        int i, sizeSpec1Used;
        int sizeSpec1 = pSmsData->nCoeff;
        int sizeSpec2 = pSynthParams->sizeHop;
        int sizeFft = pSynthParams->sizeHop << 1; /* 50% overlap, so sizeFft is 2x sizeHop */

        /* if no gain or no coefficients return  */
        if (*(pSmsData->pFStocGain) <= 0)
                return 0;

        if ((pFMagSpectrum = (float *) calloc(sizeSpec2, sizeof(float))) == NULL)
                return -1;
        if ((pFPhaseSpectrum = (float *) calloc(sizeSpec2, sizeof(float))) == NULL)
                return -1;
        *(pSmsData->pFStocGain) = sms_dBToMag(*(pSmsData->pFStocGain));

        /* scale the coefficients to normal amplitude */
        /*! \todo why is it also multiplied by 2? */
        for (i = 0; i < sizeSpec1; i++)
                pSmsData->pFStocCoeff[i] *= 2 * *(pSmsData->pFStocGain) ;

        sizeSpec1Used = sizeSpec1 * pSynthParams->iSamplingRate / 
                pSynthParams->iOriginalSRate;
        /* sizeSpec1Used cannot be more than what is available  \todo check by graph */
        if(sizeSpec1Used  > sizeSpec1) sizeSpec1Used = sizeSpec1;
        sms_spectralApprox (pSmsData->pFStocCoeff, sizeSpec1, sizeSpec1Used,
                        pFMagSpectrum, sizeSpec2, sizeSpec1Used);

        /* generate random phases */
        //float one_over_half_max = 1.0 / HALF_MAX;
        for (i = 0; i < sizeSpec2; i++)
                pFPhaseSpectrum[i] =  TWO_PI * sms_random();

#ifdef FFTW
        memset (pSynthParams->fftw.pSpectrum, 0, (sizeSpec2 + 1) * sizeof(fftwf_complex));

        float fPower;
        /* covert from polar to complex */
        pSynthParams->fftw.pSpectrum[0][0] = pFMagSpectrum[0] * cos (pFPhaseSpectrum[0]);
        /* \todo nyquist component was not computed in the old code.. is it unnecessary? */
        pSynthParams->fftw.pSpectrum[sizeSpec2][0] =
                pFMagSpectrum[sizeSpec2] * cos (pFPhaseSpectrum[sizeSpec2]);
     
        for (i = 1; i< sizeSpec2; i++)
	{
		fPower = pFMagSpectrum[i];
		pSynthParams->fftw.pSpectrum[i][0] =  fPower * cos (pFPhaseSpectrum[i]);
		pSynthParams->fftw.pSpectrum[i][1] = fPower * sin (pFPhaseSpectrum[i]); 
	}    

        fftwf_execute(pSynthParams->fftw.plan);

        /*! \todo why muliploed by .25? */
	for (i = 0; i < sizeFft; i++)
		pFBuffer[i] +=  pSynthParams->fftw.pWaveform[i] 
                        * pSynthParams->pFStocWindow[i] * 0.25 * pSynthParams->fStocGain; //.5;
 
#else /* using realft */        

        /* adjust gain */
        for( i = 1; i < sizeSpec2; i++)
                pFMagSpectrum[i] *= pSynthParams->fStocGain;


        sms_invQuickSpectrumW (pFMagSpectrum, pFPhaseSpectrum, 
                               sizeFft, pFBuffer, sizeFft, 
                               pSynthParams->pFStocWindow);
#endif

        free (pFMagSpectrum);
        free (pFPhaseSpectrum);
        return 1;
}

/*! \brief  synthesizes one frame of SMS data
 *
 * \param pSmsData      input SMS data
 * \param pFSynthesis      output sound buffer  
 * \param pSynthParams   synthesis parameters
 * \return -1 if failed (malloc or bad synthesis parameters) or 1 if success
 */
int sms_synthesize (SMS_Data *pSmsData, float *pFSynthesis,  
                  SMS_SynthParams *pSynthParams)
{
        static float *pFBuffer = NULL;
        int i;
        int sizeHop = pSynthParams->sizeHop;
  
        if (pFBuffer == NULL)
        {
                if((pFBuffer = (float *) calloc(sizeHop*2, sizeof(float))) == NULL)
                        return -1;
        }
  
        memcpy ( pFBuffer, (float *)(pFBuffer+sizeHop), 
                sizeof(float) * sizeHop);
        memset (pFBuffer+sizeHop, 0, sizeof(float) * sizeHop);

        // RTE DEBUG //////////////////
/*         static int frame = 0; */
/*         static float max = 0; */
/*         int sizeFft = pSynthParams->sizeHop << 1; /\* 50% overlap, so sizeFft is 2x sizeHop *\/ */
/*         printf("~~FRAME #%d:  \n", frame++); */
/*         for(i = 0; i < sizeFft; i++) */
/*         { */
/*                 //printf("%f, ", pFBuffer[i]); */
/*                 if(pFBuffer[i] > max) */
/*                 { */
/*                         max = pFBuffer[i]; */
/*                         printf("new max: %f  -- ", max); */
/*                 }; */
/*         } */
/*         printf("\n"); */
        /////////////////////////////
        
        /* decide which combo of synthesis methods to use */
        if(pSynthParams->iSynthesisType == SMS_STYPE_ALL)
        {
                if(pSynthParams->iDetSynthType == SMS_DET_IFFT &&
                   pSynthParams->iStochasticType == SMS_STOC_IFFT)
                {
                        printf("sms_synthesize: COMBO_SYNTH not implemented yet.");
                        return (-1);
                }
                else /* can't use combo STFT, synthesize seperately and sum */
                {
                        if(pSynthParams->iDetSynthType == SMS_DET_IFFT)
                                SineSynthIFFT (pSmsData, pFBuffer, pSynthParams);
                        else /*pSynthParams->iDetSynthType == SMS_DET_SIN*/
                        {
                                sms_sineSynthFrame (pSmsData, pFBuffer, pSynthParams->sizeHop,
                                                    &(pSynthParams->prevFrame), pSynthParams->iSamplingRate);
                        }
                        StocSynthApprox (pSmsData, pFBuffer, pSynthParams);
                }

        }
        else if(pSynthParams->iSynthesisType == SMS_STYPE_DET)
        {
                if(pSynthParams->iDetSynthType == SMS_DET_IFFT)
                        SineSynthIFFT (pSmsData, pFBuffer, pSynthParams);
                else /*pSynthParams->iDetSynthType == SMS_DET_SIN*/
                {
                        sms_sineSynthFrame (pSmsData, pFBuffer, pSynthParams->sizeHop,
                                        &(pSynthParams->prevFrame), pSynthParams->iSamplingRate);
                }
        }
        else /* pSynthParams->iSynthesisType == SMS_STYPE_STOC */
                StocSynthApprox(pSmsData, pFBuffer, pSynthParams);

     
        /* de-emphasize the sound and normalize*/
        for(i = 0; i < sizeHop; i++)
                pFSynthesis[i] = sms_deEmphasis(pFBuffer[i]);

        return (1);
}

