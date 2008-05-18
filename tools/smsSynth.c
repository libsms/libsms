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
#define USAGE "Usage: smsSynth [-s samplingRate] [-d] [-n] <inputSmsFile> <outputSoundFile>"

double *pFSTab = NULL, *pFSincTab = NULL;
SOUND_BUFFER soundBuffer, synthBuffer;
ANAL_FRAME **ppFrames, *pFrames;
short MaxDelayFrames;

int main (int argc, char *argv[])
{
	char *pChInputSmsFile = NULL, *pChOutputSoundFile = NULL;
	SMSHeader *pSmsHeader;
	FILE *pSmsFile;
	SMS_DATA smsRecord1, smsRecord2, newSmsRecord;
	short *pSSynthesis;
	long iError, iSample, i, iLastSample, iLeftRecord, iRightRecord;

	int iSamplingRate = 44100, fRecordLoc;
	SYNTH_PARAMS synthParams;
  
	synthParams.iSynthesisType = 0;

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
	AllocSmsRecord (pSmsHeader, &smsRecord1);
	AllocSmsRecord (pSmsHeader, &smsRecord2);

        printf("sizeHop: %d, iFrameRate: %d, fOriginalSRate: %d \n", pSmsHeader->sizeHop, pSmsHeader->iFrameRate,
               pSmsHeader->iOriginalSRate);

	synthParams.iOriginalSRate = pSmsHeader->iOriginalSRate;
	synthParams.iStochasticType = pSmsHeader->iStochasticType;
        printf("StochasticType: %d\n", synthParams.iStochasticType);
	synthParams.sizeHop = SIZE_SYNTH_FRAME;
  
	if ((pSSynthesis = (short *) calloc(synthParams.sizeHop, sizeof(short)))
	    == NULL)
		quit ("Could not allocate memory for pSSynthesis");

/* 	if (iSamplingRate == 44100 &&  */
/* 	    synthParams.iOriginalSRate == 44100) */
/* 		synthParams.iSamplingRate = 44100; */
/* 	else */
/* 	{ */
/* 		synthParams.iSamplingRate = 22050; */
/* 		fprintf(stderr, "Sampling Rate set to 22050 Hz\n"); */
/* 	} */
        if(synthParams.iStochasticType == STOC_WAVEFORM)
        {
                printf("original samplerate forced for resynthesis of stocastic waveform: %d\n", synthParams.iOriginalSRate);
                synthParams.iSamplingRate = synthParams.iOriginalSRate;
        }
        else synthParams.iSamplingRate = iSamplingRate;

	CreateOutputSoundFile (synthParams, pChOutputSoundFile);
        
	synthParams.origSizeHop = synthParams.iSamplingRate / pSmsHeader->iFrameRate;
        printf("sizeHop: %d, origSizeHop, %d\n", synthParams.sizeHop, synthParams.origSizeHop);
        synthParams.pFStocWindow = 
		(float *) calloc(synthParams.sizeHop * 2, sizeof(float));
	Hanning (synthParams.sizeHop * 2, synthParams.pFStocWindow);
	synthParams.pFDetWindow = 
		(float *) calloc(synthParams.sizeHop * 2, sizeof(float));
	IFFTwindow (synthParams.sizeHop * 2, synthParams.pFDetWindow);
      
	AllocateSmsRecord (&synthParams.previousFrame, pSmsHeader->nTrajectories, 
	                   1 + pSmsHeader->nStochasticCoeff, 1,
                           pSmsHeader->sizeHop, pSmsHeader->iStochasticType);
	AllocateSmsRecord (&newSmsRecord, pSmsHeader->nTrajectories, 
	                   pSmsHeader->nStochasticCoeff, 0,
                           pSmsHeader->sizeHop, pSmsHeader->iStochasticType);

	/* prepare sinc and sine tables only the first time */
	if (pFSincTab == NULL)
		PrepSinc ();
	if (pFSTab == NULL)
		PrepSine (2046);
 
	iSample = 0;
	iLastSample = pSmsHeader->nRecords * synthParams.origSizeHop;
        
        /* make fftw plan and buffer TODO: make a synth initializer function*/
        int sizeFFT = synthParams.sizeHop ; //? 
        // for in-place c2r transform, 2 extra values are needed: fftw manual sec.4.3.4
        synthParams.pCfftIn =  fftwf_malloc(sizeof(fftwf_complex) * (sizeFFT / 2 + 1));
        synthParams.pFfftOut = fftwf_malloc(sizeof(float) * sizeFFT);
        synthParams.fftPlan =  fftwf_plan_dft_c2r_1d( sizeFFT, synthParams.pCfftIn,
                                                      synthParams.pFfftOut, FFTW_ESTIMATE);
        
        /* //########## RTE DEBUG ############### */
        FILE *df;
        df = fopen("wave.txt", "w");
        int ii;
        int fc = 0; 
        synthParams.realftOut = (float *) calloc((sizeFFT<<1)+1, sizeof(float));
        //      printf("(sizeFFT<<1) +1: %d", (sizeFFT <<1) + 1);
        /* // ################################### */



	while (iSample < iLastSample)
	{
		fRecordLoc = (float) iSample / synthParams.origSizeHop;
		iLeftRecord = MIN (pSmsHeader->nRecords - 1, floor (fRecordLoc));
		iRightRecord = 
			(iLeftRecord < pSmsHeader->nRecords - 2) 
			? (1+ iLeftRecord) : iLeftRecord;
		GetSmsRecord (pSmsFile, pSmsHeader, iLeftRecord, &smsRecord1);
		GetSmsRecord (pSmsFile, pSmsHeader, iRightRecord,&smsRecord2);
		InterpolateSmsRecords (&smsRecord1, &smsRecord2, &newSmsRecord, 
				                   fRecordLoc - iLeftRecord);

                SmsSynthesis (&newSmsRecord, pSSynthesis, &synthParams);
		WriteToOutputFile (pSSynthesis, synthParams.sizeHop);
    
		iSample += synthParams.sizeHop;
		if (iSample % (synthParams.sizeHop * 40) == 0)
			fprintf(stderr,"%.2f ", iSample / (float) synthParams.iSamplingRate);
                
                //RTE DEBUG ################
                for(ii = 0; ii < synthParams.sizeHop ; ii++)
                        fprintf(df, "%d ", pSSynthesis[ii]);

                //##########################
                

	}
        printf("\n");

        //RTE DEBUG ################


        free (synthParams.realftOut);
        fclose(df);
        //##########################

	/* write and close output sound file */
	WriteOutputFile ();
	free (pSSynthesis);
	free (pSmsHeader);
        fftwf_free(synthParams.pCfftIn);
        fftwf_free(synthParams.pFfftOut);
	fftwf_destroy_plan(synthParams.fftPlan);


	return(1);
}
