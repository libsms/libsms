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
 *    main program for smsAnal
 *
 */
#include "sms.h"
#include "smsAnal.h"



void usage (void)
{
    fprintf (stderr, "\n"
             "Usage: smsAnal [options]  <inputSoundFile> <outputSmsFile>\n"
             "\n"
             "analyzes a sound file a stores it in a binary SMS file. All parameters are numbers."
             "See the man page for details.\n\n"
             "Options:\n"
             "      --help (this message)\n"
             "      -d    debugMode (default 0, none)\n"
             "      -f    format (default 1, harmonic)\n"
             "      -q    soundType (default 0, sound phrase)\n"
             "      -x    analysis direction (default 0, direct)\n"
             " STFT parameters:\n"
             "      -s    windowSize (default 3.5 periods)\n"
             "      -i    windowType (default 1, Blackman-Harris 62 db)\n"
             "      -r    frameRate (default 400 hz)\n"
             " Peak Detection parameters:\n"
             "      -j    highestFreq (default 12000 hz)\n"
             "      -k    minPeakMag (default 0 dB)\n"
             " Harmonic Detection parameters:\n"
             "      -y    refHarmonic (default 1)\n"
             "      -m    minRefHarmMag (default 30 dB)\n"
             "      -z    refHarmDiffFromMax (default 30 dB)\n"
             "      -u    defaultFund (default 100 hz)\n"
             "      -l    lowestFund (default 50 hz)\n"
             "      -h    highestFund (default 1000 hz)\n"
             " Peak Continuation parameters:\n"
             "      -n    nGuides (default 100)\n"
             "      -p    nTrajectories (default 60)\n"
             "      -v    freqDeviation (default .45)\n"
             "      -t    peakContGuide (default .4)\n"
             "      -o    fundContToGuide (default .5)\n"
             " Trajectory Cleaning parameters:\n"
             "      -g    cleanTaj (default 1, yes)\n"
             "      -a    minTrajLength (default .1 seconds)\n"
             "      -v    maxSleepingTime (default .1 seconds)\n"
             " Stochastic Analysis parameters:\n"
             "      -e    stochasticType (default 2, approximated spectrum)\n"
             "      -c    nStocCoeff (default 32)\n"
             );
        exit(1);
}


/* function to compute the SMS representation from a sound file
 *
 * SMS_SndHeader *pSoundHeader; 	header input soundfile
 * SMS_Header *pSmsHeader;		  pointer to SMS header
 * FILE *pSmsFile;            pointer to output SMS file
 * ANAL_PARAMS *pAnalParams;		analysis parameters
 *
 */
/* static int ComputeSms (SMS_SndHeader *pSoundHeader, SMS_Header *pSmsHeader, */
/*                        FILE *pSmsFile, ANAL_PARAMS *pAnalParams) */
/* { */
/* 	short pSoundData[MAX_SIZE_WINDOW]; */
/* 	SMS_DATA smsData; */
/* 	long iStatus = 0, iSample = 0, iNextSizeRead = 0, sizeNewData = 0; */
/* 	short iDoAnalysis = 1, iRecord = 0; */
/*         /\* allocate output SMS record *\/ */
/* 	AllocSmsRecord (pSmsHeader, &smsData); */

/* 	iNextSizeRead = (pAnalParams->iDefaultSizeWindow + 1) / 2.0; */

  
/* 	if (pAnalParams->iAnalysisDirection == REVERSE) */
/* 		iSample = pSoundHeader->nSamples; */

/* 	/\* loop for analysis *\/ */
/* 	while(iDoAnalysis > 0) */
/* 	{ */

