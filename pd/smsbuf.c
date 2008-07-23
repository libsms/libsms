#include "m_pd.h"
#include "sms.h"
#include "smspd.h"
#include "smsbuf.h"

/* ------------------------ smsbuf ----------------------------- */

static t_class *smsbuf_class;

/* typedef struct _smsbuf */
/* { */
/*         t_object x_obj;  */
/*         t_canvas *canvas; */
/*         t_symbol *s_filename; */
/*         t_symbol *bufname; */
/*         t_int nframes; */
/* 	FILE *pSmsFile;  */
/*         SMSHeader *pSmsHeader; */
/*         SMS_DATA *pSmsData; */
/* } t_smsbuf; */


/* open function:
 * 1. get fullname in a system independant manner
 * 2. read file header
 * 3. initialize the synthesizer based on header/synth params
 * 4. because we are synthesizing from file, allocate space for sms frames
 */
static void smsbuf_open(t_smsbuf *x, t_symbol *filename)
{
        long iError;
        int i;
        t_symbol *fullname;
        x->s_filename = gensym(filename->s_name);
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
                        FreeSmsRecord(&x->pSmsData[i]);
        }

        if ((iError = GetSmsHeader (fullname->s_name, &x->pSmsHeader, &x->pSmsFile)) < 0)
//        if ((iError = GetSmsHeader (fullname->s_name, &pHeader, &x->pSmsFile)) < 0)
	{
                pd_error(x, "smsbuf_open: %s", SmsReadErrorStr(iError));
                return;
        }
        //post("smsheader address: %p ", x->smsBuf.pSmsHeader);


        //SmsInitSynth( x->smsBuf.pSmsHeader, &x->synthParams );
        
        /* allocate memory for nframes of SMS_DATA */

        x->nframes = x->pSmsHeader->nRecords;
        post("nframes: %d ", x->nframes);
        /*Buffer the entire file in smsBuf.  For now, I'm doing this the simplest way possible.*/        
        // will this be faster with a malloc? try once everything is setup */
        x->pSmsData = calloc(x->nframes, sizeof(SMS_DATA));
        for( i = 0; i < x->nframes; i++ )
        {
                AllocSmsRecord (x->pSmsHeader,  &x->pSmsData[i]);
                GetSmsRecord (x->pSmsFile, x->pSmsHeader, i, &x->pSmsData[i]);
        }

        //x->gp.gp_stub = (t_gstub*)&x->smsBuf.pSmsData; 
        //outlet_pointer(x->outlet_ptr, &x->gp);

//        post("nRecords: %d ", x->pSmsHeader->nRecords);//x->nframes);
        x->ready = 1;
        post("sms file buffered: %s ", filename->s_name );
        return;
}

static void smsbuf_info(t_smsbuf *x)
{
        // RTE TODO: once this pointer here works, post all header info, plus length of orig sample
        SMSHeader *pHeader = x->pSmsHeader;

        post("sms file : %s ", x->s_filename->s_name );
//        post("original file length: %f seconds ", (float)  pHeader->nRecords *
//             x->synthParams.origSizeHop / pHeader->iOriginalSRate );
        post("__header info__");
/*         post("fOriginalSRate: %d, iFrameRate: %d, origSizeHop: %d", */
/*              pHeader->iOriginalSRate, pHeader->iFrameRate, x->synthParams.origSizeHop); */



}

/* static void smsbuf_dsp(t_smsbuf *x, t_signal **sp) */
/* { */
/*         x->synthParams.iSamplingRate =  sp[0]->s_sr;//todo: check if changed and recompute */
/*         //need x and 2 vectors (in/out), and lastly the vector size: */
/*         dsp_add(smsbuf_perform, 4, x,  sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n); */
/* } */

static void *smsbuf_new(t_symbol *bufname)
{
        t_smsbuf *x = (t_smsbuf *)pd_new(smsbuf_class);
//        x->outlet_ptr = outlet_new(&x->x_obj, gensym("atom"));

        x->canvas = canvas_getcurrent();
        x->s_filename = NULL;
        x->nframes= 0;
        x->ready= 0;

        //todo: make a default name if none is given:
        //if (!*s->s_name) s = gensym("delwrite~");
        // ?? do in need to check if bufname already exists? ??
        pd_bind(&x->x_obj.ob_pd, bufname);
        x->bufname = bufname;
        post("bufname: %s", bufname->s_name);

        SmsInit();
    
        return (x);
}

static void smsbuf_free(t_smsbuf *x)
{
        int i;
        if(x->pSmsHeader != NULL) 
        {
                for( i = 0; i < x->nframes; i++)
                        FreeSmsRecord(&x->pSmsData[i]);
        }
        pd_unbind(&x->x_obj.ob_pd, x->bufname);
}
void smsbuf_setup(void)
{
        smsbuf_class = class_new(gensym("smsbuf"), (t_newmethod)smsbuf_new, 
                                       (t_method)smsbuf_free, sizeof(t_smsbuf), 0, A_DEFSYM, 0);
//        CLASS_MAINSIGNALIN(smsbuf_class, t_smsbuf, f);
//        class_addmethod(smsbuf_class, (t_method)smsbuf_dsp, gensym("dsp"), 0);
        class_addmethod(smsbuf_class, (t_method)smsbuf_open, gensym("open"), A_DEFSYM, 0);
        class_addmethod(smsbuf_class, (t_method)smsbuf_info, gensym("info"),  0);
}
