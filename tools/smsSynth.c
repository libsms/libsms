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
 *    smsSynth - program for synthesizing an sms file from the command line.
 *
 */

#include "sms.h"
#include <popt.h>

const char *help_header_text =
        "\n\n"
        "Usage: smsSynth [options]  <inputSmsFile> <outputSoundFile>\n"
        "\n\n"
        "synthesize an analysis (.sms) file made with smsAnal. "
        "output file format is 32bit floating-point WAV or AIFF."
        "\n\n";


int main (int argc, const char *argv[])
{
	char *pChInputSmsFile = NULL, *pChOutputSoundFile = NULL;
	SMS_Header *pSmsHeader = NULL;
	FILE *pSmsFile; /* pointer to sms file to be synthesized */
	SMS_Data smsFrameL, smsFrameR, smsFrame; /* left, right, and interpolated frames */
	float *pFSynthesis; /* waveform synthesis buffer */
	long iSample, i, nSamples, iLeftFrame, iRightFrame;
        float fFrameLoc; /* exact sms frame location, used to interpolate smsFrame */
        float fFsRatio,  fLocIncr;
        int verbose = 0;
        int iSoundFileType = 0; /* wav file */
        int doInterp = 1;
        float timeFactor = 1.0;
	SMS_SynthParams synthParams;
        sms_initSynthParams(&synthParams); /* set some default params that may be updated */

	int optc;   /* switch */
	poptContext pc;
        struct poptOption	options[] =
                {
                        {"verbose", 'v', POPT_ARG_NONE, &verbose, 0, 
                         "verbose mode", 0},
                        {"samplerate", 'r', POPT_ARG_INT, &synthParams.iSamplingRate, 0, 
                         "sampling rate of output sound (default is original)", "int"},
                        {"synth-type", 's', POPT_ARG_INT, &synthParams.iSynthesisType, 0, 
                         "synthesis type (0: all (default), 1: deterministic only , 2: stochastic only)", "int"},
                        {"det-synth-type", 'd', POPT_ARG_INT, &synthParams.iDetSynthType, 0, 
                         "method of deterministic synthesis type (0: IFFT (default) , 1: oscillator bank)", "int"},
                        {"hop", 'h', POPT_ARG_INT, &synthParams.sizeHop, 0, 
                         "sizeHop (default 128) 128 <= sizeHop <= 8092, rounded to a power of 2", "int"},
                        {"time-factor", 't', POPT_ARG_FLOAT, &timeFactor, 0, 
                         "time factor (default 1): positive value to scale by overall time", "float"},
                        {"stoc-gain", 'g', POPT_ARG_FLOAT, &synthParams.modParams.resGain, 0, 
                         "stochastic gain (default 1): positive value to multiply into stochastic gain", "float"},
                        {"transpose", 'x', POPT_ARG_FLOAT, &synthParams.modParams.transpose, 0, 
                         "transpose factor (default 0): value based on the Equal Tempered Scale", "float"},
                        {"interp", 'i', POPT_ARG_INT, &doInterp, 0, 
                         "interpolate between frames when time scaling (default on, 0=off)", "int"},
                        {"file-type", 'f', POPT_ARG_INT, &iSoundFileType, 0, 
                         "output soundfile type (default 0): 0 is wav, 1 is aiff", "int"},
                        POPT_AUTOHELP
                        POPT_TABLEEND
                };

        pc = poptGetContext("smsSynth", argc, argv, options, 0);
        poptSetOtherOptionHelp(pc, help_header_text);

        if (argc <= 1)
        {
                poptPrintUsage(pc,stderr,0);
                return 1;
        }

        while ((optc = poptGetNextOpt(pc)) > 0) {
/*                 switch (optc) { */
/*                         /\* specific arguments are handled here *\/ */
/*                 } */
        }
        if (optc < -1) 
        {
                /* an error occurred during option processing */
                printf("%s: %s\n",
                       poptBadOption(pc, POPT_BADOPTION_NOALIAS),
                       poptStrerror(optc));
                return 1;
        }

	pChInputSmsFile = (char *) poptGetArg(pc);
	pChOutputSoundFile = (char *) poptGetArg(pc);
        /* parsing done */
        
        sms_getHeader (pChInputSmsFile, &pSmsHeader, &pSmsFile);

	if (sms_errorCheck())
	{
                printf("error in sms_getHeader: %s", sms_errorString());
                exit(EXIT_FAILURE);
	}	    

        sms_init();
        sms_initSynth( pSmsHeader, &synthParams );
        /* disable interpolation for residual resynthesis with original phases (not implemented yet) */
        if(pSmsHeader->iStochasticType == SMS_STOC_IFFT)
                doInterp = 0;

        synthParams.modParams.doTranspose = 1; /* turns on transposing (whether there is a value or not */
        synthParams.modParams.doResGain = 1; /* turns on transposing (whether there is a value or not */

        if(verbose)
        {
                printf("__arguments__\n");
                printf("samplingrate: %d \nsynthesis type: ", synthParams.iSamplingRate);
                printf(" do frame interpolation: %d \n", doInterp);
                if(synthParams.iSynthesisType == SMS_STYPE_ALL) printf("all ");
                else if(synthParams.iSynthesisType == SMS_STYPE_DET) printf("deterministic only ");
                else if(synthParams.iSynthesisType == SMS_STYPE_STOC) printf("stochastic only ");
                printf("\ndeteministic synthesis method: ");
                if(synthParams.iDetSynthType == SMS_DET_IFFT) printf("ifft ");
                else if(synthParams.iDetSynthType == SMS_DET_SIN) printf("oscillator bank ");
                printf("\nsizeHop: %d \n", synthParams.sizeHop);
                printf("time factor: %f \n", timeFactor);
                printf("stochastic gain factor: %f \n", synthParams.modParams.resGain);
                printf("frequency transpose factor: %f \n", synthParams.modParams.transpose);
                printf("__header info__\n");
                printf("original samplingrate: %d, iFrameRate: %d, origSizeHop: %d\n", pSmsHeader->iSamplingRate,
                       pSmsHeader->iFrameRate, synthParams.origSizeHop);
                printf("original file length: %f seconds \n",
                       pSmsHeader->nFrames / (float) pSmsHeader->iFrameRate);
                if(!iSoundFileType) printf("output soundfile type: wav \n");
                else  printf("output soundfile type: aiff \n");
        }

        /* initialize libsndfile for writing a soundfile */
	sms_createSF ( pChOutputSoundFile, synthParams.iSamplingRate, iSoundFileType);

	/* setup for synthesis from file */
        if(doInterp)
        {
                sms_allocFrameH (pSmsHeader, &smsFrameL);
                sms_allocFrameH (pSmsHeader, &smsFrameR);
        }
        sms_allocFrameH (pSmsHeader, &smsFrame); /* the actual frame to be handed to synthesizer */

	if ((pFSynthesis = (float *) calloc(synthParams.sizeHop, sizeof(float)))
	    == NULL)
        {
                printf ("Could not allocate memory for pFSynthesis");
                exit(0);
        }

	iSample = 0;
        /* number of samples is a factor of the ratio of samplerates */
        /* multiply by timeFactor to increase samples to desired file length */
        /* pSmsHeader->iSamplingRate is from original analysis */
        fFsRatio = (float) synthParams.iSamplingRate / pSmsHeader->iSamplingRate;
	nSamples = pSmsHeader->nFrames * synthParams.origSizeHop * timeFactor * fFsRatio;

        /* divide timeFactor out to get the correct frame */
        fLocIncr = pSmsHeader->iSamplingRate / 
                ( synthParams.origSizeHop * synthParams.iSamplingRate * timeFactor); 

	while (iSample < nSamples)
	{
		if(doInterp)
                {
                        fFrameLoc =  iSample *  fLocIncr;
                        /* left and right frames around location, gaurding for end of file */
                        iLeftFrame = MIN (pSmsHeader->nFrames - 1, floor (fFrameLoc)); 
                        iRightFrame = (iLeftFrame < pSmsHeader->nFrames - 2)
                                ? (1+ iLeftFrame) : iLeftFrame;
                        sms_getFrame (pSmsFile, pSmsHeader, iLeftFrame, &smsFrameL);
                        sms_getFrame (pSmsFile, pSmsHeader, iRightFrame,&smsFrameR);
                        sms_interpolateFrames (&smsFrameL, &smsFrameR, &smsFrame,
                                               fFrameLoc - iLeftFrame);
                }
                else
                {
                        sms_getFrame (pSmsFile, pSmsHeader, (int) iSample * fLocIncr, &smsFrame);
                        printf("frame: %d \n",  (int) (iSample * fLocIncr));
                }
                sms_modify(&smsFrame, &synthParams.modParams); 
                sms_synthesize (&smsFrame, pFSynthesis, &synthParams);
		sms_writeSound (pFSynthesis, synthParams.sizeHop);
    
		iSample += synthParams.sizeHop;

                if(verbose)
                {
                        if (iSample % (synthParams.sizeHop * 20) == 0)
                                fprintf(stderr,"%.2f ", iSample / (float) synthParams.iSamplingRate);
                }

	}

        if(verbose)
        {
                printf("\nfile length: %f seconds\n", (float) iSample / synthParams.iSamplingRate);
        }
        printf("wrote %ld samples in %s\n",  iSample, pChOutputSoundFile);

	/* close output sound file, free memory and exit */
	sms_writeSF ();
        if(doInterp)
        {
                sms_freeFrame(&smsFrameL);
                sms_freeFrame(&smsFrameR);
        }
        sms_freeFrame(&smsFrame);
	free (pFSynthesis);
        free (pSmsHeader);
        sms_freeSynth(&synthParams);
        sms_free();
	return(1);
}
