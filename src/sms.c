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
/*! \file sms.c
 * \brief initialization, free, and debug functions
 */

#include "sms.h"

char *pChDebugFile = "debug.txt"; /*!< debug text file */
FILE *pDebug; /*!< pointer to debug file */

float *sms_window_spec;
float  *sms_tab_sine, *sms_tab_sinc;

#define RAND_STDMATH 1
#define HALF_MAX 1073741823.5  /*!< half the max of a 32-bit word */
#define INV_HALF_MAX (1.0 / HALF_MAX)

/*! \brief initialize global data
 *
 * Currently, just generating the sine and sinc tables.
 * This is necessary before both analysis and synthesis.
 * \todo why return something here?
 */
int sms_init( void )
{
	if (sms_tab_sine == NULL)
        {
                sms_prepSine (2046); //try 4096
                sms_prepSinc (4096);

#ifdef FFTW
        printf("libsms: using FFTW fft routines\n");
#else
#ifdef OOURA 
        printf("libsms: using OOURA fft routines\n");
#else
        printf("libsms: using realft fft routines\n");
#endif /* OOURA */
#endif /* FFTW */
        }

        return (1);
}

/*! \brief free global data
 *
 * deallocates memory allocated to global arrays (windows and tables)
 *
 */
void sms_free( void )
{
        sms_clearSine();
        sms_clearSinc();

	if(sms_window_spec) free(sms_window_spec);

        //fftwf_cleanup();

}

/*! \brief initialize analysis data structure's arrays
 * 
 *  based on the SMS_AnalParams current settings, this function will
 *  initialize the sound, synth, and fft arrays. It is necessary before analysis.
 *  there can be multple SMS_AnalParams at the same time
 *
 * \param pAnalParams    pointer to analysis paramaters
 * \return error code \see SMS_ERRORS
 */
int sms_initAnalysis ( SMS_AnalParams *pAnalParams)
{

        SMS_SndBuffer *pSynthBuf = &pAnalParams->synthBuffer;
        SMS_SndBuffer *pSoundBuf = &pAnalParams->soundBuffer;

	int sizeBuffer = (pAnalParams->iMaxDelayFrames * pAnalParams->sizeHop) + SMS_MAX_WINDOW;
	int i;

        sms_allocFrame (&pAnalParams->prevFrame, pAnalParams->nGuides, 
                           pAnalParams->nStochasticCoeff, 1, pAnalParams->iStochasticType);
  
	/* sound buffer */
	if ((pSoundBuf->pFBuffer = (float *) calloc(sizeBuffer, sizeof(float)))
	    == NULL)
		return (SMS_MALLOC);
	pSoundBuf->iMarker = -sizeBuffer;
	pSoundBuf->iFirstGood = sizeBuffer;
	pSoundBuf->sizeBuffer = sizeBuffer;
  
	/* deterministic synthesis buffer */
	pSynthBuf->sizeBuffer = pAnalParams->sizeHop << 1;
	if ((pSynthBuf->pFBuffer = 
	      (float *) calloc(pSynthBuf->sizeBuffer, sizeof(float))) == NULL)
		return (SMS_MALLOC);
	pSynthBuf->iMarker = -sizeBuffer;
	pSynthBuf->iMarker = pSynthBuf->sizeBuffer;
  
	/* buffer of analysis frames */
	if ((pAnalParams->pFrames = (SMS_AnalFrame *) calloc(pAnalParams->iMaxDelayFrames, sizeof(SMS_AnalFrame))) 
	    == NULL)
		return (SMS_MALLOC);
	if ((pAnalParams->ppFrames = 
	     (SMS_AnalFrame **) calloc(pAnalParams->iMaxDelayFrames, sizeof(SMS_AnalFrame *)))
	     == NULL)
		return (SMS_MALLOC);
  
	/* initialize the frame pointers and allocate memory */
	for (i = 0; i < pAnalParams->iMaxDelayFrames; i++)
	{
		pAnalParams->pFrames[i].iStatus = SMS_FRAME_EMPTY;
		(pAnalParams->pFrames[i].deterministic).nTracks = pAnalParams->nGuides;
		if (((pAnalParams->pFrames[i].deterministic).pFSinFreq =
		    (float *)calloc (pAnalParams->nGuides, sizeof(float))) == NULL)
			return (SMS_MALLOC);
		if (((pAnalParams->pFrames[i].deterministic).pFSinAmp =
		    (float *)calloc (pAnalParams->nGuides, sizeof(float))) == NULL)
			return (SMS_MALLOC);
		if (((pAnalParams->pFrames[i].deterministic).pFSinPha =
		    (float *) calloc (pAnalParams->nGuides, sizeof(float))) == NULL)
			return (SMS_MALLOC);
		pAnalParams->ppFrames[i] = &pAnalParams->pFrames[i];
	}

	/* initialitze window buffer for spectrum */
	if(!sms_window_spec)
                sms_window_spec = (float *) calloc (SMS_MAX_WINDOW, sizeof(float));

        /* allocate memory for FFT */

#ifdef FFTW
        pAnalParams->fftw.pWaveform = fftwf_malloc(sizeof(float) * SMS_MAX_WINDOW);
        pAnalParams->fftw.pSpectrum = fftwf_malloc(sizeof(fftwf_complex) * (SMS_MAX_WINDOW / 2 + 1));

        /* arg... this crashes and my head isn't clear enough to see why.. */
/*         if(sms_allocFourierForward( pAnalParams->fftw.pWaveform, pAnalParams->fftw.pSpectrum, */
/*                                     SMS_MAX_WINDOW) != SMS_OK) */
/*                 return(-1); */
                
#endif

	return (SMS_OK);
}