/* 		if (pAnalParams->iAnalysisDirection == REVERSE) */
/* 		{ */
/* 			if ((iSample - iNextSizeRead) >= 0) */
/* 				sizeNewData = iNextSizeRead; */
/* 			else */
/* 				sizeNewData = iSample; */
/* 				iSample -= sizeNewData; */
/* 		} */
/* 		else */
/* 		{ */
/* 			iSample += sizeNewData; */
/* 			if((iSample + iNextSizeRead) < pSoundHeader->nSamples) */
/* 				sizeNewData = iNextSizeRead; */
/* 			else */
/* 				sizeNewData = pSoundHeader->nSamples - iSample; */
/* 		} */
/* 		/\* get one frame of sound *\/ */
/* 		if (GetSoundData (pSoundHeader, pSoundData, sizeNewData, iSample) < 0) */
/* 		{ */
/* 			fprintf(stderr, "ComputeSms: could not read sound record %d\n", iRecord); */
/* 			break; */
/* 		} */
/* 		/\* perform analysis of one frame of sound *\/ */
/* 		iStatus = SmsAnalysis (pSoundData, sizeNewData, &smsData,  */
/* 		                       pAnalParams, &iNextSizeRead); */

/* 		/\* if there is an output SMS record, write it *\/ */
/* 		if (iStatus == 1) */
/* 		{ */
/* 			WriteSmsRecord (pSmsFile, pSmsHeader, &smsData); */
/* 			if(1)//todo: add verbose flag */
/*                         { */
/*                                 if (iRecord % 10 == 0) */
/*                                         fprintf (stderr, "%.2f ", */
/*                                                  iRecord / (float) pSmsHeader->iFrameRate); */
/*                         } */
/* 			iRecord++; */
/* 		} */
/* 		else if (iStatus == -1) /\* done *\/ */
/* 		{ */
/* 			iDoAnalysis = 0; */
/* 			pSmsHeader->nRecords = iRecord; */
/* 		} */

/* 	} */
/*         printf("\n"); */
/* 	pSmsHeader->fResidualPerc = pAnalParams->fResidualPercentage / iRecord; */
/* 	return (1); */
/* } */

/* function to initialize the input command  arguments
 * 
 * ARGUMENTS *pArguments;	command arguments 
 *
 */
static int InitArguments (ARGUMENTS *pArguments)
{
	pArguments->iDebugMode = 0;
	pArguments->iFormat = SMS_FORMAT_H;
	pArguments->iSoundType = TYPE_MELODY;
	pArguments->iAnalysisDirection = DIRECT;
	pArguments->fWindowSize = 3.5;
	pArguments->iWindowType = BLACKMAN_HARRIS_70;
	pArguments->iFrameRate = 400;
	pArguments->fHighestFreq = 12000.;
	pArguments->fMinPeakMag = 0;
	pArguments->fFreqDeviation = .45;
	pArguments->iRefHarmonic = 1;
	pArguments->fMinRefHarmMag = 30;
	pArguments->fRefHarmMagDiffFromMax = 30;
	pArguments->fDefaultFund = 100;
	pArguments->fLowestFund = 50;
	pArguments->fHighestFund = 1000;
	pArguments->nGuides = 100;
	pArguments->nTrajectories = 60;
	pArguments->fPeakContToGuide = .4;
	pArguments->fFundContToGuide = .5;
	pArguments->iCleanTraj = 1;
	pArguments->fMinTrajLength = .1;
	pArguments->fMaxSleepingTime = .1;
	pArguments->iStochasticType =STOC_APPROX;
	pArguments->nStochasticCoeff = 32;
	return (1);
}

/* function to compute the record size from the command arguments
 *
 * ARGUMENTS *pArguments;	pointer to command arguments 
 *
 */
/* static int ComputeRecordBSize (ARGUMENTS *pArguments, int iHopSize) */
/* { */
/* 	int iSize, nDet; */
  
/* 	if (pArguments->iFormat == SMS_FORMAT_H || */
/* 	    pArguments->iFormat == SMS_FORMAT_IH) */
/* 		nDet = 2;// freq, mag */
/*         else nDet = 3; // freq, mag, phase */

/* 	iSize = sizeof (float) * (nDet * pArguments->nTrajectories); */

/* 	if (pArguments->iStochasticType == STOC_WAVEFORM) */
/*         {       //numSamples */
/*                 iSize += sizeof(float) * iHopSize; */
/*         } */
/*         else if(pArguments->iStochasticType == STOC_IFFT) */
/*         { */
/*                 //sizeFFT*2 */
/*         } */
/*         else if(pArguments->iStochasticType == STOC_APPROX) */
/*         {       //stocCoeff + 1 (gain) */
/*                 iSize += sizeof(float) * (pArguments->nStochasticCoeff + 1); */
/*         } */
/*         printf("iSize: %d \n", iSize); */
/* 	return (iSize); */
/* }	      */

