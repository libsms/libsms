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

#define SINC_TABLE_SIZE 4096

float *sms_tab_sinc;
static float fSincScale;

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