/*! \brief initialize synthesis data structure's arrays
 * 
 *  Initialize the synthesis and fft arrays. It is necessary before synthesis.
 *  there can be multple SMS_SynthParams at the same time
 *  This function also sets some initial values that will create a sane synthesis
 *  environment.
 *
 * This function requires an SMS_Header because it may be called to synthesize
 * a stored .sms file, which contains a header with necessary information.
 *
 * \param pSmsHeader      pointer to SMS_Header
 * \param pSynthParams    pointer to synthesis paramaters
 * \return error code \see SMS_ERRORS
 */
int sms_initSynth( SMS_Header *pSmsHeader, SMS_SynthParams *pSynthParams )
{

        /* set synthesis parameters from arguments and header */
	pSynthParams->iOriginalSRate = pSmsHeader->iOriginalSRate;
	pSynthParams->origSizeHop = pSynthParams->iOriginalSRate / pSmsHeader->iFrameRate;
	pSynthParams->iStochasticType = pSmsHeader->iStochasticType;
        if(pSynthParams->iSamplingRate <= 0)  pSynthParams->iSamplingRate = pSynthParams->iOriginalSRate;

        /*initialize transposing value to 1, no transpose */
        pSynthParams->fTranspose = 1.0;

        /* initialize stochastic gain multiplier, 1 is no gain */
	pSynthParams->fStocGain = 1.0;

        /*! \todo: round sizeHop to power of 2 */
        int sizeHop = pSynthParams->sizeHop;

        pSynthParams->pFStocWindow = 
		(float *) calloc(sizeHop * 2, sizeof(float));
        sms_getWindow( sizeHop * 2, pSynthParams->pFStocWindow, SMS_WIN_HANNING );
	pSynthParams->pFDetWindow =
		(float *) calloc(sizeHop * 2, sizeof(float));
        sms_getWindow( sizeHop * 2, pSynthParams->pFDetWindow, SMS_WIN_IFFT );

        /* allocate memory for analysis data - size of original hopsize */
	sms_allocFrame (&pSynthParams->prevFrame, pSmsHeader->nTracks, 
                         1 + pSmsHeader->nStochasticCoeff, 1, pSmsHeader->iStochasticType);

        /* allocate memory for FFT - big enough for output buffer (new hopsize)*/
        int sizeFft = sizeHop << 1;
#ifdef FFTW
        if((pSynthParams->fftw.pSpectrum =  fftwf_malloc(sizeof(fftwf_complex) * (sizeFft / 2 + 1)))
           == NULL)
        {
                printf("\n sms_initSynth: could not allocate spectrum array for fftw \n");
                return (SMS_FFTWERR);
        }
        if((pSynthParams->fftw.pWaveform = fftwf_malloc(sizeof(float) * sizeFft))
           == NULL)
        {
                printf("\n sms_initSynth: could not allocate waveform array for fftw \n");
                return (SMS_FFTWERR);
        }
        if((pSynthParams->fftw.plan =
            fftwf_plan_dft_c2r_1d( sizeFft, pSynthParams->fftw.pSpectrum,
                                   pSynthParams->fftw.pWaveform, FFTW_ESTIMATE)) == NULL)
        {
                printf("\n sms_initSynth: could not make fftw plan \n");
                return (SMS_FFTWERR);
        }
#else        
        /*debugging realft */
        pSynthParams->realftOut = (float *) calloc(sizeFft+1, sizeof(float));
#endif

        return (SMS_OK);
}

