#include "sms.h"
#include "smspd.h"
#include <string.h>

t_class *smspd_class;

typedef struct smspd 
{
  t_object t_obj;
} t_smspd;


static void *smspd_new(void)
{
  t_smspd *x = (t_smspd *)pd_new(smspd_class);
  return (x);
}

void smspd_setup(void) 
{

        smspd_class = class_new(gensym("smspd"), smspd_new, 0,
                           sizeof(t_smspd), CLASS_NOINLET, 0);

        //call object setup routines
        smsbuf_setup();
        smsanal_setup();
        smssynth_tilde_setup();
        //smsedit_setup();

        post("smspd external library - July 20, 2008");
}


/* ------------------------ smsbuf ----------------------------- */

static t_class *smsbuf_class;

typedef struct _smsbuf
{
        t_object x_obj;
        t_canvas *canvas;
        t_symbol *filename;
        t_symbol *bufname;
        int nframes;
        int ready;
        int allocated;
	FILE *pSmsFile; //does this need to be in the struct?
        char param_string[1024];
        SMS_Header smsHeader;
        SMS_Data *smsData;
} t_smsbuf;

/* NEXT::::: allocation and deallocation is f-ed up */
void smsbuf_dealloc(t_smsbuf *x)
{
        int i;
        for( i = 0; i < x->nframes; i++)
                sms_freeRecord(&x->smsData[i]);
        free(x->smsData);
}

/* a function to allocate the desired number of SMS_Data 
   frames, usable by other externals in the sms library as well */
void smsbuf_alloc(t_smsbuf *x)
{
        if(x->allocated)
        {
                post("smsbuf already allocated, deallocating");
                smsbuf_dealloc(x);
        }
        int i;
        /* will this be faster with one malloc? try once everything is setup */
        x->smsData = calloc(x->nframes, sizeof(SMS_Data));
        for( i = 0; i < x->nframes; i++ )
        {
                sms_allocRecordH (&x->smsHeader,  &x->smsData[i]);
                ///DUHHHH I am not getting the info out of a file..
                //sms_getRecord (x->pSmsFile, &x->smsHeader, i, &x->smsData[i]);
        }

}

/* function to open a file and load into the buffer:
 * 1. get fullname in a system independant manner
 * 2. read file header
 * 3. initialize the synthesizer based on header/synth params
 * 4. because we are synthesizing from file, allocate space for sms frames
 */
static void smsbuf_open(t_smsbuf *x, t_symbol *filename)
{
        SMS_Header *pSmsHeader;
        long iError;
        int i;
        t_symbol *fullname;
        x->filename = gensym(filename->s_name);
        fullname = getFullPathName(filename, x->canvas);

//        printf("smsheader address: %p ", &x->pSmsBuf->pSmsHeader);

        if(fullname == NULL)
        {
                pd_error(x, "smsbuf_open: cannot find file: %s", filename->s_name);
                return;
        }
        else post("file: %s", fullname->s_name);
        //check if a file has been opened, close and init if necessary
        if(x->nframes != 0)
        {
                post("smsbuf_open: re-initializing");
                for( i = 0; i < x->nframes; i++)
                        sms_freeRecord(&x->smsData[i]);
        }

        if ((iError = sms_getHeader (fullname->s_name, &pSmsHeader, &x->pSmsFile)) < 0)
//        if ((iError = sms_getHeader (fullname->s_name, &pHeader, &x->pSmsFile)) < 0)
	{
                pd_error(x, "smsbuf_open: %s", sms_errorString(iError));
                return;
        }
        //post("smsheader address: %p ", x->smsBuf.pSmsHeader);


        /* allocate memory for nframes of SMS_Data */
        x->nframes = pSmsHeader->nFrames;
        if(0)post("nframes: %d ", x->nframes);
        /*Buffer the entire file in smsBuf.  For now, I'm doing this the simplest way possible.*/        
        /* will this be faster with one malloc? try once everything is setup */
        x->smsData = calloc(x->nframes, sizeof(SMS_Data));
        for( i = 0; i < x->nframes; i++ )
        {
                sms_allocRecordH (pSmsHeader,  &x->smsData[i]);
                sms_getRecord (x->pSmsFile, pSmsHeader, i, &x->smsData[i]);
        }

        /* copy header to buffer */
        CopySmsHeader( pSmsHeader, &x->smsHeader, x->param_string );

//        post("nFrames: %d ", x->pSmsHeader->nFrames);//x->nframes);
        x->ready = 1;
        post("sms file buffered: %s ", filename->s_name );
        return;
}

