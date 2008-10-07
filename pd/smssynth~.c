#include "m_pd.h"
#include "sms.h"
#include "smspd.h"

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
        t_float f; /* dummy for signal inlet */
        t_float transpose, stocgain;
        SMS_SynthParams synthParams;
        t_smsbuf *smsbuf;
        SMS_Data interpolatedFrame;
        t_int verbose;
        t_int ready;
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

        //check if a file has been opened, if so re-init
        if(x->ready)
        {
                post("smssynth_open: re-initializing synth");
                x->ready = 0;
                sms_freeSynth(&x->synthParams);
                sms_freeFrame(&x->interpolatedFrame);
        }

        x->smsbuf =
        (t_smsbuf *)pd_findbyclass(x->bufname, smsbuf_class);

        if(!x->smsbuf)
        {
                pd_error(x, "smssynth~: %s was not found", x->bufname->s_name);
                return;
        }
        if(!x->smsbuf->ready)
        {
                pd_error(x, "smsbuf not ready");
                return;
        }
        else if(x->verbose)
                post("smssynth: [smsbuf %s] was successfully found ", x->bufname->s_name);
 
        post("(BEFORE) smssynth: %d frames in buffer (before sms_initSynth)", x->smsbuf->nframes);
        post("smsbuf status: %d", x->smsbuf->ready);
        post("iFrameRate:(#1) %d ", x->smsbuf->smsHeader.iFrameRate);

        sms_initSynth( &x->smsbuf->smsHeader, &x->synthParams );

        // why is nFrames here reading as zero after loading a file in smsbuf??
        post("smssynth: %d frames in buffer (after sms_initSynth)", x->smsbuf->nframes);
        post("smsbuf status: %d", x->smsbuf->ready);
        post("iFrameRate:(#2) %d ", x->smsbuf->smsHeader.iFrameRate);

        x->smsbuf =
        (t_smsbuf *)pd_findbyclass(x->bufname, smsbuf_class);

        if(!x->smsbuf)
        {
                pd_error(x, "smssynth~: %s was not found", x->bufname->s_name);
                return;
        }
        if(!x->smsbuf->ready)
        {
                pd_error(x, "smsbuf not ready");
                return;
        }
        else if(x->verbose)
                post("smssynth: [smsbuf %s] was successfully found (again)", x->bufname->s_name);
        post("iFrameRate(#3): %d ", x->smsbuf->smsHeader.iFrameRate);

	/* setup for interpolated synthesis from buffer */
        // I guess I am always ignoring phase information for now..
	sms_allocFrame (&x->interpolatedFrame, x->smsbuf->smsHeader.nTracks,
	                   x->smsbuf->smsHeader.nStochasticCoeff, 0,
                           x->smsbuf->smsHeader.iStochasticType);

        x->ready = 1;
}

/* the signal in is not currently used; is there a benifit to control the synthesis by signal rate? */
static t_int *smssynth_perform(t_int *w)
{
        t_smssynth *x = (t_smssynth *)(w[1]);
        t_sample *in = (t_float *)(w[2]);
        t_sample *out = (t_float *)(w[3]);
        int n = (int)(w[4]);
        
        if(x->ready && x->smsbuf->ready)        
        {
                float f;
                int i, iLeftFrame, iRightFrame;
                //int nFrames = x->smsbuf->smsHeader.nFrames;
                int nFrames = x->smsbuf->nframes;
                if(x->synthBufPos >= x->synthParams.sizeHop)
                {
                        if(x->f >= nFrames)
                                x->f = nFrames -1;
                        if(x->f < 0) x->f = 0;
                
                        iLeftFrame = MIN (nFrames - 1, floor (x->f)); 
                        iRightFrame = (iLeftFrame < nFrames - 2)
                                ? (1+ iLeftFrame) : iLeftFrame;

                        sms_interpolateFrames (&x->smsbuf->smsData[iLeftFrame], &x->smsbuf->smsData[iRightFrame],
                                                &x->interpolatedFrame, x->f - iLeftFrame);
                        
                        sms_synthesize (&x->interpolatedFrame, x->synthBuf, &x->synthParams);
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
        if(x->verbose) post("transpose: %f", x->synthParams.fTranspose);
}

static void smssynth_stocgain(t_smssynth *x, t_float f)
{
/*         x->stocgain = f; */
        x->synthParams.fStocGain = f;
        if(x->verbose) post("stochastic gain: %fx", x->synthParams.fStocGain);
}

static void smssynth_synthtype(t_smssynth *x, t_float f)
{
        x->synthParams.iSynthesisType = (int) f;
        if(x->verbose)
        {
                switch (x->synthParams.iSynthesisType)
                {
                case SMS_STYPE_ALL:
                        post("synthesis type: %d, all", x->synthParams.iSynthesisType);
                        break;
                case SMS_STYPE_DET:
                        post("synthesis type: %d, only deterministic", x->synthParams.iSynthesisType);
                        break;
                case SMS_STYPE_STOC:
                        post("synthesis type: %d, only stochastic", x->synthParams.iSynthesisType);
                        break;
                }
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
/*         post("fOriginalSRate: %d, iFrameRate: %d, origSizeHop: %d", */
/*              x->pSmsHeader->iOriginalSRate, x->pSmsHeader->iFrameRate, x->synthParams.origSizeHop); */
/*         post("original file length: %f seconds ", (float)  x->pSmsHeader->nFrames * */
/*              x->synthParams.origSizeHop / x->pSmsHeader->iOriginalSRate ); */



}

static void smssynth_dsp(t_smssynth *x, t_signal **sp)
{
        x->synthParams.iSamplingRate =  sp[0]->s_sr;
        //need x and 2 vectors (in/out), and lastly the vector size:
        dsp_add(smssynth_perform, 4, x,  sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void *smssynth_new(t_symbol *bufname)
{
        t_smssynth *x = (t_smssynth *)pd_new(smssynth_class);
        outlet_new(&x->x_obj, gensym("signal"));
        
        x->bufname = bufname;
        x->verbose = 1;
        x->smsbuf = NULL;
        x->ready = 0;

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
                sms_freeFrame(&x->interpolatedFrame);
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
        class_addmethod(smssynth_class, (t_method)smssynth_synthtype, gensym("synthtype"), A_DEFFLOAT, 0);
        
}