/*! \brief free analysis data
 * 
 * frees all the memory allocated to an SMS_AnalParams by
 * sms_initAnalysis
 *
 * \param pAnalParams    pointer to analysis data structure
 */
void sms_freeAnalysis( SMS_AnalParams *pAnalParams )
{
       int i;
        for (i = 0; i < pAnalParams->iMaxDelayFrames; i++)
	{
                free((pAnalParams->pFrames[i].deterministic).pFSinFreq);
                free((pAnalParams->pFrames[i].deterministic).pFSinAmp);
                free((pAnalParams->pFrames[i].deterministic).pFSinPha);
        }

        sms_freeFrame(&pAnalParams->prevFrame);
        free(pAnalParams->soundBuffer.pFBuffer);
        free(pAnalParams->synthBuffer.pFBuffer);
        free(pAnalParams->pFrames);
        free(pAnalParams->ppFrames);

#ifdef FFTW
        fftwf_free(pAnalParams->fftw.pWaveform);
        fftwf_free(pAnalParams->fftw.pSpectrum);
        fftwf_destroy_plan(pAnalParams->fftw.plan);
#endif
}

/*! \brief free analysis data
 * 
 * frees all the memory allocated to an SMS_SynthParams by
 * sms_initSynthesis
 *
 * \todo is there a way to make sure the plan has been made
 * already? as it is, it crashes if this is called without one
 * \param pSynthParams    pointer to synthesis data structure
 */
void sms_freeSynth( SMS_SynthParams *pSynthParams )
{
        //printf("\n sms_freeSynth \n");
        free(pSynthParams->pFStocWindow);        
        free(pSynthParams->pFDetWindow);
        sms_freeFrame(&pSynthParams->prevFrame);

#ifdef FFTW
        fftwf_destroy_plan(pSynthParams->fftw.plan);
        fftwf_free(pSynthParams->fftw.pSpectrum);
        fftwf_free(pSynthParams->fftw.pWaveform);
#else
        free (pSynthParams->realftOut);
#endif
}

/*! \brief give default values to an SMS_AnalParams struct 
 * 
 * This will fill an SMS_AnalParams with values that work
 * for common analyses.  It is useful to start with and then
 * adjust the parameters manually to fit a particular sound
 *
 * Certain things are hard coded in here that will have to 
 * be updated later (i.e. samplerate), so it is best to call this
 * function first, then fill whatever parameters need to be 
 * adjusted.
 * 
 * \param pAnalParams    pointer to analysis data structure
 */
void sms_initAnalParams (SMS_AnalParams *pAnalParams)
{
	pAnalParams->iDebugMode = 0;
	pAnalParams->iFormat = SMS_FORMAT_H;
	pAnalParams->iFrameRate = 400;
	pAnalParams->iStochasticType =SMS_STOC_APPROX;
	pAnalParams->nStochasticCoeff = 32;
	pAnalParams->fLowestFundamental = 50;
	pAnalParams->fHighestFundamental = 1000;
	pAnalParams->fDefaultFundamental = 100;
	pAnalParams->fPeakContToGuide = .4;
	pAnalParams->fFundContToGuide = .5;
	pAnalParams->fFreqDeviation = .45;
        pAnalParams->iSamplingRate = 44100; /*should be reset to correct rate */
	pAnalParams->iDefaultSizeWindow = 
		(int)((pAnalParams->iSamplingRate / pAnalParams->fDefaultFundamental) *
		pAnalParams->fSizeWindow / 2) * 2 + 1; /* odd length */
        pAnalParams->sizeHop = 110;
	pAnalParams->fSizeWindow = 3.5;
	pAnalParams->nGuides = 100;
	pAnalParams->iCleanTracks = 1;
	pAnalParams->fMinRefHarmMag = 30;
	pAnalParams->fRefHarmMagDiffFromMax = 30;
	pAnalParams->iRefHarmonic = 1;
	pAnalParams->iMinTrackLength = 40; /*!< depends on iFrameRate normally */
	pAnalParams->iMaxSleepingTime = 40; /*!< depends on iFrameRate normally */
	pAnalParams->fHighestFreq = 12000.;
	pAnalParams->fMinPeakMag = 0;
	pAnalParams->iSoundType = SMS_SOUND_TYPE_MELODY;
	pAnalParams->iAnalysisDirection = SMS_DIR_FWD;
	pAnalParams->iWindowType = SMS_WIN_BH_70;
        pAnalParams->iSizeSound = 0; /*!< no sound yet */
	pAnalParams->iMaxDelayFrames = 
		MAX(pAnalParams->iMinTrackLength, pAnalParams->iMaxSleepingTime) + 2 +
			SMS_DELAY_FRAMES;
	pAnalParams->fResidualPercentage = 0;
}

