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

/*! \brief overlap factor, or the number of analysis windows that fit in one FFT */
#define SMS_OVERLAP_FACTOR 2  

float *sms_window_spec;

/* 
 * function to compute a complex spectrum from a waveform 
 * returns the size of the complex spectrum
 *              
 * float *pFWaveform;	   pointer to input waveform 
 * int sizeWindow;	           size of analysis window
 * float *pFMagSpectrum;   pointer to output magnitude spectrum 
 * float *pFPhaseSpectrum; pointer to output phase spectrum 
 * \todo document the differences between the different spectrums
 */
int sms_spectrum (float *pFWaveform, int sizeWindow, float *pFMagSpectrum, 
              float *pFPhaseSpectrum, SMS_AnalParams *pAnalParams)
{
        /* sizeFft is a power of 2 that is greater than 2x sizeWindow */
        int sizeFft = (int) pow (2.0, 
		          (float)(1 + (floor (log ((float)(SMS_OVERLAP_FACTOR * 
		                                           sizeWindow)) / LOG2))));
	int sizeMag = sizeFft >> 1;
        int iMiddleWindow = (sizeWindow+1) >> 1; 
        int i, iOffset;
        float fReal, fImag;
	static int iOldSizeWindow = 0;

  
#ifdef FFTW


	/* compute window when necessary */
	if (iOldSizeWindow != sizeWindow) 
        {
                sms_getWindow (sizeWindow, sms_window_spec, pAnalParams->iWindowType);
                
                // is this the right size fft?
                if((pAnalParams->fftPlan =  fftwf_plan_dft_r2c_1d( sizeFft, pAnalParams->pWaveform,
                                                                   pAnalParams->pSpectrum, FFTW_ESTIMATE)) == NULL)

                {
                        printf("Spectrum: could not make fftw plan of size: %d \n", sizeFft);
                        exit(1);
                }
        }
	iOldSizeWindow = sizeWindow;

        memset(pAnalParams->pWaveform, 0, sizeFft * sizeof(float));
        memset(pAnalParams->pSpectrum, 0, (sizeMag + 1) * sizeof(fftwf_complex));

/*         printf(" sizeWindow: %d, iMiddleWindow: %d, sizeFft: %d, sizeMag: %d \n", sizeWindow, iMiddleWindow, */
/*                sizeFft, sizeMag ); */

	/* apply window to waveform and center window around 0 */
        // removed 1 offset when writing windowed waveform
	iOffset = sizeFft - (iMiddleWindow - 1);
	for (i=0; i<iMiddleWindow-1; i++)
		pAnalParams->pWaveform[iOffset + i] =  sms_window_spec[i] * pFWaveform[i];
	iOffset = iMiddleWindow - 1;
	for (i=0; i<iMiddleWindow; i++)
		pAnalParams->pWaveform[i] = sms_window_spec[iOffset + i] * pFWaveform[iOffset + i];

        /****** RTE DEBUG *********/
/*         printf("\nBEFORE fftw::::::::::::: \n"); */
/*         for(i = 0; i < sizeFft; i++) */
/*                 printf("[%d]%f, ", i, pAnalParams->pWaveform[i]); */
        /***************************/
  
        fftwf_execute(pAnalParams->fftPlan);

        /****** RTE DEBUG *********/
/*         printf("\nAFTER fftw::::::::::::: \n"); */
/*         for(i = 0; i < sizeMag+1; i++) */
/*                 printf("[%d](%f, %f),  ", i, pAnalParams->pSpectrum[i][0], */
/*                        pAnalParams->pSpectrum[i][1]); */
        /***************************/

	/* convert from rectangular to polar coordinates */
	for (i = 1; i < sizeMag; i++) 
	{
		fReal = pAnalParams->pSpectrum[i][0];
		fImag = pAnalParams->pSpectrum[i][1];
      
		if (fReal != 0 || fImag != 0)
		{
			pFMagSpectrum[i] = TO_DB (sqrt (fReal * fReal + fImag * fImag));
			pFPhaseSpectrum[i] = atan2 (-fImag, fReal);
		}
	}
        /*rte: DC and Nyquist combine to make pFMagSpectrum[0]? */
        fReal = pAnalParams->pSpectrum[0][0];
        fImag = pAnalParams->pSpectrum[sizeMag][0];
        pFMagSpectrum[0] = TO_DB (sqrt (fReal * fReal + fImag * fImag));
        pFPhaseSpectrum[0] = atan2(-fImag, fReal);

#else /* using realft() */        
	if (iOldSizeWindow != sizeWindow) 
        {
                sms_getWindow (sizeWindow, sms_window_spec, pAnalParams->iWindowType);
        }
	iOldSizeWindow = sizeWindow;

        int it2;
        float *pFBuffer;
	/* allocate buffer */    
	if ((pFBuffer = (float *) calloc(sizeFft+1, sizeof(float))) == NULL)
		return -1;

/*         printf(" sizeWindow: %d, iMiddleWindow: %d, sizeFft: %d, sizeMag: %d \n", sizeWindow, iMiddleWindow, */
/*                sizeFft, sizeMag ); */
  
  
	/* apply window to waveform and center window around 0 */
	iOffset = sizeFft - (iMiddleWindow - 1);
	for (i=0; i<iMiddleWindow-1; i++)
		pFBuffer[1+(iOffset + i)] =  sms_window_spec[i] * pFWaveform[i];
	iOffset = iMiddleWindow - 1;
	for (i=0; i<iMiddleWindow; i++)
		pFBuffer[1+i] = sms_window_spec[iOffset + i] * pFWaveform[iOffset + i];
  
        /****** RTE DEBUG *********/
/*         printf("\nBEFORE realft::::::::::::: \n"); */
/*         for(i = 0; i < sizeFft+1; i++) */
/*                 printf("[%d]%f, ", i, pFBuffer[i]); */
        /***************************/
	realft (pFBuffer, sizeMag, 1);
        /****** RTE DEBUG *********/
/*         printf("\nAFTER realft::::::::::::: \n"); */
/*         for(i = 0; i < sizeFft+1; i++) */
/*                 printf("[%d]%f, ", i, pFBuffer[i]); */
        /***************************/
  
	/* convert from rectangular to polar coordinates */
	for (i = 0; i < sizeMag; i++)
	{
		it2 = i << 1;
		fReal = pFBuffer[it2+1];
		fImag = pFBuffer[it2+2];
      
		if (fReal != 0 || fImag != 0)
		{
			pFMagSpectrum[i] = TO_DB (sqrt (fReal * fReal + fImag * fImag));
			pFPhaseSpectrum[i] = atan2 (-fImag, fReal);
		}
	}
	free (pFBuffer);
#endif/*FFTW*/
        
	return (sizeMag);
}

