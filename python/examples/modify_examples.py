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

# This file shows 3 sms modification examples:
# 1. Morphing between sounds by applying the spectral envelope of one to the other
# 2. Transposition without maintaining the spectral envelope
# 3. Transposition maintaining the original spectral envelope

from scipy import asarray, int16
from scipy.io.wavfile import write
from pysms import (zeros, analyze, synthesize, SMS_ENV_FBINS, 
                   SMS_ModifyParams, sms_modify, sms_initModify, sms_freeModify)
from time import time
from sys import exit

start_time = time()

# In the morph example, the spectral envelope from the source file will be
# applied to the target file.
# For all other examples, the source file will be modified.
source = "soopastar.wav"
target = "flute.wav"

# the maximum frequency of the highest partial detected (and of the spectral envelope)
max_freq = 12000

# The envelope_interp_factor only applies to the morph example
# if the envelope_interp_factor is 0, the original target envelope will be used (no morph)
# if it is 1, the original source envelope will be used
# any value in between will result in a linear interpolation between the two envelopes
envelope_interp_factor = 1

# In the transposition examples, transpose by this number of semitones
transposition = 4

# ----------------------------------------------------------------------------------------
# Impose a spectral envelope from one sound onto another set of sinudoidal tracks
# note: mod_params needs to be inititialized with sms_initModify to allocate the envelope array
# (not necessary in transpose examples)

# Analyze files
source_frames, source_sms_header, source_snd_header = analyze(source, env_type=SMS_ENV_FBINS, env_order=80)
target_frames, target_sms_header, target_snd_header = analyze(target, env_type=SMS_ENV_FBINS, env_order=80)

source_num_tracks = source_sms_header.nTracks
num_tracks = target_sms_header.nTracks
num_frames = min(len(source_frames), len(target_frames))

if num_tracks != source_num_tracks:
    print "Error: sound sources have a different number of tracks"
    exit()

# Set modification parameters
mod_params = SMS_ModifyParams()
sms_initModify(source_sms_header, mod_params)
mod_params.doSinEnv = True 
mod_params.sinEnvInterp = envelope_interp_factor

source_env_mags = zeros(mod_params.sizeSinEnv)

for frame_number in range(num_frames):
    source_frame = source_frames[frame_number]
    target_frame = target_frames[frame_number]
    
    # get the source envelope and put in the mod_params
    source_frame.getSinEnv(source_env_mags)
    mod_params.setSinEnv(source_env_mags)
    # call modifications
    sms_modify(target_frame, mod_params)

# change the output number of frames to the minimum of the two frame counts
target_frames = target_frames[0:num_frames]
target_sms_header.nFrames = num_frames

# Synthesis
morph = synthesize(target_frames, target_sms_header)

# convert audio to int values
morph *= 32767
morph *= 0.25 # soopastar sample clips so make output quieter
morph = asarray(morph, int16)

# write output files
write("modify_example_morph.wav", target_snd_header.iSamplingRate, morph)
print "wrote modify_example_morph.wav"

# ----------------------------------------------------------------------------------------
# Transpose without maintaining envelope  

# Set modification parameters
mod_params = SMS_ModifyParams()
mod_params.doTranspose = True
mod_params.transpose = transposition 

for frame_number in range(len(source_frames)):
    source_frame = source_frames[frame_number]
    sms_modify(source_frame, mod_params)

# Synthesis
transpose = synthesize(source_frames, source_sms_header)

# convert audio to int values
transpose /= transpose.max() # normalize max gain to 1 (soopastar clips)
transpose *= 32767
transpose = asarray(transpose, int16)

# write output files
write("modify_example_transpose.wav", source_snd_header.iSamplingRate, transpose)
print "wrote modify_example_transpose.wav"

# ----------------------------------------------------------------------------------------
# Transpose maintaining spectral envelope

# Have to analyze source again for now, should really be a way to copy/clone frames
source_frames, source_sms_header, source_snd_header = analyze(source, env_type=SMS_ENV_FBINS, env_order=80)

# Set modification parameters
mod_params = SMS_ModifyParams()
mod_params.doSinEnv = True
mod_params.doTranspose = True
mod_params.transpose = transposition 
mod_params.maxFreq = max_freq

for frame_number in range(len(source_frames)):
    source_frame = source_frames[frame_number]
    sms_modify(source_frame, mod_params)

# Synthesis
transpose_with_env = synthesize(source_frames, source_sms_header)

# convert audio to int values
transpose_with_env /= transpose_with_env.max() # normalize max gain to 1 (soopastar clips)
transpose_with_env *= 32767
transpose_with_env = asarray(transpose_with_env, int16)

# write output files
write("modify_example_transpose_with_env.wav", source_snd_header.iSamplingRate, transpose_with_env)
print "wrote modify_example_transpose_with_env.wav"

# ----------------------------------------------------------------------------------------
print "Running time: ", int(time() - start_time), "seconds."