/*! \brief set window size for next frame 
 *
 * adjusts the next window size to fit the currently detected fundamental 
 * frequency, or resets to a default window size if unstable.
 *
 * \param iCurrentFrame         number of current frame
 * \param pAnalParams          analysis parameters
 * \return the size of the next window in samples
 */
int sms_sizeNextWindow (int iCurrentFrame, SMS_AnalParams *pAnalParams)
{
	float fFund = pAnalParams->ppFrames[iCurrentFrame]->fFundamental,
        fPrevFund = pAnalParams->ppFrames[iCurrentFrame-1]->fFundamental;
	int sizeWindow;
  
	/* if the previous fundamental was stable use it to set the window size */
	if (fPrevFund > 0 &&
	    fabs(fPrevFund - fFund) / fFund <= .2)
		sizeWindow = (int) ((pAnalParams->iSamplingRate / fFund) *
			pAnalParams->fSizeWindow * .5) * 2 + 1;
	/* otherwise use the default size window */
	else
		sizeWindow = pAnalParams->iDefaultSizeWindow;
  
	if (sizeWindow > SMS_MAX_WINDOW)
	{
		fprintf (stderr, "sms_sizeNextWindow error: sizeWindow (%d) too big, set to %d\n", sizeWindow, 
		         SMS_MAX_WINDOW);
		sizeWindow = SMS_MAX_WINDOW;
	}
  
	return (sizeWindow);
}

/*! \brief initialize the current frame
 *
 * initializes arrays to zero and sets the correct sample position.
 * Special care is taken at the end the sample source (if there is
 * not enough samples for an entire frame.
 *
 * \param iCurrentFrame            frame number of current frame in buffer
 * \param pAnalParams             analysis parameters
 * \param sizeWindow               size of analysis window 
 * \return -1 on error
 */
int sms_initFrame (int iCurrentFrame, SMS_AnalParams *pAnalParams, 
                      int sizeWindow)
{
	/* clear deterministic data */
	memset ((float *) pAnalParams->ppFrames[iCurrentFrame]->deterministic.pFSinFreq, 0, 
	        sizeof(float) * pAnalParams->nGuides);
	memset ((float *) pAnalParams->ppFrames[iCurrentFrame]->deterministic.pFSinAmp, 0, 
	        sizeof(float) * pAnalParams->nGuides);
	memset ((float *) pAnalParams->ppFrames[iCurrentFrame]->deterministic.pFSinPha, 0, 
	        sizeof(float) * pAnalParams->nGuides);
	/* clear peaks */
	memset ((void *) pAnalParams->ppFrames[iCurrentFrame]->pSpectralPeaks, 0,
	        sizeof (SMS_Peak) * SMS_MAX_NPEAKS);

	pAnalParams->ppFrames[iCurrentFrame]->nPeaks = 0;
	pAnalParams->ppFrames[iCurrentFrame]->fFundamental = 0;
  
	pAnalParams->ppFrames[iCurrentFrame]->iFrameNum =  
		pAnalParams->ppFrames[iCurrentFrame - 1]->iFrameNum + 1;
	pAnalParams->ppFrames[iCurrentFrame]->iFrameSize =  sizeWindow;
  
	/* if first frame set center of data around 0 */
	if(pAnalParams->ppFrames[iCurrentFrame]->iFrameNum == 1)
		pAnalParams->ppFrames[iCurrentFrame]->iFrameSample = 0;
	/* increment center of data by sizeHop */
	else
		pAnalParams->ppFrames[iCurrentFrame]->iFrameSample = 
			pAnalParams->ppFrames[iCurrentFrame-1]->iFrameSample + pAnalParams->sizeHop;
  	 
	/* check for error */
	if (pAnalParams->soundBuffer.iMarker >
	         pAnalParams->ppFrames[iCurrentFrame]->iFrameSample - (sizeWindow+1)/2)
	{
		printf("sms_initFrame error: runoff on the sound buffer\n");
		return(-1);
	} 

	/* check for end of sound */
	if ((pAnalParams->ppFrames[iCurrentFrame]->iFrameSample + (sizeWindow+1)/2) >=
	    pAnalParams->iSizeSound)
	{
		pAnalParams->ppFrames[iCurrentFrame]->iFrameNum =  -1;
		pAnalParams->ppFrames[iCurrentFrame]->iFrameSize =  0;
		pAnalParams->ppFrames[iCurrentFrame]->iStatus =  SMS_FRAME_END;
	}
	else
                /* good status, ready to start computing */
		pAnalParams->ppFrames[iCurrentFrame]->iStatus = SMS_FRAME_READY;
        return(SMS_OK);
}