/* function to get the user arguments 
 *
 * char *argv[];           command line 
 * int argc;               number of command arguments
 * ARGUMENTS *pArguments;	 command arguments
 *
 */
static int GetArguments (char *argv[], int argc, ARGUMENTS *pArguments)
{
	int i;
	float frameRateOrOverlapFactor = 0;

	for (i=1; i<argc-2; i++) 
	{
		if (*(argv[i]++) == '-') 
		{
			switch (*(argv[i]++)) 
			{
				case 'd': if (sscanf(argv[i],"%d", 
				              &pArguments->iDebugMode) < 0)
					printf("GetArguments: Invalid debug mode");
					break;
				case 'f':  if (sscanf(argv[i],"%d", &pArguments->iFormat) < 1) 
					printf("GetArguments: Invalid format");
					break;
				case 'q': if (sscanf(argv[i],"%d", 
				              &pArguments->iSoundType) < 0) 
					printf("GetArguments: Invalid sound type");
					break;
				case 'x': if (sscanf(argv[i],"%d", 
				              &pArguments->iAnalysisDirection) < 0) 
					printf("GetArguments: Invalid value for analysis direction");
					break;
				case 'i': if (sscanf(argv[i],"%d", 
				              &pArguments->iWindowType) < 0) 
					printf("GetArguments: Invalid value for window type");
					break;
				case 's': if (sscanf(argv[i],"%f", 
				              &pArguments->fWindowSize) < 1)
					printf("GetArguments: Invalid window size");
					break;
				case 'r':  sscanf(argv[i],"%f", 
						  &frameRateOrOverlapFactor);
					break;
				case 'j': if (sscanf(argv[i],"%f", 
				              &pArguments->fHighestFreq) < 0) 
					printf("GetArguments: Invalid highestFreq");
					break;
				case 'k': if (sscanf(argv[i],"%f", 
				              &pArguments->fMinPeakMag) < 0) 
					printf("GetArguments: Invalid minimum peak magnitude");
					break;
				case 'y': if (sscanf(argv[i],"%d", 
				              &pArguments->iRefHarmonic) < 1) 
					printf("GetArguments: Invalid reference harmonic");
					break;
				case 'm': if (sscanf(argv[i],"%f", 
				              &pArguments->fMinRefHarmMag) < 0) 
					printf("GetArguments: Invalid minimum fundamental magnitude");
					break;
				case 'z': if (sscanf(argv[i],"%f", 
				              &pArguments->fRefHarmMagDiffFromMax) < 0) 
					printf("GetArguments: Invalid maximum fundamental magnitude difference \
								from maximum peak");
					break;
				case 'l': if (sscanf(argv[i],"%f",
				              &pArguments->fLowestFund) < 1) 
					printf("GetArguments: Invalid lowest fundamental");
					break;
				case 'h': if (sscanf(argv[i],"%f",
				              &pArguments->fHighestFund) < 1) 
					printf("GetArguments: Invalid highest fundamental");
					break;
				case 'u': if (sscanf(argv[i],"%f", 
				              &pArguments->fDefaultFund) < 1) 
					printf("GetArguments: Invalid default fundamental");
					break;
				case 'n': if (sscanf(argv[i],"%d", &pArguments->nGuides) < 1) 
					printf("GetArguments: Invalid number of guides");
					break;
				case 'p': if (sscanf(argv[i],"%d", 
				              &pArguments->nTrajectories) < 1) 
					printf("GetArguments: Invalid number of trajectories");
					break;
				case 'v': if (sscanf(argv[i],"%f", 
				              &pArguments->fFreqDeviation) < 0) 
					printf("GetArguments: Invalid frequency deviation");
					break;
				case 't': if (sscanf(argv[i],"%f",
				              &pArguments->fPeakContToGuide) < 0) 
					printf("GetArguments: Invalid peak contribution to guide");
					break;
				case 'o': if (sscanf(argv[i],"%f", 
				              &pArguments->fFundContToGuide) < 0) 
					printf("GetArguments: Invalid fundamental contribution to guide");
					break;
				case 'g': if (sscanf(argv[i],"%d", 
				              &pArguments->iCleanTraj) < 0) 
					printf("GetArguments: Invalid value for CleanTraj");
					break;
				case 'a': if (sscanf(argv[i],"%f",
				              &pArguments->fMinTrajLength) < 0) 
					printf("GetArguments: Invalid minimum trajectory length");
					break;
				case 'b': if (sscanf(argv[i],"%f", 
				              &pArguments->fMaxSleepingTime) < 0) 
					printf("GetArguments: Invalid minimum sleeping time");
					break;
				case 'c': if (sscanf(argv[i],"%d",
				              &pArguments->nStochasticCoeff) < 1) 
					printf("GetArguments: Invalid number of coefficients");
					break;
				case 'e': if (sscanf(argv[i],"%d",
				              &pArguments->iStochasticType) < 0) 
					printf("GetArguments: Invalid stochastic type");
					break;
                        default:   usage();
			}
		}
	}
  		
	if (frameRateOrOverlapFactor > 0.0)
		pArguments->iFrameRate = frameRateOrOverlapFactor;
	else if (frameRateOrOverlapFactor < 0.0)
	{ 
		/* -overlapFactor specified */
      	/* overlap ratio K = windowSize * frameRate / 
		   defaultFund = w * r / u => r = K * u / w */
		pArguments->iFrameRate = (int) ((-frameRateOrOverlapFactor) * 
			                             pArguments->fDefaultFund /
			                             pArguments->fWindowSize);
		printf("Frame rate = %d\n", pArguments->iFrameRate);
	}


	return (1);
}

