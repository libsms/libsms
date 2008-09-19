#ifndef smspd_h
#define smspd_h

#include "m_pd.h"

t_class *smsbuf_class;

typedef struct _smsbuf
{
        t_object x_obj;
        t_canvas *canvas;
        t_symbol *filename;
        t_symbol *bufname;
        int nframes;
        int ready;
        int allocated;
	FILE *pSmsFile;
        char param_string[1024];
        SMS_Header smsHeader;
        SMS_Data *smsData;
} t_smsbuf;

/* -_-_-_-_-_-_- objects in the smspd library -_-_-_-_-_-_-_- */
void smsbuf_setup(void);
void smsanal_setup(void);
void smssynth_tilde_setup(void);
//void smsedit_setup(void);

//method for opening file in canvas directory.
t_symbol* getFullPathName( t_symbol *infilename,  t_canvas *smsCanvas); 

// copy one sms header (such as from a file) to another ( the buffer).
void CopySmsHeader( SMS_Header *pFileHeader, SMS_Header *pBufHeader, char *paramString  );

#endif// smspd_h
