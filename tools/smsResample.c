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
//#define USAGE 

void usage (void)
{
        fprintf (stderr, "\n"
                 "Usage: smsResample [factor] <inputSmsFile> <outputSmsFile>"
                 "\n\n");
        exit(1);
}

int main (int argc, char *argv[])
{
	char *pChInputSmsFile = NULL, *pChOutputSmsFile = NULL;
	SMS_Header *pSmsHeader;
	FILE *pInSmsFile, *pOutSmsFile;
	SMS_Data inSmsData;
	int iError, iFactor, i;
  
	/* get user arguments */
	if (argc != 4) usage();
    
	if (sscanf(argv[1],"%d",&iFactor) < 1)
        {
		printf("Invalid factor");
                exit(1);
        }

	pChInputSmsFile = argv[2];
	pChOutputSmsFile = argv[3];

	if ((iError = sms_getHeader (pChInputSmsFile, &pSmsHeader,
	                            &pInSmsFile)) < 0)
	{
                printf("error in sms_getHeader: %s", sms_errorString(iError));
                exit(EXIT_FAILURE);
	}	    
  
	sms_allocFrameH (pSmsHeader, &inSmsData);
	sms_writeHeader (pChOutputSmsFile, pSmsHeader, &pOutSmsFile);

	for (i = 1 + iFactor; i < pSmsHeader->nFrames; i += iFactor)
	{
		sms_getFrame (pInSmsFile, pSmsHeader, i, &inSmsData);
		sms_writeFrame (pOutSmsFile, pSmsHeader, &inSmsData);
	}
  
	pSmsHeader->nFrames /= iFactor;
	pSmsHeader->iFrameRate /= iFactor;
  
	/* rewrite the header and close the output SMS file */
	sms_writeFile (pOutSmsFile, pSmsHeader);

	fclose (pInSmsFile);
	free (pSmsHeader);
	return 0;
}
