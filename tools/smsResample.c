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
 *    main program for smsResample
 *
 */
#include "sms.h"
#define USAGE "Usage: smsResample [factor] <inputSmsFile> <outputSmsFile>"

//short MaxDelayFrames;
//float FResidualPerc;
//SMS_SndBuffer soundBuffer, synthBuffer;
//SMS_AnalFrame **ppFrames, *pFrames;

int main (int argc, char *argv[])
{
	char *pChInputSmsFile = NULL, *pChOutputSmsFile = NULL;
	SMS_Header *pSmsHeader;
	FILE *pInSmsFile, *pOutSmsFile;
	SMS_Data inSmsData;
	int iError, iFactor, i;
  
	/* get user arguments */
	if (argc != 4) 
		quit(USAGE);   
    
	if (sscanf(argv[1],"%d",&iFactor) < 1)
		quit("Invalid factor");

	pChInputSmsFile = argv[2];
	pChOutputSmsFile = argv[3];

	if ((iError = GetSmsHeader (pChInputSmsFile, &pSmsHeader,
	                            &pInSmsFile)) < 0)
	{
		if (iError == SMS_NOPEN)
			quit ("cannot open file");
		if (iError == SMS_RDERR)
			quit("read error");
		if (iError == SMS_NSMS)
			quit ("not an SMS file");
		if (iError == SMS_MALLOC)
			quit ("cannot allocate memory");
			quit ("error");
	}	    
  
	AllocSmsRecord (pSmsHeader, &inSmsData);
	WriteSmsHeader (pChOutputSmsFile, pSmsHeader, &pOutSmsFile);

	for (i = 1 + iFactor; i < pSmsHeader->nFrames; i += iFactor)
	{
		GetSmsRecord (pInSmsFile, pSmsHeader, i, &inSmsData);
		WriteSmsRecord (pOutSmsFile, pSmsHeader, &inSmsData);
	}
  
	pSmsHeader->nFrames /= iFactor;
	pSmsHeader->iFrameRate /= iFactor;
  
	/* rewrite the header and close the output SMS file */
	WriteSmsFile (pOutSmsFile, pSmsHeader);

	fclose (pInSmsFile);
	free (pSmsHeader);
	return 0;
}
