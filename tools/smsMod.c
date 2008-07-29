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
/*
 *
 *    main program for smsMod
 *
 *		used to change sotchastic magnitudes and set a fundamental frequency
 */
#include "sms.h"
#define USAGE "Usage: smsMod [-s stochasticFactor] [-f fundFreq] <inputSmsFile> <outputSmsFile>"

short MaxDelayFrames;
float FResidualPerc;
SOUND_BUFFER soundBuffer, synthBuffer;
ANAL_FRAME **ppFrames, *pFrames;

int main (int argc, char *argv[])
{
  char *pChInputSmsFile = NULL, *pChOutputSmsFile = NULL;
  SMS_Header *pSmsHeader;
	FILE *pSmsFile;
  SMS_Data smsData;
  int iError, i;
  float fStocFactor = 1, fFundamental = 1;
  
	/* get user arguments */  
	if (argc > 3) 
   {
     for (i=1; i<argc-2; i++) 
		{
			if (*(argv[i]++) == '-') 
			{
				switch (*(argv[i]++)) 
				{
					case 's':  if (sscanf(argv[i],"%f",&fStocFactor) < 1)
						quit("Invalid stochastic factor");
						break;
					case 'f':  if (sscanf(argv[i],"%f",&fFundamental) < 0) 
						quit("Invalid fundamental");
						break;
					default:   quit(USAGE);
				}
			}
		}
   }
  
  pChInputSmsFile = argv[argc-2];
  pChOutputSmsFile = argv[argc-1];
  
  if((iError = GetSmsHeader (pChInputSmsFile, &pSmsHeader, &pSmsFile)) < 0)
    {
      if(iError == SMS_NOPEN)
	quit("cannot open file");
      if(iError == SMS_RDERR)
	quit("read error");
      if(iError == SMS_NSMS)
	quit("not an SMS file");
      if(iError == SMS_MALLOC)
	quit("cannot allocate memory");
      quit("error");
    }	    
  
	pSmsHeader->fFrequency = fFundamental;
	AllocSmsRecord (pSmsHeader, &smsData);

  for(i = 1; i < pSmsHeader->nFrames; i++)
  {
    GetSmsRecord (pSmsFile, pSmsHeader, i, &smsData);
    *smsData.pFStocGain += fStocFactor;
  }
  
  WriteSmsFile (pSmsFile, pSmsHeader);

  free(pSmsHeader);
  return 0;
}
