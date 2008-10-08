#include "sms.h"
#include "smspd.h"

/* ------------------------ smsedit ----------------------------- */

static t_class *smsedit_class;

typedef struct _smsedit
{
        t_object x_obj; 
        t_smsbuf *smsbuf;
        int verbose;
} t_smsedit;

static void smsedit_buffer(t_smsedit *x, t_symbol *bufname)
{

/*         x->smsbuf = */
/*         (t_smsbuf *)pd_findbyclass(bufname, smsbuf_class); */

/*         if(!x->smsbuf) */
/*         { */
/*                 pd_error(x, "smsanal: %s was not found", bufname->s_name); */
/*                 return; */
/*         } */
/*         else post("smsanal is using buffer: %s", bufname->s_name); */
        x->smsbuf = smspd_buffer(bufname);
        if(!x->smsbuf)
                pd_error(x, "smsedit_new: smsbuf %s was not found", bufname->s_name);
        else if(x->verbose)post("smsedit is using buffer: %s", bufname->s_name);
}

/* just using this function to check if I have the right pointer to data  */
static void smsedit_printframe(t_smsedit *x, float f)
{
        if(!x->smsbuf)
        {
                pd_error(x, "smsedit: no pointer to an smsbuf");
                return;
        }
        if(!x->smsbuf->ready)
        {
                pd_error(x, "smsedit: smsbuf is not ready");
                return;
        }

        if(f >= x->smsbuf->nframes || f < 0) return;
               
        int i;
        int frame = (int) f;
                
        post("----- smsbuf (%s):: frame: %d, timetag: %f -----", x->smsbuf->bufname->s_name, frame, f / x->smsbuf->smsHeader.iFrameRate);
        for(i = 0; i < x->smsbuf->smsHeader.nTracks; i++)
                if(x->smsbuf->smsData[frame].pFSinMag[i] > 0.00000001 )
                        post("harmonic %d : %f[%f]", i, x->smsbuf->smsData[frame].pFSinFreq[i],
                             x->smsbuf->smsData[frame].pFSinMag[i]);

}

/* creator function */
static void *smsedit_new(t_symbol *s, int argcount, t_atom *argvec)
{
        t_smsedit *x = (t_smsedit *)pd_new(smsedit_class);

        int i;
        t_symbol *bufname;

        x->verbose = 1;

        for (i = 0; i < argcount; i++)
        {
                if (argvec[i].a_type == A_SYMBOL)
                {
                        bufname = argvec[i].a_w.w_symbol;
                        x->smsbuf = smspd_buffer(bufname);
                        if(!x->smsbuf)
                                pd_error(x, "smsedit_new: smsbuf %s was not found", bufname->s_name);
                }
        }

        return (x);


}

static void smsedit_free(t_smsedit *x)
{

}

void smsedit_setup(void)
{
        smsedit_class = class_new(gensym("smsedit"), (t_newmethod)smsedit_new, 
                                       (t_method)smsedit_free, sizeof(t_smsedit), 0, A_GIMME, 0);

        class_addmethod(smsedit_class, (t_method)smsedit_buffer, gensym("buffer"), A_DEFSYM, 0);
        class_addmethod(smsedit_class, (t_method)smsedit_printframe, gensym("printframe"), A_DEFFLOAT, 0);

}
