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

static double Sinc (double x, short N)	
{
	return (sin ((N/2) * x) / sin (x/2));
}

/* function to prepare the main lobe of a frequency domain 
 *  BlackmanHarris92 window
 *
 */
int PrepSinc ()
{
	short N = 512, i, m;
	float fA[4] = {.35875, .48829, .14128, .01168},
		fMax = 0;
	double fTheta = -4.0 * TWO_PI / N, 
	       fThetaIncr = (8.0 * TWO_PI / N) / (SINC_TABLE_SIZE);

	if((sms_tab_sinc = (float *) calloc (SINC_TABLE_SIZE, sizeof(float))) == 0)
		return (0);

	for(i = 0; i < SINC_TABLE_SIZE; i++) 
	{
		for (m = 0; m < 4; m++)
			sms_tab_sinc[i] +=  -1 * (fA[m]/2) * 
				(Sinc (fTheta - m * TWO_PI/N, N) + 
			     Sinc (fTheta + m * TWO_PI/N, N));
		fTheta += fThetaIncr;
	}
	fMax = sms_tab_sinc[(int) SINC_TABLE_SIZE / 2];
	for (i = 0; i < SINC_TABLE_SIZE; i++) 
		sms_tab_sinc[i] = sms_tab_sinc[i] / fMax;

	return (1);
}

/*
 * fTheta has to be from 0 to 8
 */
double SincTab (double fTheta)
{
	long index = (long) (.5 + SINC_TABLE_SIZE * fTheta / 8.0);

	return (sms_tab_sinc[index]);
}
