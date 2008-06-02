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


SOUND_BUFFER soundBuffer, synthBuffer;
ANAL_FRAME **ppFrames, *pFrames;
short MaxDelayFrames;

void usage (void)
{
    fprintf (stderr, "\n"
             "Usage: smsSynth [options]  <inputSmsFile> <outputSoundFile>\n"
             "\n"
             "Options:\n"
             "      -v     print out verbose information\n"
             "      -r     sampling rate of output sound (default is original)\n"
             "      -s     synthesis type (1: all, 2: deterministic only , 3: stochastic only)\n"
             "      -d     method of deterministic synthesis type (1: IFFT, 2: oscillator bank)\n"
             "      -h     sizeHop (default 128) 128 <= sizeHop <= 8092, rounded to a power of 2 \n"
             "      -t     time factor (default 1): positive value to multiply by overall time \n"
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
	SMS_DATA smsRecordL, smsRecordR, newSmsRecord; /* left, right, and interpolated records */
	float *pFSynthesis; /* waveform synthesis buffer */
	long iError, iSample, i, nSamples, iLeftRecord, iRightRecord;
        int verboseMode = 0;
        float fRecordLoc; /* exact sms frame location, used to interpolate newSmsRecord */
        int  detSynthType, synthType, sizeHop, iSamplingRate; /*  argument holders */
        float timeFactor = 1.0;
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
                                        synthParams.iSamplingRate = iSamplingRate;
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
 
        SmsInit();
        SmsInitSynth( pSmsHeader, &synthParams );

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

        /* initialize libsndfile for writing a soundfile */
	CreateOutputSoundFile (synthParams, pChOutputSoundFile);

	/* setup for synthesis from file */
	AllocSmsRecord (pSmsHeader, &smsRecordL);
	AllocSmsRecord (pSmsHeader, &smsRecordR);
	AllocateSmsRecord (&newSmsRecord, pSmsHeader->nTrajectories, 
	                   pSmsHeader->nStochasticCoeff, 0,
                           synthParams.origSizeHop, pSmsHeader->iStochasticType);

	if ((pFSynthesis = (float *) calloc(synthParams.sizeHop, sizeof(float)))
	    == NULL)
		quit ("Could not allocate memory for pFSynthesis");

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
		GetSmsRecord (pSmsFile, pSmsHeader, iLeftRecord, &smsRecordL);
		GetSmsRecord (pSmsFile, pSmsHeader, iRightRecord,&smsRecordR);
		InterpolateSmsRecords (&smsRecordL, &smsRecordR, &newSmsRecord,
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

        if(verboseMode)
        {
                printf("\nfile length: %f seconds\n", (float) iSample / synthParams.iSamplingRate);
        }
        printf("wrote %ld samples in %s\n",  iSample, pChOutputSoundFile);

	/* close output sound file, free memory and exit */
	WriteOutputFile ();
	free (pFSynthesis);
        free (pSmsHeader);
        SmsFreeSynth(&synthParams);
	return(1);
}
