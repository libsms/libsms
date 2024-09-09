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

import numpy as np
try:
    from . import _pysms as pysms
except ModuleNotFoundError as e:
    import pysms

def synthesize(frames, sms_header, synth_type=pysms.SMS_STYPE_ALL,
               det_synth_type=pysms.SMS_DET_IFFT, 
               hop_size=pysms.SMS_MIN_SIZE_FRAME): 
    pysms.sms_init() 
    synth_params = pysms.SMS_SynthParams() 
    pysms.sms_initSynthParams(synth_params)
    synth_params.iSynthesisType = synth_type
    synth_params.iDetSynthType = det_synth_type 
    synth_params.sizeHop = hop_size 
    pysms.sms_initSynth(sms_header, synth_params)
    interp_frame = pysms.SMS_Data() 
    pysms.sms_allocFrameH(sms_header, interp_frame)

    synth_samples = np.zeros(synth_params.sizeHop, dtype=np.float32)
    num_synth_samples = 0
    target_synth_samples = len(frames) * synth_params.origSizeHop
    audio_output = np.array([], dtype=np.float32)
    loc_incr = 1.0 / synth_params.origSizeHop
    current_frame = 0

    # Synthesis loop
    while num_synth_samples < target_synth_samples:
        interp_factor = loc_incr * num_synth_samples
        left_frame_loc = int(min(sms_header.nFrames - 1, np.floor(interp_factor)))
        if(left_frame_loc < sms_header.nFrames - 2): 
            right_frame_loc = left_frame_loc + 1
        else: 
            right_frame_loc = left_frame_loc
        left_frame = frames[left_frame_loc]
        right_frame = frames[right_frame_loc]
        pysms.sms_interpolateFrames(left_frame, right_frame, interp_frame, interp_factor - left_frame_loc)
        pysms.sms_synthesize(interp_frame, synth_samples, synth_params)
        audio_output = np.hstack((audio_output, synth_samples))
        num_synth_samples += synth_params.sizeHop
          
    pysms.sms_freeFrame(interp_frame)
    pysms.sms_freeSynth(synth_params)
    pysms.sms_free()
    return audio_output