static void smsbuf_info(t_smsbuf *x)
{
        if(x->ready)
        {
                post("sms file : %s ", x->bufname->s_name );
                //post("sms file : %s ", x->filename->s_name );
                post("original file length: %f seconds ", (float)  x->smsHeader.nFrames /
                     x->smsHeader.iFrameRate);
                post("__header contents__");
                post("Number of Frames: %d", x->smsHeader.nFrames);
                post("Frame rate (Hz) = %d", x->smsHeader.iFrameRate);
                post("Number of trajectories = %d", x->smsHeader.nTrajectories);
                post("Number of stochastic coefficients = %d",
                     x->smsHeader.nStochasticCoeff);
                if(x->smsHeader.iFormat == SMS_FORMAT_H) 
                        post("Format = harmonic");
                else if(x->smsHeader.iFormat == SMS_FORMAT_IH) 
                        post("Format = inharmonic");
                else if(x->smsHeader.iFormat == SMS_FORMAT_HP)
                        post("Format = harmonic with phase");
                else if(x->smsHeader.iFormat == SMS_FORMAT_IHP)
                        post("Format = inharmonic with phase");
                if(x->smsHeader.iStochasticType == SMS_STOC_WAVE) post("Stochastic type = waveform");
                else if(x->smsHeader.iStochasticType == SMS_STOC_IFFT) post("Stochastic type = IFFT");
                else if(x->smsHeader.iStochasticType == SMS_STOC_APPROX)
                        post("Stochastic type = line segment magnitude spectrum approximation ");
                else if(x->smsHeader.iStochasticType == SMS_STOC_NONE) post("Stochastic type = none");
                post("Original sampling rate = %d", x->smsHeader.iOriginalSRate);  

                if (x->smsHeader.nTextCharacters > 0)
                        post("ANALISIS ARGUMENTS: %s", x->smsHeader.pChTextCharacters);
        }
        else post("smsbuf (%s) not ready", x->bufname->s_name);
}

static void smsbuf_printframe(t_smsbuf *x, float f)
{
        if(f >= x->nframes) return;
               
        int i;
        int frame = (int) f;

        if(x->ready)
        {
                post("----- smsbuf (%s):: frame: %d, timetag: %f -----", x->bufname->s_name, frame, f / x->smsHeader.iFrameRate);
                for(i = 0; i < x->smsHeader.nTrajectories; i++)
                                       if(x->smsData[frame].pFMagTraj[i] > 0.00000001 )
                                               post("harmonic %d : %f[%f]", i, x->smsData[frame].pFFreqTraj[i],
                                                    x->smsData[frame].pFMagTraj[i]);
        }
        else post("smsbuf (%s) not ready", x->bufname->s_name);
}
static void *smsbuf_new(t_symbol *bufname)
{
        t_smsbuf *x = (t_smsbuf *)pd_new(smsbuf_class);

        x->canvas = canvas_getcurrent();
        x->filename = NULL;
        x->nframes= 0;
        x->ready= 0;

        //todo: make a default name if none is given:
        //if (!*s->s_name) s = gensym("delwrite~");
        // ?? do in need to check if bufname already exists? ??
        pd_bind(&x->x_obj.ob_pd, bufname);
        x->bufname = bufname;
        post("bufname: %s", bufname->s_name);

        sms_init();
    
        return (x);
}

static void smsbuf_free(t_smsbuf *x)
{
        int i;
        sms_free();

        //smsbuf_dealloc(x);
        if(x->allocated)
        {
                for( i = 0; i < x->nframes; i++)
                        sms_freeRecord(&x->smsData[i]);

                free(x->smsData);
        }

        pd_unbind(&x->x_obj.ob_pd, x->bufname);
}
void smsbuf_setup(void)
{
        smsbuf_class = class_new(gensym("smsbuf"), (t_newmethod)smsbuf_new, 
                                       (t_method)smsbuf_free, sizeof(t_smsbuf), 0, A_DEFSYM, 0);
        class_addmethod(smsbuf_class, (t_method)smsbuf_open, gensym("open"), A_DEFSYM, 0);
        class_addmethod(smsbuf_class, (t_method)smsbuf_info, gensym("info"),  0);
        class_addmethod(smsbuf_class, (t_method)smsbuf_printframe, gensym("printframe"), A_DEFFLOAT, 0);
}

/* ------------------------ smsanal ----------------------------- */

static t_class *smsanal_class;

typedef struct _smsanal
{
        t_object x_obj; 
        t_canvas *canvas;
        t_symbol *filename;
        t_smsbuf *smsbuf;
        SMS_AnalParams anal_params;
        int nTrajectories;
        int verbose;
        float    *wavetable;	
        int      wavetablesize;
} t_smsanal;

