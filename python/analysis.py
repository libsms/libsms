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

def analyze(audio_file, frame_rate=300, window_size=1001, window_type=SMS_WIN_HAMMING, num_stoc_coeffs=128, default_fundamental=100, highest_freq=12000, env_type=0, env_order=0):
    sms_init()
    snd_header = SMS_SndHeader()
    sms_header = SMS_Header()
    analysis_params = SMS_AnalParams()

    sms_header.nStochasticCoeff = num_stoc_coeffs
    analysis_params.fDefaultFundamental = default_fundamental
    analysis_params.fHighestFreq = highest_freq 
    analysis_params.specEnvParams.iType = env_type 
    analysis_params.specEnvParams.iOrder = env_order

    window = zeros(window_size)
    sms_getWindow(window, window_type)

    # Try to open the input file
    if(sms_openSF(audio_file, snd_header)):
        raise NameError("error opening sound file: " + sms_errorString())

    sms_initAnalysis(analysis_params, snd_header)
    sms_fillHeader(sms_header, analysis_params, "analysis_synthesis.py")

    analysis_frames = []
    num_samples = snd_header.nSamples
    num_frames = 0
    sample_offset = 0
    size_new_data = 0
    do_analysis = True

    # Analysis loop
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

    sms_freeAnalysis(analysis_params)
    sms_free()
    return analysis_frames, sms_header, snd_header

