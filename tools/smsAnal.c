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

int main (int argc, const char *argv[])
{
    FILE *pOutputSmsFile; 
    SMS_Data smsData;
    SMS_Header smsHeader;

    float pSoundData[SMS_MAX_WINDOW];
    SMS_SndHeader soundHeader;

    char *pChInputSoundFile = NULL, *pChOutputSmsFile = NULL;
    int verbose = 0;
    int iDoAnalysis = 1;
    int iFrame = 0;
    long iStatus = 0, iSample = 0, sizeNewData = 0;

    int optc;   /* switch */
    poptContext pc;

    SMS_AnalParams analParams;
    sms_initAnalParams(&analParams);    /* initialize arguments to defaults*/

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
        {"min-ref-harm-mag", 'z', POPT_ARG_FLOAT, &analParams.fRefHarmMagDiffFromMax, 0, 
            " maximum dB difference between the harmonic used for reference and the maximum peak (default 30)", "float"}, /*\todo check this doc*/
        {"default-fund", 'u', POPT_ARG_FLOAT, &analParams.fDefaultFundamental, 0, 
            "default fundamental frequency (hz), used to set initial window size, or window size for entire sound if inharmonic (default 100)", "float"}, 
        {"lowest-fund", 'l', POPT_ARG_FLOAT, &analParams.fDefaultFundamental, 0, 
            "lowest fundamental frequency(hz), or frequency in inharmonic analysis, to search for (default 50)", "float"}, 
        {"highest-fund", 'h', POPT_ARG_FLOAT, &analParams.fDefaultFundamental, 0, 
            "highest fundamental frequency to search for, has no effect on inharmonic analysis (default 1000)", "float"}, 
        /* Peak Continuation parameters */
        {"guides", 'n', POPT_ARG_INT, &analParams.nGuides, 0, 
            "number of guides to use in partial tracking (default 100)", "int"},
        {"tracks", 'p', POPT_ARG_INT, &analParams.nTracks, 0, 
            "number of output partial tracks (default 60)", "int"},
        {"freq-deviation", 'w', POPT_ARG_FLOAT, &analParams.fFreqDeviation, 0, 
            "maximum permitted frequency deviation from guide frequency (default .45)", "float"}, 
        {"peak-cont-guide", 't', POPT_ARG_FLOAT, &analParams.fPeakContToGuide, 0, 
            "contribution of the frequency of the previous peak of a given trajectory to the current guide frequency value (default .4).", "float"}, 
        {"fund-cont-guide", 'o', POPT_ARG_FLOAT, &analParams.fFundContToGuide, 0, 
            "contribution of the fundamental frequency of the previous peak of a given trajectory to the current guide frequency value (default .5).", "float"}, 
        /* Track Cleaning parameters:\n" */
        {"clean-track", 'g', POPT_ARG_INT, &analParams.iCleanTracks, 0, 
            "turn on/off track cleaning (default is on, 1)", "int"}, 
        {"min-track-length", 'a', POPT_ARG_FLOAT, &analParams.iMinTrackLength, 0, 
            "minimum track length in seconds (0.1)", "float"}, 
        {"max-sleeping-time", 'b', POPT_ARG_FLOAT, &analParams.iMaxSleepingTime, 0, 
            "maximum time a frame can sleep in seconds (0.1)", "float"},  /* this doc is horrible */
        /* Stochastic Analysis parameters */
        {"stochastic", 'e', POPT_ARG_INT, &analParams.iStochasticType, 0, 
            "turn on/off stochastic analysis (default is on, 1)", "int"}, 
        {"stoch-coeff", 'c', POPT_ARG_INT, &analParams.nStochasticCoeff, 0, 
            "number of stochastic coefficients in approximation (default 128)", "int"}, 
        /* spectral enveloping parameters */
        {"se",0, POPT_ARG_INT, &analParams.specEnvParams.iType, 0, 
            "spectral enveloping type (0, off)", "int"},
        {"co", 0, POPT_ARG_INT, &analParams.specEnvParams.iOrder, 0, 
            "discrete cepstrum order (25)", "int"},
        {"la", 0, POPT_ARG_FLOAT, &analParams.specEnvParams.fLambda, 0, 
            "lambda, regularizing coefficient (0.00001)", "float"}, 
        {"an", 0, POPT_ARG_NONE, &analParams.specEnvParams.iAnchor, 0, 
            "turn on anchoring of spectral envelope endpoints", 0}, 
        {"mef", 0, POPT_ARG_INT, &analParams.specEnvParams.iMaxFreq, 0, 
            "maximum envelope frequency (default is highest-freq", "int"}, 
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

    if(verbose)
    {
        printf("\n===sound file info===\n");
        printf("samples: %d, samplerate: %d, seconds: %f \n", soundHeader.nSamples, soundHeader.iSamplingRate, 
                soundHeader.nSamples / (float)soundHeader.iSamplingRate);
        printf("number of channels: %d, read channel: %d \n", soundHeader.channelCount, soundHeader.iReadChannel); 
        printf("\n===analysis parameters===\n");
        printf("sizeHop: %d, nFrames: %d \n", analParams.sizeHop, analParams.nFrames);
        /* \todo: print analysis window type (by name) here */
        if(analParams.specEnvParams.iType != SMS_ENV_NONE)
        {
            printf("\n===spectral envelope parameters===\n");
            if(analParams.specEnvParams.iType == SMS_ENV_CEP)
                printf("type: cepstral coefficients, ");
            else if(analParams.specEnvParams.iType == SMS_ENV_FBINS)
                printf("type: frequency bins, ");
            else
                printf("warning: unknown spectral envelope type! \n\n ");
            printf("order: %d, lambda: %f, max frequency: %d \n", analParams.specEnvParams.iOrder,
                    analParams.specEnvParams.fLambda, analParams.specEnvParams.iMaxFreq);
        }
        printf("\n===header info string===\n %s", smsHeader.pChTextCharacters);
        printf("\n\ndoing analysis now:\n");
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
        if (sms_getSound(&soundHeader, sizeNewData, pSoundData, iSample, &analParams))
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
    if (analParams.iDebugMode == SMS_DBG_SYNC)
        sms_writeDebugFile ();

    printf("wrote %d analysis frames to %s\n", iFrame, pChOutputSmsFile);

    /* cleanup */
    sms_freeFrame(&smsData);
    sms_freeAnalysis(&analParams);
    sms_free();
    return 0;   
}

