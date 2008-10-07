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
/*! \file windows.c
 * \brief functions for creating various windows
 * 
 * Use sms_getWindow() for selecting which window will be made
 */
#include "sms.h"

/*! \brief window to be used in the IFFT synthesis
 * 
 * contains both an inverse Blackman-Harris and triangular
 * \todo improve this documentation with references and explanations
 * \param sizeWindow the size of the window
 * \param pFWindow pointer to an array that will hold the window
 */
void IFFTwindow (int sizeWindow, float *pFWindow)
{

	int     i;
	float a0 = .35875, a1 = .48829, a2 = .14128, a3 = .01168;
	double fConst = TWO_PI / sizeWindow, fIncr = 2.0 /sizeWindow, fVal = 0;

	/* compute inverse of window */
	for(i = 0; i < sizeWindow; i++) 
	{
		pFWindow[i] = 1 / (a0 - a1 * cos(fConst * i) +
			a2 * cos(fConst * 2 * i) - a3 * cos(fConst * 3 * i));
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
}

/*! \brief BlackmanHarris window with 62dB rolloff
 * 
 * \todo where did these come from?
 * \param sizeWindow the size of the window
 * \param pFWindow pointer to an array that will hold the window
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

/*! \brief BlackmanHarris window with 70dB rolloff
 * 
 * \param sizeWindow the size of the window
 * \param pFWindow pointer to an array that will hold the window
 */
void BlackmanHarris70 (int sizeWindow, float *pFWindow)
{
	int     i;
	double fSum = 0;
	/* for 3 term -70.83 */
	float a0 = .42323, a1 = .49755, a2 = .07922;
	double fConst = TWO_PI / sizeWindow;

	/* compute window */
	for(i = 0; i < sizeWindow; i++) 
	{
		fSum += pFWindow[i] = a0 - a1 * cos(fConst * i) +
			a2 * cos(fConst * 2 * i);
	}

	fSum = fSum / 2;
  
	/* scale function */
	for (i = 0; i < sizeWindow; i++)
        {
		pFWindow[i] = pFWindow[i] / fSum;
        }
}

/*! \brief BlackmanHarris window with 74dB rolloff
 * 
 * \param sizeWindow the size of the window
 * \param pFWindow pointer to an array that will hold the window
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

/*! \brief BlackmanHarris window with 92dB rolloff
 * 
 * \param sizeWindow the size of the window
 * \param pFWindow pointer to an array that will hold the window
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

/*! \brief default BlackmanHarris window (70dB rolloff)
 * 
 * \param sizeWindow the size of the window
 * \param pFWindow pointer to an array that will hold the window
 */
void BlackmanHarris (int sizeWindow, float *pFWindow)
{
	 BlackmanHarris70 (sizeWindow, pFWindow);
}

/*! \brief Hamming window
 *
 * \param sizeWindow   window size
 * \param pFWindow      window array
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

/*! \brief Hanning window
 *
 * \param sizeWindow   window size
 * \param pFWindow      window array
 */
void Hanning (int sizeWindow, float *pFWindow)
{
  int i;

  for(i = 0; i < sizeWindow; i++) 
    pFWindow[i] = (sin(PI*i/(sizeWindow-1)))*(sin(PI*i/(sizeWindow-1)));
}

/*! \brief main function for getting various windows
 *
 * \see SMS_WINDOWS for the different window types available
 * \param sizeWindow   window size
 * \param pFWindow      window array
 * \param iWindowType the desired window type defined by #SMS_WINDOWS 
 */
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





