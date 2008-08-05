#include "m_pd.h"
#include "sms.h"
#include "smspd.h"

/* ------------------------ smsanal ----------------------------- */

static t_class *smsanal_class;

typedef struct _smsanal
{
        t_object x_obj; 
        t_canvas *canvas;
        t_symbol *s_filename;
        t_int i_frame, i_frameSource, synthBufPos;
        t_int nframes;
        t_float *synthBuf;
        t_float f;
	FILE *pSmsFile; 
        SYNTH_PARAMS synthParams;
//        t_smsBuffer smsBuf;
        t_outlet *outlet_ptr;
        t_atom a_ptr;
} t_smsanal;

static void smsanal_sizehop(t_smsanal *x, t_float *fSizeHop)
{
        //what is minimum hopsize?
        post("TODO: set sizeHop and re-init");
}

static void smsanal_source(t_smsanal *x, t_float f_source)
{
        if(f_source > .0001)
        {
//                x->i_frameSource = SOURCE_SIGNAL;
                post("smsanal_source: frame source set to signal");
        }
        else
        {
//                x->i_frameSource = SOURCE_FLOAT;
                post("smsanal_source: frame source set to float");
        }
}

/* open function:
 * 1. get fullname in a system independant manner
 * 2. read file header
 * 3. initialize the synthesizer based on header/synth params
 * 4. because we are synthesizing from file, allocate space for sms frames
 */
static void smsanal_open(t_smsanal *x, t_symbol *filename)
{
        long iError;
        t_symbol *fullname;
//        SMSHeader *pHeader = (SMSHeader *)x->smsBuf.pSmsHeader;
        x->s_filename = gensym(filename->s_name);
        fullname = getFullPathName(filename, x->canvas);

//        printf("smsheader address: %p ", &x->pSmsBuf->pSmsHeader);

        if(fullname == NULL)
        {
                pd_error(x, "smsanal_open: cannot find file: %s", filename->s_name);
                return;
        }
        else post("file: %s", fullname->s_name);
        //check if a file has been opened, close and init if necessary
        if(x->nframes != 0)
        {
                post("smsanal_open: re-initializing");
                SmsFreeSynth(&x->synthParams);                
/*                 sms_freeRecord(&x->smsRecordL); */
/*                 sms_freeRecord(&x->smsRecordR); */
/*                 sms_freeRecord(&x->newSmsRecord); */
        }

/*         if ((iError = sms_getHeader (fullname->s_name, &x->smsBuf.pSmsHeader, &x->pSmsFile)) < 0) */
/* //        if ((iError = sms_getHeader (fullname->s_name, &pHeader, &x->pSmsFile)) < 0) */
/* 	{ */
/*                 pd_error(x, "smsanal_open: %s", sms_errorString(iError)); */
/*                 return; */
/*         } */
        //post("smsheader address: %p ", x->smsBuf.pSmsHeader);
 

        //SmsInitSynth( x->smsBuf.pSmsHeader, &x->synthParams );
        
        /* allocate memory for nframes of SMS_DATA */

//        x->nframes = x->smsBuf.pSmsHeader->nRecords;
        post("nframes: %d ", x->nframes);
        /*Buffer the entire file in smsBuf.  For now, I'm doing this the simplest way possible.*/        
        // will this be faster with a malloc? try once everything is setup */
//        x->smsBuf.pSmsData = calloc(x->nframes, sizeof(SMS_DATA));
        int i;
        for( i = 0; i < x->nframes; i++ )
        {
//                sms_allocRecordH (x->smsBuf.pSmsHeader,  &x->smsBuf.pSmsData[i]);
//                sms_getRecord (x->pSmsFile, x->smsBuf.pSmsHeader, i, &x->smsBuf.pSmsData[i]);
        }

        //x->gp.gp_stub = (t_gstub*)&x->smsBuf.pSmsData; 
        //outlet_pointer(x->outlet_ptr, &x->gp);

//        post("nRecords: %d ", x->pSmsHeader->nRecords);//x->nframes);
        post("sms file buffered: %s ", filename->s_name );
        return;
}

static void smsanal_info(t_smsanal *x)
{
        // RTE TODO: once this pointer here works, post all header info, plus length of orig sample
//        SMSHeader *pHeader = x->smsBuf.pSmsHeader;

        post("sms file : %s ", x->s_filename->s_name );
//        post("original file length: %f seconds ", (float)  pHeader->nRecords *
//             x->synthParams.origSizeHop / pHeader->iOriginalSRate );
        post("__header info__");
//        post("fOriginalSRate: %d, iFrameRate: %d, origSizeHop: %d",
//             pHeader->iOriginalSRate, pHeader->iFrameRate, x->synthParams.origSizeHop);



}

/* static void smsanal_dsp(t_smsanal *x, t_signal **sp) */
/* { */
/*         x->synthParams.iSamplingRate =  sp[0]->s_sr;//todo: check if changed and recompute */
/*         //need x and 2 vectors (in/out), and lastly the vector size: */
/*         dsp_add(smsanal_perform, 4, x,  sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n); */
/* } */

static void *smsanal_new(void)
{
        t_smsanal *x = (t_smsanal *)pd_new(smsanal_class);
//        x->outlet_ptr = outlet_new(&x->x_obj, gensym("atom"));
        //needs to replaced with outlet_anything
        x->canvas = canvas_getcurrent();
        x->s_filename = NULL;
        x->nframes= 0;

        x->synthParams.iSynthesisType = STYPE_ALL;
        x->synthParams.iDetSynthType = DET_IFFT;
        x->synthParams.sizeHop = x->synthBufPos = 512;

        x->synthParams.iSamplingRate = 44100; //should be updated once audio is turned on
        
        SmsInit();
    
        x->synthBuf = (t_float *) calloc(x->synthParams.sizeHop, sizeof(t_float));

        
        return (x);
}

static void smsanal_free(t_smsanal *x)
{
        int i;
/*         if(x->smsBuf.pSmsHeader != NULL)  */
/*         { */
/*                 //SmsFreeSynth(&x->synthParams); */
/*                 for( i = 0; i < x->nframes; i++) */
/*                         sms_freeRecord(&x->smsBuf.pSmsData[i]); */
/*         } */
}
void smsanal_setup(void)
{
        smsanal_class = class_new(gensym("smsanal"), (t_newmethod)smsanal_new, 
                                       (t_method)smsanal_free, sizeof(t_smsanal), 0, A_DEFFLOAT, 0);
//        CLASS_MAINSIGNALIN(smsanal_class, t_smsanal, f);
//        class_addmethod(smsanal_class, (t_method)smsanal_dsp, gensym("dsp"), 0);
        class_addmethod(smsanal_class, (t_method)smsanal_open, gensym("open"), A_DEFSYM, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_info, gensym("info"),  0);
        class_addmethod(smsanal_class, (t_method)smsanal_sizehop, gensym("sizeHop"), A_DEFFLOAT, 0);
        class_addmethod(smsanal_class, (t_method)smsanal_source, gensym("source"), A_DEFFLOAT, 0);
        
}