/* function to fill SMS header of the output file
 *
 * SMS_Header *pSmsHeader; 	pointer to SMS header
 * int iRecordBSize;		size in bytes of an output record
 * int nRecords;		number of records in output file
 * ARGUMENTS arguments;		user arguments
 *
 */
char pChTextString[1024]; // RTE TODO: encapsulate this somehow

static int FillSmsHeader (SMS_Header *pSmsHeader, 
                          int nRecords, ARGUMENTS arguments,
                          int iOriginalSRate, int iHopSize)
{

        InitSmsHeader (pSmsHeader);

        pSmsHeader->nRecords = nRecords;
        pSmsHeader->iFormat = arguments.iFormat;
        pSmsHeader->iFrameRate = arguments.iFrameRate;
        pSmsHeader->iStochasticType = arguments.iStochasticType;
        pSmsHeader->nTrajectories = arguments.nTrajectories;
	if(arguments.iStochasticType != STOC_APPROX)
		pSmsHeader->nStochasticCoeff = 0;
        else
                pSmsHeader->nStochasticCoeff = arguments.nStochasticCoeff;
        pSmsHeader->iOriginalSRate = iOriginalSRate;
        pSmsHeader->iRecordBSize = GetRecordBSize(pSmsHeader);

        sprintf (pChTextString, 
                 "format %d, soundType %d, analysisDirection %d, windowSize %.2f,"
                 " windowType %d, frameRate %d, highestFreq %.2f, minPeakMag %.2f,"
                 " refHarmonic %d, minRefHarmMag %.2f, refHarmMagDiffFromMax %.2f,"
                 " defaultFund %.2f, lowestFund %.2f, highestFund %.2f, nGuides %d,"
                 " nTrajectories %d, freqDeviation %.2f, peakContToGuide %.2f,"
                 " fundContToGuide %.2f, cleantTraj %d, minTrajLength %.2f,"
                 "maxSleepingTime %.2f, stochasticType %d, nStocCoeff %d\n", 	
                 arguments.iFormat, arguments.iSoundType,
                 arguments.iAnalysisDirection, arguments.fWindowSize, 
                 arguments.iWindowType, arguments.iFrameRate,
                 arguments.fHighestFreq, arguments.fMinPeakMag,
                 arguments.iRefHarmonic, arguments.fMinRefHarmMag, 
                 arguments.fRefHarmMagDiffFromMax,  
                 arguments.fDefaultFund, arguments.fLowestFund,
                 arguments.fHighestFund, arguments.nGuides,
                 arguments.nTrajectories, arguments.fFreqDeviation, 
                 arguments.fPeakContToGuide, arguments.fFundContToGuide,
                 arguments.iCleanTraj, arguments.fMinTrajLength,
                 arguments.fMaxSleepingTime,  arguments.iStochasticType,
                 arguments.nStochasticCoeff);
       
        pSmsHeader->nTextCharacters = strlen (pChTextString) + 1;
        pSmsHeader->pChTextCharacters = (char *) pChTextString;
        
        return (1);
}
   
