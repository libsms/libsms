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

/* window to be used in the IFFT synthesis */

void IFFTwindow (int sizeWindow, float *pFWindow)
{

	int     i;
	float a0 = .35875, a1 = .48829, a2 = .14128, a3 = .01168;
	double fConst = TWO_PI / sizeWindow, fIncr = 2.0 /sizeWindow, fVal = 0;
// RTE DEBUG ///////////////////////////////////////
        FILE *dd;
        dd = fopen("debug.txt", "w");
  
	/* compute inverse of window */
	for(i = 0; i < sizeWindow; i++) 
	{
		pFWindow[i] = 1 / (a0 - a1 * cos(fConst * i) +
			a2 * cos(fConst * 2 * i) - a3 * cos(fConst * 3 * i));
                fprintf(dd, "%f ", pFWindow[i]);
	}
  
	/* scale function by a triangular */
	for (i = 0; i < sizeWindow / 2; i++)
	{
		pFWindow[i] = fVal * pFWindow[i]  / 2.787457;
		fVal += fIncr;
	}
	for (i = sizeWindow / 2; i < sizeWindow; i++)
	{
		pFWindow[i] = fVal * pFWindow[i]  / 2.787457;
		fVal -= fIncr;
	}
/* 	for(i = 0; i < sizeWindow; i++)  */
/*                 fprintf(dd, "%f ", pFWindow[i]); */
        fclose(dd);
        
}

/* function to create a backmanHarris window
 * int     sizeWindow;   window size
 * float  *pFWindow;      window array
 */
void BlackmanHarris62 (int sizeWindow, float *pFWindow)
{
	int     i;
	double fSum = 0;
	/* for 3 term -62.05 */
	float a0 = .44959, a1 = .49364, a2 = .05677; 
	double fConst = TWO_PI / sizeWindow;
  
	/* compute window */
	for(i = 0; i < sizeWindow; i++) 
	{
		fSum += pFWindow[i] = a0 - a1 * cos(fConst * i) +
			a2 * cos(fConst * 2 * i);
	}

	/* I do not know why I now need this factor of two */
	fSum = fSum / 2;
  
	/* scale function */
	for (i = 0; i < sizeWindow; i++)
		pFWindow[i] = pFWindow[i] / fSum;
}

/* function to create a backmanHarris window
 * int     sizeWindow;   window size
 * float  *pFWindow;      window array
 */
void BlackmanHarris70 (int sizeWindow, float *pFWindow)
{
	int     i;
	double fSum = 0;
	/* for 3 term -70.83 */
	float a0 = .42323, a1 = .49755, a2 = .07922;
	double fConst = TWO_PI / sizeWindow;

// RTE DEBUG ///////////////////////////////////////
/*         FILE *dd; */
/*         dd = fopen("debug.txt", "w"); */
/*         static float max = 0.; */
  
	/* compute window */
	for(i = 0; i < sizeWindow; i++) 
	{
		fSum += pFWindow[i] = a0 - a1 * cos(fConst * i) +
			a2 * cos(fConst * 2 * i);
/*                 fprintf(dd, "%f ", pFWindow[i]); */
	}

/*         fclose(dd); */


	/* I do not know why I now need this factor of two */
	fSum = fSum / 2;
  
	/* scale function */
	for (i = 0; i < sizeWindow; i++)
        {
		pFWindow[i] = pFWindow[i] / fSum;
        }
}

/* function to create a backmanHarris window
 * int     sizeWindow;   window size
 * float  *pFWindow;      window array
 */
void BlackmanHarris74 (int sizeWindow, float *pFWindow)
{
	int     i;
	double fSum = 0;
	/* for -74dB  from the Nuttall paper */
	float a0 = .40217, a1 = .49703, a2 = .09892, a3 = .00188;
	double fConst = TWO_PI / sizeWindow;
  
	/* compute window */
	for(i = 0; i < sizeWindow; i++) 
	{
		fSum += pFWindow[i] = a0 - a1 * cos(fConst * i) +
			a2 * cos(fConst * 2 * i) + a3 * cos(fConst * 3 * i);
	}

	/* I do not know why I now need this factor of two */
	fSum = fSum / 2;
  
	/* scale function */
	for (i = 0; i < sizeWindow; i++)
		pFWindow[i] = pFWindow[i] / fSum;
}

/* function to create a backmanHarris window
 * int     sizeWindow;   window size
 * float  *pFWindow;      window array
 */
void BlackmanHarris92 (int sizeWindow, float *pFWindow)
{
	int     i;
	double fSum = 0;
	/* for -92dB */
	float a0 = .35875, a1 = .48829, a2 = .14128, a3 = .01168;
	double fConst = TWO_PI / sizeWindow;
  
	/* compute window */
	for(i = 0; i < sizeWindow; i++) 
	{
		fSum += pFWindow[i] = a0 - a1 * cos(fConst * i) +
			a2 * cos(fConst * 2 * i) + a3 * cos(fConst * 3 * i);
	}

	/* I do not know why I now need this factor of two */
	fSum = fSum / 2;
  
	/* scale function */
	for (i = 0; i < sizeWindow; i++)
		pFWindow[i] = pFWindow[i] / fSum;
}

void BlackmanHarris (int sizeWindow, float *pFWindow)
{
	 BlackmanHarris70 (sizeWindow, pFWindow);
}

/* function to design a Hamming window
 * int     sizeWindow;   window size
 * float  *pFWindow;      window array
 */
void Hamming (int sizeWindow, float *pFWindow)
{
	int     i;
	float fSum = 0;

	for(i = 0; i < sizeWindow; i++) 
		fSum += pFWindow[i] = 0.54 - 0.46*cos(2.0*PI*i/(sizeWindow-1));
   
	fSum = fSum / 2;

	for(i = 0; i < sizeWindow; i++) 
		pFWindow[i] /= fSum;
}

/* function to design a Hanning window
 * int     sizeWindow;   window size
 * float  *pFWindow;      window array
 */
void Hanning (int sizeWindow, float *pFWindow)
{
  int i;

  for(i = 0; i < sizeWindow; i++) 
    pFWindow[i] = (sin(PI*i/(sizeWindow-1)))*(sin(PI*i/(sizeWindow-1)));
}


void sms_getWindow (int sizeWindow, float *pFWindow, int iWindowType)
{
	switch (iWindowType)
	{
        case SMS_WIN_BH_62: 
                BlackmanHarris62 (sizeWindow, pFWindow);
                break;
        case SMS_WIN_BH_70: 
                BlackmanHarris70 (sizeWindow, pFWindow);			
                break;
        case SMS_WIN_BH_74: 
                BlackmanHarris74 (sizeWindow, pFWindow);
                break;
        case SMS_WIN_BH_92: 
                BlackmanHarris92 (sizeWindow, pFWindow);
                break;
        case SMS_WIN_HAMMING: 
                Hamming (sizeWindow, pFWindow);
                break;
        case SMS_WIN_HANNING: 
                Hanning (sizeWindow, pFWindow);
                break;
        case SMS_WIN_IFFT: 
                IFFTwindow (sizeWindow, pFWindow);
                break;
        default:
                BlackmanHarris (sizeWindow, pFWindow);
	}
}





