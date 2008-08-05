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
 * function to interpolate two arrays, the output is an array in between the
 *  two input ones
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
