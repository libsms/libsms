%module (docstring="Python SWIG-wrapped module of libsms") sms
%{
#define SWIG_FILE_WITH_INIT
#include "../src/sms.h"
%}

%include "sms_doxy.i"
%include "../src/sms.h"
%include "numpy.i"
%init %{
import_array();


%}



 // below is without numpy
%include "carrays.i"
%array_class(int, intArray);
%array_class(float, floatArray);

%inline %{

        float getFloat(float *array, int index)
        {
                return array[index];
        }
        %}
