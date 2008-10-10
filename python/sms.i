%module sms
%{
#define SWIG_FILE_WITH_INIT
#include "../src/sms.h"
int sms_init( void );  
%}

%include "../src/sms.h"
