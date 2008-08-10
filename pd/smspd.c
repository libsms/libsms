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

