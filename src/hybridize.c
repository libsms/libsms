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
/*! \file hybridize.c
 * \brief not yet functional... todo after original release
 */

#include "sms.h"

static float *pFWindow1 = NULL, *pFWindow2 = NULL;
static int sizeFft1, sizeFft2, sizeMag1, sizeMag2, sizeSmooth;
static float *pFMagSpectrum1, *pFMagSpectrum2, *pFPhaseSpectrum1, 
	     *pFPhaseSpectrum2, *pFMagEnv, *pFEnvBuffer, *pFMagEnvFilt;

#ifndef ENV_THRESHOLD
#define ENV_THRESHOLD     .01
#endif

/*! \brief initialize static arrays 
 *
 * \param sizeWave1        size of waveform 1
 * \param sizeWave2        size of waveform 2
 * \param pHybParams    parameters for hybridization
 */
static int InitializeHybrid (int sizeWave1, int sizeWave2, SMS_HybParams *pHybParams)
{
    if ((pFWindow1 = (float *) calloc(sizeWave1, sizeof(float))) == NULL)
      return -1;
    if ((pFWindow2 = (float *) calloc(sizeWave2, sizeof(float))) == NULL)
      return -1;

    sms_getWindow(sizeWave1, pFWindow1, SMS_WIN_HAMMING);
    sms_getWindow(sizeWave2, pFWindow2, SMS_WIN_HAMMING);
    sizeFft1 = (int) pow(2.0,
                 (double)(1+(floor(log((double)sizeWave1) / LOG2))));
    sizeFft2 = (int) pow(2.0, 
                 (double) (1+(floor(log((double)sizeWave2) / LOG2))));
    sizeMag1 = sizeFft1 >> 1;
    sizeMag2 = sizeFft2 >> 1;
    sizeSmooth = 1 + pHybParams->iSmoothOrder;
    if ((pFMagSpectrum2 = 
          (float *) calloc(sizeMag2, sizeof(float))) == NULL)
      return -1;
    if ((pFMagSpectrum1 = 
          (float *) calloc(sizeMag1, sizeof(float))) == NULL)
      return -1;
    pFPhaseSpectrum2 = NULL;
    if ((pFPhaseSpectrum1 = 
          (float *) calloc(sizeMag1, sizeof(float))) == NULL)
      return -1;
    if ((pFMagEnv = 
          (float *) calloc(sizeMag1, sizeof(float))) == NULL)
      return -1;
    if ((pFEnvBuffer = 
          (float *) calloc(sizeMag1 * sizeSmooth, sizeof(float))) == NULL)
      return -1;
    if ((pFMagEnvFilt = 
          (float *) calloc(sizeMag1, sizeof(float))) == NULL)
      return -1;

    return 1;
}

 
/*! \brief free buffers
 */
void freeBuffers ()
{
	free (pFWindow1);
	pFWindow1 = NULL;
	free (pFWindow2);
	pFWindow2 = NULL;
	if (pFMagSpectrum2)
		free (pFMagSpectrum2);
	if (pFMagSpectrum1)
		free (pFMagSpectrum1);
	if (pFPhaseSpectrum1)
		free (pFPhaseSpectrum1);
	if (pFPhaseSpectrum2)
		free (pFPhaseSpectrum2);
	free (pFMagEnv);
	free (pFEnvBuffer);
	free (pFMagEnvFilt);
}

/*! \brief compress / expand a given spectral envelope 
 *
 * \param pFEnv hmm..
 * \param pFSpec hmmm...
 * \param pFWeight hmmmm...
 * \param sizeSpec size of the spectrum
 */
static void CompExp(float *pFEnv, float *pFSpec, float *pFWeight, int sizeSpec)
{
  float *pFEnd = pFSpec + sizeSpec;		
  do 
  {
      if (*pFWeight == 2.) *pFSpec = 1.;
      else if (!*pFWeight) *pFEnv = 1.;
      else if (*pFWeight > 1 && *pFSpec)
          *pFSpec = (float)pow(10.,(2.-*pFWeight)*log10((double)*pFSpec)); 
      else if (*pFWeight < 1 && *pFEnv)
          *pFEnv = (float)pow(10.,*pFWeight*log10((double)*pFEnv)); 
      ++pFWeight; 
      ++pFSpec;
      ++pFEnv;
  }
  while (pFSpec<pFEnd);

}