static void smsanal_sizehop(t_smsanal *x, t_float fSizeHop)
{
        //what is minimum hopsize?
        post("TODO: set sizeHop and re-init");
}

static void smsanal_debug(t_smsanal *x, t_float debugMode)
{
        if(x->verbose) post("debug mode: %d. TODO: say what this is debugging", (int) debugMode);
        x->anal_params.iDebugMode = debugMode;
}

static void smsanal_buffer(t_smsanal *x, t_symbol *bufname)
{
        // get the pointer to the desired buffer (todo: still have to set the buffer symbol name)
        x->smsbuf =
        (t_smsbuf *)pd_findbyclass(bufname, smsbuf_class);

        if(!x->smsbuf)
        {
                error("smsbuf: %s was not found", bufname->s_name);
                return;
        }
        else post("smsanal is using buffer: %s", bufname->s_name);

}
/* open function:
 * 1. get fullname in a system independant manner
 * 2. read file header
 * 3. initialize the synthesizer based on header/synth params
 * 4. because we are synthesizing from file, allocate space for sms frames
 */
static void smsanal_sf(t_smsanal *x, t_symbol *filename)
{
        if(!x->smsbuf)
        {
                error("smsanal_sf: set the buffer pointer before analysis");
                return;
        }
        x->smsbuf->ready = 0;

        long iNextSizeRead = 0;
        int i;
        int iDoAnalysis = 1;
        int iSample = 0;
        int iStatus = 0;
        int iRecord = 0;
        int sizeNewData = 0;
        int sizeNextRead = 0;
        long iError;
        t_symbol *fullname;
	short pSoundData[SMS_MAX_WINDOW]; //todo: change to float
	SMS_SndHeader SoundHeader;

        x->filename = gensym(filename->s_name);
        fullname = getFullPathName(filename, x->canvas);

        if(fullname == NULL)
        {
                error("smsanal_open: cannot find file: %s", filename->s_name);
                return;
        }
        else post("file: %s", fullname->s_name);

        //check if a file has been opened, close and init if necessary
        if(x->smsbuf->nframes != 0)
        {
                post("smsanal_open: re-initializing (not doing anything here yet)");
                for( i = 0; i < x->smsbuf->nframes; i++)
                        sms_freeRecord(&x->smsbuf->smsData[i]);

                free(x->smsbuf->smsData);
        }

	/* open input sound */
	sms_openSF (fullname->s_name , &SoundHeader);

        x->anal_params.iSamplingRate = SoundHeader.iSamplingRate;
        x->anal_params.iDefaultSizeWindow = 
                (int)((x->anal_params.iSamplingRate / x->anal_params.fDefaultFundamental) 
                      * x->anal_params.fSizeWindow / 2) * 2 + 1;
        /* define the hopsize for each record */
	x->anal_params.sizeHop = (int)(SoundHeader.iSamplingRate / 
	                (float) x->anal_params.iFrameRate);

        /* define how many records*/
	x->smsbuf->nframes = 3 + SoundHeader.nSamples / 
                (float) x->anal_params.sizeHop;
        x->anal_params.iSizeSound = SoundHeader.nSamples;
        if(x->verbose)
        {
                post("smsanal: set default size window to %d", x->anal_params.iDefaultSizeWindow);
                post("smsanal: set sizeHop to %d", x->anal_params.sizeHop);
                post("smsanal: ready to analyze %d frames", x->smsbuf->nframes);
        }
        /* need to supply sms header information for incase the analysis 
           will be written to file (by smsbuf) */
	sms_fillHeader (&x->smsbuf->smsHeader, x->smsbuf->nframes,
                        &x->anal_params, SoundHeader.iSamplingRate, 
                         x->nTrajectories);

        sprintf (x->smsbuf->param_string,
                 "created by [smsanal] with parameters: format %d, soundType %d, "
                 "analysisDirection %d, windowSize %.2f,"
                 " windowType %d, frameRate %d, highestFreq %.2f, minPeakMag %.2f,"
                 " refHarmonic %d, minRefHarmMag %.2f, refHarmMagDiffFromMax %.2f,"
                 " defaultFund %.2f, lowestFund %.2f, highestFund %.2f, nGuides %d,"
                 " nTrajectories %d, freqDeviation %.2f, peakContToGuide %.2f,"
                 " fundContToGuide %.2f, cleantTraj %d, minTrajLength %d,"
                 "iMaxsleepingTime %d, stochasticType %d, nStocCoeff %d\n", 	
                 x->anal_params.iFormat, x->anal_params.iSoundType,
                 x->anal_params.iAnalysisDirection, x->anal_params.fSizeWindow, 
                 x->anal_params.iWindowType, x->anal_params.iFrameRate,
                 x->anal_params.fHighestFreq, x->anal_params.fMinPeakMag,
                 x->anal_params.iRefHarmonic, x->anal_params.fMinRefHarmMag, 
                 x->anal_params.fRefHarmMagDiffFromMax,  
                 x->anal_params.fDefaultFundamental, x->anal_params.fLowestFundamental,
                 x->anal_params.fHighestFundamental, x->anal_params.nGuides,
                 x->nTrajectories, x->anal_params.fFreqDeviation, 
                 x->anal_params.fPeakContToGuide, x->anal_params.fFundContToGuide,
                 x->anal_params.iCleanTraj, x->anal_params.iMinTrajLength,
                 x->anal_params.iMaxSleepingTime,  x->anal_params.iStochasticType,
                 x->anal_params.nStochasticCoeff);
       
        x->smsbuf->smsHeader.nTextCharacters = strlen (x->smsbuf->param_string) + 1;
        x->smsbuf->smsHeader.pChTextCharacters = x->smsbuf->param_string;;

       sms_initAnalysis (&x->anal_params);


       /* will this be faster with one malloc? try once everything is setup */
        x->smsbuf->smsData = calloc(x->smsbuf->nframes, sizeof(SMS_Data));
        for( i = 0; i < x->smsbuf->nframes; i++ )
                sms_allocRecordH (&x->smsbuf->smsHeader,  &x->smsbuf->smsData[i]);

        x->smsbuf->allocated = 1;

       iNextSizeRead = (x->anal_params.iDefaultSizeWindow + 1) * 0.5;
       
       if (x->anal_params.iAnalysisDirection == SMS_DIR_REV)
               iSample = x->anal_params.iSizeSound;
       
       /* loop for analysis */
       while(iDoAnalysis > 0)
       {
               //post("analyzing frame %d", iRecord);
               if (x->anal_params.iAnalysisDirection == SMS_DIR_REV)
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
                       if((iSample + iNextSizeRead) < SoundHeader.nSamples)
                               sizeNewData = iNextSizeRead;
                       else
                               sizeNewData = SoundHeader.nSamples - iSample;
               }
		/* get one frame of sound */
		if (sms_getSound (&SoundHeader, pSoundData, sizeNewData, iSample) < 0)
		{
			error("smsanal_sf: could not read sound record %d\n", iRecord);
			break;
		}
		/* perform analysis of one frame of sound */
		iStatus = sms_analyze (pSoundData, sizeNewData, &x->smsbuf->smsData[iRecord], 
		                       &x->anal_params, &iNextSizeRead);
                

		/* if there is an output SMS record, write it */
		if (iStatus == 1)
		{
			//sms_writeRecord (pOutputSmsFile, &smsHeader, &smsData);
			if(0)//todo: add verbose flag
                        {
                                if (iRecord % 10 == 0)
                                        post("%.2f ", iRecord / (float) x->smsbuf->smsHeader.iFrameRate);
                        }
			iRecord++;
		}
		else if (iStatus == -1) /* done */
		{
			iDoAnalysis = 0;
			x->smsbuf->smsHeader.nFrames = iRecord;
		}

	}
       post(" smsanal: analyzed %d frames from soundfile.", iRecord);
       x->smsbuf->smsHeader.fResidualPerc = x->anal_params.fResidualPercentage / iRecord;
       x->smsbuf->ready = 1;
       return;
}