/* function to fill the analysis parameters from the user arguments
 *
 * ARGUMENTS arguments;			      user arguments
 * ANAL_PARAMS *pAnalParams;    	analysis parameters
 * SMS_SndHeader *pSoundHeader;	      pointer to header of input sound
 * int iHopSize;			            hop size of analysis frame
 *
 */  
static int FillAnalParams (ARGUMENTS arguments, ANAL_PARAMS *pAnalParams,
                           SMS_SndHeader *pSoundHeader, int iHopSize)
{
	/* fill analysis parameters structure */
	pAnalParams->iDebugMode = arguments.iDebugMode;
	pAnalParams->iFormat = arguments.iFormat;
	pAnalParams->iSoundType = arguments.iSoundType;
	pAnalParams->iAnalysisDirection = arguments.iAnalysisDirection;
	pAnalParams->iSizeSound = pSoundHeader->nSamples;
	pAnalParams->iWindowType = arguments.iWindowType;
	pAnalParams->iSamplingRate = pSoundHeader->iSamplingRate;
	pAnalParams->fSizeWindow = arguments.fWindowSize;
	pAnalParams->iDefaultSizeWindow = 
		(int)((pAnalParams->iSamplingRate / arguments.fDefaultFund) *
		pAnalParams->fSizeWindow / 2) * 2 + 1; /* odd length */
	pAnalParams->sizeHop = iHopSize;
	pAnalParams->fHighestFreq = arguments.fHighestFreq;
	pAnalParams->fMinPeakMag = arguments.fMinPeakMag;
	pAnalParams->iStochasticType = arguments.iStochasticType;  
	pAnalParams->fLowestFundamental = arguments.fLowestFund;
	pAnalParams->fHighestFundamental = arguments.fHighestFund;
	pAnalParams->fDefaultFundamental = arguments.fDefaultFund;
	pAnalParams->fPeakContToGuide = arguments.fPeakContToGuide;
	pAnalParams->fFundContToGuide = arguments.fFundContToGuide;
	pAnalParams->fFreqDeviation = arguments.fFreqDeviation;
	pAnalParams->nGuides = MAX (arguments.nGuides, arguments.nTrajectories);
	pAnalParams->iCleanTraj = arguments.iCleanTraj;
	pAnalParams->fMinRefHarmMag = arguments.fMinRefHarmMag;
	pAnalParams->fRefHarmMagDiffFromMax = arguments.fRefHarmMagDiffFromMax;
	pAnalParams->iRefHarmonic = arguments.iRefHarmonic;
	pAnalParams->iMinTrajLength = 
		arguments.fMinTrajLength * arguments.iFrameRate;
	pAnalParams->iMaxSleepingTime = 
		arguments.fMaxSleepingTime * arguments.iFrameRate;
	pAnalParams->iMaxDelayFrames = 
		MAX(pAnalParams->iMinTrajLength, pAnalParams->iMaxSleepingTime) + 2 +
			DELAY_FRAMES;
	pAnalParams->fResidualPercentage = 0;

	return (1);

}

/* main of the program
 *
 */
