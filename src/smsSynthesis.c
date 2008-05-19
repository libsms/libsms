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


static int SineSynthIFFT (SMS_DATA *pSmsData, float *pFBuffer, 
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

//        pFFftBuffer = (float *) calloc(sizeFft+1, sizeof(float));
        memset (pSynthParams->realftOut, 0, (sizeFft +1) * sizeof(float));
        for (i = 0; i < nTraj; i++)
        {
                if (((fMag = pSmsData->pFMagTraj[i]) > 0) &&
                    ((fFreq = pSmsData->pFFreqTraj[i]) < iHalfSamplingRate))
                {
                        if (pSynthParams->previousFrame.pFMagTraj[i] <= 0)
                                pSynthParams->previousFrame.pFPhaTraj[i] = 
                                        TWO_PI * ((random() - HALF_MAX) / HALF_MAX);
                        // can fMag here be stored as magnitude instead of DB within smsData?
                        fMag = TO_MAG (fMag);
                        fTmp = pSynthParams->previousFrame.pFPhaTraj[i] +
                                (TWO_PI * fFreq / pSynthParams->iSamplingRate) * sizeMag;
                        fPhase = fTmp - floor(fTmp / TWO_PI) * TWO_PI;
                        fLoc = sizeFft * fFreq  / pSynthParams->iSamplingRate;
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


        // fill complex fftw buffer:
        // pSynthParams->realftOut[i*2] is complex (sin), while fftw_complex[i][0] is real.
        // so they have to be switched -- I think this is backwards, actually... ARG
        for(i = 0; i < (sizeMag/2 +1); i++)
        {
                pSynthParams->pCfftIn[i][0] = pSynthParams->realftOut[i*2];
                pSynthParams->pCfftIn[i][1] = pSynthParams->realftOut[i*2+1];
        }
        fftwf_execute(pSynthParams->fftPlan);

        //////// RTE DEBUG ////////////////////////////////////////////////////////
/*         if( pSynthParams->realftOut[0] != 0  && fc++ < 10 ) */
/*         { */
/*                 ff = 1; */
/* /\*                 printf("fftw (size %d):\n", (int) sizeMag); *\/ */
/* /\*                 int ii; *\/ */
/* /\*                 for(ii = 0; ii < sizeMag; ii++) *\/ */
/* /\*                 { *\/ */
/* /\*                         printf("#%d: %f, ", ii, pSynthParams->pFfftOut[ii]); *\/ */
/* /\*                 } *\/ */
/* /\*                 printf("\n"); *\/ */

/*                 printf("realft  (size %d):\n", (int) sizeMag); */

/*                 for(i = 0; i < sizeMag; i++) */
/*                 { */
/*                         printf("#%d: %f, ", (int) i, pSynthParams->realftOut[i]); */
/*                 } */
/*                 printf("\n"); */

/*         } */
        /////////////////////////////////////////////////////////////////////////////////



        realft (pSynthParams->realftOut-1, sizeMag, -1);

        if( ff )
        {
                printf("realft  (after):\n");

                for(i = 0; i < sizeMag; i++)
                {
                        printf("#%d: %f, ", (int) i, pSynthParams->realftOut[i]);
                }
                printf("\n");


        }

#ifdef FFTW//not right yet... 
        {
                //realft needs to be multiplied by 2/n, which is apparently done elsewhere.
                //... so does this need to be mulitplied by 0.5 because fftw has gain of 1/n ?
                // well since it is about 300x too big in amp, it isn't the basis of the problem
                for(i = 0, k = sizeMag; i < sizeMag; i++, k++) 
                        pFBuffer[i] += pSynthParams->pFfftOut[k] * pSynthParams->pFDetWindow[i];
                for(i= sizeMag, k = 0; i < sizeFft; i++, k++) 
                        pFBuffer[i] +=  pSynthParams->pFfftOut[k] * pSynthParams->pFDetWindow[i];
        }
#else 
        {
            
                
                for(i = 0, k = sizeMag; i < sizeMag; i++, k++) 
                        pFBuffer[i] += pSynthParams->realftOut[k] * pSynthParams->pFDetWindow[i];
                for(i= sizeMag, k = 0; i < sizeFft; i++, k++) 
                        pFBuffer[i] +=  pSynthParams->realftOut[k] * pSynthParams->pFDetWindow[i];

        }
#endif

        return (1); 
}


/* synthesis of one frame of the stochastic component using spectral envelopes */
static int StocSynthIFFT (SMS_DATA *pSmsData, float *pFBuffer, 
                          SYNTH_PARAMS *pSynthParams)
{
        float *pFMagSpectrum, *pFPhaseSpectrum;
        int i, nSegments = pSmsData->nCoeff, nSegmentsUsed;
        int sizeFft = pSynthParams->sizeHop << 1, sizeMag = pSynthParams->sizeHop;

        /* if no gain or no coefficients return */
        if (*(pSmsData->pFStocGain) <= 0)
                return 0;

        if ((pFMagSpectrum = (float *) calloc(sizeMag, sizeof(float))) == NULL)
                return -1;
        if ((pFPhaseSpectrum = (float *) calloc(sizeMag, sizeof(float))) == NULL)
                return -1;
        *(pSmsData->pFStocGain) = TO_MAG(*(pSmsData->pFStocGain));

        /* scale the coefficients to normal amplitude */
        for (i = 0; i < nSegments; i++)
                pSmsData->pFStocCoeff[i] *= 2 * *(pSmsData->pFStocGain);

        nSegmentsUsed = nSegments * pSynthParams->iSamplingRate / 
                pSynthParams->iOriginalSRate;
        SpectralApprox (pSmsData->pFStocCoeff, nSegments, nSegmentsUsed,
                        pFMagSpectrum, sizeMag, nSegmentsUsed);

        /* generate random phases */
        for (i = 0; i < sizeMag; i++)
                pFPhaseSpectrum[i] =  TWO_PI * ((random() - HALF_MAX) / HALF_MAX);

        InverseQuickSpectrumW (pFMagSpectrum, pFPhaseSpectrum, 
                               sizeFft, pFBuffer, sizeFft, 
                               pSynthParams->pFStocWindow);
        free (pFMagSpectrum);
        free (pFPhaseSpectrum);
        return 1;
}

/* synthesizes one frame of SMS data
 *
 * SMS_DATA *pSmsData;      input SMS data
 * short *pSSynthesis;      output sound buffer  RTE: switching to float
 * SYNTH_PARAMS *pSynthParams;   synthesis parameters
 */
int SmsSynthesis (SMS_DATA *pSmsData, float *pFSynthesis, 
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
        
        /* synthesize stochastic component */
        if (pSynthParams->iSynthesisType != 1)
        {
                if(pSynthParams->iStochasticType == STOC_WAVEFORM)
                {
                        //copy stocWave to pFSynthesis
                        for(i = 0, j = 0; i < sizeHop; i++, j++)
                        {
                                if(j >= pSmsData->nSamples) j = 0;
                                pFBuffer[i] += pSmsData->pFStocWave[j];
                        }
                }
                else if(pSynthParams->iStochasticType == STOC_STFT)
                {
                        //do ifft 
                }
                else if(pSynthParams->iStochasticType == STOC_APPROX)
                {
                        StocSynthIFFT (pSmsData, pFBuffer, pSynthParams);
                }
        }
        /* synthesize deterministic component */
        if (pSynthParams->iSynthesisType != 2)
        {
                SineSynthIFFT (pSmsData, pFBuffer, pSynthParams);
        }
     
        /* de-emphasize the sound */
        for(i = 0; i < sizeHop; i++)
                pFSynthesis[i] = SHORT_TO_FLOAT * DeEmphasis(pFBuffer[i]);

        return (1);
}