static void smsanal_array(t_smsanal *x, t_symbol *arrayname, t_float samplerate)
{
        if(!x->smsbuf)
        {
                error("smsanal_sf: set the buffer pointer before analysis");
                return;
        }
        x->smsbuf->ready = 0;

        long iNextSizeRead = 0;
        int i;
        int iDoAnalysis = 1;
        int iSample = 0;
        int iStatus = 0;
        int iRecord = 0;
        int sizeNewData = 0;
        int sizeNextRead = 0;
        long iError;
        t_garray *a;
	short pSoundData[SMS_MAX_WINDOW];
	SMS_SndHeader SoundHeader;

        if(x->smsbuf->nframes != 0)
        {
                post("smsanal_open: re-initializing (not doing anything here yet)");
                for( i = 0; i < x->smsbuf->nframes; i++)
                        sms_freeRecord(&x->smsbuf->smsData[i]);

                free(x->smsbuf->smsData);
        }

        if (!(a = (t_garray *)pd_findbyclass(arrayname, garray_class)))
        {
                pd_error(x, "%s: no such array", arrayname->s_name);
                return;
        }
        else if (!garray_getfloatarray(a, &x->wavetablesize, &x->wavetable))
        {
                pd_error(x, "%s: bad template for tabread", arrayname->s_name);
                return;
        }
        else if(x->verbose) //table exists
                post("wavetablesize: %d, samplerate: %d", x->wavetablesize, (int) samplerate );

        if(!x->smsbuf)
        {
                error("smsanal_sf: set the buffer pointer before analysis");
                return;
        }

        x->anal_params.iSamplingRate = (int)samplerate;
        x->anal_params.iDefaultSizeWindow = 
                (int)((x->anal_params.iSamplingRate / x->anal_params.fDefaultFundamental) 
                      * x->anal_params.fSizeWindow / 2) * 2 + 1;

        /* define the hopsize for each record */
	x->anal_params.sizeHop = x->anal_params.iSamplingRate / x->anal_params.iFrameRate;

        /* define how many records*/
	x->smsbuf->nframes = 3 + x->wavetablesize / 
                (float) x->anal_params.sizeHop;
        x->anal_params.iSizeSound = x->wavetablesize;
        if(x->verbose)
        {
                post("smsanal: set default size window to %d", x->anal_params.iDefaultSizeWindow);
                post("smsanal: set sizeHop to %d", x->anal_params.sizeHop);
                post("smsanal: ready to analyze %d frames", x->smsbuf->nframes);
        }


        /* need to supply sms header information for incase the analysis 
           will be written to file (by smsbuf) */
	sms_fillHeader (&x->smsbuf->smsHeader, x->smsbuf->nframes,
                        &x->anal_params, x->anal_params.iSamplingRate, 
                         x->nTrajectories);

        sprintf (x->smsbuf->param_string,
                 "created by [smsanal] with parameters: format %d, soundType %d, "
                 "analysisDirection %d, windowSize %.2f,"
                 " windowType %d, frameRate %d, highestFreq %.2f, minPeakMag %.2f,"
                 " refHarmonic %d, minRefHarmMag %.2f, refHarmMagDiffFromMax %.2f,"
                 " defaultFund %.2f, lowestFund %.2f, highestFund %.2f, nGuides %d,"
                 " nTrajectories %d, freqDeviation %.2f, peakContToGuide %.2f,"
                 " fundContToGuide %.2f, cleantTraj %d, minTrajLength %d,"
                 "iMaxsleepingTime %d, stochasticType %d, nStocCoeff %d\n", 	
                 x->anal_params.iFormat, x->anal_params.iSoundType,
                 x->anal_params.iAnalysisDirection, x->anal_params.fSizeWindow, 
                 x->anal_params.iWindowType, x->anal_params.iFrameRate,
                 x->anal_params.fHighestFreq, x->anal_params.fMinPeakMag,
                 x->anal_params.iRefHarmonic, x->anal_params.fMinRefHarmMag, 
                 x->anal_params.fRefHarmMagDiffFromMax,  
                 x->anal_params.fDefaultFundamental, x->anal_params.fLowestFundamental,
                 x->anal_params.fHighestFundamental, x->anal_params.nGuides,
                 x->nTrajectories, x->anal_params.fFreqDeviation, 
                 x->anal_params.fPeakContToGuide, x->anal_params.fFundContToGuide,
                 x->anal_params.iCleanTraj, x->anal_params.iMinTrajLength,
                 x->anal_params.iMaxSleepingTime,  x->anal_params.iStochasticType,
                 x->anal_params.nStochasticCoeff);
       
        x->smsbuf->smsHeader.nTextCharacters = strlen (x->smsbuf->param_string) + 1;
        x->smsbuf->smsHeader.pChTextCharacters = x->smsbuf->param_string;;

       sms_initAnalysis (&x->anal_params);

        x->smsbuf->smsData = calloc(x->smsbuf->nframes, sizeof(SMS_Data));
        for( i = 0; i < x->smsbuf->nframes; i++ )
                sms_allocRecordH (&x->smsbuf->smsHeader,  &x->smsbuf->smsData[i]);

       iNextSizeRead = (x->anal_params.iDefaultSizeWindow + 1) * 0.5;
       
       if (x->anal_params.iAnalysisDirection == SMS_DIR_REV)
               iSample = x->anal_params.iSizeSound;

       /* loop for analysis */
       while(iDoAnalysis > 0)
       {
               //post("analyzing frame %d", iRecord);
               if (x->anal_params.iAnalysisDirection == SMS_DIR_REV)
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
                       if((iSample + iNextSizeRead) < x->wavetablesize)
                               sizeNewData = iNextSizeRead;
                       else
                               sizeNewData = x->wavetablesize - iSample;
               }
		/* get one frame of sound */
/* 		if (sms_getSound (&SoundHeader, pSoundData, sizeNewData, iSample) < 0) */
/* 		{ */
/* 			error("smsanal_sf: could not read sound record %d\n", iRecord); */
/* 			break; */
/* 		} */

		/* perform analysis of one frame of sound */
/* 		iStatus = sms_analyze (pSoundData, sizeNewData, &x->smsbuf->smsData[iRecord],  */
/* 		                       &x->anal_params, &iNextSizeRead); */

               /*until sms_analyze will act on floating point data, the sound chunk has to be converted to 16-bit integer
                 shorts */
               for(i = 0; i < sizeNewData; i++)
               {
                       pSoundData[i] = (short) (x->wavetable[iSample + i] * FLOAT_TO_SHORT);
                       printf("[%d] %d, ", i, pSoundData[i]);
               }
               iStatus = sms_analyze (pSoundData, sizeNewData, &x->smsbuf->smsData[iRecord], 
		                       &x->anal_params, &iNextSizeRead);
                

		/* if there is an output SMS record, write it */
		if (iStatus == 1)
		{
			//sms_writeRecord (pOutputSmsFile, &smsHeader, &smsData);
			if(0)//todo: add verbose flag
                        {
                                if (iRecord % 10 == 0)
                                        post("%.2f ", iRecord / (float) x->smsbuf->smsHeader.iFrameRate);
                        }
			iRecord++;
		}
		else if (iStatus == -1) /* done */
		{
			iDoAnalysis = 0;
			x->smsbuf->smsHeader.nFrames = iRecord;
		}

	}
       post(" smsanal: analyzed %d frames from soundfile.", iRecord);
       x->smsbuf->smsHeader.fResidualPerc = x->anal_params.fResidualPercentage / iRecord;
       x->smsbuf->ready = 1;
       return;

}

