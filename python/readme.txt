-- commands to build sms python wrappers -- 

swig -python sms.i
python setup.py build_ext --inplace

-- toread: http://www.swig.org/Doc1.3/Python.html#Python
