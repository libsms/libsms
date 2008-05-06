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

short MaxDelayFrames;
float FResidualPerc;
char pChTextString[1024];

#define USAGE "Usage: smsAnal [-d debugMode][-f format][-q soundType][-x analysisDirection][-s windowSize][-i windowType][-r frameRate][-j highestFreq][-k minPeakMag][-y refHarmonic][-u defaultFund][-l lowestFund][-h highestFund][-m minRefHarmMag][-z refHarmMagDiffFromMax][-n nGuides][-p nTrajectories][-v freqDeviation][-t peakContToGuide][-o fundContToGuide][-g cleanTraj][-a minTrajLength][-b maxSleepingTime][-e stochasticType][-c nStocCoeff] <inputSoundFile> <outputSmsFile>\n"


/* function to compute the SMS representation from a sound file
 *
 * SNDHeader *pSoundHeader; 	header input soundfile
 * SMSHeader *pSmsHeader;		  pointer to SMS header
 * FILE *pSmsFile;            pointer to output SMS file
 * ANAL_PARAMS analParams;		analysis parameters
 *
 */
static int ComputeSms (SNDHeader *pSoundHeader, SMSHeader *pSmsHeader,
                       FILE *pSmsFile, ANAL_PARAMS analParams)
{
	short pSoundData[MAX_SIZE_WINDOW];
	SMS_DATA smsData;
	long iStatus = 0, iSample = 0, iNextSizeRead = 0, sizeNewData = 0;
	short iDoAnalysis = 1, iRecord = 0;

        /* allocate output SMS record */
	AllocSmsRecord (pSmsHeader, &smsData);
      
	iNextSizeRead = (analParams.iDefaultSizeWindow + 1) / 2.0;
  
	if (analParams.iAnalysisDirection == REVERSE)
		iSample = pSoundHeader->nSamples;

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
			if((iSample + iNextSizeRead) < pSoundHeader->nSamples)
				sizeNewData = iNextSizeRead;
			else
				sizeNewData = pSoundHeader->nSamples - iSample;
		}

		/* get one frame of sound */
		if (GetSoundData (pSoundHeader, pSoundData, sizeNewData, iSample) < 0)
		{
			fprintf(stderr, "ComputeSms: could not read sound record %d\n", iRecord);
			break;
		}
		/* perform analysis of one frame of sound */
		iStatus = SmsAnalysis (pSoundData, sizeNewData, &smsData, 
		                       analParams, &iNextSizeRead);


		/* if there is an output SMS record, write it */
		if (iStatus == 1)
		{
			WriteSmsRecord (pSmsFile, pSmsHeader, &smsData);
			if (iRecord % 10 == 0)
				fprintf (stderr,"%.2f ", 
				         iRecord / (float) pSmsHeader->iFrameRate);
			iRecord++;
		}
		else if (iStatus == -1)
		{
			iDoAnalysis = 0;
			pSmsHeader->nRecords = iRecord;
		}
	}
        fprintf(stderr, "\n");
	pSmsHeader->fResidualPerc = FResidualPerc / iRecord;
	return (1);
}

/* function to initialize the input command  arguments
 * 
 * ARGUMENTS *pArguments;	command arguments 
 *
 */
