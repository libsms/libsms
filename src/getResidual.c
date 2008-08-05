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

// RTE DEBUG //////////////////
char *fn0 = "blarg0.txt";
FILE *f0;
char *fn1 = "blarg1.txt";
FILE *f1;
///////////////////////////////////

/* function to implement a pole-zero filter
 * the returned value is the  filtered sample
 * 
 * float *pFa;        numerator coefficients
 * float *pFb;        denominator coefficients
 * int nCoeff;        number of coefficients
 * float fInput;      input sample
 */
static float ZeroPoleFilter (float *pFa, float *pFb, int nCoeff, float fInput,
                             float *pD)
{
	double fOut = 0;
	int iSection;
    
	pD[0] = fInput;
	for (iSection = nCoeff-1; iSection > 0; iSection--)
	{
		fOut = fOut + pFa[iSection] * pD[iSection];
		pD[0] = pD[0] - pFb[iSection] * pD[iSection];
		pD[iSection] = pD [iSection-1];
	}
	fOut = fOut + pFa[0] * pD[0];
	return((float) fOut);
}

	/* cutoff 1500 Hz */
/*	static float pFCoeff32k[10] =  {0.679459, -2.71784, 4.07676, -2.71784, 
		0.679459, 1, -3.23168, 3.97664, -2.20137, 0.461665};  
	static float pFCoeff36k[10] =  {0.709489, -2.83796, 4.25694, -2.83796, 
		0.709489, 1, -3.31681, 4.17425, -2.3574, 0.503375};
	static float pFCoeff40k[10] =  {0.734408, -2.93763, 4.40645, -2.93763, 
		0.734408, 1, -3.38497, 4.33706, -2.48914, 0.539355};
	static float pFCoeff441k[10] =  {0.755893, -3.02357, 4.53536, -3.02357, 
		0.755893, 1, -3.44205, 4.47657, -2.6043, 0.571374};
	static float pFCoeff48k[10] =  {0.773347, -3.09339, 4.64008, -3.09339, 
		0.773347, 1, -3.48731, 4.58929, -2.69888, 0.598065};
*/


/* function to filter a waveform with a high-pass filter with cutoff
 *  at 1500 Hz  
 * 
 * float *pFResidual;        residual signal
 * int sizeResidual;         size of signal
 * int iSamplingRate;      sampling rate of signal                                                    
 */
static void FilterResidual (float *pFResidual, int sizeResidual, 
                             int iSamplingRate, float *pD)
{
	/* cutoff 800Hz */
	static float pFCoeff32k[10] =  {0.814255, -3.25702, 4.88553, -3.25702, 
		0.814255, 1, -3.58973, 4.85128, -2.92405, 0.66301};
	static float pFCoeff36k[10] =  {0.833098, -3.33239, 4.99859, -3.33239, 
		0.833098, 1, -3.63528, 4.97089, -3.02934,0.694052};
	static float pFCoeff40k[10] =  {0.848475, -3.3939, 5.09085, -3.3939, 
		0.848475, 1, -3.67173, 5.068, -3.11597, 0.71991}; 
	static float pFCoeff441k[10] =  {0.861554, -3.44622, 5.16932, -3.44622, 
		0.861554, 1, -3.70223, 5.15023, -3.19013, 0.742275};
	static float pFCoeff48k[10] =  {0.872061, -3.48824, 5.23236, -3.48824, 
		0.872061, 1, -3.72641, 5.21605, -3.25002, 0.76049};
	float *pFCoeff, fSample = 0;
	int i;
  
	if (iSamplingRate <= 32000)
		pFCoeff = pFCoeff32k;
	else if (iSamplingRate <= 36000)
		pFCoeff = pFCoeff36k;
	else if (iSamplingRate <= 40000)
		pFCoeff = pFCoeff40k;
	else if (iSamplingRate <= 44100)
		pFCoeff = pFCoeff441k;
	else
		pFCoeff = pFCoeff48k;
  
	for(i = 0; i < sizeResidual; i++)
	{
		/* try to avoid underflow when there is nothing to filter */
		if (i > 0 && fSample == 0 && pFResidual[i] == 0)
			return;
      
		fSample = pFResidual[i];
		pFResidual[i] = 
			ZeroPoleFilter (&pFCoeff[0], &pFCoeff[5], 5, fSample, pD);
	}
}

/* get the residual waveform
 * return 0 if no representation, 1 if got a representation
 *
 * float *pFSynthesis;       deterministic component
 * float *pFOriginal;        original waveform
 * float *pFResidual;        output residual waveform
 * int sizeWindow;           size of buffer
 * SMS_Data *pSmsData;       pointer to output SMS data
 * SMS_AnalParams *pAnalParams;   analysis parameters
 */