/*! \brief get deviation from average fundamental
 *\
 * \param pAnalParams             pointer to analysis params
 * \param iCurrentFrame        number of current frame 
 * \return deviation value or -1 if really off
 */
float sms_fundDeviation ( SMS_AnalParams *pAnalParams, int iCurrentFrame)
{
	float fFund, fSum = 0, fAverage, fDeviation = 0;
        int i;

	/* get the sum of the past few fundamentals */
	for (i = 0; i < SMS_MIN_GOOD_FRAMES; i++)
	{
		fFund = pAnalParams->ppFrames[iCurrentFrame-i]->fFundamental;
		if(fFund <= 0)
			return(-1);
		else
			fSum += fFund;
	}
  
	/* find the average */
	fAverage = fSum / SMS_MIN_GOOD_FRAMES;
  
	/* get the deviation from the average */
	for (i = 0; i < SMS_MIN_GOOD_FRAMES; i++)
		fDeviation += fabs(pAnalParams->ppFrames[iCurrentFrame-i]->fFundamental - fAverage);
  
	/* return the deviation from the average */
	return (fDeviation / (SMS_MIN_GOOD_FRAMES * fAverage));
}


/*! \brief function to create the debug file 
 *
 * \param pAnalParams             pointer to analysis params
 * \return error value \see SMS_ERRORS 
*/
int sms_createDebugFile (SMS_AnalParams *pAnalParams)
{
	if ((pDebug = fopen(pChDebugFile, "w+")) == NULL) 
	{
		fprintf(stderr, "Cannot open debugfile: %s\n", pChDebugFile);
		return(SMS_WRERR);
	}
        else return(SMS_OK);
}

/*! \brief  function to write to the debug file
 *
 * writes three arrays of equal size to a debug text
 * file ("./debug.txt"). There are three arrays for the 
 * frequency, magnitude, phase sets. 
 * 
 * \param pFBuffer1 pointer to array 1
 * \param pFBuffer2 pointer to array 2
 * \param pFBuffer3 pointer to array 3
 * \param sizeBuffer the size of the buffers
 */
void sms_writeDebugData (float *pFBuffer1, float *pFBuffer2, 
                             float *pFBuffer3, int sizeBuffer)
{
	int i;
	static int counter = 0;

	for (i = 0; i < sizeBuffer; i++)
		fprintf (pDebug, "%d %d %d %d\n", counter++, (int)pFBuffer1[i],
		         (int)pFBuffer2[i], (int)pFBuffer3[i]);

}

/*! \brief  function to write the residual sound file to disk
 *
 * writes the "debug.txt" file to disk and closes the file.
 */
void sms_writeDebugFile ()
{
	fclose (pDebug);
}

/*! \brief convert from magnitude to decibel
 *
 * \param x      magnitude (0-1)
 * \return         decibel (0-100)
 */
float sms_magToDB( float x)
{
    if (x <= 0) return (0);
    else
    {
        float val = 100 + 20./LOG10 * log(x);
        return (val < 0 ? 0 : val);
    }
}

/*! \brief convert from decibel to magnitude
 *
 * \param x     decibel (0-100)
 * \return        magnitude (0-1)
 */
float sms_dBToMag( float x)
{
    if (x <= 0)
        return(0);
    else
    {
        if (x > 485)
            x = 485;
    }
    return (exp((LOG10 * 0.05) * (x-100.)));
}

/*! \brief get a string containing information about the error code 
 *
 * \param iError error code \see SMS_ERRORS
 * \return  pointer to a char string
 */
const char* sms_errorString(int iError)
{
        switch(iError)
        {
        case SMS_NOPEN: return ("cannot open input file");
                break;
        case SMS_NSMS: return ("input file not an SMS file");
                break;
        case SMS_MALLOC: return ("cannot allocate memory for input file");
                break;
        case SMS_RDERR: return ("read error in input file");
                break;
        case SMS_WRERR: return ("cannot write output file");
                break;
        case SMS_SNDERR: return ("problem with libsndfile");
                break;
        default: return ("error undefined"); 
        }
}

/*! \brief random number genorator
 *
 * 
 * \return random number between -1 and 1
 */
float sms_random()
{
        float f;
#ifdef RAND_STDMATH
        f = ((random() - HALF_MAX) * INV_HALF_MAX);
#else
        
#endif
        return(f);
}