/* 
 * function to compute a complex spectrum from a waveform 
 * returns the size of the complex spectrum
 *              
 * short *pIWaveform;	   pointer to input waveform
 * float pFWindow;	   pointer to analysis window 
 * int sizeWindow;	   size of analysis window
 * float *pFMagSpectrum;   pointer to output spectrum 
 * float *pFPhaseSpectrum; pointer to output spectrum
 * int sizeFft;		   size of FFT 
 */
/* int QuickSpectrum (short *pIWaveform, float *pFWindow, int sizeWindow,  */
/*                    float *pFMagSpectrum, float *pFPhaseSpectrum, int sizeFft) */
/* { */
/* 	int sizeMag = sizeFft >> 1, i, it2; */
/* 	float *pFBuffer, fReal, fImag; */
  
/* 	/\* allocate buffer *\/     */
/* 	if ((pFBuffer = (float *) calloc(sizeFft+1, sizeof(float))) == NULL) */
/* 		return -1; */
    
/* 	/\* apply window to waveform *\/ */
/* 	for (i = 0; i < sizeWindow; i++) */
/* 		pFBuffer[i] =  pFWindow[i] * pIWaveform[i]; */
  
/* 	/\* compute real FFT *\/ */
/* 	realft (pFBuffer-1, sizeMag, 1); */
  
/* 	/\* convert from rectangular to polar coordinates *\/ */
/* 	for (i = 0; i < sizeMag; i++) */
/* 	{ */
/* 		it2 = i << 1; */
/* 		fReal = pFBuffer[it2]; */
/* 		fImag = pFBuffer[it2+1]; */
      
/* 		if (fReal != 0 || fImag != 0) */
/* 		{ */
/* 			pFMagSpectrum[i] = sqrt(fReal * fReal + fImag * fImag); */
/* 			if (pFPhaseSpectrum) */
/* 				pFPhaseSpectrum[i] = atan2(fImag, fReal); */
/* 		} */
/* 	} */
/* 	free(pFBuffer); */
  
/* 	return (sizeMag); */
/* } */