int sms_residual (float *pFSynthesis, float *pFOriginal,  
                 float *pFResidual, int sizeWindow, SMS_AnalParams *pAnalParams)
{
	static float fResidualMag = 0, fOriginalMag = 0, *pD = NULL, 
		*pFWindow = NULL;
	float fScale = 1, fCurrentResidualMag = 0, fCurrentOriginalMag = 0;
	int i;
   
	/* allocate memory for filter coefficients */
	if (pD == NULL)
		pD = (float *) calloc(5, sizeof(float));	    

        // RTE: why is the window made here.. why not globally or stored in SMS_AnalParams?
	if (pFWindow == NULL)
	{
		if ((pFWindow = (float *) calloc(sizeWindow, sizeof(float))) == NULL)
			return -1;
		//Hamming (sizeWindow, pFWindow);
                sms_getWindow( sizeWindow, pFWindow, SMS_WIN_HAMMING);
	}

        ///// RTE DEBUG ///////////////////////////////
        if(f0 == NULL){
                f0 =fopen(fn0, "w+");
                f1 =fopen(fn1, "w+");
        }

        float rms = 0.0f;
	for (i=0; i<sizeWindow; i++)
                rms += pFOriginal[i] * pFOriginal[i];
        rms = sqrt(rms / sizeWindow);
//        printf("[%d] rms: %f \n", pAnalParams->ppFrames[0]->iFrameNum, rms);

        ///////////////////////////////////////


	/* get residual */
	for (i=0; i<sizeWindow; i++)
        {
                pFResidual[i] = pFOriginal[i] - pFSynthesis[i];
                fprintf(f0, "%f ", pFOriginal[i]); ///// RTE DEBUG
                fprintf(f1, "%f ", pFSynthesis[i]); ///// RTE DEBUG
        }

  
	/* get energy of residual */
	for (i=0; i<sizeWindow; i++)
		fCurrentResidualMag += fabsf( pFResidual[i] * pFWindow[i]);



	/* if residual is big enough compute coefficients */
	if (fCurrentResidualMag/sizeWindow > .01)
	{  
		/* get energy of original */
		for (i=0; i<sizeWindow; i++)
			fCurrentOriginalMag += fabs( pFOriginal[i] * pFWindow[i]);
  
		fOriginalMag = 
			.5 * (fCurrentOriginalMag/sizeWindow + fOriginalMag);
		fResidualMag = 
			.5 * (fCurrentResidualMag/sizeWindow + fResidualMag);
  
		/* scale residual if need to be */
		if (fResidualMag > fOriginalMag)
		{
			fScale = fOriginalMag / fResidualMag;
			for (i=0; i<sizeWindow; i++)
				pFResidual[i] *= fScale;

      
			if (pAnalParams->iDebugMode == SMS_DBG_ALL ||
				pAnalParams->iDebugMode == SMS_DBG_STOC_ANAL)
				fprintf(stdout, 
				        "getResidual: frame %d: scaled residual by %f\n", 
				        pAnalParams->ppFrames[0]->iFrameNum, fScale);
		}
   
		/* store residual percentage in global variable */
		pAnalParams->fResidualPercentage += fCurrentResidualMag / fCurrentOriginalMag;
		if (pAnalParams->iDebugMode == SMS_DBG_ALL ||
		    pAnalParams->iDebugMode == SMS_DBG_STOC_ANAL)
			fprintf (stdout, "getResidual: frame %d, residual perc %f \n",
			         pAnalParams->ppFrames[0]->iFrameNum, 
			         fCurrentResidualMag / fCurrentOriginalMag);
		if (pAnalParams->iDebugMode == 13)
			fprintf (stdout, "%f\n",
			         fCurrentResidualMag / fCurrentOriginalMag);

		if (pAnalParams->iDebugMode == SMS_DBG_SYNC)
			sms_writeDebugData (pFOriginal+sizeWindow/2, 
			                  pFSynthesis+sizeWindow/2,
		                    pFResidual+sizeWindow/2, sizeWindow/2);

		if (pAnalParams->iDebugMode == SMS_DBG_RESIDUAL)
		{
			sms_createResSF( pAnalParams );
			sms_writeResSound (pFResidual+sizeWindow/2, sizeWindow/2);
		}
		/* filter residual with a high pass filter (it solves some problems) */
		FilterResidual (pFResidual, sizeWindow, pAnalParams->iSamplingRate, pD);

		return (1);
	}
	if (pAnalParams->iDebugMode == SMS_DBG_SYNC)
		sms_writeDebugData (pFOriginal+sizeWindow/2, 
			                pFSynthesis+sizeWindow/2,
		                  pFResidual+sizeWindow/2, sizeWindow/2);

	if (pAnalParams->iDebugMode == SMS_DBG_RESIDUAL)
	{
		sms_createResSF( pAnalParams );
		sms_writeResSound (pFResidual+sizeWindow/2, sizeWindow/2);
	}
	return (0);
}