/* filter each magnitude in a spectrum by the surounding magnitudes 
 */ 
static int FilterMagEnv (float *pFMagEnv, float *pFMagEnvFilt, int sizeMag)
{
  int sizeEnvBuffer = sizeSmooth * sizeMag;
   memcpy ((char *) pFEnvBuffer, (char *) (pFEnvBuffer+sizeMag), 
	  sizeof(float) * sizeEnvBuffer);
   memcpy ((char *) (pFEnvBuffer+sizeEnvBuffer-sizeMag), (char *) pFMagEnv, 
	  sizeof(float) * sizeMag);
  sms_filterArray (pFEnvBuffer, sizeMag, sizeSmooth, pFMagEnvFilt);
  return 1;
}

/* multiply spectral envelope of one sound with magnitude spectrum of the other 
 *
 * pFMagEnv		envelope from hybridizing sound
 * float *pFMagSpectrum		magnitude spectrum of excitation sound
 * int sizeMag			size of magnitude spectrum
 * SMS_HybParams params		control parameters
 */
static int MultiplySpectra (float *pFMagEnv, float *pFMagSpectrum, int sizeMag, 
                            float *pFMagSpectrum2, int sizeMag2, 
                            SMS_HybParams *pHybParams)
{
	float *pFBuffer, fMag = 0, fAverageMag, fMagEnv = 0, fAverageMagEnv, 
		fHybAverage, fMagHyb = 0, fAverageMagHyb;
	int  i;

	/* allocate buffers */    
	if ((pFBuffer = (float *) calloc(sizeMag, sizeof(float))) == NULL)
		return -1;

	/* get energy of excitation spectrum and envelope and normalize them */
	for (i = 0; i < sizeMag; i++)
	{
		fMag += pFMagSpectrum[i];
		fMagEnv += pFMagEnv[i];
	}

	for (i = 0; i < sizeMag2; i++)
		fMagHyb += pFMagSpectrum2[i];

	fAverageMag = MAX (fMag / sizeMag, ENV_THRESHOLD);
	fAverageMagEnv = MAX (fMagEnv / sizeMag, ENV_THRESHOLD);
	fAverageMagHyb = MAX (fMagHyb / sizeMag2, ENV_THRESHOLD);

	for (i = 0; i < sizeMag; i++)
	{
		pFMagSpectrum[i] /= fAverageMag;
		pFMagEnv[i] /= fAverageMagEnv;
	}
  
	/* compress the spectral envelope from one or other sound */
	if (pHybParams->pCompressionEnv)
	{
		if (pHybParams->sizeCompressionEnv == sizeMag1)
			CompExp (pFMagEnv, pFMagSpectrum, pHybParams->pCompressionEnv, sizeMag);
		else return -1;
	}

	/* do the hybridation */
	for (i = 0; i < sizeMag; i++)
		pFMagSpectrum[i] *=  pFMagEnv[i];

	/* get the new average magnitude */
	fHybAverage = pHybParams->fMagBalance * fAverageMagHyb + 
		(1 - pHybParams->fMagBalance) * fAverageMag;

	if (fHybAverage < .0001 || fHybAverage > 1000) fHybAverage = 1;

	/* scale the new spectrum by the new average magnitude */
	for (i = 0; i < sizeMag; i++)
		pFMagSpectrum[i] *= fHybAverage;

	free (pFBuffer);
	return 1;
}

/*
 * function to hybridize the magnitude of two waveforms
 *
 * short *pIWaveform1		excitation waveform
 * int sizeWave1		    size of excitation waveform
 * short *pIWaveform2		hybridization waveform
 * int sizeWave2		    size of hybridization waveform
 * float *pFWaveform		output waveform (hybridized)
 * SMS_HybParams params		control parameters
 *
 */
void HybridizeMag (float *pIWaveform1, int sizeWave1, float *pIWaveform2, 
                   int sizeWave2, float *pFWaveform, SMS_HybParams *pHybParams)
{
	int i;
	double fMag1 = 0, fMag2 = 0, fScalar;

	for (i = 0; i < sizeWave1; i++)
	{
		pFWaveform[i] = pIWaveform1[i] * pFWindow1[i];
		fMag1 += fabs (pFWaveform[i]);
	}
	fMag1 /= sizeWave1;

	for (i = 0; i < sizeWave2; i++)
		fMag2 += fabs (pIWaveform2[i] * pFWindow2[i]);
	fMag2 /= sizeWave2;

	fScalar = pHybParams->fMagBalance * fMag2 + 
				    (1- pHybParams->fMagBalance) * fMag1;

	for (i = 0; i < sizeWave1; i++)
		pFWaveform[i] = (pIWaveform1[i] / fMag1) * fScalar;
}