int main (int argc, char *argv[])
{
	ARGUMENTS arguments;
	ANAL_PARAMS analParams;

	FILE *pOutputSmsFile; 
	SMS_DATA smsData;
	SMS_Header smsHeader;

	short pSoundData[MAX_SIZE_WINDOW];
	SMS_SndHeader SoundHeader;

	char *pChInputSoundFile = NULL, *pChOutputSmsFile = NULL;
	int iHopSize, nRecords;
	long iStatus = 0, iSample = 0, iNextSizeRead = 0, sizeNewData = 0;
	short iDoAnalysis = 1, iRecord = 0;

	/* initialize arguments */
	InitArguments (&arguments);

	/* get user arguments */
	if (argc > 3) 
		GetArguments (argv, argc, &arguments);
	else if (argc < 2 || !strcmp( argv[1], "--help"))
                usage();

	pChInputSoundFile = argv[argc-2];
	pChOutputSmsFile = argv[argc-1];
 
	/* open input sound */
	OpenSound (pChInputSoundFile, &SoundHeader);

	/* check default fundamental */
	if (arguments.fDefaultFund < arguments.fLowestFund)
	{
		arguments.fDefaultFund = arguments.fLowestFund;
		fprintf (stderr,"fDefaultFundamental set to %f \n", 
			    arguments.fDefaultFund);
	}
	if (arguments.fDefaultFund > arguments.fHighestFund)
	{
		arguments.fDefaultFund = arguments.fHighestFund;
		fprintf (stderr,"fDefaultFundamental set to %f \n", 
		         arguments.fDefaultFund);
	}

        /* define the hopsize for each record */
	iHopSize = (int)(SoundHeader.iSamplingRate / 
	                (float) arguments.iFrameRate);
        /* define how many records*/
	nRecords = 3 + SoundHeader.nSamples / (float) iHopSize;

        /* initialize everything */
	FillAnalParams (arguments, &analParams, &SoundHeader, iHopSize);
	FillSmsHeader (&smsHeader, nRecords, arguments,
	               SoundHeader.iSamplingRate, iHopSize);
	WriteSmsHeader (pChOutputSmsFile, &smsHeader, &pOutputSmsFile);
	if (analParams.iDebugMode == DEBUG_SYNC)
		CreateDebugFile (&analParams);
	if (analParams.iDebugMode == DEBUG_RESIDUAL)
		CreateResidualFile (&analParams);
        SmsInit();
        SmsInitAnalysis (&smsHeader, &analParams);

        /* allocate output SMS record */
	AllocSmsRecord (&smsHeader, &smsData);

	/* perform analysis */
	//ComputeSms (&SoundHeader, &smsHeader, pOutputSmsFile, &analParams);
    
	iNextSizeRead = (analParams.iDefaultSizeWindow + 1) / 2.0;

	if (analParams.iAnalysisDirection == REVERSE)
		iSample = SoundHeader.nSamples;

	/* loop for analysis */
	while(iDoAnalysis > 0)
	{

		if (analParams.iAnalysisDirection == REVERSE)
		{
			if ((iSample - iNextSizeRead) >= 0)
				sizeNewData = iNextSizeRead;
			else
				sizeNewData = iSample;
				iSample -= sizeNewData;
		}
		else
		{
			iSample += sizeNewData;
			if((iSample + iNextSizeRead) < SoundHeader.nSamples)
				sizeNewData = iNextSizeRead;
			else
				sizeNewData = SoundHeader.nSamples - iSample;
		}
		/* get one frame of sound */
		if (GetSoundData (&SoundHeader, pSoundData, sizeNewData, iSample) < 0)
		{
			fprintf(stderr, "ComputeSms: could not read sound record %d\n", iRecord);
			break;
		}
		/* perform analysis of one frame of sound */
		iStatus = SmsAnalysis (pSoundData, sizeNewData, &smsData, 
		                       &analParams, &iNextSizeRead);

		/* if there is an output SMS record, write it */
		if (iStatus == 1)
		{
			WriteSmsRecord (pOutputSmsFile, &smsHeader, &smsData);
			if(1)//todo: add verbose flag
                        {
                                if (iRecord % 10 == 0)
                                        fprintf (stderr, "%.2f ",
                                                 iRecord / (float) smsHeader.iFrameRate);
                        }
			iRecord++;
		}
		else if (iStatus == -1) /* done */
		{
			iDoAnalysis = 0;
			smsHeader.nRecords = iRecord;
		}

	}
        printf("\n");
	
        smsHeader.fResidualPerc = analParams.fResidualPercentage / iRecord;

	/* write an close output files */
	WriteSmsFile (pOutputSmsFile, &smsHeader);
	if (analParams.iDebugMode == DEBUG_RESIDUAL)
		WriteResidualFile ();
	if (analParams.iDebugMode == DEBUG_SYNC)
		WriteDebugFile ();

        /* cleanup */
        SmsFreeAnalysis(&analParams);
	return 0;	
}

