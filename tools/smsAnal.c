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

int verbose = 0;

void usage (void)
{
    fprintf (stderr, "\n"
             "Usage: smsAnal [options]  <inputSoundFile> <outputSmsFile>\n"
             "\n"
             "analyzes a sound file a stores it in a binary SMS file. All parameters are numbers."
             "See the man page for details.\n\n"
             "Options:\n"
             "      --help (this message)\n"
             "      -v    verbose mode (default 0, none)\n"
             "      -d    debugMode (default 0, none)\n"
             "      -f    format (default 0, harmonic)\n"
             "      -q    soundType (default 0, sound phrase)\n"
             "      -x    analysis direction (default 0, direct)\n"
             " STFT parameters:\n"
             "      -s    windowSize (default 3.5 periods)\n"
             "      -i    windowType (default 1, Blackman-Harris 62 db)\n"
             "      -r    frameRate (default 300 hz)\n"
             " Peak Detection parameters:\n"
             "      -j    highestFreq (default 12000 hz)\n"
             "      -k    minPeakMag (default -100 dB)\n"
             " Harmonic Detection parameters:\n"
             "      -y    refHarmonic (default 1)\n"
             "      -m    minRefHarmMag (default 30 dB)\n"
             "      -z    refHarmDiffFromMax (default 30 dB)\n"
             "      -u    defaultFund (default 100 hz)\n"
             "      -l    lowestFund (default 50 hz)\n"
             "      -h    highestFund (default 1000 hz)\n"
             " Peak Continuation parameters:\n"
             "      -n    nGuides (default 100)\n"
             "      -p    nTracks (default 60)\n"
             "      -w    freqDeviation (default .45)\n"
             "      -t    peakContGuide (default .4)\n"
             "      -o    fundContToGuide (default .5)\n"
             " Track Cleaning parameters:\n"
             "      -g    cleanTrack (default 1, yes)\n"
             "      -a    minTrackLength (default .1 seconds)\n"
             "      -b    maxSleepingTime (default .1 seconds)\n"
             " Stochastic Analysis parameters:\n"
             "      -e    stochasticType (default 1, approximated spectrum). 0=none \n"
             "      -c    nStocCoeff (default 64)\n"
             );
        exit(1);
}

/* function to get the user arguments 
 *
 * char *argv[];           command line 
 * int argc;               number of command arguments
 * SMS_AnalParams *pAnalParams;	 analysis parameters structure
 *
 */
