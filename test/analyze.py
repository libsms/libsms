#!/usr/bin/env python

# smsAnal wrapper so I don't have to keep typing in
# command line arguments

import os

#infile ='/home/r/samples/instrumental/horn/Flugel/Flug-d5.aiff'
infile = 'audio/piano.aiff '
#smsfile = 'flugalD5.sms '
smsfile = 'piano.sms '
makeYaml = True

framerate = 400 #analysis windows / sec (hz). default: 400
arg = '-r100 '
#arg = '-r'+framerate+' ' doesn't work like this in python.. 
arg += '-e3 ' # stochastic representation type (default: 2, line segments)

smsAnal = '../tools/smsAnal '
analysis = smsAnal + arg + infile + smsfile
print analysis
os.system(analysis)

if makeYaml:
    yamlfile  = os.path.splitext(os.path.basename(smsfile))[0]+'.yaml ' 
    arg =  ' ' #-t2 '
    smsToYaml = '../tools/smsToYaml '
    toyaml = smsToYaml + arg + smsfile + yamlfile
    print toyaml
    os.system(toyaml)


