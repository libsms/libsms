# Copyright (c) 2010 John Glover, National University of Ireland, Maynooth
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
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

from pysms import *
import numpy as np

def synthesize(frames, sms_header, synth_type=0, det_synth_type=SMS_DET_IFFT, hop_size=SMS_MIN_SIZE_FRAME): 
    sms_init() 
    interp_frame = SMS_Data() 
    synth_params = SMS_SynthParams() 
    synth_params.iSynthesisType = synth_type
    synth_params.iDetSynthType = det_synth_type 
    synth_params.sizeHop = hop_size 
    synth_params.iSamplingRate = 0

    sms_initSynth(sms_header, synth_params)
    sms_allocFrame(interp_frame, sms_header.nTracks, sms_header.nStochasticCoeff, 0, sms_header.iStochasticType, sms_header.nEnvCoeff)

    synth_samples = zeros(synth_params.sizeHop)
    num_synth_samples = 0
    target_synth_samples = sms_header.nFrames * synth_params.origSizeHop
    loc_incr = 1.0 / synth_params.origSizeHop
    audio_output = array([])

    # Synthesis loop
    while num_synth_samples < target_synth_samples:
        interp_factor = loc_incr * num_synth_samples
        left_frame_loc = int(min(sms_header.nFrames - 1, floor(interp_factor)))
        right_frame_loc = left_frame_loc + 1 if (left_frame_loc < sms_header.nFrames - 2) else left_frame_loc
        left_frame = frames[left_frame_loc]
        right_frame = frames[right_frame_loc]
        sms_interpolateFrames(left_frame, right_frame, interp_frame, interp_factor - left_frame_loc)
        sms_synthesize(interp_frame, synth_samples, synth_params)
        audio_output = hstack((audio_output, synth_samples))
        num_synth_samples += synth_params.sizeHop
          
    sms_freeFrame(interp_frame)
    sms_freeSynth(synth_params)
    sms_free()
    return audio_output