static int GetArguments (char *argv[], int argc, SMS_AnalParams *pAnalParams)
{
	int i;
	float frameRateOrOverlapFactor = 0;

	for (i=1; i<argc-2; i++) 
	{
		if (*(argv[i]++) == '-') 
		{
			switch (*(argv[i]++)) 
			{
                        case 'v': verbose = 1;
                                break;
                        case 'd': if (sscanf(argv[i],"%d", 
                                             &pAnalParams->iDebugMode) < 0)
					printf("GetArguments: Invalid debug mode");
                                break;
                        case 'f':  if (sscanf(argv[i],"%d", &pAnalParams->iFormat) < 1) 
					printf("GetArguments: Invalid format");
                                break;
                        case 'q': if (sscanf(argv[i],"%d", 
                                             &pAnalParams->iSoundType) < 0) 
					printf("GetArguments: Invalid sound type");
                                break;
                        case 'x': if (sscanf(argv[i],"%d", 
                                             &pAnalParams->iAnalysisDirection) < 0) 
					printf("GetArguments: Invalid value for analysis direction");
                                break;
                        case 'i': if (sscanf(argv[i],"%d", 
                                             &pAnalParams->iWindowType) < 0) 
					printf("GetArguments: Invalid value for window type");
                                break;
                        case 's': if (sscanf(argv[i],"%f", 
                                             &pAnalParams->fSizeWindow) < 1)
					printf("GetArguments: Invalid window size");
                                break;
                        case 'r':  sscanf(argv[i],"%f", 
                                          &frameRateOrOverlapFactor);
                                break;
                        case 'j': if (sscanf(argv[i],"%f", 
                                             &pAnalParams->fHighestFreq) < 0) 
					printf("GetArguments: Invalid highestFreq");
                                break;
                        case 'k': if (sscanf(argv[i],"%f", 
                                             &pAnalParams->fMinPeakMag) < 0) 
					printf("GetArguments: Invalid minimum peak magnitude");
                                break;
                        case 'y': if (sscanf(argv[i],"%d", 
                                             &pAnalParams->iRefHarmonic) < 1) 
					printf("GetArguments: Invalid reference harmonic");
                                break;
                        case 'm': if (sscanf(argv[i],"%f", 
                                             &pAnalParams->fMinRefHarmMag) < 0) 
					printf("GetArguments: Invalid minimum fundamental magnitude");
                                break;
                        case 'z': if (sscanf(argv[i],"%f", 
                                             &pAnalParams->fRefHarmMagDiffFromMax) < 0) 
					printf("GetArguments: Invalid maximum fundamental magnitude difference \
								from maximum peak");
                                break;
                        case 'l': if (sscanf(argv[i],"%f",
                                             &pAnalParams->fLowestFundamental) < 1) 
					printf("GetArguments: Invalid lowest fundamental");
                                break;
                        case 'h': if (sscanf(argv[i],"%f",
                                             &pAnalParams->fHighestFundamental) < 1) 
					printf("GetArguments: Invalid highest fundamental");
                                break;
                        case 'u': if (sscanf(argv[i],"%f", 
                                             &pAnalParams->fDefaultFundamental) < 1) 
					printf("GetArguments: Invalid default fundamental");
                                break;
                        case 'n': if (sscanf(argv[i],"%d", &pAnalParams->nGuides) < 1) 
					printf("GetArguments: Invalid number of guides");
                                break;
                        case 'p': if (sscanf(argv[i],"%d", 
                                             &pAnalParams->nTracks) < 1) 
					printf("GetArguments: Invalid number of tracks");
                                break;
                        case 'w': if (sscanf(argv[i],"%f", 
                                             &pAnalParams->fFreqDeviation) < 0) 
					printf("GetArguments: Invalid frequency deviation");
                                break;
                        case 't': if (sscanf(argv[i],"%f",
                                             &pAnalParams->fPeakContToGuide) < 0) 
					printf("GetArguments: Invalid peak contribution to guide");
                                break;
                        case 'o': if (sscanf(argv[i],"%f", 
                                             &pAnalParams->fFundContToGuide) < 0) 
					printf("GetArguments: Invalid fundamental contribution to guide");
                                break;
                        case 'g': if (sscanf(argv[i],"%d", 
                                             &pAnalParams->iCleanTracks) < 0) 
					printf("GetArguments: Invalid value for CleanTracks");
                                break;
                        case 'a': if (sscanf(argv[i],"%d",
                                             &pAnalParams->iMinTrackLength) < 0) 
					printf("GetArguments: Invalid minimum track length");
                                break;
                        case 'b': if (sscanf(argv[i],"%d", 
                                             &pAnalParams->iMaxSleepingTime) < 0) 
					printf("GetArguments: Invalid minimum sleeping time");
                                break;
                        case 'c': if (sscanf(argv[i],"%d",
                                             &pAnalParams->nStochasticCoeff) < 1) 
					printf("GetArguments: Invalid number of coefficients");
                                break;
                        case 'e': if (sscanf(argv[i],"%d",
                                             &pAnalParams->iStochasticType) < 0) 
					printf("GetArguments: Invalid stochastic type");
                                break;
                        default:   usage();
			}
		}
	}
  		
	if (frameRateOrOverlapFactor > 0.0)
		pAnalParams->iFrameRate = frameRateOrOverlapFactor;
	else if (frameRateOrOverlapFactor < 0.0)
	{ 
		/* -overlapFactor specified */
                /* overlap ratio K = windowSize * frameRate / 
		   defaultFund = w * r / u => r = K * u / w */
		pAnalParams->iFrameRate = (int) ((-frameRateOrOverlapFactor) * 
			                             pAnalParams->fDefaultFundamental /
			                             pAnalParams->fSizeWindow);
		printf("Frame rate = %d\n", pAnalParams->iFrameRate);
	}


	return (1);
}

