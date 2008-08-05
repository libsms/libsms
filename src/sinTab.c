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

static float fSineScale;
float *sms_tab_sine;

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

