#include "sms.h"
#include "smspd.h"
#include <string.h>

static t_class *smspd_class;

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

        post("smspd external library - September 16, 2008");

}

/* -_-_-_-_-_-_-_-_-_-_-helper functions -_-_-_-_-_-_-_-_-_-_- */

/* method for opening file in canvas directory. */
t_symbol* getFullPathName( t_symbol *infilename,  t_canvas *canvas)
{
    char nameout[MAXPDSTRING], namebuf[MAXPDSTRING];
    char dirresult[MAXPDSTRING], *nameresult;
    char *dirname;
    char *sym = infilename->s_name;    
    dirname = canvas_getdir(canvas)->s_name;

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
        pBufHeader->nTracks = pFileHeader->nTracks;
        pBufHeader->nStochasticCoeff = pFileHeader->nStochasticCoeff;
        pBufHeader->iOriginalSRate = pFileHeader->iOriginalSRate;
   
        pBufHeader->iRecordBSize = sms_recordSizeB(pBufHeader);

        pBufHeader->nTextCharacters = pFileHeader->nTextCharacters;
        strcpy(paramString, pFileHeader->pChTextCharacters);
        pBufHeader->pChTextCharacters = paramString;
        
}

/* ------------------------ smsbuf ----------------------------- */

/* probably will need these to make sure dellocation of buffer only happens once */
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
                sms_allocRecordH (&x->smsHeader,  &x->smsData[i]);
}

/* function to open a file and load into the buffer:
 * 1. get fullname in a system independant manner
 * 2. read file header
 * 3. initialize the synthesizer based on header/synth params
 * 4. because we are synthesizing from file, allocate space for sms frames
 */
static void smsbuf_open(t_smsbuf *x, t_symbol *filename)
{
        SMS_Header *pSmsHeader; /* is this necessary, or can I just copy the header
                                   to my smsbuf with sms_getHeader? */
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

        if ((iError = sms_getHeader (fullname->s_name, &pSmsHeader, &x->pSmsFile)) > 0)
//        if ((iError = sms_getHeader (fullname->s_name, &pHeader, &x->pSmsFile)) < 0)
	{
                pd_error(x, "smsbuf_open: %s", sms_errorString(iError));
                return;
        }
        //post("smsheader address: %p ", x->smsBuf.pSmsHeader);


        /* allocate memory for nframes of SMS_Data */
        x->nframes = pSmsHeader->nFrames;
        if(1)post("nframes: %d ", x->nframes);
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

static void smsbuf_save(t_smsbuf *x, t_symbol *filename)
{
        if(!x->ready)
        {
                pd_error(x, "smsbuf_save: buffer is not ready");
                return;
        }
        
        int i;
        int iError = 0;
        FILE *pOutputSmsFile;
        //todo: make local write possible (to current directory) - is it by default with fopen?
        /* open file for writing */
        iError = sms_writeHeader(filename->s_name, &x->smsHeader, &pOutputSmsFile);
        if(iError < 0) 
        {
                pd_error(x, "smsbuf_save: %s", sms_errorString(iError));
                return;
        }
        /* write the frames */
        for(i = 0; i < x->nframes; i++)
                sms_writeRecord (pOutputSmsFile, &x->smsHeader, &x->smsData[i]);

        /* save and close file */
        sms_writeFile (pOutputSmsFile, &x->smsHeader);
        post("smsbuf: wrote %d frames from buffer %s to file %s ", x->nframes, x->bufname->s_name, filename->s_name);
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
                post("Number of tracks = %d", x->smsHeader.nTracks);
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
                if(x->smsHeader.iStochasticType == SMS_STOC_IFFT) post("Stochastic type = IFFT");
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
                for(i = 0; i < x->smsHeader.nTracks; i++)
                                       if(x->smsData[frame].pFSinMag[i] > 0.00000001 )
                                               post("harmonic %d : %f[%f]", i, x->smsData[frame].pFSinFreq[i],
                                                    x->smsData[frame].pFSinMag[i]);
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
        class_addmethod(smsbuf_class, (t_method)smsbuf_save, gensym("save"), A_DEFSYM, 0);
        class_addmethod(smsbuf_class, (t_method)smsbuf_info, gensym("info"),  0);
        class_addmethod(smsbuf_class, (t_method)smsbuf_printframe, gensym("printframe"), A_DEFFLOAT, 0);
}