/*
 * function to interpolate two arrays, the output is an array in between the
 *  two input ones
 *
 * moved from interpolateArrays.c, because it doesn't seem to be useful
 * in anything but hybridizing routines.
 *
 * float *pFArray1       pointer to first array
 * int sizeArray1        size of first array
 * float *pFArray2       pointer to second array
 * int sizeArray2        size of second array
 * float *pFArrayOut     pointer to output array
 * int sizeArrayOut      size of output array
 * float fInterpFactor   interpolation factor
 */
int InterpolateArrays (float *pFArray1, int sizeArray1, float *pFArray2,
                       int sizeArray2, float *pFArrayOut, int sizeArrayOut,
                       float fInterpFactor)
{
  int i;
  float *pFArrayOne, *pFArrayTwo;

  if ((pFArrayOne = (float *) calloc (sizeArrayOut, sizeof(float))) 
       == NULL)
     return -1;
  if ((pFArrayTwo = (float *) calloc (sizeArrayOut, sizeof(float))) 
       == NULL)
     return -1;

  /* make the two array of sizeArrayOut */
  sms_spectralApprox (pFArray1, sizeArray1, sizeArray1, pFArrayOne, sizeArrayOut,
		  sizeArray1);
  sms_spectralApprox (pFArray2, sizeArray2, sizeArray2, pFArrayTwo, sizeArrayOut,
		  sizeArray2);

  /* interpolate the two arrays */
  for (i = 0; i < sizeArrayOut; i++)
    pFArrayOut[i] = pFArrayOne[i] + fInterpFactor * 
			(pFArrayTwo[i] - pFArrayOne[i]);

  free (pFArrayOne);
  free (pFArrayTwo);
  return 1;

}

/*! \brief hybridize two waveforms
 *
 * \param pFWaveform1		excitation waveform
 * \param sizeWave1		    size of excitation waveform
 * \param pFWaveform2		hybridization waveform
 * \param sizeWave2		    size of hybridization waveform
 * \param pFWaveform		output waveform (hybridized)
 * \param pHybParams		pointer to strucutre of control parameterscontrol parameters
 *
 */
void sms_hybridize (float *pFWaveform1, int sizeWave1, float *pFWaveform2, 
               int sizeWave2, float *pFWaveform, SMS_HybParams *pHybParams)
{
	/* initialize static variables and arrays */
	if (pFWindow1 == NULL)
		InitializeHybrid (sizeWave1, sizeWave2, pHybParams);
   
	/* if there is not spectral changes perform a magnitude hybridization */
	if (!pHybParams->pCompressionEnv)
	{
		HybridizeMag (pFWaveform1, sizeWave1, pFWaveform2, sizeWave2, 
		              pFWaveform, pHybParams);
		return ;
	}
	/* compute the two spectra */
	sms_quickSpectrum (pFWaveform1, pFWindow1, sizeWave1, pFMagSpectrum1,
	               pFPhaseSpectrum1, sizeFft1);
	sms_quickSpectrum (pFWaveform2, pFWindow2, sizeWave2, pFMagSpectrum2,
	               pFPhaseSpectrum2, sizeFft2);

	/* approximate the second spectrum by line segments and obtain a magnitude 
	 * spectrum of size sizeMag1 */
	sms_spectralApprox (pFMagSpectrum2, sizeMag2, sizeMag2, pFMagEnv, sizeMag1, 
	                pHybParams->nCoefficients);

	/* filter the smoothed spectrum */
	FilterMagEnv (pFMagEnv, pFMagEnvFilt, sizeMag1);

	/* hybridize the two spectra */
	MultiplySpectra (pFMagEnvFilt, pFMagSpectrum1, sizeMag1, pFMagSpectrum2, 
	                 sizeMag2, pHybParams);

	/* perform the inverse FFT from the hybridized spectrum */
	sms_invQuickSpectrum (pFMagSpectrum1, pFPhaseSpectrum1, sizeFft1, 
	                      pFWaveform, sizeWave1);

}
