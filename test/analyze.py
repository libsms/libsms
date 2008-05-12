#!/usr/bin/env python

# smsAnal wrapper so I don't have to keep typing in
# command line arguments

import os

#infile ='/home/r/samples/instrumental/horn/Flugel/Flug-d5.aiff'

#smsfile = 'flugalD5.sms '

infile = 'audio/piano.aiff '
smsfile = 'piano.sms '










arg = '-r400 ' # framerate of analysis in hertz
arg += '-u65 '  # defaukt fundamental  in hertz
arg += '-e0 ' # stochastic representation type (default: 2, line segments)


makeYaml = True
makeSynth = True
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


if makeSynth:
    # make combined synth
    synthfile  = os.path.splitext(os.path.basename(smsfile))[0]+'Synth.wav ' 
    arg = ' '
    smsSynth = '../tools/smsSynth'
    toSynth = smsSynth + arg + smsfile + synthfile
    print toSynth
    os.system(toSynth)

    # make deterministic synth
    synthfile  = os.path.splitext(os.path.basename(smsfile))[0]+'Det.wav ' 
    arg = ' -d '
    toSynth = smsSynth + arg + smsfile + synthfile
    print toSynth
    os.system(toSynth)

    # make stochastic synth
    synthfile  = os.path.splitext(os.path.basename(smsfile))[0]+'Stoc.wav ' 
    arg = ' -n '
    toSynth = smsSynth + arg + smsfile + synthfile
    print toSynth
    os.system(toSynth)


print 'analyze.py is finished.'
