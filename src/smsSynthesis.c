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
/* synthesis of one frame of the deterministic component using the IFFT */

/* //########## RTE DEBUG ############### */
/* FILE *debugFile; */
int fc = 0, ff;
float max, min;
/* // ################################### */


static int SineSynthIFFT (SMS_Data *pSmsData, float *pFBuffer, 
                          SYNTH_PARAMS *pSynthParams)
{
        long sizeFft = pSynthParams->sizeHop << 1; 
        long iHalfSamplingRate = pSynthParams->iSamplingRate >> 1;
        long sizeMag = pSynthParams->sizeHop;
        long nBins = 8;
        long  nTraj = pSmsData->nTraj;
        long iFirstBin, k, i, l, b;
        float fMag=0, fFreq=0, fPhase=0, fLoc, fSin, fCos, fBinRemainder, 
                fTmp, fNewMag,  fIndex;
        float fSamplingPeriod = 1.0 / pSynthParams->iSamplingRate;
#ifdef FFTW

        // fftw buffers need to memset after the first frame
        memset (pSynthParams->pSpectrum, 0, (sizeMag + 1) * sizeof(fftwf_complex));
//        memset (pSynthParams->pWaveform, 0, sizeFft * sizeof(float));

        for (i = 0; i < nTraj; i++)
        {
                if (((fMag = pSmsData->pFMagTraj[i]) > 0) &&
                    ((fFreq = pSmsData->pFFreqTraj[i]) < iHalfSamplingRate))
                {
                        if (pSynthParams->previousFrame.pFMagTraj[i] <= 0)
                        {
                                pSynthParams->previousFrame.pFPhaTraj[i] = 
                                        TWO_PI * ((random() - HALF_MAX) / HALF_MAX);
                        }
                        // can fMag here be stored as magnitude instead of DB within smsData?
                        fMag = TO_MAG (fMag);
                        fTmp = pSynthParams->previousFrame.pFPhaTraj[i] +
                                TWO_PI * fFreq * fSamplingPeriod * sizeMag;
                        fPhase = fTmp - floor(fTmp / TWO_PI) * TWO_PI;
                        fLoc = sizeFft * fFreq  * fSamplingPeriod;
                        iFirstBin = (int) fLoc - 3;
                        fBinRemainder = fLoc - floor (fLoc);
                        fSin = SinTab (fPhase);
                        fCos = SinTab (fPhase + PI_2);
                        for (k = 1, l = iFirstBin; k <= nBins; k++, l++)
                        {
                                fIndex = (k - fBinRemainder);
                                if (fIndex > 7.999) fIndex = 0;
                                fNewMag = fMag * SincTab (fIndex);
                                if (l > 0 && l < sizeMag)
                                {
                                        pSynthParams->pSpectrum[l][0] += fNewMag * fCos;
                                        pSynthParams->pSpectrum[l][1] += fNewMag * fSin;
                                }
                                else if (l == 0) /* DC component - purely real*/
                                {
                                        /*pSynthParams->pSpectrum[l][1] += 2 * fNewMag * fSin;*/
                                        pSynthParams->pSpectrum[l][0] += 2 * fNewMag * fCos;
                                }
                                else if (l < 0)
                                {
                                        b = abs(l);
                                        pSynthParams->pSpectrum[b][0] -= fNewMag * fCos; /* minus? */
                                        pSynthParams->pSpectrum[b][1] += fNewMag * fSin;
                                }
                                else if (l > sizeMag)
                                {
                                        b = sizeMag - (l - sizeMag);
                                        pSynthParams->pSpectrum[b][0] -= fNewMag * fCos; /* minus again..? */
                                        pSynthParams->pSpectrum[b][1] += fNewMag * fSin;
                                }
                                else if (l == sizeMag) /* Nyquist Component - purely real*/
                                {
                                        /*pSynthParams->pSpectrum[l][1] += 2 * fNewMag * fSin;*/
                                        pSynthParams->pSpectrum[l][0] += 2 * fNewMag * fCos;
                                }
                        }
                }
                pSynthParams->previousFrame.pFMagTraj[i] = fMag;
                pSynthParams->previousFrame.pFPhaTraj[i] = fPhase;
                pSynthParams->previousFrame.pFFreqTraj[i] = fFreq;
        }

        fftwf_execute(pSynthParams->fftPlan);

        for(i = 0, k = sizeMag; i < sizeMag; i++, k++)
                pFBuffer[i] += pSynthParams->pWaveform[k] * pSynthParams->pFDetWindow[i] * 0.5;
        for(i= sizeMag, k = 0; i < sizeFft; i++, k++)
                pFBuffer[i] +=  pSynthParams->pWaveform[k] * pSynthParams->pFDetWindow[i] * 0.5;

#else //using realft

        memset (pSynthParams->realftOut, 0, (sizeFft +1) * sizeof(float));
        for (i = 0; i < nTraj; i++)
        {
                if (((fMag = pSmsData->pFMagTraj[i]) > 0) &&
                    ((fFreq = pSmsData->pFFreqTraj[i]) < iHalfSamplingRate))
                {
                        if (pSynthParams->previousFrame.pFMagTraj[i] <= 0)
                        {
                                pSynthParams->previousFrame.pFPhaTraj[i] = 
                                        TWO_PI * ((random() - HALF_MAX) / HALF_MAX);
                        }
                        // can fMag here be stored as magnitude instead of DB within smsData?
                        fMag = TO_MAG (fMag);
                        fTmp = pSynthParams->previousFrame.pFPhaTraj[i] +
                                TWO_PI * fFreq * fSamplingPeriod * sizeMag;
                        fPhase = fTmp - floor(fTmp / TWO_PI) * TWO_PI;
                        fLoc = sizeFft * fFreq  * fSamplingPeriod;
                        iFirstBin = (int) fLoc - 3;
                        fBinRemainder = fLoc - floor (fLoc);
                        fSin = SinTab (fPhase);
                        fCos = SinTab (fPhase + PI_2);
                        for (k = 1, l = iFirstBin; k <= nBins; k++, l++)
                        {
                                fIndex = (k - fBinRemainder);
                                if (fIndex > 7.999) fIndex = 0;
                                fNewMag = fMag * SincTab (fIndex);
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
                pSynthParams->previousFrame.pFMagTraj[i] = fMag;
                pSynthParams->previousFrame.pFPhaTraj[i] = fPhase;
                pSynthParams->previousFrame.pFFreqTraj[i] = fFreq;
        }

        realft (pSynthParams->realftOut-1, sizeMag, -1);

        for(i = 0, k = sizeMag; i < sizeMag; i++, k++) 
                pFBuffer[i] += pSynthParams->realftOut[k] * pSynthParams->pFDetWindow[i];
        for(i= sizeMag, k = 0; i < sizeFft; i++, k++) 
                pFBuffer[i] +=  pSynthParams->realftOut[k] * pSynthParams->pFDetWindow[i];

#endif // FFTW

        return (1); 
}

/* synthesis of one frame of the stochastic component using approximated 
 * spectral envelopes and random phases*/
static int StocSynthApprox (SMS_Data *pSmsData, float *pFBuffer, 
                          SYNTH_PARAMS *pSynthParams)
{
        float *pFMagSpectrum, *pFPhaseSpectrum;
        int i, sizeSpec1Used;
        int sizeSpec1 = pSmsData->nCoeff;
        int sizeSpec2 = pSynthParams->sizeHop;
        int sizeFft = pSynthParams->sizeHop << 1; // 50% overlap, so fft is 2x hop

        /* if no gain or no coefficients return  */
        if (*(pSmsData->pFStocGain) <= 0)
                return 0;

        if ((pFMagSpectrum = (float *) calloc(sizeSpec2, sizeof(float))) == NULL)
                return -1;
        if ((pFPhaseSpectrum = (float *) calloc(sizeSpec2, sizeof(float))) == NULL)
                return -1;
        *(pSmsData->pFStocGain) = TO_MAG(*(pSmsData->pFStocGain));

        /* scale the coefficients to normal amplitude */
        for (i = 0; i < sizeSpec1; i++)
                pSmsData->pFStocCoeff[i] *= 2 * *(pSmsData->pFStocGain);

        sizeSpec1Used = sizeSpec1 * pSynthParams->iSamplingRate / 
                pSynthParams->iOriginalSRate;
        /* sizeSpec1Used cannot be more than what is available RTE TODO check that these look right */
        if(sizeSpec1Used  > sizeSpec1) sizeSpec1Used = sizeSpec1;
        SpectralApprox (pSmsData->pFStocCoeff, sizeSpec1, sizeSpec1Used,
                        pFMagSpectrum, sizeSpec2, sizeSpec1Used);

        /* generate random phases */
        float one_over_half_max = 1.0 / HALF_MAX;
        for (i = 0; i < sizeSpec2; i++)
                pFPhaseSpectrum[i] =  TWO_PI * ((random() - HALF_MAX) * one_over_half_max);

#ifdef FFTW
        memset (pSynthParams->pSpectrum, 0, (sizeSpec2 + 1) * sizeof(fftwf_complex));

        float fPower;
        /* covert from polar to complex */
        pSynthParams->pSpectrum[0][0] = pFMagSpectrum[0] * cos (pFPhaseSpectrum[0]);
//      Nyquist component isn't generated above..
//        pSynthParams->pSpectrum[sizeSpec2][0] = 
//                pFmagSpectrum[sizeSpec2] * cos (pFPhaseSpectrum[sizeSpec20]);
     
        for (i = 1; i< sizeSpec2; i++)
	{
		fPower = pFMagSpectrum[i];
		pSynthParams->pSpectrum[i][0] =  fPower * cos (pFPhaseSpectrum[i]);   //real
		pSynthParams->pSpectrum[i][1] = fPower * sin (pFPhaseSpectrum[i]); //imaginary
	}    

        fftwf_execute(pSynthParams->fftPlan);

	for (i = 0; i < sizeFft; i++)
		pFBuffer[i] +=  pSynthParams->pWaveform[i] 
                        * pSynthParams->pFStocWindow[i] * 0.25; //.5;
 
#else        
        InverseQuickSpectrumW (pFMagSpectrum, pFPhaseSpectrum, 
                               sizeFft, pFBuffer, sizeFft, 
                               pSynthParams->pFStocWindow);
#endif



        free (pFMagSpectrum);
        free (pFPhaseSpectrum);
        return 1;
}

/* synthesizes one frame of SMS data
 *
 * SMS_Data *pSmsData;      input SMS data
 * short *pSSynthesis;      output sound buffer  RTE: switching to float
 * SYNTH_PARAMS *pSynthParams;   synthesis parameters
 */
int SmsSynthesis (SMS_Data *pSmsData, float *pFSynthesis,  
                  SYNTH_PARAMS *pSynthParams)
{
        static float *pFBuffer = NULL;
        int i, j, sizeHop = pSynthParams->sizeHop;
  
        if (pFBuffer == NULL)
        {
                if((pFBuffer = (float *) calloc(sizeHop*2, sizeof(float))) == NULL)
                        return -1;
        }
  
        memcpy ((char *) pFBuffer, (char *)(pFBuffer+sizeHop), 
                sizeof(float) * sizeHop);
        memset ((char *)(pFBuffer+sizeHop), 0, sizeof(float) * sizeHop);
        
        /* decide which combo of synthesis methods to use */
        if(pSynthParams->iSynthesisType == SMS_STYPE_ALL)
        {
                if(pSynthParams->iDetSynthType == SMS_DET_IFFT &&
                   pSynthParams->iStochasticType == SMS_STOC_IFFT)
                        quit("SmsSynthesis: COMBO_SYNTH not implemented yet.");
                else /* can't use combo STFT, synthesize seperately and sum */
                {
                        if(pSynthParams->iDetSynthType == SMS_DET_IFFT)
                        {
                                SineSynthIFFT (pSmsData, pFBuffer, pSynthParams);
                        }
                        else /*pSynthParams->iDetSynthType == SMS_DET_SIN*/
                        {
                                FrameSineSynth (pSmsData, pFBuffer, pSynthParams->sizeHop,
                                                &(pSynthParams->previousFrame), pSynthParams->iSamplingRate);
                        }
                        if(pSynthParams->iStochasticType == SMS_STOC_WAVE)
                        {
                                //copy stocWave to pFSynthesis
                                for(i = 0, j = 0; i < sizeHop; i++, j++)
                                {
                                        if(j >= pSmsData->nSamples) j = 0;
                                        pFBuffer[i] += pSmsData->pFStocWave[j];
                                }
//                                printf("3\n");

                        }
                        else if(pSynthParams->iStochasticType == SMS_STOC_IFFT)
                        {
                                quit("SmsSynthesis: SMS_STOC_IFFT not implemented yet.");
                        }
                        else if(pSynthParams->iStochasticType == SMS_STOC_APPROX)
                        {
                                StocSynthApprox (pSmsData, pFBuffer, pSynthParams);
//                              printf("4\n");
                        }
                }

                
        }
        else if(pSynthParams->iSynthesisType == SMS_STYPE_DET)
        {
                if(pSynthParams->iDetSynthType == SMS_DET_IFFT)
                {
                        SineSynthIFFT (pSmsData, pFBuffer, pSynthParams);
//                        printf("5\n");
                }
                else /*pSynthParams->iDetSynthType == SMS_DET_SIN*/
                {
                        FrameSineSynth (pSmsData, pFBuffer, pSynthParams->sizeHop,
                                        &(pSynthParams->previousFrame), pSynthParams->iSamplingRate);
//                        printf("6\n");
                }
        }
        else /* pSynthParams->iSynthesisType == SMS_STYPE_STOC */
        {
                if(pSynthParams->iStochasticType == SMS_STOC_WAVE)
                {
                        //copy stocWave to pFSynthesis
                        for(i = 0, j = 0; i < sizeHop; i++, j++)
                        {
                                if(j >= pSmsData->nSamples) j = 0;
                                pFBuffer[i] += pSmsData->pFStocWave[j];
                        }
//                        printf("7\n");
                }
                else if(pSynthParams->iStochasticType == SMS_STOC_IFFT)
                {
                        quit("SmsSynthesis: SMS_STOC_IFFT not implemented yet.");
                }
                else /*pSynthParams->iStochasticType == SMS_STOC_APPROX*/
                {
                        StocSynthApprox(pSmsData, pFBuffer, pSynthParams);
//                        printf("8\n");
                        
                }
        }
     
        /* de-emphasize the sound and normalize*/
        for(i = 0; i < sizeHop; i++)
                pFSynthesis[i] = SHORT_TO_FLOAT * DeEmphasis(pFBuffer[i]);

        return (1);
}

