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


//SMS_SndBuffer soundBuffer, synthBuffer;
//SMS_AnalFrame **ppFrames, *pFrames;
//short MaxDelayFrames;

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
	SMS_Header *pSmsHeader;
	FILE *pSmsFile; /* pointer to sms file to be synthesized */
	SMS_Data smsRecordL, smsRecordR, newSmsRecord; /* left, right, and interpolated records */
	float *pFSynthesis; /* waveform synthesis buffer */
	long iError, iSample, i, nSamples, iLeftRecord, iRightRecord;
        int verboseMode = 0;
        float fRecordLoc; /* exact sms frame location, used to interpolate newSmsRecord */
        float fFsRatio,  fLocIncr; 
        int  detSynthType, synthType, sizeHop, iSamplingRate; /*  argument holders */
        float timeFactor = 1.0;
	SMS_SynthParams synthParams;
	synthParams.iSynthesisType = SMS_STYPE_ALL;
        synthParams.iDetSynthType = SMS_DET_IFFT;
	synthParams.sizeHop = SMS_MIN_SIZE_FRAME;
	synthParams.iSamplingRate = 44100;

	if (argc > 3) 
	{
		for (i=1; i<argc-2; i++) 
			if (*(argv[i]++) == '-') 
				switch (*(argv[i]++)) 
				{
                                case 'r':  if (sscanf(argv[i],"%d",&iSamplingRate) < 0 )
                                        {
						printf("error: invalid sampling rate");
                                                exit(1);
                                        }
                                        synthParams.iSamplingRate = iSamplingRate;
                                        break;
                                case 's': sscanf(argv[i], "%d", &synthType);
                                        if(synthType < 1 || synthType > 3)
                                        {
                                                printf("error: detSynthType must be 1, 2, or  3");
                                                exit(1);
                                        }
                                        synthParams.iSynthesisType = synthType;
                                        break;
                                case 'd': sscanf(argv[i], "%d", &detSynthType);
                                        if(detSynthType < 1 || detSynthType > 2)
                                        {
                                                printf("error: detSynthType must be 1 or 2");
                                                exit(1);
                                        }
                                        synthParams.iDetSynthType = detSynthType;
                                        break;
                                case 'h': sscanf(argv[i], "%d", &sizeHop);
                                        if(sizeHop < SMS_MIN_SIZE_FRAME || sizeHop > SMS_MAX_WINDOW) 
                                        {
                                                printf("error: invalid sizeHop");
                                                exit(1);
                                        }
                                        synthParams.sizeHop = sizeHop;
                                        //RTE TODO: round to power of 2 (is it necessary?)
                                        break;
                                case 't':  if (sscanf(argv[i],"%f",&timeFactor) < 0 )
                                        {
						printf("error: invalid time factor");
                                                exit(1);
                                        }
                                        break;
                                case 'v': verboseMode = 1;
                                        break;
                                default:   usage();
				}
	}
	else if (argc < 2) usage();

	pChInputSmsFile = argv[argc-2];
	pChOutputSoundFile = argv[argc-1];
        
	if ((iError = sms_getHeader (pChInputSmsFile, &pSmsHeader, &pSmsFile)) < 0)
	{
                printf("error in sms_getHeader: %s", sms_errorString(iError));
                exit(EXIT_FAILURE);
	}	    
 
        sms_init();
        sms_initSynth( pSmsHeader, &synthParams );

        if(verboseMode)
        {
                printf("__arguments__\n");
                printf("samplingrate: %d \nsynthesis type: ", synthParams.iSamplingRate);
                if(synthParams.iSynthesisType == SMS_STYPE_ALL) printf("all ");
                else if(synthParams.iSynthesisType == SMS_STYPE_DET) printf("deterministic only ");
                else if(synthParams.iSynthesisType == SMS_STYPE_STOC) printf("stochastic only ");
                printf("\ndeteministic synthesis method: ");
                if(synthParams.iDetSynthType == SMS_DET_IFFT) printf("ifft ");
                else if(synthParams.iDetSynthType == SMS_DET_SIN) printf("oscillator bank ");
                printf("\nsizeHop: %d \n", synthParams.sizeHop);
                printf("time factor: %f \n", timeFactor);
                printf("__header info__\n");
                printf("fOriginalSRate: %d, iFrameRate: %d, origSizeHop: %d\n", 
                       pSmsHeader->iOriginalSRate, pSmsHeader->iFrameRate, synthParams.origSizeHop);
                printf("original file length: %f seconds \n", (float)  pSmsHeader->nFrames * 
                       synthParams.origSizeHop / pSmsHeader->iOriginalSRate );
        }

        /* initialize libsndfile for writing a soundfile */
	sms_createSF (synthParams, pChOutputSoundFile);

	/* setup for synthesis from file */
	sms_allocRecordH (pSmsHeader, &smsRecordL);
	sms_allocRecordH (pSmsHeader, &smsRecordR);
	sms_allocRecord (&newSmsRecord, pSmsHeader->nTrajectories, 
	                   pSmsHeader->nStochasticCoeff, 0,
                           synthParams.origSizeHop, pSmsHeader->iStochasticType);

	if ((pFSynthesis = (float *) calloc(synthParams.sizeHop, sizeof(float)))
	    == NULL)
        {
                printf ("Could not allocate memory for pFSynthesis");
                exit(0);
        }

#ifdef FFTW
        printf("## using fftw3 ##  \n");
#endif

	iSample = 0;
        /* number of samples is a factor of the ratio of samplerates */
        /* multiply by timeFactor to increase samples to desired file length */
        fFsRatio = (float) synthParams.iSamplingRate / pSmsHeader->iOriginalSRate;
	nSamples = pSmsHeader->nFrames * synthParams.origSizeHop * timeFactor * fFsRatio;

        /* divide timeFactor out to get the correct record */
        fLocIncr = pSmsHeader->iOriginalSRate / 
                ( synthParams.origSizeHop * synthParams.iSamplingRate * timeFactor); 


	while (iSample < nSamples)
	{
		fRecordLoc =  iSample *  fLocIncr;
                // left and right records around location, gaurding for end of file
		iLeftRecord = MIN (pSmsHeader->nFrames - 1, floor (fRecordLoc)); 
		iRightRecord = (iLeftRecord < pSmsHeader->nFrames - 2)
			? (1+ iLeftRecord) : iLeftRecord;
		sms_getRecord (pSmsFile, pSmsHeader, iLeftRecord, &smsRecordL);
		sms_getRecord (pSmsFile, pSmsHeader, iRightRecord,&smsRecordR);
		sms_interpolateRecords (&smsRecordL, &smsRecordR, &newSmsRecord,
                                       fRecordLoc - iLeftRecord);

                sms_synthesize (&newSmsRecord, pFSynthesis, &synthParams);
		sms_writeSound (pFSynthesis, synthParams.sizeHop);
    
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
        //why aren't we freeing the records?
	sms_writeSF ();
	free (pFSynthesis);
        free (pSmsHeader);
        sms_freeSynth(&synthParams);
        sms_free();
	return(1);
}
