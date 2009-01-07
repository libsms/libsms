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
  
	if (iOldSizeWindow != sizeWindow) 
        {
                sms_getWindow (sizeWindow, pAnalParams->pFSpectrumWindow, pAnalParams->iWindowType);
        }
	iOldSizeWindow = sizeWindow;

        int it2;
        float *pFBuffer;
	/* allocate buffer */    

	if ((pFBuffer = (float *) calloc(sizeFft, sizeof(float))) == NULL)
		return -1;

	/* apply window to waveform and center window around 0 (zero-phase windowing)*/
	iOffset = sizeFft - (iMiddleWindow - 1);
	for (i=0; i<iMiddleWindow-1; i++)
		pFBuffer[iOffset + i] =  pAnalParams->pFSpectrumWindow[i] * pFWaveform[i];
	iOffset = iMiddleWindow - 1;
	for (i=0; i<iMiddleWindow; i++)
		pFBuffer[i] = pAnalParams->pFSpectrumWindow[iOffset + i] * pFWaveform[iOffset + i];
  
        sms_fft(sizeFft, pFBuffer);
  
	/* convert from rectangular to polar coordinates */
	for (i = 0; i < sizeMag; i++)
	{ 
		it2 = i << 1; //even numbers 0-N
		fReal = pFBuffer[it2]; /*odd numbers 1->N+1 */
		fImag = pFBuffer[it2+1]; /*even numbers 2->N+2 */
      
		if (fReal != 0 || fImag != 0) /*!< \todo is this necessary or even helping? */
		{
			pFMagSpectrum[i] = sms_magToDB (sqrt (fReal * fReal + fImag * fImag));
			pFPhaseSpectrum[i] = atan2 (-fImag, fReal);
		}
	}

	free (pFBuffer);
        
	return (sizeMag);
}

/*! \brief compute a complex spectrum from a waveform 
 *              
 * \param pFWaveform	       pointer to input waveform
 * \param pFWindow	       pointer to analysis window 
 * \param sizeWindow	       size of analysis window
 * \param pFMagSpectrum     pointer to output magnitude spectrum 
 * \param pFPhaseSpectrum  pointer to output phase spectrum
 * \param sizeFft		       size of FFT 
 * \return the size of the complex spectrum
 */
int sms_quickSpectrum (float *pFWaveform, float *pFWindow, int sizeWindow, 
                       float *pFMagSpectrum, float *pFPhaseSpectrum, int sizeFft)
{
	int sizeMag = sizeFft >> 1, i;
	float fReal, fImag;
        int it2;
        float *pFBuffer;
	/* allocate buffer */    
        /* \todo why cannot the fft just be done on pFWaveform in-place? */
	if ((pFBuffer = (float *) calloc(sizeFft+1, sizeof(float))) == NULL)
		return -1;
    
	/* apply window to waveform */
	for (i = 0; i < sizeWindow; i++)
                pFBuffer[i] =  pFWindow[i] * pFWaveform[i];
  
	/* compute real FFT */
        sms_fft(sizeFft, pFBuffer); 
  
	/* convert from rectangular to polar coordinates */
        sms_spectrumRtoP(sizeMag, pFBuffer, pFMagSpectrum, pFPhaseSpectrum);

	free(pFBuffer);
  
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
        sms_ifft(sizeFft, pFBuffer); 
 
	/* assume the output array has been taken care off */
	for (i = 0; i < sizeWave; i++)
		pFWaveform[i] +=  pFBuffer[i];
 
	free(pFBuffer);
  
	return (sizeMag);
}
 
/*! \brief function for a quick inverse spectrum, windowed
 * \todo move fftwf_execute into here
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
        sms_ifft(sizeFft, pFBuffer); 

 	/* assume the output array has been taken care off */
        /* \todo is a seperate pFBuffer necessary here?
           it seems like multiplying the window into the waveform
           would be fine, without pFBuffer */
	for (i = 0; i < sizeWave; i++)
		pFWaveform[i] +=  (pFBuffer[i] * pFWindow[i] * .5);

	free (pFBuffer);
  
	return (sizeMag);
}
/*! \brief convert spectrum from Rectangular to Polar coordinates
 *              
 * \param sizeMag	       size of 
 * \param pFReal	       pointer to input FFT real array (real/imag floats)
 * \param pFMAg	       pointer to float array of magnitude spectrum
 * \param pFPhase	       pointer to float array of phase spectrum
 */ 
void sms_spectrumRtoP( int sizeMag, float *pFReal, float *pFMag, float *pFPhase)
{
        int i, it2;
        float fReal, fImag;

	for (i=0; i<sizeMag; i++)
	{
		it2 = i << 1;
		fReal = pFReal[it2];
		fImag = pFReal[it2+1];
      
                pFMag[i] = sqrtf(fReal * fReal + fImag * fImag);
                if (pFPhase)
                        pFPhase[i] = atan2f(fImag, fReal);
	}


}

/*! \brief compute magnitude spectrum of a DFT
 *              
 * \param sizeMag	       size of output Magnitude (half of input real FFT)
 * \param pFReal	       pointer to input FFT real array (real/imag floats)
 * \param pFMAg	       pointer to float array of magnitude spectrum
 */
void sms_spectrumMag( int sizeMag, float *pInRect, float *pOutMag)
{
        int i, it2;
        float fReal, fImag;

	for (i=0; i<sizeMag; i++)
	{
		it2 = i << 1;
		fReal = pInRect[it2];
		fImag = pInRect[it2+1];
                pOutMag[i] = sqrtf(fReal * fReal + fImag * fImag);
	}
}