static void *smsanal_new(t_symbol *s, int argcount, t_atom *argvec)
{
        t_smsanal *x = (t_smsanal *)pd_new(smsanal_class);

        int i;

        x->canvas = canvas_getcurrent();
        x->nTrajectories = 30;
        x->smsbuf = NULL;
        x->verbose = 1;

        sms_initAnalParams (&x->anal_params);

        for (i = 0; i < argcount; i++)
        {
                if (argvec[i].a_type == A_SYMBOL)
                {
                        smsanal_buffer(x, argvec[i].a_w.w_symbol);
                }
        }

        return (x);
}

static void smsanal_free(t_smsanal *x)
{
        sms_freeAnalysis(&x->anal_params);
}
void smsanal_setup(void)
{
        smsanal_class = class_new(gensym("smsanal"), (t_newmethod)smsanal_new, 
                                       (t_method)smsanal_free, sizeof(t_smsanal), 0, A_GIMME, 0);

        class_addmethod(smsanal_class, (t_method)smsanal_buffer, gensym("buffer"), A_DEFSYM, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_sf, gensym("soundfile"), A_DEFSYM, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_array, gensym("array"), A_DEFSYM, A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_sizehop, gensym("sizehop"), A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_debug, gensym("debug"), A_DEFFLOAT, 0);
}

