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

#define SIN_TABLE_SIZE 4096//was 2046
#define SINC_TABLE_SIZE 4096

static float fSineScale;
static float fSincScale;
float *sms_tab_sine;
float *sms_tab_sinc;

/* prepares the sine table, returns 1 if allocations made, 0 on failure
 * int nTableSize;    size of table
 */
int sms_prepSine (int nTableSize)
{
  register int i;
  float fTheta;
  
  if((sms_tab_sine = (float *)malloc(nTableSize*sizeof(float))) == 0)
    return (0);
  fSineScale =  (float)(TWO_PI) / (float)(nTableSize - 1);
  fTheta = 0.0;
  for(i = 0; i < nTableSize; i++) 
  {
    fTheta = fSineScale * (float)i;
    sms_tab_sine[i] = sin(fTheta);
  }
  return (1);
}
/* clear sine table */
void sms_clearSine()
{
  if(sms_tab_sine)
    free(sms_tab_sine);
  sms_tab_sine = 0;
}

/* function that returns approximately sin(fTheta)
 * float fTheta;    angle in radians
 */
float sms_sine (float fTheta)
{
  float fSign = 1.0, fT;
  int i;
  
  fTheta = fTheta - floor(fTheta / TWO_PI) * TWO_PI;
  
  if(fTheta < 0)
  {
    fSign = -1;
    fTheta = -fTheta;
  }
  
  i = fTheta / fSineScale + .5;
  fT = sms_tab_sine[i];
  
  if (fSign == 1)
    return(fT);
  else
    return(-fT);
}

static float Sinc (float x, float N)	
{
	return (sinf ((N/2) * x) / sinf (x/2));
}

/* function to prepare the main lobe of a frequency domain 
 *  BlackmanHarris92 window
 *
 */
int sms_prepSinc (int nTableSize)
{
        int i, m;
	float N = 512.0;
	float fA[4] = {.35875, .48829, .14128, .01168},
		fMax = 0;
	float fTheta = -4.0 * TWO_PI / N, 
	       fThetaIncr = (8.0 * TWO_PI / N) / (nTableSize);

	if((sms_tab_sinc = (float *) calloc (nTableSize, sizeof(float))) == 0)
		return (0);

	for(i = 0; i < nTableSize; i++) 
	{
		for (m = 0; m < 4; m++)
			sms_tab_sinc[i] +=  -1 * (fA[m]/2) * 
				(Sinc (fTheta - m * TWO_PI/N, N) + 
			     Sinc (fTheta + m * TWO_PI/N, N));
		fTheta += fThetaIncr;
	}
	fMax = sms_tab_sinc[(int) nTableSize / 2];
	for (i = 0; i < nTableSize; i++) 
		sms_tab_sinc[i] = sms_tab_sinc[i] / fMax;

        fSincScale = (float) nTableSize / 8.0;

	return (1);
}
/* clear sine table */
void sms_clearSinc()
{
  if(sms_tab_sinc)
    free(sms_tab_sinc);
  sms_tab_sinc = 0;
}

/*
 * fTheta has to be from 0 to 8
 */
float sms_sinc (float fTheta)
{
	int index = (int) (.5 + fSincScale * fTheta);

	return (sms_tab_sinc[index]);
}
