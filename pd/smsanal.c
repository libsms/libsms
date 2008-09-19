#include "m_pd.h"
#include "sms.h"
#include "smspd.h"
#include <pthread.h>

/* ------------------------ smsanal ----------------------------- */

#define ANALYSIS_FROM_FILE 0
#define ANALYSIS_FROM_ARRAY 1

static t_class *smsanal_class;
//extern t_class *smsbuf_class;

typedef struct _smsanal
{
        t_object x_obj; 
        t_float f; /* dummy for signal inlet */
        t_canvas *canvas;
        t_symbol *filename;
        t_smsbuf *smsbuf;
        SMS_AnalParams anal_params;
        int ntrajectories;
        int iDoAnalysis;
        int iStatus;
        int iSample;
        int iNextSizeRead;
        int verbose;
        int analysisType; /* 0 for file, 1 for array */
        int iRecord;
        t_outlet *outlet_iRecord;
        float    *wavetable;	
        int      nSamples;
	short pSoundData[SMS_MAX_WINDOW]; //todo: change to float
	SMS_SndHeader soundHeader;
} t_smsanal;

static void smsanal_sizehop(t_smsanal *x, t_float fSizeHop)
{
        //what is minimum hopsize?
        post("TODO: set sizeHop and re-init");
}

static void smsanal_buffer(t_smsanal *x, t_symbol *bufname)
{
        // get the pointer to the desired buffer (todo: still have to set the buffer symbol name)
        x->smsbuf =
        (t_smsbuf *)pd_findbyclass(bufname, smsbuf_class);

        if(!x->smsbuf)
        {
                pd_error(x, "smsanal: %s was not found", bufname->s_name);
                return;
        }
        else post("smsanal is using buffer: %s", bufname->s_name);

}

void *smsanal_childthread(void *zz)
{
        t_smsanal *x = zz;
        int i;
        int sizeNewData = 0;
        /* loop for analysis */
        while(x->iDoAnalysis > 0)
        {
                //post("analyzing frame %d", iRecord);
                if (x->anal_params.iAnalysisDirection == SMS_DIR_REV)
                {
                        if ((x->iSample - x->iNextSizeRead) >= 0)
                                sizeNewData = x->iNextSizeRead;
                        else
                                sizeNewData = x->iSample;
                        x->iSample -= sizeNewData;
                }
                else
                {
                        x->iSample += sizeNewData;
                        if((x->iSample + x->iNextSizeRead) < x->nSamples)
                                sizeNewData = x->iNextSizeRead;
                        else
                                sizeNewData = x->nSamples - x->iSample;
                }
		/* get one frame of sound */
                if(x->analysisType == ANALYSIS_FROM_FILE)
                {
                        if (sms_getSound (&x->soundHeader, x->pSoundData, sizeNewData, x->iSample) < 0)
                        {
                                pd_error(x, "smsanal_sf: could not read sound record %d\n", x->iRecord);
                                pthread_exit(NULL);
                                break; //return
                        }
                }
                else
                {
                        /*until sms_analyze will act on floating point data, the sound chunk has to be converted to 16-bit integer
                          shorts */
                        for(i = 0; i < sizeNewData; i++)
                                x->pSoundData[i] = (short) (x->wavetable[x->iSample + i] * FLOAT_TO_SHORT);
                }
		/* perform analysis of one frame of sound */
		x->iStatus = sms_analyze (x->pSoundData, sizeNewData, &x->smsbuf->smsData[x->iRecord], 
                                          &x->anal_params, &x->iNextSizeRead);
                

		/* if there is an output SMS record, write it */
		if (x->iStatus == 1)
		{
                        outlet_float(x->outlet_iRecord, (float)++x->iRecord);
		}
		else if (x->iStatus == -1) /* done */
		{
			x->iDoAnalysis = 0;
			x->smsbuf->smsHeader.nFrames = x->iRecord;
		}

	}
       post(" smsanal: analyzed %d frames from soundfile.", x->iRecord);
       x->smsbuf->smsHeader.fResidualPerc = x->anal_params.fResidualPercentage / x->iRecord;
       x->smsbuf->ready = 1;

        pthread_exit(NULL);
}