/* ------------------------ smssynth~ ----------------------------- */

#define SOURCE_FLOAT 1
#define SOURCE_SIGNAL 2

static t_class *smssynth_class;

typedef struct _smssynth
{
        t_object x_obj; 
        t_canvas *canvas;
        t_symbol *bufname;
        t_int i_frame, i_frameSource, synthBufPos;
        t_float *synthBuf;
        t_float f, transpose, stocgain;
        SMS_SynthParams synthParams;
        t_smsbuf *smsbuf;
        SMS_Data interpolatedRecord;
} t_smssynth;

static void smssynth_buffer(t_smssynth *x, t_symbol *bufname)
{
        long iError;

        if(!*bufname->s_name)
        {
                if(!*x->bufname->s_name)
                {
                        post("... don't have a bufname");
                        return;
                }
                else post("using initial bufname: %s", x->bufname->s_name);
        }
        else
        {
                post("new bufname: %s", bufname->s_name);
                x->bufname = bufname;
        }
        x->smsbuf =
        (t_smsbuf *)pd_findbyclass(x->bufname, smsbuf_class);

        if(!x->smsbuf)
        {
                error("smssynth~: %s was not found", x->bufname->s_name);
                return;
        }
        if(!x->smsbuf->ready)
        {
                error("smsbuf not ready");
                return;
        }
        else post("smsbuf IS REAAAAADDDDYY");
 
        //check if a file has been opened, if so re-init
        if(x->smsbuf->nframes != 0)
        {
                post("smssynth_open: re-initializing synth");
                sms_freeSynth(&x->synthParams);              
                sms_freeRecord(&x->interpolatedRecord);
        }
        sms_initSynth( &x->smsbuf->smsHeader, &x->synthParams );
        
	/* setup for interpolated synthesis from buffer */
        // I guess I am always ignoring phase information for now..
	sms_allocRecord (&x->interpolatedRecord, x->smsbuf->smsHeader.nTrajectories, 
	                   x->smsbuf->smsHeader.nStochasticCoeff, 0,
                           x->synthParams.origSizeHop, x->smsbuf->smsHeader.iStochasticType);

        post("smssynth_buffer: %d frames", x->smsbuf->smsHeader.nFrames);
}