static int InitArguments (ARGUMENTS *pArguments)
{
	pArguments->iDebugMode = 0;
	pArguments->iFormat = FORMAT_HARMONIC;
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

/* save the arguments as a string in the header of the SMS file
 *
 * ARGUMENTS arguments;		user arguments
 *
 */
static int SaveArguments (ARGUMENTS arguments)
{
  sprintf (pChTextString, 
           "format %d, soundType %d, analysisDirection %d, windowSize %.2f, windowType %d, frameRate %d, highestFreq %.2f, minPeakMag %.2f, refHarmonic %d, minRefHarmMag %.2f, refHarmMagDiffFromMax %.2f, defaultFund %.2f, lowestFund %.2f, highestFund %.2f, nGuides %d, nTrajectories %d, freqDeviation %.2f, peakContToGuide %.2f, fundContToGuide %.2f, cleantTraj %d, minTrajLength %.2f, maxSleepingTime %.2f, stochasticType %d, nStocCoeff %d\n", 	
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
  
  return (1);
}
 
/* function to compute the record size from the command arguments
 *
 * ARGUMENTS *pArguments;	pointer to command arguments 
 *
 */
static int ComputeRecordBSize (ARGUMENTS *pArguments, int iHopSize)
{
	int iSize, nDet;
  
	if (pArguments->iFormat == FORMAT_HARMONIC ||
	    pArguments->iFormat == FORMAT_INHARMONIC)
		nDet = 2;// freq, mag
        else nDet = 3; // freq, mag, phase

	iSize = sizeof (float) * (nDet * pArguments->nTrajectories);

	if (pArguments->iStochasticType == STOC_WAVEFORM)
        {       //numSamples
                iSize += sizeof(float) * iHopSize;
        }
        else if(pArguments->iStochasticType == STOC_STFT)
        {
                //sizeFFT*2
        }
        else if(pArguments->iStochasticType == STOC_APPROX)
        {       //stocCoeff + 1 (gain)
                iSize += sizeof(float) * (pArguments->nStochasticCoeff + 1);
        }
	return (iSize);
}	     

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
				default:   printf(USAGE);
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
 * SMSHeader *pSmsHeader; 	pointer to SMS header
 * int iRecordBSize;		size in bytes of an output record
 * int nRecords;		number of records in output file
 * ARGUMENTS arguments;		user arguments
 *
 */
static int FillSmsHeader (SMSHeader *pSmsHeader, int iRecordBSize, 
                          int nRecords, ARGUMENTS arguments,
                          int iSamplingRate, int iHopSize)
{
        InitSmsHeader (pSmsHeader);
        pSmsHeader->iRecordBSize = iRecordBSize;
        pSmsHeader->nRecords = nRecords;
        pSmsHeader->iFormat = arguments.iFormat;
        pSmsHeader->iFrameRate = arguments.iFrameRate;
        pSmsHeader->iStochasticType = arguments.iStochasticType;
        pSmsHeader->nTrajectories = arguments.nTrajectories;
        pSmsHeader->nStochasticCoeff = arguments.nStochasticCoeff;
        pSmsHeader->iOriginalSRate = iSamplingRate;
        pSmsHeader->sizeHop = iHopSize;

        SaveArguments (arguments);
       
        pSmsHeader->nTextCharacters = strlen (pChTextString) + 1;
        pSmsHeader->pChTextCharacters = (char *) pChTextString;
        
        return (1);
}
   
/* function to fill the analysis parameters from the user arguments
 *
 * ARGUMENTS arguments;			      user arguments
 * ANAL_PARAMS *pAnalParams;    	analysis parameters
 * SNDHeader *pSoundHeader;	      pointer to header of input sound
 * int iHopSize;			            hop size of analysis frame
 *
 */  
static int FillAnalParams (ARGUMENTS arguments, ANAL_PARAMS *pAnalParams,
                           SNDHeader *pSoundHeader, int iHopSize)
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
	MaxDelayFrames = 
		MAX(pAnalParams->iMinTrajLength, pAnalParams->iMaxSleepingTime) + 2 +
			DELAY_FRAMES;
	FResidualPerc = 0;
	return (1);
}

/* main of the program
 *
 */
int main (int argc, char *argv[])
{

	char *pChInputSoundFile = NULL, *pChOutputSmsFile = NULL;
	int iRecordBSize, iHopSize, nRecords;
	ARGUMENTS arguments;
	SMSHeader smsHeader;
	ANAL_PARAMS analParams;
	SNDHeader SoundHeader;
	FILE *pOutputSmsFile; 

	/* initialize arguments */
	InitArguments (&arguments);

	/* get user arguments */
	if (argc > 3) 
		GetArguments (argv, argc, &arguments);
	else if (argc < 2)
                {
                        fprintf(stderr, USAGE);
                        exit(1);
                }
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

	/* check if no stochastic component */ //don't think I need this anymore... set above
	if(arguments.iStochasticType != STOC_APPROX)
		arguments.nStochasticCoeff = 0;


	iHopSize = (int)(SoundHeader.iSamplingRate / 
	                (float) arguments.iFrameRate);
	iRecordBSize = ComputeRecordBSize (&arguments, iHopSize);
	nRecords = 3 + SoundHeader.nSamples / (float) iHopSize;

	FillAnalParams (arguments, &analParams, &SoundHeader, iHopSize);
	FillSmsHeader (&smsHeader, iRecordBSize, nRecords, arguments,
	               SoundHeader.iSamplingRate, iHopSize);
	WriteSmsHeader (pChOutputSmsFile, &smsHeader, &pOutputSmsFile);
	if (analParams.iDebugMode == DEBUG_SYNC)
		CreateDebugFile (analParams);
	if (analParams.iDebugMode == DEBUG_RESIDUAL)
		CreateResidualFile (analParams);
		
	/* perform analysis */
	ComputeSms (&SoundHeader, &smsHeader, pOutputSmsFile, analParams);
    
	/* write an close output files */
	WriteSmsFile (pOutputSmsFile, &smsHeader);
	if (analParams.iDebugMode == DEBUG_RESIDUAL)
		WriteResidualFile ();
	if (analParams.iDebugMode == DEBUG_SYNC)
		WriteDebugFile ();

	return 0;	
}

