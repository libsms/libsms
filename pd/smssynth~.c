#include "m_pd.h"
#include "sms.h"
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ smssynth~ ----------------------------- */


#define SOURCE_FLOAT 1
#define SOURCE_SIGNAL 2

// helper functions




static t_class *smssynth_class;

typedef struct _smssynth
{
        t_object x_obj; 
        t_canvas *canvas;
        t_symbol *s_filename;
        t_int i_frame, i_frameSource, synthBufPos;
        t_float *synthBuf;
        t_float f;
	FILE *pSmsFile; 
        SYNTH_PARAMS synthParams;
        SMSHeader *pSmsHeader;
        SMS_DATA smsRecordL, smsRecordR, newSmsRecord;
} t_smssynth;

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
                        if(x->f >= x->pSmsHeader->nRecords)
                                x->f = x->pSmsHeader->nRecords -1;
                        if(x->f < 0) x->f = 0;
                
                        iLeftRecord = MIN (x->pSmsHeader->nRecords - 1, floor (x->f)); 
                        iRightRecord = (iLeftRecord < x->pSmsHeader->nRecords - 2)
                                ? (1+ iLeftRecord) : iLeftRecord;
                
                        GetSmsRecord (x->pSmsFile, x->pSmsHeader, iLeftRecord, &x->smsRecordL);
                        GetSmsRecord (x->pSmsFile, x->pSmsHeader, iRightRecord,&x->smsRecordR);
                        InterpolateSmsRecords (&x->smsRecordL, &x->smsRecordR, &x->newSmsRecord,
                                               x->f - iLeftRecord);
                
                        SmsSynthesis (&x->newSmsRecord, x->synthBuf, &x->synthParams);
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

/* open function:
** todo: just make this an init function, for when the buffer signals a change
 */
static void smssynth_open(t_smssynth *x, t_symbol *filename)
{
        long iError;
        t_symbol *fullname;

/*         x->s_filename = gensym(filename->s_name); */
/*         fullname = getFullPathName(filename, x->canvas); */
/*         if(fullname == NULL) */
/*         { */
                pd_error(x, "smssynth_open: cannot find file: %s", filename->s_name);
                return;
/*         } */
 
        //check if a file has been opened, close and init if necessary
        if(x->pSmsHeader != NULL)
        {
                post("smssynth_open: re-initializing");
                SmsFreeSynth(&x->synthParams);                
                FreeSmsRecord(&x->smsRecordL);
                FreeSmsRecord(&x->smsRecordR);
                FreeSmsRecord(&x->newSmsRecord);
        }
        
        if ((iError = GetSmsHeader (x->s_filename->s_name, &x->pSmsHeader, &x->pSmsFile)) < 0)
	{
                pd_error(x, "smssynth_open: %s", SmsReadErrorStr(iError));
                return;
        }

        SmsInitSynth( x->pSmsHeader, &x->synthParams );
        
	/* setup for synthesis from file */
        /* needs 3 frame buffers: left, right, and interpolated */
	AllocSmsRecord (x->pSmsHeader, &x->smsRecordL);
	AllocSmsRecord (x->pSmsHeader, &x->smsRecordR);
        // I guess I am always ignoring phase information for now..
	AllocateSmsRecord (&x->newSmsRecord, x->pSmsHeader->nTrajectories, 
	                   x->pSmsHeader->nStochasticCoeff, 0,
                           x->synthParams.origSizeHop, x->pSmsHeader->iStochasticType);

        post("sms file initialized: %s ", filename->s_name );

}

static void smssynth_info(t_smssynth *x)
{

        post("sms file : %s ", x->s_filename->s_name );
        post("__arguments__");
        post("samplingrate: %d  ", x->synthParams.iSamplingRate);
        if(x->synthParams.iSynthesisType == STYPE_ALL) 
                post("synthesis type: all ");
        else if(x->synthParams.iSynthesisType == STYPE_DET) 
                post("synthesis type: deterministic only ");
        else if(x->synthParams.iSynthesisType == STYPE_STOC) 
                post("synthesis type: stochastic only ");
        if(x->synthParams.iDetSynthType == DET_IFFT) 
                post("deteministic synthesis method: ifft ");
        else if(x->synthParams.iDetSynthType == DET_OSC) 
                post("deteministic synthesis method: oscillator bank ");
        post("sizeHop: %d ", x->synthParams.sizeHop);
        post("__header info__");
        post("fOriginalSRate: %d, iFrameRate: %d, origSizeHop: %d",
             x->pSmsHeader->iOriginalSRate, x->pSmsHeader->iFrameRate, x->synthParams.origSizeHop);
        post("original file length: %f seconds ", (float)  x->pSmsHeader->nRecords *
             x->synthParams.origSizeHop / x->pSmsHeader->iOriginalSRate );



}

static void smssynth_dsp(t_smssynth *x, t_signal **sp)
{
        x->synthParams.iSamplingRate =  sp[0]->s_sr;//todo: check if changed and recompute
        //need x and 2 vectors (in/out), and lastly the vector size:
        dsp_add(smssynth_perform, 4, x,  sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void *smssynth_new(void)
{
        t_smssynth *x = (t_smssynth *)pd_new(smssynth_class);
        outlet_new(&x->x_obj, gensym("signal"));
        
        x->canvas = canvas_getcurrent();
        x->s_filename = NULL;
        x->i_frameSource = SOURCE_FLOAT;

        x->synthParams.iSynthesisType = STYPE_ALL;
        x->synthParams.iDetSynthType = DET_IFFT;
        x->synthParams.sizeHop = x->synthBufPos = 512;

        x->synthParams.iSamplingRate = 44100; //should be updated once audio is turned on
        
        SmsInit();
    
        x->synthBuf = (t_float *) calloc(x->synthParams.sizeHop, sizeof(t_float));
        
        return (x);
}

static void smssynth_free(t_smssynth *x)
{
        if(x->pSmsHeader != NULL) 
        {
                SmsFreeSynth(&x->synthParams);
                FreeSmsRecord(&x->smsRecordL);
                FreeSmsRecord(&x->smsRecordR);
                FreeSmsRecord(&x->newSmsRecord);
        }
}
void smssynth_tilde_setup(void)
{
        smssynth_class = class_new(gensym("smssynth~"), (t_newmethod)smssynth_new, 
                                       (t_method)smssynth_free, sizeof(t_smssynth), 0, A_DEFFLOAT, 0);
        CLASS_MAINSIGNALIN(smssynth_class, t_smssynth, f);
        class_addmethod(smssynth_class, (t_method)smssynth_dsp, gensym("dsp"), 0);
        class_addmethod(smssynth_class, (t_method)smssynth_open, gensym("open"), A_DEFSYM, 0);
        class_addmethod(smssynth_class, (t_method)smssynth_info, gensym("info"),  0);
        class_addmethod(smssynth_class, (t_method)smssynth_sizehop, gensym("sizeHop"), A_DEFFLOAT, 0);
        class_addmethod(smssynth_class, (t_method)smssynth_source, gensym("source"), A_DEFFLOAT, 0);
        
}