static t_int *smssynth_perform(t_int *w)
{
        t_smssynth *x = (t_smssynth *)(w[1]);
        t_sample *in = (t_float *)(w[2]);
        t_sample *out = (t_float *)(w[3]);
        int n = (int)(w[4]);
        
        if(x->smsbuf != NULL && x->smsbuf->ready)        
        {
                float f;
                int i, iLeftRecord, iRightRecord;
                //SMS_Header *pSmsHeader = x->;
                //SMS_Data *pSmsData;
                int nFrames = x->smsbuf->smsHeader.nFrames;
                if(x->synthBufPos >= x->synthParams.sizeHop)
                {
                        if(x->f >= nFrames)
                                x->f = nFrames -1;
                        if(x->f < 0) x->f = 0;
                
                        iLeftRecord = MIN (nFrames - 1, floor (x->f)); 
                        iRightRecord = (iLeftRecord < nFrames - 2)
                                ? (1+ iLeftRecord) : iLeftRecord;

                        sms_interpolateRecords (&x->smsbuf->smsData[iLeftRecord], &x->smsbuf->smsData[iRightRecord],
                                                &x->interpolatedRecord, x->f - iLeftRecord);
                        
                        sms_synthesize (&x->interpolatedRecord, x->synthBuf, &x->synthParams);
                        x->synthBufPos = 0;
                }
                //check when blocksize is larger than hopsize... will probably crash
                for (i = 0; i < n; i++, x->synthBufPos++)
                        out[i] = x->synthBuf[x->synthBufPos];
        }
        else  while(n--) *out++ = 0;

        return (w+5);
}
static void smssynth_sizehop(t_smssynth *x, t_float *fSizeHop)
{
        //what is minimum hopsize?
        post("TODO: set sizeHop and re-init");
}

static void smssynth_transpose(t_smssynth *x, t_float f)
{
/*         x->transpose = f; */
        x->synthParams.fTranspose = TEMPERED_TO_FREQ( f );
        post("transpose: %f", x->synthParams.fTranspose);
}

static void smssynth_stocgain(t_smssynth *x, t_float f)
{
/*         x->stocgain = f; */
        x->synthParams.fStocGain = f;
        post("stocgain: %f", x->synthParams.fStocGain);
}


static void smssynth_info(t_smssynth *x)
{

        post("__arguments__");
        post("samplingrate: %d  ", x->synthParams.iSamplingRate);
        if(x->synthParams.iSynthesisType == SMS_STYPE_ALL) 
                post("synthesis type: all ");
        else if(x->synthParams.iSynthesisType == SMS_STYPE_DET) 
                post("synthesis type: deterministic only ");
        else if(x->synthParams.iSynthesisType == SMS_STYPE_STOC) 
                post("synthesis type: stochastic only ");
        if(x->synthParams.iDetSynthType == SMS_DET_IFFT) 
                post("deteministic synthesis method: ifft ");
        else if(x->synthParams.iDetSynthType == SMS_DET_SIN) 
                post("deteministic synthesis method: oscillator bank ");
        post("sizeHop: %d ", x->synthParams.sizeHop);
        post("__header info__");
/*         post("fOriginalSRate: %d, iFrameRate: %d, origSizeHop: %d", */
/*              x->pSmsHeader->iOriginalSRate, x->pSmsHeader->iFrameRate, x->synthParams.origSizeHop); */
/*         post("original file length: %f seconds ", (float)  x->pSmsHeader->nFrames * */
/*              x->synthParams.origSizeHop / x->pSmsHeader->iOriginalSRate ); */



}