int main (int argc, char *argv[])
{
	SMS_AnalParams analParams;

	FILE *pOutputSmsFile; 
	SMS_Data smsData;
	SMS_Header smsHeader;

	float pSoundData[SMS_MAX_WINDOW];
	SMS_SndHeader soundHeader;

	char *pChInputSoundFile = NULL, *pChOutputSmsFile = NULL;
	int iHopSize;
	int iDoAnalysis = 1;
        int iFrame = 0;
	long iStatus = 0, iSample = 0, sizeNewData = 0;

	/* initialize arguments */
        sms_initAnalParams(&analParams);
	/* get user arguments */
	if (argc > 3) 
		GetArguments (argv, argc, &analParams);
	else if (argc < 2 || !strcmp( argv[1], "--help"))
                usage();

	pChInputSoundFile = argv[argc-2];
	pChOutputSmsFile = argv[argc-1];
 
	/* open input sound */
	if (sms_openSF(pChInputSoundFile, &soundHeader))
	{
                printf("error in sms_openSF: %s \n", sms_errorString());
                exit(EXIT_FAILURE);
	}	    

        /* initialize everything */
        sms_init();
        sms_initAnalysis (&analParams, &soundHeader);
        
	sms_fillHeader (&smsHeader, &analParams, "smsAnal");
	sms_writeHeader (pChOutputSmsFile, &smsHeader, &pOutputSmsFile);

	if (analParams.iDebugMode == SMS_DBG_SYNC)
		sms_createDebugFile (&analParams);
	if (analParams.iDebugMode == SMS_DBG_RESIDUAL)
		sms_createResSF (analParams.iSamplingRate);

        if(verbose)
        {
                printf("\n===Sound File Info====\n");
                printf("samples: %d, samplerate: %d, seconds: %f \n", soundHeader.nSamples, soundHeader.iSamplingRate, 
                       soundHeader.nSamples / (float)soundHeader.iSamplingRate);
                printf("number of channels: %d, read channel: %d \n", soundHeader.channelCount, soundHeader.iReadChannel); 
                printf("\n===Analysis Parameters====\n");
		printf("sizeHop: %d, nFrames: %d \n", analParams.sizeHop, analParams.nFrames);
                /* \todo: print analysis window type (by name) here */
		printf("header info string: %s", smsHeader.pChTextCharacters);

        }


        /* allocate output SMS record */
	sms_allocFrameH (&smsHeader, &smsData);

	/* perform analysis */

	if (analParams.iAnalysisDirection == SMS_DIR_REV)
		iSample = soundHeader.nSamples;

        if(verbose) printf("\ndoing analysis now:\n");
        static int loopnum = 0;
	while(iDoAnalysis > 0)
	{
                loopnum++;
		if (analParams.iAnalysisDirection == SMS_DIR_REV)
		{
			if ((iSample - analParams.sizeNextRead) >= 0)
				sizeNewData = analParams.sizeNextRead;
			else
				sizeNewData = iSample;
                        iSample -= sizeNewData;
		}
		else
		{
			iSample += sizeNewData;
			if((iSample + analParams.sizeNextRead) < soundHeader.nSamples)
				sizeNewData = analParams.sizeNextRead;
			else
				sizeNewData = soundHeader.nSamples - iSample;
		}
		/* get one frame of sound */
		if (sms_getSound (&soundHeader, sizeNewData, pSoundData, iSample))
		{
			printf("error: could not read sound frame %d\n", iFrame);
                        printf("error message in sms_getSound: %s \n", sms_errorString());
			break;
		}
		/* perform analysis of one frame of sound */
//                printf("frame: %d, loop: %d, sizeNewData: %ld \n", iFrame, loopnum, sizeNewData);
		iStatus = sms_analyze (sizeNewData, pSoundData, &smsData,
		                       &analParams);

		/* if there is an output SMS record, write it */
		if (iStatus == 1)
		{
			if(sms_writeFrame (pOutputSmsFile, &smsHeader, &smsData))
                        {
                                printf("error: could not write sms frame %d\n", iFrame);
                                printf("error message in sms_writeFrame: %s \n", sms_errorString());
                                break;
                        }
			if(verbose)
                        {
/*                                 if (iFrame % 10 == 0) */
/*                                         printf ("frame: %d, %.2f \n", iFrame, iFrame / (float) smsHeader.iFrameRate); */
                                if (iFrame % 10 == 0)
                                        printf ("%.2f ", iFrame / (float) smsHeader.iFrameRate);

                        }
			iFrame++;
		}
		else if (iStatus == -1) /* done */
		{
			iDoAnalysis = 0;
			smsHeader.nFrames = iFrame;
		}
	}


        smsHeader.fResidualPerc = analParams.fResidualAccumPerc / iFrame;
        if(verbose)
        {
                printf("\n");
                printf("residual percentage: %f \n", smsHeader.fResidualPerc);
        }                
        if(smsHeader.nFrames != analParams.nFrames && verbose)
                printf("warning: wrong number of analyzed frames: analParams: %d, smsHeader: %d \n", 
                       analParams.nFrames, smsHeader.nFrames); 
	/* write an close output files */
	sms_writeFile (pOutputSmsFile, &smsHeader);
	if (analParams.iDebugMode == SMS_DBG_RESIDUAL)
		sms_writeResSF ();
	if (analParams.iDebugMode == SMS_DBG_SYNC)
		sms_writeDebugFile ();

        printf("wrote %d analysis frames to %s\n", iFrame, pChOutputSmsFile);

        /* cleanup */
        sms_freeFrame(&smsData);
        sms_freeAnalysis(&analParams);
        sms_free();
	return 0;	
}