static void smsanal_sf(t_smsanal *x, t_symbol *filename)
{
        if(!x->smsbuf)
        {
                pd_error(x, "smsanal_sf: set the buffer pointer before analysis");
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
        pthread_t childthread;

        x->filename = gensym(filename->s_name);
        fullname = getFullPathName(filename, x->canvas);

        if(fullname == NULL)
        {
                pd_error(x, "smsanal_open: cannot find file: %s", filename->s_name);
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
	sms_openSF (fullname->s_name , &x->soundHeader);
        x->nSamples = x->soundHeader.nSamples;

        x->anal_params.iSamplingRate = x->soundHeader.iSamplingRate;
        x->anal_params.iDefaultSizeWindow = 
                (int)((x->anal_params.iSamplingRate / x->anal_params.fDefaultFundamental) 
                      * x->anal_params.fSizeWindow / 2) * 2 + 1;
        /* define the hopsize for each record */
	x->anal_params.sizeHop = (int)(x->soundHeader.iSamplingRate / 
	                (float) x->anal_params.iFrameRate);

        /* define how many records*/
	x->smsbuf->nframes = 3 + x->soundHeader.nSamples / 
                (float) x->anal_params.sizeHop;
        x->anal_params.iSizeSound = x->soundHeader.nSamples;
        if(x->verbose)
        {
                post("smsanal: set default size window to %d", x->anal_params.iDefaultSizeWindow);
                post("smsanal: set sizeHop to %d", x->anal_params.sizeHop);
                post("smsanal: ready to analyze %d frames", x->smsbuf->nframes);
        }
        /* need to supply sms header information for incase the analysis 
           will be written to file (by smsbuf) */
	sms_fillHeader (&x->smsbuf->smsHeader, x->smsbuf->nframes,
                        &x->anal_params, x->soundHeader.iSamplingRate, 
                         x->ntrajectories);

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
                 x->ntrajectories, x->anal_params.fFreqDeviation, 
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

        x->smsbuf->allocated = 1; // ?? why is this necessary again?

       x->iNextSizeRead = (x->anal_params.iDefaultSizeWindow + 1) * 0.5;
       
       if (x->anal_params.iAnalysisDirection == SMS_DIR_REV)
               x->iSample = x->anal_params.iSizeSound;
       else   x->iSample = 0;
       x->analysisType = ANALYSIS_FROM_FILE;
       x->iRecord = 0;
       x->iStatus = 0;
       x->iDoAnalysis = 1;
       //smsanal_loop(x);
       pthread_create(&childthread, 0, smsanal_childthread, (void *)x);

       return;
}

static void smsanal_array(t_smsanal *x, t_symbol *arrayname, t_float samplerate)
{
        //post("cansvas_dspstate: %d", canvas_dspstate);

        if(!x->smsbuf)
        {
                pd_error(x, "smsanal_sf: set the buffer pointer before analysis");
                return;
        }
        x->smsbuf->ready = 0;

        int iNextSizeRead = 0;
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
        pthread_t childthread;

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
        else if (!garray_getfloatarray(a, &x->nSamples, &x->wavetable))
        {
                pd_error(x, "%s: bad template for tabread", arrayname->s_name);
                return;
        }
        else if(x->verbose) //table exists
                post("wavetablesize: %d, samplerate: %d", x->nSamples, (int) samplerate );

        if(!x->smsbuf)
        {
                pd_error(x, "smsanal_sf: set the buffer pointer before analysis");
                return;
        }

        x->anal_params.iSamplingRate = (int)samplerate;
        x->anal_params.iDefaultSizeWindow = 
                (int)((x->anal_params.iSamplingRate / x->anal_params.fDefaultFundamental) 
                      * x->anal_params.fSizeWindow / 2) * 2 + 1;

        /* define the hopsize for each record */
	x->anal_params.sizeHop = x->anal_params.iSamplingRate / x->anal_params.iFrameRate;

        /* define how many records*/
	x->smsbuf->nframes = 3 + x->nSamples / 
                (float) x->anal_params.sizeHop;
        x->anal_params.iSizeSound = x->nSamples;
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
                         x->ntrajectories);

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
                 x->ntrajectories, x->anal_params.fFreqDeviation, 
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

        x->iNextSizeRead = (x->anal_params.iDefaultSizeWindow + 1) * 0.5;
       
       if (x->anal_params.iAnalysisDirection == SMS_DIR_REV)
               x->iSample = x->anal_params.iSizeSound;
       else   x->iSample = 0;
       x->analysisType = ANALYSIS_FROM_ARRAY;
       x->iRecord = 0;
       x->iStatus = 0;
       x->iDoAnalysis = 1;
       //smsanal_loop(x);
       pthread_create(&childthread, 0, smsanal_childthread, (void *)x);

       return;

}

/* all the analysis parameters */
static void smsanal_debug(t_smsanal *x, t_float debugMode)
{
        //int dm = (int) debugMode;
        switch ((int)debugMode)
        {
        case SMS_DBG_NONE: post("debug mode: disabled");
                break;
        case SMS_DBG_INIT: post("debug mode: initialization functions");
                break;
        case SMS_DBG_PEAK_DET: post("debug mode: peak detection function");
                break;
        case SMS_DBG_HARM_DET: post("debug mode: harmonic detection function");
                break;
        case SMS_DBG_PEAK_CONT: post("debug mode: peak continuation function");
                break;
        case SMS_DBG_CLEAN_TRAJ: post("debug mode: clean trajectories function");
                break;
        case SMS_DBG_SINE_SYNTH: post("debug mode: sine synthesis function");
                break;
        case SMS_DBG_STOC_ANAL: post("debug mode: stochastic analysis function");
                break;
        case SMS_DBG_STOC_SYNTH: post("debug mode: stochastic synthesis function");
                break;
        case SMS_DBG_SMS_ANAL: post("debug mode: top level analysis function");
                break;
        case SMS_DBG_ALL: post("debug mode: everything");
                break;
        case SMS_DBG_RESIDUAL: post("debug mode: write residual to file");
                break;
        case SMS_DBG_SYNC: post("debug mode: write original, synthesis and residual to a text file ");
                break;
        default: return;
        }
        x->anal_params.iDebugMode = debugMode;
}

static void smsanal_format(t_smsanal *x, t_float f)
{
        int i = (int) f;
        if(i < 0 || i > 3) 
        {
                pd_error(x, "smsanal_format: has to be 0-3");
                return;
        }
        x->anal_params.iFormat = (int) f;
        if(x->verbose)
        {
                switch(x->anal_params.iFormat)
                {
                case SMS_FORMAT_H: post("smsanal: format set to harmonic");
                        break;
                case SMS_FORMAT_IH: post("smsanal: format set to inharmonic");
                        break;
                case SMS_FORMAT_HP: post("smsanal: format set to harmonic with phase");
                        break;
                case SMS_FORMAT_IHP: post("smsanal: format set to inharmonic with phase");
                        break;
                default: break;
                }
        }
}

static void smsanal_soundtype(t_smsanal *x, t_float f)
{
        int i = (int) f;
        if(i < 0 || i > 1) 
        {
                pd_error(x, "smsanal_soundtype: has to be 0 or 1");
                return;
        }
        x->anal_params.iSoundType = i;
        if(x->verbose)
        {
                switch(x->anal_params.iSoundType)
                {
                case SMS_SOUND_TYPE_MELODY:
                        post("smsanal: soundtype set to melody");
                        break;
                case SMS_SOUND_TYPE_NOTE:
                        post("smsanal: soundtype set to single note");
                        break;
                default: break;
                }
        }
}

static void smsanal_direction(t_smsanal *x, t_float f)
{
        int i = (int) f;
        if(i < 0 || i > 1) 
        {
                pd_error(x, "smsanal_direction: has to be 0 or 1");
                return;
        }
        x->anal_params.iAnalysisDirection = i;
        if(x->verbose)
        {
                switch(i)
                {
                case SMS_DIR_FWD:
                        post("smsanal: direction set to foward (left to right)");
                        break;
                case SMS_DIR_REV:
                        post("smsanal: direction set to reverse (right to left)");
                        break;
                default: break;
                }
        }
}

static void smsanal_windowsize(t_smsanal *x, t_float f)
{
        if(f < 1) 
        {
                pd_error(x, "smsanal_windowsize: cannot be less than 1 period");
                return;
        }
        if(x->verbose) post("smsanal: set the window size to %0.2f periods", f);
       x->anal_params.fSizeWindow = f;
}

static void smsanal_windowtype(t_smsanal *x, t_float f)
{
        int i = (int) f;
        if(i < 0 || i > 4)
        {
                pd_error(x, "smsanal_windowtype: invalid value for window type");
                return;
        }
        x->anal_params.iWindowType = i;
        if(x->verbose)
        {
                switch(i)
                {
                case SMS_WIN_HAMMING:
                        post("smsanal: window type set to hamming");
                        break;
                case SMS_WIN_BH_62:
                        post("smsanal: window type set to blackman-harris, 62dB cutoff");
                        break;
                case SMS_WIN_BH_70:
                        post("smsanal: window type set to blackman-harris, 70dB cutoff");
                        break;
                case SMS_WIN_BH_74:
                        post("smsanal: window type set to blackman-harris, 74dB cutoff");
                        break;
                case SMS_WIN_BH_92:
                        post("smsanal: window type set to blackman-harris, 92dB cutoff");
                        break;
                default: return;
                }
        }
}

static void smsanal_framerate(t_smsanal *x, t_float f)
{
        int i = (int) f;
        if(i < 1) 
        {
                pd_error(x, "smsanal_framerate: cannot be less than 1 hz");
                return;
        }
        if(x->verbose) post("smsanal: set the framerate to %d hz", i);
        x->anal_params.iFrameRate = i;
}

static void smsanal_highestfreq(t_smsanal *x, t_float f)
{
        if(f < 0) 
        {
                pd_error(x, "smsanal_highestfreq: invalid frequency");
                return;
        }
        if(x->verbose) post("smsanal: set the highest frequency to %0.2f periods", f);
        x->anal_params.fHighestFreq = f;
}

static void smsanal_minpeakmag(t_smsanal *x, t_float f)
{
        if(f < 0) 
        {
                pd_error(x, "smsanal_minpeakmag: invalid minimum peak magnitude");
                return;
        }
        if(x->verbose) post("smsanal: set the minimum peak magnitude to %0.2f periods", f);
        x->anal_params.fMinPeakMag = f;
}

static void smsanal_refharmonic(t_smsanal *x, t_float f)
{
        int i = (int) f;
        if(i < 1) 
        {
                post("smsanal_refharmonic: cannot be less than 1");
                return;
        }
        if(x->verbose) post("smsanal: set the reference harmonic to %d ", i);
        x->anal_params.iRefHarmonic = i;
}

static void smsanal_minrefharmmag(t_smsanal *x, t_float f)
{
        if(f < 0) 
        {
                pd_error(x, "smsanal_minrefharmmag: invalid minimum fundamental magnitude");
                return;
        }
        if(x->verbose) post("smsanal: set the minimum fundamental magnitude to %0.2f dB", f);
        x->anal_params.fMinRefHarmMag = f;
}

static void smsanal_refharmdifffrommax(t_smsanal *x, t_float f)
{
        if(f < 0) 
        {
                pd_error(x, "smsanal_refharmdifffrommax: Invalid maximum fundamental magnitude difference from maximum peak");
                return;
        }
        if(x->verbose) post("smsanal: set the minimum fundamental magnitude to %0.2f dB", f);
        x->anal_params.fRefHarmMagDiffFromMax = f;
}

static void smsanal_defaultfund(t_smsanal *x, t_float f)
{
        if(f < 1) 
        {
                post("smsanal_defaultfund: cannot be less than 1");
                return;
        }
        if(x->verbose) post("smsanal: set the default fundamental to %0.2f ", f);
        x->anal_params.fDefaultFundamental = f;
}

static void smsanal_lowestfund(t_smsanal *x, t_float f)
{
        if(f < 1) 
        {
                pd_error(x, "smsanal_lowestfund: cannot be less than 1");
                return;
        }
        if(x->verbose) post("smsanal: set the lowest fundamental to %0.2f ", f);
        x->anal_params.fLowestFundamental = f;
}

static void smsanal_highestfund(t_smsanal *x, t_float f)
{
        if(f < 1) 
        {
                pd_error(x, "smsanal_defaultfund: cannot be less than 1");
                return;
        }
        if(x->verbose) post("smsanal: set the highest fundamental to %0.2f ", f);
        x->anal_params.fHighestFundamental = f;
}

static void smsanal_nguides(t_smsanal *x, t_float f)
{
        int i = (int) f;
        if(i < 1) 
        {
                post("smsanal_nguides: cannot be less than 1");
                return;
        }
        if(x->verbose) post("smsanal: set the number of guides to %d ", i);
        x->anal_params.nGuides = i;
}

static void smsanal_ntrajectories(t_smsanal *x, t_float f)
{
        int i = (int) f;
        if(i < 1) 
        {
                pd_error(x, "smsanal_ntrajectories: cannot be less than 1");
                return;
        }
        if(x->verbose) post("smsanal: set the number of trajectories to %d ", i);
        x->ntrajectories = i;
}

static void smsanal_freqdeviation(t_smsanal *x, t_float f)
{
        if(f < 0) 
        {
                pd_error(x, "smsanal_freqdeviation: cannot be less than 0");
                return;
        }
        if(x->verbose) post("smsanal: set the frequency deviation to %0.2f ", f);
        x->anal_params.fFreqDeviation = f;
}

static void smsanal_peakcontribution(t_smsanal *x, t_float f)
{
        if(f < 0) 
        {
                pd_error(x, "smsanal_peakcontribution: cannot be less than 0");
                return;
        }
        if(x->verbose) post("smsanal: set the peak contribution to guide to %0.2f ", f);
        x->anal_params.fPeakContToGuide = f;
}

static void smsanal_fundcontribution(t_smsanal *x, t_float f)
{
        if(f < 0) 
        {
                pd_error(x, "smsanal_fundcontribution: cannot be less than 0");
                return;
        }
        if(x->verbose) post("smsanal: set the fundamental contribution to guide to %0.2f ", f);
        x->anal_params.fFundContToGuide = f;
}

static void smsanal_cleantraj(t_smsanal *x, t_float f)
{
        int i = (int) f;
        if(i < 0 || i > 1) 
        {
                pd_error(x, "smsanal_cleantraj: has to be 0 or 1");
                return;
        }
        if(x->verbose)
        {
                if(i) post("smsanal: clean trajectories is on");
                else post("smsanal: clean trajectories is off");
        }
        x->anal_params.iCleanTraj = i;
}

static void smsanal_mintrajlength(t_smsanal *x, t_float f)
{
        int i = (int) f;
        if(i < 1) 
        {
                pd_error(x, "smsanal_mintrajlength: cannot be less than 1");
                return;
        }
        if(x->verbose) post("smsanal: set the minimum trajectory length to %d ", i);
        x->anal_params.iMinTrajLength = i;
}

static void smsanal_maxsleepingtime(t_smsanal *x, t_float f)
{
        int i = (int) f;
        if(i < 1) 
        {
                pd_error(x, "smsanal_maxsleepingtime: cannot be less than 1");
                return;
        }
        if(x->verbose) post("smsanal: set the maximum sleeping time to %d ", i);
        x->anal_params.iMaxSleepingTime = i;
}

static void smsanal_stochastictype(t_smsanal *x, t_float f)
{
        int i = (int) f;
        if(i < 0 || i > 3)
        {
                pd_error(x, "smsanal_stochastictype: has to be between 0 - 3");
                return;
        }
        x->anal_params.iStochasticType = i;
        if(x->verbose)
        {
                switch(i)
                {
                case SMS_STOC_WAVE:
                        post("smsanal: stochastic type set to waveform");
                        break;
                case SMS_STOC_IFFT:
                        post("smsanal: stochastic type set to inverse-FFT (not currently used)");
                        x->anal_params.iStochasticType = 2;
                        break;
                case SMS_STOC_APPROX:
                        post("smsanal: stochastic type set to spectrum approximation");
                        break;
                case SMS_STOC_NONE:
                        post("smsanal: stochastic analysis disabled");
                        break;
                default: return;
                }
        }
}

static void smsanal_ncoefficients(t_smsanal *x, t_float f)
{
        int i = (int) f;
        if(i < 1) 
        {
                pd_error(x, "smsanal_ncoefficients: cannot be less than 1");
                return;
        }
        if(x->verbose) post("smsanal: set the maximum sleeping time to %d ", i);
        x->anal_params.nStochasticCoeff = i;
}

static t_int *smsanal_perform(t_int *w)
{
        t_smsanal *x = (t_smsanal *)(w[1]);
        t_sample *in = (t_float *)(w[2]);

        int n = (int)(w[3]);
        return(w+4);
}

/* dsp indicator */
static void smsanal_dsp(t_smsanal *x, t_signal **sp)
{
        dsp_add(smsanal_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

/* creator function */
static void *smsanal_new(t_symbol *s, int argcount, t_atom *argvec)
{
        t_smsanal *x = (t_smsanal *)pd_new(smsanal_class);

        int i;
        x->outlet_iRecord = outlet_new(&x->x_obj,  gensym("float"));

        x->canvas = canvas_getcurrent();
        x->ntrajectories = 30;
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

        CLASS_MAINSIGNALIN(smsanal_class, t_smsanal, f);
        class_addmethod(smsanal_class, (t_method)smsanal_dsp, gensym("dsp"), 0);

        class_addmethod(smsanal_class, (t_method)smsanal_buffer, gensym("buffer"), A_DEFSYM, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_sf, gensym("soundfile"), A_DEFSYM, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_array, gensym("array"), A_DEFSYM, A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_sizehop, gensym("sizehop"), A_DEFFLOAT, 0);
        /* analysis parameters */
        class_addmethod(smsanal_class, (t_method)smsanal_debug, gensym("debug"), A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_format, gensym("format"), A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_soundtype, gensym("soundtype"), A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_direction, gensym("direction"), A_DEFFLOAT, 0);
        /* stft parameters */
        class_addmethod(smsanal_class, (t_method)smsanal_windowsize, gensym("windowsize"), A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_windowtype, gensym("windowtype"), A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_framerate, gensym("framerate"), A_DEFFLOAT, 0);
        /* peak detection parameters -- need to update .pd file from here down */
        class_addmethod(smsanal_class, (t_method)smsanal_highestfreq, gensym("highestfreq"), A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_minpeakmag, gensym("minpeakmag"), A_DEFFLOAT, 0);
        /* harmonic detection parameters */
        class_addmethod(smsanal_class, (t_method)smsanal_refharmonic, gensym("refharmonic"), A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_minrefharmmag, gensym("minrefharmmag"), A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_refharmdifffrommax, gensym("refharmdifffrommax"), A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_defaultfund, gensym("defaultfund"), A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_lowestfund, gensym("lowestfund"), A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_highestfund, gensym("highestfund"), A_DEFFLOAT, 0);
        /* peak continuation parameters */
        class_addmethod(smsanal_class, (t_method)smsanal_nguides, gensym("nguides"), A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_ntrajectories, gensym("ntrajectories"), A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_freqdeviation, gensym("freqdeviation"), A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_peakcontribution, gensym("peakcontribution"), A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_fundcontribution, gensym("fundcontribution"), A_DEFFLOAT, 0);
        /* trajectory cleaning parameters */
        class_addmethod(smsanal_class, (t_method)smsanal_cleantraj, gensym("cleantraj"), A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_mintrajlength, gensym("mintrajlength"), A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_maxsleepingtime, gensym("maxsleepingtime"), A_DEFFLOAT, 0);
        /* stochastic analysis parameters */
        class_addmethod(smsanal_class, (t_method)smsanal_stochastictype, gensym("stochastictype"), A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_ncoefficients, gensym("ncoefficients"), A_DEFFLOAT, 0);
}

