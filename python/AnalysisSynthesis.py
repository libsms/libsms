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

from scipy import asarray, int16
from scipy.io.wavfile import write
from optparse import OptionParser
import SMS

# Parse command line options
parser = OptionParser()
parser.add_option("-f", dest="output_file", default="analysis_syntheis.wav", help="Output file name")
parser.add_option("-r", dest="frame_rate", default=300, type="int", help="Frame rate")
parser.add_option("-c", dest="num_stoc_coeffs", default=128, type="int", help="Number of stochastic coefficients")
parser.add_option("-u", dest="default_fundamental", default=100, type="int", help="Default fundamental frequency")
parser.add_option("-w", dest="window_size", default=1001, type="int", help="Window Size")
parser.add_option("-i", dest="window_type", default=0, type="int", help="Window Type")
parser.add_option("-j", dest="highest_freq", default=12000, type="int", help="Highest Frequency Component")
parser.add_option("-e", dest="env_type", default=0, type="int", help="Envelope Type")
parser.add_option("-o", dest="env_order", default=0, type="int", help="Envelope Order")
parser.add_option("-s", dest="synth_type", default=0, type="int", help="Synthesis Type")
parser.add_option("-p", dest="synth_hop_size", default=128, type="int", help="Synthesis Hop Size")
parser.add_option("-d", dest="synth_method", default=0, type="int", help="Synthesis Method (IFFT/Oscillator Bank)")
parser.add_option("-v", dest="verbose", action="store_true", default=False, help="Display information")
(options, args) = parser.parse_args()
if len(args) != 1:
    print "Error: You must specify an input file, and can specify additional options if required."
    print "Run with the -h option for a list of options."
    exit()

# Analysis
analysis_frames, sms_header, snd_header = SMS.analyze(args[0], options.frame_rate, options.window_size, \
                                                      options.window_type, options.num_stoc_coeffs, \
						      options.default_fundamental, options.highest_freq, \
						      options.env_type, options.env_order)

# Print analysis info if verbose flag is set
if options.verbose:      
    print "Number of frames:", sms_header.nFrames
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
audio_output = SMS.synthesize(analysis_frames, sms_header, \
	                      options.synth_type, options.synth_method, options.synth_hop_size)

# convert audio to int values
audio_output *= 32767
audio_output = asarray(audio_output, int16)

# write output file
write(options.output_file, snd_header.iSamplingRate, audio_output)
