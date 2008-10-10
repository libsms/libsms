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
/*! \file spectrum.c
 * \brief functions to convert between frequency (spectrum) and time (wavefrom) domain
 */
#include "sms.h"

/*! \brief overlap factor, or the number of analysis windows that fit in one FFT */
#define SMS_OVERLAP_FACTOR 2  

/*! \brief pointer to the window array for sms_spectrum */
float *sms_window_spec;

/*! \brief compute a complex spectrum from a waveform 
 *              
 * \param pFWaveform	           pointer to input waveform 
 * \param sizeWindow	           size of analysis window
 * \param pFMagSpectrum        pointer to output magnitude spectrum 
 * \param pFPhaseSpectrum     pointer to output phase spectrum 
 * \param pAnalParams             pointer to structure of analysis parameters
 * \return the size of the complex spectrum, or -1 if failure
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
        static int fftwPlanned = 0;

	/* compute window when necessary and re-plan fftw*/
	if (iOldSizeWindow != sizeWindow) 
        {       
                sms_getWindow (sizeWindow, sms_window_spec, pAnalParams->iWindowType);

                if(fftwPlanned)
                        fftwf_destroy_plan(pAnalParams->fftw.plan);

                if((pAnalParams->fftw.plan =  fftwf_plan_dft_r2c_1d( sizeFft, pAnalParams->fftw.pWaveform,
                                                                   pAnalParams->fftw.pSpectrum, FFTW_ESTIMATE)) == NULL)

                {
                        printf("Spectrum: could not make fftw plan of size: %d \n", sizeFft);
                        return(-1); 
                }
                fftwPlanned = 1;
        }
	iOldSizeWindow = sizeWindow;

        /*! \todo why is this memset necessary if the waveform is overwritten below? */
        memset(pAnalParams->fftw.pWaveform, 0, sizeFft * sizeof(float));

	/* apply window to waveform and center window around 0 */
	iOffset = sizeFft - (iMiddleWindow - 1);
	for (i=0; i<iMiddleWindow-1; i++)
		pAnalParams->fftw.pWaveform[iOffset + i] =  sms_window_spec[i] * pFWaveform[i];
	iOffset = iMiddleWindow - 1;
	for (i=0; i<iMiddleWindow; i++)
		pAnalParams->fftw.pWaveform[i] = sms_window_spec[iOffset + i] * pFWaveform[iOffset + i];

        fftwf_execute(pAnalParams->fftw.plan);

	/* convert from rectangular to polar coordinates */
	for (i = 1; i < sizeMag; i++) 
	{
		fReal = pAnalParams->fftw.pSpectrum[i][0];
		fImag = pAnalParams->fftw.pSpectrum[i][1];
      
		if (fReal != 0 || fImag != 0)
		{
			pFMagSpectrum[i] = sms_magToDB (sqrt (fReal * fReal + fImag * fImag));
                        /* \todo the phase spectrum is inverted! find out why.. */
			/*pFPhaseSpectrum[i] = atan2 (-fImag, fReal); */
			pFPhaseSpectrum[i] = -atan2 (-fImag, fReal);
		}
	}
        /*DC and Nyquist  \todo might not be necessary? */
        fReal = pAnalParams->fftw.pSpectrum[0][0];
        fImag = pAnalParams->fftw.pSpectrum[sizeMag][0];
        pFMagSpectrum[0] = sms_magToDB (sqrt (fReal * fReal + fImag * fImag));
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


	if ((pFBuffer = (float *) calloc(sizeFft, sizeof(float))) == NULL)
		return -1;

	/* apply window to waveform and center window around 0 */
	iOffset = sizeFft - (iMiddleWindow - 1);
	for (i=0; i<iMiddleWindow-1; i++)
		pFBuffer[iOffset + i] =  sms_window_spec[i] * pFWaveform[i];
	iOffset = iMiddleWindow - 1;
	for (i=0; i<iMiddleWindow; i++)
		pFBuffer[i] = sms_window_spec[iOffset + i] * pFWaveform[iOffset + i];
  
	//realft (pFBuffer, sizeMag, 1);
        sms_rdft(sizeFft, pFBuffer, 1);
  
	/* convert from rectangular to polar coordinates */
	for (i = 0; i < sizeMag; i++)
	{ /*skips pFBuffer[0] because it is always 0 with realft */
		it2 = i << 1; //even numbers 0-N
		fReal = pFBuffer[it2]; /*odd numbers 1->N+1 */
		fImag = pFBuffer[it2+1]; /*even numbers 2->N+2 */
      
		if (fReal != 0 || fImag != 0) /*!< \todo is this necessary or even helping? */
		{
			pFMagSpectrum[i] = sms_magToDB (sqrt (fReal * fReal + fImag * fImag));
			pFPhaseSpectrum[i] = atan2 (-fImag, fReal);
		}
	}

/* 	if ((pFBuffer = (float *) calloc(sizeFft+1, sizeof(float))) == NULL) */
/* 		return -1; */

/* 	/\* apply window to waveform and center window around 0 *\/ */
/* 	iOffset = sizeFft - (iMiddleWindow - 1); */
/* 	for (i=0; i<iMiddleWindow-1; i++) */
/* 		pFBuffer[1+(iOffset + i)] =  sms_window_spec[i] * pFWaveform[i]; */
/* 	iOffset = iMiddleWindow - 1; */
/* 	for (i=0; i<iMiddleWindow; i++) */
/* 		pFBuffer[1+i] = sms_window_spec[iOffset + i] * pFWaveform[iOffset + i]; */
  
