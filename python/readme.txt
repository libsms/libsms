#! /bin/bash
# script to build pysms python module

swig -python pysms.i &&
python setup.py build_ext --inplace

