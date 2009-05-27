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
#include <popt.h>

const char *help_header_text =
        "\n\n"
        "Usage: smsAnal [options]  <inputSoundFile> <outputSmsFile>\n"
        "\n"
        "analyzes a sound file a stores it in a binary SMS file. All parameters are numbers. "
        "See the man page for details.\n\n"
        "Options:\n";

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
             " Spectral Envelope parameters:\n"
             "      -blahhhhhh don't have any more room!! time to use argp.h?"
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
/*                         case 'v': verbose = 1; */
/*                                 break; */
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

int main (int argc, const char *argv[])
{
	FILE *pOutputSmsFile; 
	SMS_Data smsData;
	SMS_Header smsHeader;

	float pSoundData[SMS_MAX_WINDOW];
	SMS_SndHeader soundHeader;

	char *pChInputSoundFile = NULL, *pChOutputSmsFile = NULL;
        int verbose = 0;
	int iHopSize;
	int iDoAnalysis = 1;
        int iFrame = 0;
	long iStatus = 0, iSample = 0, sizeNewData = 0;

	int optc;   /* switch */
	poptContext pc;

	SMS_AnalParams analParams;
        sms_initAnalParams(&analParams); 	/* initialize arguments to defaults*/

        struct poptOption options[] =
                {
                        {"verbose", 'v', POPT_ARG_NONE, &verbose, 0, 
                         "verbose mode", 0},
                        {"debug", 'd', POPT_ARG_INT, &analParams.iDebugMode, 0, 
                         "debug mode (0)", "int"},
                        {"format", 'f', POPT_ARG_INT, &analParams.iFormat, 0, 
                         "analysis format (0, harmonic)", "int"},
                        {"sound-type", 'q', POPT_ARG_INT, &analParams.iSoundType, 0, 
                         "sound type (0, phrase)", "int"},
                        {"direction", 'x', POPT_ARG_INT, &analParams.iAnalysisDirection, 0, 
                         "analysis direction (0, forward)", "int"},
                        /* STFT Parameters: */
                        {"window-size", 's', POPT_ARG_FLOAT, &analParams.fSizeWindow, 0, 
                         "size of the window in f0 periods (3.5)", "float"},
                        {"window-type", 'i', POPT_ARG_INT, &analParams.iWindowType, 0, 
                         "window type (1, blackman harris 70 dB)", "int"},
                        {"frame-rate", 'r', POPT_ARG_INT, &analParams.iFrameRate, 0, 
                         "frame rate in hertz (300)", "int"},
                        /* Peak Detection Parameters */
                        {"highest-freq", 'j', POPT_ARG_FLOAT, &analParams.fHighestFreq, 0, 
                         "highest frequency to look for peaks (12000hz)", "float"},
                        {"min-peak-mag", 'k', POPT_ARG_FLOAT, &analParams.fMinPeakMag, 0, 
                         "minimum peak magnitude (0 normalized dB, which corresponds to -100dB)", "float"}, /*\todo check this doc*/
                        /* Harmonic Detection Parameters */
                        {"ref-harmonic", 'y', POPT_ARG_INT, &analParams.iRefHarmonic, 0, 
                         "reference harmonic number in series (1)", "int"},
                        {"min-ref-harm-mag", 'm', POPT_ARG_FLOAT, &analParams.fMinPeakMag, 0, 
                         "minimum reference harmonic magnitude (30 normalized dB)", "float"}, /*\todo check this doc*/
                        /* spectral enveloping parameters */
                        {"spec-env-type", 'e', POPT_ARG_INT, &analParams.specEnvParams.iType, 0, 
                         "spectral enveloping type (0, off)", "int"},
                        {"cep-order", 'o', POPT_ARG_INT, &analParams.specEnvParams.iOrder, 0, 
                         "discrete cepstrum order (25)", "int"},
                        {"lambda", 'l', POPT_ARG_FLOAT, &analParams.specEnvParams.fLambda, 0, 
                         "lambda, regularizing coefficient (0.00001)", "float"}, 
                        {"anchor", 'a', POPT_ARG_NONE, &analParams.specEnvParams.iAnchor, 0, 
                         "turn on anchoring of spectral envelope endpoints", 0}, 
                        POPT_AUTOHELP
                        POPT_TABLEEND
                };


        pc = poptGetContext("smsAnal", argc, argv, options, 0);
        poptSetOtherOptionHelp(pc, help_header_text);

        while ((optc = poptGetNextOpt(pc)) > 0) {
                switch (optc) {
                        /* specific arguments are handled here */
                case 'v':
                        verbose = 1;
                default:
                        ;
                }
        }
        if (optc < -1) 
        {
                /* an error occurred during option processing */
                printf("%s: %s\n",
                       poptBadOption(pc, POPT_BADOPTION_NOALIAS),
                       poptStrerror(optc));
                return 1;
        }
        if (argc < 3)
        {
                poptPrintUsage(pc,stderr,0);
                return 1;
        }
	pChInputSoundFile = (char *) poptGetArg(pc);
	pChOutputSmsFile = (char *) poptGetArg(pc);
        /* parsing done */


	/* open input sound */
	if (sms_openSF(pChInputSoundFile, &soundHeader))
	{
                printf("error in sms_openSF: %s \n", sms_errorString());
                exit(EXIT_FAILURE);
	}	    

        /* initialize everything */
        sms_init();
        /* TODO NExt: go from here through all the functions that need to look at specEnvParams */
        sms_initAnalysis (&analParams, &soundHeader);
        
	sms_fillHeader (&smsHeader, &analParams, "smsAnal");
	sms_writeHeader (pChOutputSmsFile, &smsHeader, &pOutputSmsFile);

        /* allocate output SMS record */
	sms_allocFrameH (&smsHeader, &smsData);

	/* perform analysis */

	if (analParams.iAnalysisDirection == SMS_DIR_REV)
		iSample = soundHeader.nSamples;

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
                if(analParams.specEnvParams.iType != SMS_ENV_NONE)
                {
                        printf("discrete cepstrum spectral envelope parameters: \n");
                        printf("order: %d, lambda: %f, max frequency: %d \n", analParams.specEnvParams.iOrder,
                               analParams.specEnvParams.fLambda, analParams.specEnvParams.iMaxFreq);
                }
		printf("header info string: %s", smsHeader.pChTextCharacters);
                printf("\ndoing analysis now:\n");
        }

	while(iDoAnalysis > 0)
	{
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
		iStatus = sms_analyze (sizeNewData, pSoundData, &smsData,
		                       &analParams);

		/* if there is an output SMS record, write it */
		if (iStatus == 1)
		{
			sms_writeFrame (pOutputSmsFile, &smsHeader, &smsData);
                        if(sms_errorCheck())
                        {
                                printf("error: could not write sms frame %d:\n", iFrame);
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

