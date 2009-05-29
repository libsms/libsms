# Copyright (c) 2009 John Glover, National University of Ireland, Maynooth
#  
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, M  02111-1307  USA

from pysms import *
from scipy import array, asarray, zeros, floor, hstack, int16, float32
from pylab import plot, show
from scipy.io.wavfile import read, write
from optparse import OptionParser

# Parse command line options
parser = OptionParser()
parser.add_option("-o", dest="output_file", default="analysis_syntheis.wav", help="Output file name")
parser.add_option("-r", dest="frame_rate", type="int", help="Frame rate")
parser.add_option("-c", dest="num_stoc_coeffs", type="int", help="Number of stochastic coefficients")
parser.add_option("-u", dest="default_fundamental", type="int", help="Default fundamental frequency")
parser.add_option("-s", dest="synthesis_type", default=0, type="int", help="Synthesis Type")
parser.add_option("-v", dest="verbose", action="store_true", default=False, help="Display information")
(options, args) = parser.parse_args()
if len(args) != 1:
    print "Error: You must specify an input file, and can specify additional options if required."
    print "Run with the -h option for a list of options."
    exit()

snd_header = SMS_SndHeader()
sms_header = SMS_Header()
data = SMS_Data()
analysis_params = SMS_AnalParams()

if options.num_stoc_coeffs:
    sms_header.nStochasticCoeff = options.num_stoc_coeffs
if options.default_fundamental:
    analysis_params.fDefaultFundamental = options.default_fundamental

window_size = 301
window = zeros(window_size).astype('float32')
sms_getWindow(window, SMS_WIN_HAMMING)

# Try to open the input file
if(sms_openSF(args[0], snd_header)):
    raise NameError("error opening sound file: " + sms_errorString())

sms_init()
sms_initAnalysis(analysis_params, snd_header)
sms_fillHeader(sms_header, analysis_params, "analysis_synthesis.py")
sms_allocFrameH(sms_header, data)

analysis_frames = []
num_samples = snd_header.nSamples
num_frames = 0
sample_offset = 0
size_new_data = 0
do_analysis = True

# Analysis
while do_analysis:
    sample_offset += size_new_data
    if((sample_offset + analysis_params.sizeNextRead) < num_samples):
        size_new_data = analysis_params.sizeNextRead
    else:
        size_new_data = num_samples - sample_offset
        
    frame = zeros(size_new_data).astype('float32')
    if sms_getSound(snd_header, frame, sample_offset):
        raise NameError("error opening sound file: " + sms_errorString())
    
    data = SMS_Data()
    sms_allocFrameH(sms_header, data)
    status = sms_analyze(frame, data, analysis_params)  
    if status == 1:
        analysis_frames.append(data)
        num_frames += 1
    elif status == -1:
       do_analysis = False
       sms_header.nFrames = num_frames

if options.verbose:      
    print "Total frames:", num_frames

interp_frame = SMS_Data()
synth_params = SMS_SynthParams() 
synth_params.iSynthesisType = options.synthesis_type
synth_params.iDetSynthType = SMS_DET_IFFT
synth_params.sizeHop = SMS_MIN_SIZE_FRAME
synth_params.iSamplingRate = 0

sms_initSynth(sms_header, synth_params)
sms_allocFrame(interp_frame, sms_header.nTracks, sms_header.nStochasticCoeff, 0, sms_header.iStochasticType, sms_header.nEnvCoeff)

synth_samples = zeros(synth_params.sizeHop).astype('float32')
num_synth_samples = 0
target_synth_samples = sms_header.nFrames * synth_params.origSizeHop
loc_incr = 1.0 / synth_params.origSizeHop
audio_output = array([], dtype=float32)

if options.verbose:
    print "Sampling Rate:", synth_params.iSamplingRate
    print "synthesis type:", "all" if synth_params.iSynthesisType == SMS_STYPE_ALL else "other"
    print "deteministic synthesis method:", "ifft" if synth_params.iDetSynthType == SMS_DET_IFFT else "other"
    print "sizeHop:", synth_params.sizeHop
    print "time factor:", 1
    print "stochastic gain factor:", synth_params.fStocGain
    print "frequency transpose factor:", synth_params.fTranspose
    print "original samplingrate:", sms_header.iSamplingRate, "iFrameRate:", sms_header.iFrameRate, "origSizeHop:", synth_params.origSizeHop
    print "original file length:", sms_header.nFrames / float(sms_header.iFrameRate)

# Synthesis
while num_synth_samples < target_synth_samples:
    interp_factor = loc_incr * num_synth_samples
    left_frame_loc = int(min(sms_header.nFrames - 1, floor(interp_factor)))
    right_frame_loc = left_frame_loc + 1 if (left_frame_loc < sms_header.nFrames - 2) else left_frame_loc
    left_frame = analysis_frames[left_frame_loc]
    right_frame = analysis_frames[right_frame_loc]

    sms_interpolateFrames(left_frame, right_frame, interp_frame, interp_factor - left_frame_loc)
    sms_synthesize(interp_frame, synth_samples, synth_params)
    audio_output = hstack((audio_output, synth_samples))
    num_synth_samples += synth_params.sizeHop
              
# Clean up
sms_freeAnalysis(analysis_params)
for frame in analysis_frames:
    sms_freeFrame(frame)
sms_freeFrame(interp_frame)
sms_freeSynth(synth_params)
sms_free()

# convert to int values
audio_output *= 32767
audio_output = asarray(audio_output, int16)

# write output file
write(options.output_file, snd_header.iSamplingRate, audio_output)