/* 
 * function to compute a complex spectrum from a waveform 
 * returns the size of the complex spectrum
 *              
 * float *pFWaveform;	   pointer to input waveform
 * float pFWindow;	   pointer to analysis window 
 * int sizeWindow;	   size of analysis window
 * float *pFMagSpectrum;   pointer to output spectrum 
 * float *pFPhaseSpectrum; pointer to output spectrum
 * int sizeFft;		   size of FFT 
 */
int sms_quickSpectrum (float *pFWaveform, float *pFWindow, int sizeWindow, 
                    float *pFMagSpectrum, float *pFPhaseSpectrum, int sizeFft)
{
	int sizeMag = sizeFft >> 1, i, it2;
	float *pFBuffer, fReal, fImag;
  
	/* allocate buffer */    
	if ((pFBuffer = (float *) calloc(sizeFft+1, sizeof(float))) == NULL)
		return -1;
    
	/* apply window to waveform */
	for (i = 0; i < sizeWindow; i++)
                pFBuffer[i] =  pFWindow[i] * pFWaveform[i];
  
	/* compute real FFT */
	realft (pFBuffer-1, sizeMag, 1);
  
	/* convert from rectangular to polar coordinates */
	for (i=0; i<sizeMag; i++)
	{
		it2 = i << 1;
		fReal = pFBuffer[it2];
		fImag = pFBuffer[it2+1];
      
		if (fReal != 0 || fImag != 0)
		{
 			pFMagSpectrum[i] = sqrt(fReal * fReal + fImag * fImag);
			if (pFPhaseSpectrum)
				pFPhaseSpectrum[i] = atan2(fImag, fReal);
		}
	}
	free(pFBuffer);
  
	return (sizeMag);
}


/*
 * function to perform the inverse FFT
 * float *pFMagSpectrum        input magnitude spectrum
 * float *pFPhaseSpectrum      input phase spectrum
 * int sizeFft                 size of FFT
 * float *pFWaveform           output waveform
 * int sizeWave                size of output waveform
 */
int sms_invQuickSpectrum (float *pFMagSpectrum, float *pFPhaseSpectrum,
                          int sizeFft, float *pFWaveform, int sizeWave)
{
	int sizeMag = sizeFft >> 1, i, it2;
	float *pFBuffer, fPower;
  
	/* allocate buffer */
	if ((pFBuffer = (float *) calloc(sizeFft+1, sizeof(float))) == NULL)
		return -1;
   
	/* convert from polar coordinates to rectangular  */
	for (i = 0; i < sizeMag; i++)
	{
		it2 = i << 1;
		fPower = pFMagSpectrum[i];
		pFBuffer[it2] =  fPower * cos (pFPhaseSpectrum[i]);
		pFBuffer[it2+1] = fPower * sin (pFPhaseSpectrum[i]);
	}
	/* compute IFFT */
	realft (pFBuffer-1, sizeMag, -1);
 
	/* assume the output array has been taken care off */
	for (i = 0; i < sizeWave; i++)
		pFWaveform[i] +=  pFBuffer[i];
 
	free(pFBuffer);
  
	return (sizeMag);
}
 
/*
 * function to perform the inverse FFT, windowing the output
 * float *pFMagSpectrum        input magnitude spectrum
 * float *pFPhaseSpectrum      input phase spectrum
 * int sizeFft		       size of FFT
 * float *pFWaveform	       output waveform
 * int sizeWave                size of output waveform
 * float *pFWindow	       synthesis window
 */
int sms_invQuickSpectrumW (float *pFMagSpectrum, float *pFPhaseSpectrum, 
                           int sizeFft, float *pFWaveform, int sizeWave,
                           float *pFWindow)
{
	int sizeMag = sizeFft >> 1, i, it2;
	float *pFBuffer, fPower;
  
	/* allocate buffer */    
	if ((pFBuffer = (float *) calloc(sizeFft, sizeof(float))) == NULL)
		return -1;
   
	/* convert from polar coordinates to rectangular  */
	for (i = 0; i<sizeMag; i++)
	{
		it2 = i << 1;
		fPower = pFMagSpectrum[i];
		pFBuffer[it2] =  fPower * cos (pFPhaseSpectrum[i]);
		pFBuffer[it2+1] = fPower * sin (pFPhaseSpectrum[i]);
	}    
	/* compute IFFT */
	realft (pFBuffer-1, sizeMag, -1);
 
	/* assume the output array has been taken care off */
	for (i = 0; i < sizeWave; i++)
		pFWaveform[i] +=  (pFBuffer[i] * pFWindow[i] * .5);
 
	free (pFBuffer);
  
	return (sizeMag);
}
 
