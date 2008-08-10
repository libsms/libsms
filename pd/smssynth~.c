#include "m_pd.h"
#include "sms.h"
#include "smspd.h"
/* ------------------------ smsbuf ----------------------------- */

static t_class *smsbuf_class;

typedef struct _smsbuf
{
        t_object x_obj;
        t_canvas *canvas;
        t_symbol *filename;
        t_symbol *bufname;
        t_int nframes;
        t_int ready;
	FILE *pSmsFile; //does this need to be in the struct?
        char paramString[1024];
        SMS_Header smsHeader;
        SMS_Data *smsData;
} t_smsbuf;

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

/* open function:
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
        // will this be faster with one malloc? try once everything is setup */
        x->smsData = calloc(x->nframes, sizeof(SMS_Data));
        for( i = 0; i < x->nframes; i++ )
        {
                sms_allocRecordH (pSmsHeader,  &x->smsData[i]);
                sms_getRecord (x->pSmsFile, pSmsHeader, i, &x->smsData[i]);
        }

        /* copy header to buffer */
        CopySmsHeader( pSmsHeader, &x->smsHeader, x->paramString );

//        post("nFrames: %d ", x->pSmsHeader->nFrames);//x->nframes);
        x->ready = 1;
        post("sms file buffered: %s ", filename->s_name );
        return;
}

static void smsbuf_info(t_smsbuf *x)
{
        post("sms file : %s ", x->filename->s_name );
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

//        if(x->pSmsHeader != NULL) //shouldn't need this, if nframes = 0 nothing will happen
        {
                for( i = 0; i < x->nframes; i++)
                        sms_freeRecord(&x->smsData[i]);
        }
        pd_unbind(&x->x_obj.ob_pd, x->bufname);
}
void smsbuf_setup(void)
{
        smsbuf_class = class_new(gensym("smsbuf"), (t_newmethod)smsbuf_new, 
                                       (t_method)smsbuf_free, sizeof(t_smsbuf), 0, A_DEFSYM, 0);
        class_addmethod(smsbuf_class, (t_method)smsbuf_open, gensym("open"), A_DEFSYM, 0);
        class_addmethod(smsbuf_class, (t_method)smsbuf_info, gensym("info"),  0);
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
        t_float f;
        SMS_SynthParams synthParams;
        SMS_Header *pSmsHeader;
        SMS_Data *pSmsData;
        SMS_Data interpolatedRecord;
} t_smssynth;

static void smssynth_read(t_smssynth *x, t_symbol *bufname)
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
        t_smsbuf *smsbuf =
        (t_smsbuf *)pd_findbyclass(x->bufname, smsbuf_class);

        if(!smsbuf)
        {
                error("smsbuf: %s was not found", x->bufname->s_name);
                return;
        }
        if(!smsbuf->ready)
        {
                post("smsbuf not ready");
                return;
        }
        else post("smsbuf IS REAAAAADDDDYY");
 
        //check if a file has been opened, if so re-init
        if(x->pSmsHeader != NULL)
        {
                post("smssynth_open: re-initializing");
                sms_freeSynth(&x->synthParams);                
                sms_freeRecord(&x->interpolatedRecord);
        }
        x->pSmsHeader = &smsbuf->smsHeader;
        x->pSmsData = smsbuf->smsData;
        sms_initSynth( x->pSmsHeader, &x->synthParams );
        
	/* setup for interpolated synthesis from buffer */
        // I guess I am always ignoring phase information for now..
	sms_allocRecord (&x->interpolatedRecord, x->pSmsHeader->nTrajectories, 
	                   x->pSmsHeader->nStochasticCoeff, 0,
                           x->synthParams.origSizeHop, x->pSmsHeader->iStochasticType);

        post("nrecords: %d", x->pSmsHeader->nFrames);
}

static t_int *smssynth_perform(t_int *w)
{
        t_smssynth *x = (t_smssynth *)(w[1]);
        t_sample *in = (t_float *)(w[2]);
        t_sample *out = (t_float *)(w[3]);
        int n = (int)(w[4]);
        float f;
        int i, iLeftRecord, iRightRecord;

        if(x->pSmsHeader != NULL)        
        {
                if(x->synthBufPos >= x->synthParams.sizeHop)
                {
                        if(x->f >= x->pSmsHeader->nFrames)
                                x->f = x->pSmsHeader->nFrames -1;
                        if(x->f < 0) x->f = 0;
                
                        iLeftRecord = MIN (x->pSmsHeader->nFrames - 1, floor (x->f)); 
                        iRightRecord = (iLeftRecord < x->pSmsHeader->nFrames - 2)
                                ? (1+ iLeftRecord) : iLeftRecord;

                        sms_interpolateRecords (&x->pSmsData[iLeftRecord], &x->pSmsData[iRightRecord],
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

static void smssynth_source(t_smssynth *x, t_float f_source)
{
        if(f_source > .0001)
        {
                x->i_frameSource = SOURCE_SIGNAL;
                post("smssynth_source: frame source set to signal");
        }
        else
        {
                x->i_frameSource = SOURCE_FLOAT;
                post("smssynth_source: frame source set to float");
        }
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
        post("fOriginalSRate: %d, iFrameRate: %d, origSizeHop: %d",
             x->pSmsHeader->iOriginalSRate, x->pSmsHeader->iFrameRate, x->synthParams.origSizeHop);
        post("original file length: %f seconds ", (float)  x->pSmsHeader->nFrames *
             x->synthParams.origSizeHop / x->pSmsHeader->iOriginalSRate );



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

        x->pSmsHeader = NULL;
        x->i_frameSource = SOURCE_FLOAT;

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
        if(x->pSmsHeader != NULL) 
        {
                sms_freeSynth(&x->synthParams);
//                sms_freeRecord(&x->smsRecordL);
//                sms_freeRecord(&x->smsRecordR);
                sms_freeRecord(&x->interpolatedRecord);
        }
        sms_free();
}
void smssynth_tilde_setup(void)
{
        smssynth_class = class_new(gensym("smssynth~"), (t_newmethod)smssynth_new, 
                                       (t_method)smssynth_free, sizeof(t_smssynth), 0, A_DEFSYM, 0);
        CLASS_MAINSIGNALIN(smssynth_class, t_smssynth, f);
        class_addmethod(smssynth_class, (t_method)smssynth_dsp, gensym("dsp"), 0);
        class_addmethod(smssynth_class, (t_method)smssynth_read, gensym("read"), A_DEFSYM, 0);
        class_addmethod(smssynth_class, (t_method)smssynth_info, gensym("info"),  0);
        class_addmethod(smssynth_class, (t_method)smssynth_sizehop, gensym("sizeHop"), A_DEFFLOAT, 0);
        class_addmethod(smssynth_class, (t_method)smssynth_source, gensym("source"), A_DEFFLOAT, 0);
        
}
