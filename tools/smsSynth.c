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
 *    main program for smsSynth
 *
 */
#include "sms.h"

double *pFSTab = NULL, *pFSincTab = NULL;
SOUND_BUFFER soundBuffer, synthBuffer;
ANAL_FRAME **ppFrames, *pFrames;
short MaxDelayFrames;

void usage (void)
{
    fprintf (stderr, "\n"
             "Usage: smsSynth [options]  <inputSmsFile> <outputSoundFile>\n"
             "\n"
             "Options:\n"
             "      -v     print out verbose information"
             "	    -r      sampling rate of output sound (if = 0, original rate)\n"
             "	    -s     synthesis type (1: all, 2: deterministic only , 3: stochastic only)\n"
             "	    -d     method of deterministic synthesis type (1: IFFT, 2: oscillator bank)\n"
             "      -h     sizeHop (default 128) 128 <= sizeHop <= 8092, rounded to a power of 2"
             "      -t      time factor (default 1): positive value to multiply by overall time "
             "\n"
             "synthesize an analysis (.sms) file made with smsAnal."
             "output file format is 32bit floating-point AIFF."
             "\n\n");
    
    exit(1);
}

int main (int argc, char *argv[])
{
	char *pChInputSmsFile = NULL, *pChOutputSoundFile = NULL;
	SMSHeader *pSmsHeader;
	FILE *pSmsFile; /* pointer to sms file to be synthesized */
	SMS_DATA smsRecord1, smsRecord2, newSmsRecord; /* left, right, and interpolated records */
	float *pFSynthesis; /* waveform synthesis buffer */
	long iError, iSample, i, nSamples, iLeftRecord, iRightRecord;
        int verboseMode = 0;
        int iSamplingRate = 44100;
        float fRecordLoc; /* exact sms frame location, used to interpolate newSmsRecord */
        int  detSynthType, synthType, sizeHop; /*  argument holders */
        float timeFactor;
        int origSizeHop; /* RTE TODO: this should probably be in SMSHEADER instead of iOrigSampleRate */
	SYNTH_PARAMS synthParams;
	synthParams.iSynthesisType = STYPE_ALL;
        synthParams.iDetSynthType = DET_IFFT;
	synthParams.sizeHop = SIZE_SYNTH_FRAME;

	if (argc > 3) 
	{
		for (i=1; i<argc-2; i++) 
			if (*(argv[i]++) == '-') 
				switch (*(argv[i]++)) 
				{
                                case 'r':  if (sscanf(argv[i],"%d",&iSamplingRate) < 0 )
						quit("error: invalid sampling rate");
                                        break;
                                case 's': sscanf(argv[i], "%d", &synthType);
                                        if(synthType < 1 || synthType > 3)
                                                quit("error: detSynthType must be 1, 2, or  3");
                                        synthParams.iSynthesisType = synthType;
                                        break;
                                case 'd': sscanf(argv[i], "%d", &detSynthType);
                                        if(detSynthType < 1 || detSynthType > 2)
                                                quit("error: detSynthType must be 1 or 2");
                                        synthParams.iDetSynthType = detSynthType;
                                        break;
                                case 'h': sscanf(argv[i], "%d", &sizeHop);
                                        if(sizeHop < SIZE_SYNTH_FRAME || sizeHop > MAX_SIZE_WINDOW) 
                                                quit("error: invalid sizeHop");
                                        synthParams.sizeHop = sizeHop;
                                        //RTE TODO: round to power of 2 (is it necessary?)
                                        break;
                                case 't':  if (sscanf(argv[i],"%f",&timeFactor) < 0 )
						quit("error: invalid time factor");
                                        break;
                                case 'v': verboseMode = 1;
                                        break;
                                default:   usage();
				}
	}
	else if (argc < 2) usage();

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
  
        /* set synthesis parameters from arguments and header */
	synthParams.iOriginalSRate = pSmsHeader->iOriginalSRate;
	synthParams.origSizeHop = synthParams.iOriginalSRate / pSmsHeader->iFrameRate;
	synthParams.iStochasticType = pSmsHeader->iStochasticType;
        /* in the case of STOC_WAVEFORM, no resampling is implemented (yet),
           so the original samplerate must be used */
        if( !iSamplingRate || synthParams.iStochasticType == STOC_WAVEFORM)
        {
                synthParams.iSamplingRate = synthParams.iOriginalSRate;
                if(verboseMode) printf("original sampling rate forced \n");
        }
        else synthParams.iSamplingRate = iSamplingRate;


        if(verboseMode)
        {
                printf("__arguments__\n");
                printf("samplingrate: %d \nsynthesis type: ", synthParams.iSamplingRate);
                if(synthParams.iSynthesisType == STYPE_ALL) printf("all ");
                else if(synthParams.iSynthesisType == STYPE_DET) printf("deterministic only ");
                else if(synthParams.iSynthesisType == STYPE_STOC) printf("stochastic only ");
                printf("\ndeteministic synthesis method: ");
                if(synthParams.iDetSynthType == DET_IFFT) printf("ifft ");
                else if(synthParams.iDetSynthType == DET_OSC) printf("oscillator bank ");
                printf("\nsizeHop: %d \n", synthParams.sizeHop);
                printf("time factor: %f \n", timeFactor);
                printf("__header info__\n");
                printf("fOriginalSRate: %d, iFrameRate: %d, origSizeHop: %d\n", 
                       pSmsHeader->iOriginalSRate, pSmsHeader->iFrameRate, synthParams.origSizeHop);
                printf("original file length: %f seconds \n", (float)  pSmsHeader->nRecords * 
                       synthParams.origSizeHop / pSmsHeader->iOriginalSRate );
             }

	CreateOutputSoundFile (synthParams, pChOutputSoundFile);

	/* memory allocation */
	AllocSmsRecord (pSmsHeader, &smsRecord1);
	AllocSmsRecord (pSmsHeader, &smsRecord2);

	if ((pFSynthesis = (float *) calloc(synthParams.sizeHop, sizeof(float)))
	    == NULL)
		quit ("Could not allocate memory for pFSynthesis");

        synthParams.pFStocWindow = 
		(float *) calloc(synthParams.sizeHop * 2, sizeof(float));
	Hanning (synthParams.sizeHop * 2, synthParams.pFStocWindow);
	synthParams.pFDetWindow =
		(float *) calloc(synthParams.sizeHop * 2, sizeof(float));
	IFFTwindow (synthParams.sizeHop * 2, synthParams.pFDetWindow);

        initInverseFFTW( &synthParams );
      
	AllocateSmsRecord (&synthParams.previousFrame, pSmsHeader->nTrajectories, 
	                   1 + pSmsHeader->nStochasticCoeff, 1,
                           origSizeHop, pSmsHeader->iStochasticType);
	AllocateSmsRecord (&newSmsRecord, pSmsHeader->nTrajectories, 
	                   pSmsHeader->nStochasticCoeff, 0,
                           origSizeHop, pSmsHeader->iStochasticType);

	/* prepare sinc and sine tables only the first time */
	if (pFSincTab == NULL)
		PrepSinc ();
	if (pFSTab == NULL)
		PrepSine (2046); //try 4096
 

        /* //########## RTE DEBUG ############### */
//        FILE *df;
//        df = fopen("wave.txt", "w");
//        int ii;
//        int fc = 0; 
//        synthParams.realftOut = (float *) calloc((sizeFFT<<1)+1, sizeof(float));
        //      printf("(sizeFFT<<1) +1: %d", (sizeFFT <<1) + 1);


#ifdef FFTW
        printf("## using fftw3 ##  \n");
#endif

	iSample = 0;
        /* number of samples is a factor of the ratio of samplerates */
        /* multiply by timeFactor to increase samples to desired file length */
	nSamples = pSmsHeader->nRecords * synthParams.origSizeHop * timeFactor *
                synthParams.iSamplingRate / pSmsHeader->iOriginalSRate;
 
	while (iSample < nSamples)
	{
                /* divide timeFactor out to get the correct record */
		fRecordLoc = (float) iSample * pSmsHeader->iOriginalSRate / 
                        ( synthParams.origSizeHop * synthParams.iSamplingRate * timeFactor); 
                // left and right records around location, gaurding for end of file
		iLeftRecord = MIN (pSmsHeader->nRecords - 1, floor (fRecordLoc)); 
		iRightRecord = (iLeftRecord < pSmsHeader->nRecords - 2)
			? (1+ iLeftRecord) : iLeftRecord;
		GetSmsRecord (pSmsFile, pSmsHeader, iLeftRecord, &smsRecord1);
		GetSmsRecord (pSmsFile, pSmsHeader, iRightRecord,&smsRecord2);
		InterpolateSmsRecords (&smsRecord1, &smsRecord2, &newSmsRecord,
                                       fRecordLoc - iLeftRecord);

                SmsSynthesis (&newSmsRecord, pFSynthesis, &synthParams);
		WriteToOutputFile (pFSynthesis, synthParams.sizeHop);
    
		iSample += synthParams.sizeHop;

                if(verboseMode)
                {
                        if (iSample % (synthParams.sizeHop * 20) == 0)
                                fprintf(stderr,"%.2f ", iSample / (float) synthParams.iSamplingRate);
                }

	}
/*         int iRecord = 0; */
/*         // RTE FIXME: this disregards the current samplerate */
/* 	while (iRecord++ < pSmsHeader->nRecords) */
/* 	{ */
/* 		GetSmsRecord (pSmsFile, pSmsHeader, iRecord, &smsRecord1); */
/* // */
/*                 SmsSynthesis (&smsRecord1, pFSynthesis, &synthParams); */
/* 		WriteToOutputFile (pFSynthesis, synthParams.sizeHop); */
    
/* 		iSample += synthParams.sizeHop; */

/*                 if(verboseMode) */
/*                 { */
/*                         if (iSample % (synthParams.sizeHop * 20) == 0) */
/*                                 fprintf(stderr,"%.2f ", iSample / (float) synthParams.iSamplingRate); */
/*                 } */
/*                 //RTE DEBUG ################ */
/* /\*                 for(ii = 0; ii < synthParams.sizeHop ; ii++) *\/ */
/* /\*                         fprintf(df, "%f ", pFSynthesis[ii]); *\/ */

/*                 //########################## */
                

/* 	} */

        if(verboseMode)
        {
                printf("\nfile length: %f seconds\n", (float) iSample / synthParams.iSamplingRate);
        }
        printf("wrote %ld samples in %s\n",  iSample, pChOutputSoundFile);

        //RTE DEBUG ################
        free (synthParams.realftOut);
/*         fclose(df); */
/*         printf("synthType: %d, detType: %d\n",synthParams.iSynthesisType, synthParams.iDetSynthType); */
        //##########################

	/* write and close output sound file */
	WriteOutputFile ();
	free (pFSynthesis);
	free (pSmsHeader);
        fftwf_free(synthParams.pCfftIn);
        fftwf_free(synthParams.pFfftOut);
	fftwf_destroy_plan(synthParams.fftPlan);


	return(1);
}