/* 	realft (pFBuffer, sizeMag, 1); */
/*         //sms_rdft(sizeFft, pFBuffer, -1); */
  
/* 	/\* convert from rectangular to polar coordinates *\/ */
/* 	for (i = 0; i < sizeMag; i++) */
/* 	{ /\*skips pFBuffer[0] because it is always 0 with realft *\/ */
/* 		it2 = i << 1; //even numbers 0-N */
/* 		fReal = pFBuffer[it2+1]; /\*odd numbers 1->N+1 *\/ */
/* 		fImag = pFBuffer[it2+2]; /\*even numbers 2->N+2 *\/ */
      
/* 		if (fReal != 0 || fImag != 0) */
/* 		{ */
/* 			pFMagSpectrum[i] = sms_magToDB (sqrt (fReal * fReal + fImag * fImag)); */
/* 			pFPhaseSpectrum[i] = atan2 (-fImag, fReal); */
/* 		} */
/* 	} */

	free (pFBuffer);
#endif/*FFTW*/
        
	return (sizeMag);
}

/*! \brief compute a complex spectrum from a waveform 
 *              
 * \param pFWaveform	       pointer to input waveform
 * \param pFWindow	       pointer to analysis window 
 * \param sizeWindow	       size of analysis window
 * \param pFMagSpectrum     pointer to output spectrum 
 * \param pFPhaseSpectrum  pointer to output spectrum
 * \param sizeFft		       size of FFT 
 * \return the size of the complex spectrum
 */
int sms_quickSpectrum (float *pFWaveform, float *pFWindow, int sizeWindow, 
                       float *pFMagSpectrum, float *pFPhaseSpectrum, int sizeFft)
{
	int sizeMag = sizeFft >> 1, i;
	float fReal, fImag;

#ifdef FFTW
        /*! \todo same as todo in sms_spectrum */
        static SMS_Fourier fftData;
        /* \todo memory leak here.. */
        fftData.pWaveform = fftwf_malloc(sizeof(float) * sizeFft);
        fftData.pSpectrum = fftwf_malloc(sizeof(fftwf_complex) * (sizeFft / 2 + 1));
        fftData.plan =  fftwf_plan_dft_r2c_1d( sizeFft, fftData.pWaveform,
                                               fftData.pSpectrum, FFTW_ESTIMATE);

        memset(fftData.pWaveform, 0, sizeFft * sizeof(float));

	/* apply window to waveform */
	for (i = 0; i < sizeWindow; i++)
                fftData.pWaveform[i] =  pFWindow[i] * pFWaveform[i];

        fftwf_execute(fftData.plan);

	/*convert from rectangular to polar coordinates*/
	for (i = 1; i < sizeMag; i++)
	{
		fReal = fftData.pSpectrum[i][0];
		fImag = fftData.pSpectrum[i][1];
      
		if (fReal != 0 || fImag != 0)
		{
			pFMagSpectrum[i] = sqrt (fReal * fReal + fImag * fImag);
                        /* \todo this should be checked out of the for loop.., only once */
			if(pFPhaseSpectrum)
                                pFPhaseSpectrum[i] = atan2 (fImag, fReal); /*should be negative like above? */
		}
	}
        /*DC and Nyquist  */
        fReal = fftData.pSpectrum[0][0];
        fImag = fftData.pSpectrum[sizeMag][0];
        pFMagSpectrum[0] = sqrt (fReal * fReal + fImag * fImag);
        if (pFPhaseSpectrum)
                pFPhaseSpectrum[0] = atan2(fImag, fReal);

#else /* using realft */
  
        int it2;
        float *pFBuffer;
	/* allocate buffer */    
	if ((pFBuffer = (float *) calloc(sizeFft+1, sizeof(float))) == NULL)
		return -1;
    
	/* apply window to waveform */
	for (i = 0; i < sizeWindow; i++)
                pFBuffer[i] =  pFWindow[i] * pFWaveform[i];
  
	/* compute real FFT */
	//realft (pFBuffer-1, sizeMag, 1);
        sms_rdft(sizeFft, pFBuffer, 1); 
  
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
#endif /* FFTW */
  
	return (sizeMag);
}

/*! \todo fix for alternative fft along with hybridize..
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
	//realft (pFBuffer-1, sizeMag, -1);
        sms_rdft(sizeFft, pFBuffer, -1); 
 
	/* assume the output array has been taken care off */
	for (i = 0; i < sizeWave; i++)
		pFWaveform[i] +=  pFBuffer[i];
 
	free(pFBuffer);
  
	return (sizeMag);
}
 
/*! \brief function for a quick inverse spectrum, windowed
 * \todo move fftwf_execture into here
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
        sms_rdft(sizeFft, pFBuffer, -1); 
        //realft(pFBuffer-1, sizeMag, -1);

 	/* assume the output array has been taken care off */
        /* \todo is a seperate pFBuffer necessary here?
           it seems like multiplying the window into the waveform
           would be fine, without pFBuffer */
	for (i = 0; i < sizeWave; i++)
		pFWaveform[i] +=  (pFBuffer[i] * pFWindow[i] * .5);

	free (pFBuffer);
  
	return (sizeMag);
}
 
