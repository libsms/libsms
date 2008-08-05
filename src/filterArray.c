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

/*
 * filter each point of the current array by the surounding
 *  points using a triangular window
 *
 *  float *pFArray	two dimensional input array
 *  int size1		vertical size of pFArray
 *  int size2		horizontal size of pFArray
 *  float *pFOutArray   output array of size size1
 */
int sms_filterArray (float *pFArray, int size1, int size2, float *pFOutArray)
{
	int i, j, iPoint, iFrame, size2_2 = size2-2, size2_1 = size2-1;
	float *pFCurrentArray = pFArray + (size2_1) * size1, fVal, fWeighting, 
		fTotalWeighting, fTmpVal;

	/* find the filtered envelope */
	for (i = 0; i < size1; i++)
	{
		fVal = pFCurrentArray[i];
		fTotalWeighting = 1;
		/* filter by the surrounding points */
		for (j = 1; j < (size2_2); j++)
		{
			fWeighting = (float) size2 / (1+ j);
			/* filter on the vertical dimension */
			/* consider the lower points */
			iPoint = i - (size2_1) + j;
			if (iPoint >= 0)
			{  
				fVal += pFCurrentArray[iPoint] * fWeighting;
				fTotalWeighting += fWeighting;
			}
			/* consider the higher points */
			iPoint = i + (size2_1) - j;
			if (iPoint < size1)
			{
				fVal += pFCurrentArray[iPoint] * fWeighting;
				fTotalWeighting += fWeighting;
			}
			/*filter on the horizontal dimension */
			/* consider the previous points */
 			iFrame = j;
			fTmpVal = pFArray[iFrame*size1 + i];
			if (fTmpVal)
			{
				fVal += fTmpVal * fWeighting;
				fTotalWeighting += fWeighting;
			}
		}
		/* scale value by weighting */
		pFOutArray[i] = fVal / fTotalWeighting;
	}
	return 1;
}
