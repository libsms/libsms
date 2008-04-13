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

static float *pFWindow1 = NULL, *pFWindow2 = NULL;
static int sizeFft1, sizeFft2, sizeMag1, sizeMag2, sizeSmooth;
static float *pFMagSpectrum1, *pFMagSpectrum2, *pFPhaseSpectrum1, 
	     *pFPhaseSpectrum2, *pFMagEnv, *pFEnvBuffer, *pFMagEnvFilt;
/* 
 * initialize static arrays 
 *
 * int sizeWave1        size of waveform 1
 * int sizeWave2        size of waveform 2
 * HYB_PARAMS params    parameters for hybridization
 *
 */

static int InitializeHybrid (int sizeWave1, int sizeWave2, HYB_PARAMS params)
{
    if ((pFWindow1 = (float *) calloc(sizeWave1, sizeof(float))) == NULL)
      return -1;
    if ((pFWindow2 = (float *) calloc(sizeWave2, sizeof(float))) == NULL)
      return -1;

    Hamming (sizeWave1, pFWindow1);
    Hamming (sizeWave2, pFWindow2);
    sizeFft1 = (int) pow(2.0,
                 (double)(1+(floor(log((double)sizeWave1) / LOG2))));
    sizeFft2 = (int) pow(2.0, 
                 (double) (1+(floor(log((double)sizeWave2) / LOG2))));
    sizeMag1 = sizeFft1 >> 1;
    sizeMag2 = sizeFft2 >> 1;
    sizeSmooth = 1 + params.iSmoothOrder;
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

/* 
 * free buffers
 */
int freeBuffers ()
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
	return 1;
}

/* 
 *  Compress-Expand a given spectral envelope 
 */

static int CompExp(float *pFEnv, float *pFSpec, float *pFWeight, int sizeSpec)
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
  return 1;
}


/*
 * filter each magnitude in a spectrum by the surounding magnitudes 
 */ 
static int FilterMagEnv (float *pFMagEnv, float *pFMagEnvFilt, int sizeMag)
{
  int sizeEnvBuffer = sizeSmooth * sizeMag;
   memcpy ((char *) pFEnvBuffer, (char *) (pFEnvBuffer+sizeMag), 
	  sizeof(float) * sizeEnvBuffer);
   memcpy ((char *) (pFEnvBuffer+sizeEnvBuffer-sizeMag), (char *) pFMagEnv, 
	  sizeof(float) * sizeMag);
  FilterArray (pFEnvBuffer, sizeMag, sizeSmooth, pFMagEnvFilt);
  return 1;
}

/* 
 * multiply spectral envelope of one sound with magnitude spectrum of the other 
 *
 * float *pFMagEnv		envelope from hybridizing sound
 * float *pFMagSpectrum		magnitude spectrum of excitation sound
 * int sizeMag			size of magnitude spectrum
 * HYB_PARAMS params		control parameters
 */
static int MultiplySpectra (float *pFMagEnv, float *pFMagSpectrum, int sizeMag, 
                            float *pFMagSpectrum2, int sizeMag2, 
                            HYB_PARAMS params)
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
	if (params.pCompressionEnv)
	{
		if (params.sizeCompressionEnv == sizeMag1)
			CompExp (pFMagEnv, pFMagSpectrum, params.pCompressionEnv, sizeMag);
		else return -1;
	}

	/* do the hybridation */
	for (i = 0; i < sizeMag; i++)
		pFMagSpectrum[i] *=  pFMagEnv[i];

	/* get the new average magnitude */
	fHybAverage = params.fMagBalance * fAverageMagHyb + 
		(1 - params.fMagBalance) * fAverageMag;

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
 * HYB_PARAMS params		control parameters
 *
 */
void HybridizeMag (short *pIWaveform1, int sizeWave1, short *pIWaveform2, 
                   int sizeWave2, float *pFWaveform, HYB_PARAMS params)
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

	fScalar = params.fMagBalance * fMag2 + 
				    (1- params.fMagBalance) * fMag1;

	for (i = 0; i < sizeWave1; i++)
		pFWaveform[i] = (pIWaveform1[i] / fMag1) * fScalar;
}

/*
 * function to hybridize two waveforms
 *
 * short *pIWaveform1		excitation waveform
 * int sizeWave1		    size of excitation waveform
 * short *pIWaveform2		hybridization waveform
 * int sizeWave2		    size of hybridization waveform
 * float *pFWaveform		output waveform (hybridized)
 * HYB_PARAMS params		control parameters
 *
 */
int Hybridize (short *pIWaveform1, int sizeWave1, short *pIWaveform2, 
               int sizeWave2, float *pFWaveform, HYB_PARAMS params)
{
	/* initialize static variables and arrays */
	if (pFWindow1 == NULL)
		InitializeHybrid (sizeWave1, sizeWave2, params);
   
	/* if there is not spectral changes perform a magnitude hybridization */
	if (!params.pCompressionEnv)
	{
		HybridizeMag (pIWaveform1, sizeWave1, pIWaveform2, sizeWave2, 
		              pFWaveform, params);
		return (1);
	}
	/* compute the two spectra */
	QuickSpectrum (pIWaveform1, pFWindow1, sizeWave1, pFMagSpectrum1, 
	               pFPhaseSpectrum1, sizeFft1);
	QuickSpectrum (pIWaveform2, pFWindow2, sizeWave2, pFMagSpectrum2, 
	               pFPhaseSpectrum2, sizeFft2);

	/* approximate the second spectrum by line segments and obtain a magnitude 
	 * spectrum of size sizeMag1 */
	SpectralApprox (pFMagSpectrum2, sizeMag2, sizeMag2, pFMagEnv, sizeMag1, 
	                params.nCoefficients);

	/* filter the smoothed spectrum */
	FilterMagEnv (pFMagEnv, pFMagEnvFilt, sizeMag1);

	/* hybridize the two spectra */
	MultiplySpectra (pFMagEnvFilt, pFMagSpectrum1, sizeMag1, pFMagSpectrum2, 
	                 sizeMag2, params);

	/* perform the inverse FFT from the hybridized spectrum */
	InverseQuickSpectrum (pFMagSpectrum1, pFPhaseSpectrum1, sizeFft1, 
	                      pFWaveform, sizeWave1);
  return 1;
}
