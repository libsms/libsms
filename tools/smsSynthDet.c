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
 *    main program for smsSynthDet
 *
 */
#include "sms.h"
#define USAGE "Usage: smsSynthDet [-s samplingRate] <inputSmsFile> <outputSoundFile>"

double *pFSTab = NULL;
SMS_SndBuffer soundBuffer, synthBuffer;
SMS_AnalFrame **ppFrames, *pFrames;
short MaxDelayFrames;

int main (int argc, char *argv[])
{
	char *pChInputSmsFile = NULL, *pChOutputSoundFile = NULL;
	SMS_Header *pSmsHeader;
	FILE *pSmsFile;
	SMS_Data smsData;
	short *pSSynthesis;
	int iError, iRecord, i, iSamplingRate = 44100;
	SMS_SynthParams synthParams;
	float *pFBuffer;

	synthParams.iSynthesisType = 1;

	if (argc > 3) 
	{
		for (i=1; i<argc-2; i++) 
			if (*(argv[i]++) == '-') 
				switch (*(argv[i]++)) 
				{
					case 's':  if (sscanf(argv[i],"%d",&iSamplingRate) < 1)
						quit("Invalid sampling rate");
						break;
					case 'd': synthParams.iSynthesisType = 1;
						break;
					case 'n': synthParams.iSynthesisType = 2;
						break;
					default:   quit(USAGE);
				}
	}
	else if (argc < 2)
		quit (USAGE);

	pChInputSmsFile = argv[argc-2];
	pChOutputSoundFile = argv[argc-1];
      
	if ((iError = GetSmsHeader (pChInputSmsFile, &pSmsHeader, &pSmsFile)) < 0)
	{
		if (iError == SMS_NOPEN)
			quit ("cannot open input file");
		if (iError == SMS_RDERR)
			quit ("read error in input file");
		if (iError == SMS_NSMS)
			quit("the input file not an SMS file");
		if (iError == SMS_MALLOC)
			quit ("cannot allocate memory for input file");
		quit ("error reading input file");
	}	    
  
	/* allocate two SMS records */
	AllocSmsRecord (pSmsHeader, &smsData);

	synthParams.iOriginalSRate = pSmsHeader->iOriginalSRate;
	synthParams.iStochasticType = pSmsHeader->iStochasticType;
  
	if (iSamplingRate == 44100 && 
	    synthParams.iOriginalSRate == 44100)
		synthParams.iSamplingRate = 44100;
	else
	{
		synthParams.iSamplingRate = 22050;
		fprintf(stderr, "Sampling Rate set to 22050 Hz\n");
	}

	CreateOutputSoundFile (synthParams, pChOutputSoundFile);

	synthParams.sizeHop = synthParams.iSamplingRate / pSmsHeader->iFrameRate;
	if ((pFBuffer = (float *) calloc(synthParams.sizeHop, sizeof(float)))
	    == NULL)
		quit ("Could not allocate memory for pFBuffer");
	if ((pSSynthesis = (short *) calloc(synthParams.sizeHop, sizeof(short)))
	    == NULL)
		quit ("Could not allocate memory for pSSynthesis");

	/* prepare sine table only the first time */
	if (pFSTab == NULL)
		PrepSine (4096);
    
	AllocateSmsRecord (&synthParams.previousFrame, pSmsHeader->nTrajectories, 
	                   1 + pSmsHeader->nStochasticCoeff, 1,
                           pSmsHeader->sizeHop, pSmsHeader->iStochasticType);
  
	/* use first record as memory */
	GetSmsRecord (pSmsFile, pSmsHeader, 0, &smsData);
	CopySmsRecord (&synthParams.previousFrame, &smsData);

	for (iRecord = 1; iRecord < pSmsHeader->nFrames; iRecord++)
	{
		GetSmsRecord (pSmsFile, pSmsHeader, iRecord, &smsData);
		memset ((char *)pFBuffer, 0, sizeof(float) * synthParams.sizeHop);
		FrameSineSynth (&smsData, pFBuffer, synthParams.sizeHop, 
		                &(synthParams.previousFrame), synthParams.iSamplingRate);
		/* de-emphasize the sound */
		for(i = 0; i < synthParams.sizeHop; i++)
			pSSynthesis[i] = (short) DeEmphasis(pFBuffer[i]);

		WriteToOutputFile (pSSynthesis, synthParams.sizeHop);
		if (iRecord % 10 == 0)
			fprintf(stderr,"%.2f ", iRecord / (float) pSmsHeader->iFrameRate);
	}
  
	WriteOutputFile ();
  
	ClearSine ();
	free (pSmsHeader);
	free (pFBuffer);
	free (pSSynthesis);
	fclose (pSmsFile);
	return (1);
}
