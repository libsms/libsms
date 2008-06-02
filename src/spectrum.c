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

extern float *pFWindowSpec;

/* 
 * function to compute a complex spectrum from a waveform 
 * returns the size of the complex   spectrum
 *              
 * float *pFWaveform;	   pointer to input waveform 
 * int sizeWindow;	   size of analysis window
 * float *pFMagSpectrum;   pointer to output magnitude spectrum 
 * float *pFPhaseSpectrum; pointer to output phase spectrum 
 */
int Spectrum (float *pFWaveform, int sizeWindow, float *pFMagSpectrum, 
              float *pFPhaseSpectrum, ANAL_PARAMS analParams)
{
	int sizeFft = 
		(int) pow (2.0, 
		          (float)(1 + (floor (log ((float)(WINDOWS_IN_FFT * 
		                                           sizeWindow))
		                              / LOG2))));
	int i, it2, sizeMag = sizeFft >> 1, iMiddleWindow = (sizeWindow+1) >> 1, 
		iOffset;
	float *pFBuffer, fReal, fImag;
	static int iOldSizeWindow = 0;
  
	/* allocate buffer */    
	if ((pFBuffer = (float *) calloc(sizeFft+1, sizeof(float))) == NULL)
		return -1;
  
	/* compute window when necessary */
	if (iOldSizeWindow != sizeWindow)
		GetWindow (sizeWindow, pFWindowSpec, analParams.iWindowType);
	iOldSizeWindow = sizeWindow;
  
	/* apply window to waveform and center window around 0 */
	iOffset = sizeFft - (iMiddleWindow - 1);
	for (i=0; i<iMiddleWindow-1; i++)
		pFBuffer[1+(iOffset + i)] =  pFWindowSpec[i] * pFWaveform[i];
	iOffset = iMiddleWindow - 1;
	for (i=0; i<iMiddleWindow; i++)
		pFBuffer[1+i] = pFWindowSpec[iOffset + i] * pFWaveform[iOffset + i];
  

	realft (pFBuffer, sizeMag, 1);
  
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
int QuickSpectrum (short *pIWaveform, float *pFWindow, int sizeWindow, 
                   float *pFMagSpectrum, float *pFPhaseSpectrum, int sizeFft)
{
	int sizeMag = sizeFft >> 1, i, it2;
	float *pFBuffer, fReal, fImag;
  
	/* allocate buffer */    
	if ((pFBuffer = (float *) calloc(sizeFft+1, sizeof(float))) == NULL)
		return -1;
    
	/* apply window to waveform */
	for (i = 0; i < sizeWindow; i++)
		pFBuffer[i] =  pFWindow[i] * pIWaveform[i];
  
	/* compute real FFT */
	realft (pFBuffer-1, sizeMag, 1);
  
	/* convert from rectangular to polar coordinates */
	for (i = 0; i < sizeMag; i++)
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
int QuickSpectrumF (float *pFWaveform, float *pFWindow, int sizeWindow, 
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
int InverseQuickSpectrum (float *pFMagSpectrum, float *pFPhaseSpectrum, 
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
 * function to perform the inverse FFT
 * float *pFMagSpectrum        input magnitude spectrum
 * float *pFPhaseSpectrum      input phase spectrum
 * int sizeFft		       size of FFT
 * float *pFWaveform	       output waveform
 * int sizeWave                size of output waveform
 * float *pFWindow	       synthesis window
 */
int InverseQuickSpectrumW (float *pFMagSpectrum, float *pFPhaseSpectrum, 
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
 