static void smssynth_dsp(t_smssynth *x, t_signal **sp)
{
        x->synthParams.iSamplingRate =  sp[0]->s_sr;//todo: check if changed and recompute
        //need x and 2 vectors (in/out), and lastly the vector size:
        dsp_add(smssynth_perform, 4, x,  sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void *smssynth_new(t_symbol *bufname)
{
        t_smssynth *x = (t_smssynth *)pd_new(smssynth_class);
        outlet_new(&x->x_obj, gensym("signal"));
        
        x->bufname = bufname;

        x->smsbuf = NULL;

        x->synthParams.iSynthesisType = SMS_STYPE_ALL;
        x->synthParams.iDetSynthType = SMS_DET_IFFT;
        x->synthParams.sizeHop = x->synthBufPos = 512;

        x->synthParams.iSamplingRate = 44100; //should be updated once audio is turned on
        
        sms_init();
    
        x->synthBuf = (t_float *) calloc(x->synthParams.sizeHop, sizeof(t_float));
        
        return (x);
}

static void smssynth_free(t_smssynth *x)
{
        if(x->smsbuf->nframes != 0) 
        {
                sms_freeSynth(&x->synthParams);
                sms_freeRecord(&x->interpolatedRecord);
        }
        //sms_free();
}
void smssynth_tilde_setup(void)
{
        smssynth_class = class_new(gensym("smssynth~"), (t_newmethod)smssynth_new, 
                                       (t_method)smssynth_free, sizeof(t_smssynth), 0, A_DEFSYM, 0);
        CLASS_MAINSIGNALIN(smssynth_class, t_smssynth, f);
        class_addmethod(smssynth_class, (t_method)smssynth_dsp, gensym("dsp"), 0);
        class_addmethod(smssynth_class, (t_method)smssynth_buffer, gensym("buffer"), A_DEFSYM, 0);
        class_addmethod(smssynth_class, (t_method)smssynth_info, gensym("info"),  0);
        class_addmethod(smssynth_class, (t_method)smssynth_sizehop, gensym("sizeHop"), A_DEFFLOAT, 0);
        class_addmethod(smssynth_class, (t_method)smssynth_transpose, gensym("transpose"), A_DEFFLOAT, 0);
        class_addmethod(smssynth_class, (t_method)smssynth_stocgain, gensym("stocgain"), A_DEFFLOAT, 0);
        
}

/* ------------------------ helper functions ----------------------------- */

//method for opening file in canvas directory.
t_symbol* getFullPathName( t_symbol *infilename,  t_canvas *smsCanvas)
{
    char nameout[MAXPDSTRING], namebuf[MAXPDSTRING];
    char dirresult[MAXPDSTRING], *nameresult;
    char *dirname;
    char *sym = infilename->s_name;    
    dirname = canvas_getdir(smsCanvas)->s_name;

    if((open_via_path(dirname, sym,"", dirresult, &nameresult, MAXPDSTRING, 0)) < 0) 
            return(NULL);

    namebuf[0] = 0;
    if (*dirresult) //append directory
    {
            strcat(namebuf, dirresult);
            strcat(namebuf, "/");
    }
    strcat(namebuf, nameresult); //append name
    sys_bashfilename(namebuf, nameout);    // cross-platformize slashes
    
    if (0)
    {
            post("(open_via_path):");
            post("dirname: %s", dirname);
            post("filename->s_name: %s", sym);
            post("dirresult: %s", dirresult);
            post("nameresult: %s", nameresult);
            post("namebuf: %s", namebuf);
            post("nameout: %s ", nameout);
    }
    return(gensym( nameout ));
} 

/*this function seems to not be copying everything... or else it would be usable
  before copying the buffer from file (it crashes if it is) */
void CopySmsHeader( SMS_Header *pFileHeader, SMS_Header *pBufHeader, char *paramString  )
{
        sms_initHeader (pBufHeader);

        pBufHeader->nFrames = pFileHeader->nFrames;
        pBufHeader->iFormat = pFileHeader->iFormat;
        pBufHeader->iFrameRate = pFileHeader->iFrameRate;
        pBufHeader->iStochasticType = pFileHeader->iStochasticType;
        pBufHeader->nTrajectories = pFileHeader->nTrajectories;
        pBufHeader->nStochasticCoeff = pFileHeader->nStochasticCoeff;
        pBufHeader->iOriginalSRate = pFileHeader->iOriginalSRate;
   
        pBufHeader->iRecordBSize = sms_recordSizeB(pBufHeader);

        pBufHeader->nTextCharacters = pFileHeader->nTextCharacters;
        strcpy(paramString, pFileHeader->pChTextCharacters);
        pBufHeader->pChTextCharacters = paramString;
        
}
