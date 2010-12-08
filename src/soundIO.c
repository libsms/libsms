/* 
 * Copyright (c) 2008 MUSIC TECHNOLOGY GROUP (MTG)
 *                         UNIVERSITAT POMPEU FABRA 
 * 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */
/*! \file soundIO.c
 * \brief soundfile input and output.
 */
#include "sms.h"

SNDFILE *pSNDStream, *pOutputSNDStream, *pResidualSNDStream;
SF_INFO sfSoundHeader, sfResidualHeader, sfOutputSoundHeader;

/*! \brief open a sound file and check its header
 *
 * Defualt channel to read is 1. If the user wishes to
 * read from a different channel when calling sms_getSound,
 * they need to set SMS_SndHeader->iReadChannel to the
 * desired channel number.
 *
 * \param pChInputSoundFile    name of soundfile
 * \param pSoundHeader    information of the sound
 * \return 0 on success, -1 on failure
 */
int sms_openSF(char *pChInputSoundFile, SMS_SndHeader *pSoundHeader)
{
    memset(&sfSoundHeader, 0, sizeof (sfSoundHeader));

    if(!(pSNDStream = sf_open(pChInputSoundFile, SFM_READ, &sfSoundHeader)))
    {
        sms_error("cannot open soundfile");  
        return -1;
    }

    pSoundHeader->channelCount = sfSoundHeader.channels;
    pSoundHeader->iReadChannel = 0;
    pSoundHeader->iSamplingRate = sfSoundHeader.samplerate;
    pSoundHeader->nSamples = sfSoundHeader.frames; 
    pSoundHeader->sizeHeader = 0; 
    return 0;
}

/*! \brief close a sound file that was open for reading
*/
void sms_closeSF()
{
    sf_close(pSNDStream);
}

/*! \brief get a chunk of sound from input file 
 *
 * This function will copy to samples from
 * the channel specified by SMS_SndHeader->iReadChannel to an array,
 * which is by default the first channel.
 *
 * \param pSoundHeader       sound header information to hold extracted information
 * \param sizeSound               number of samples read
 * \param pSound             buffer for samples read
 * \param offset                      which sound frame to start reading from
 * \return 0 on success, -1 on failure
 */
int sms_getSound(SMS_SndHeader *pSoundHeader, long sizeSound, sfloat *pSound, 
                 long offset, SMS_AnalParams *pAnalParams) 
{
    int nFrames;
    int i;
    int iChannelCount = pSoundHeader->channelCount;
    int iReadChannel = pSoundHeader->iReadChannel;

    if(sf_seek(pSNDStream, offset, SEEK_SET) < 0)
    {
        sms_error ("failure trying to seek to the sound location (sf_seek)");
        return -1;
    }

    nFrames = sf_readf_float(pSNDStream, pAnalParams->inputBuffer, sizeSound);
    if(nFrames != sizeSound)
    {
        sms_error("could not read the requested number of frames");
        return (-1);
    }

    /* now need to sift through interleaved frames to build one channel
       of samples -- this should work even with one channel*/
    for(i = 0; i < nFrames; i++)
        pSound[i] = pAnalParams->inputBuffer[i*iChannelCount +iReadChannel];

    return 0;
}

/*! \brief function to create an output sound file 
 *
 * \param pChOutputSoundFile   name of output file
 * \param iSamplingRate  sampling rate of synthesis
 * \param iType output file format (0 is wav, 1 is aiff, or other is libsndfile specific)
 * \return 0 on success, -1 on failure
 */
int sms_createSF(char *pChOutputSoundFile, int iSamplingRate, int iType)
{
    memset (&sfOutputSoundHeader, 0, sizeof (sfOutputSoundHeader)) ;

    if(iType == 1)
        sfOutputSoundHeader.format = SF_FORMAT_AIFF | SF_FORMAT_FLOAT;
    else if(iType == 0)
        sfOutputSoundHeader.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    else if(iType == 2)
        sfOutputSoundHeader.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    else sfOutputSoundHeader.format = iType;
    sfOutputSoundHeader.samplerate = iSamplingRate;
    sfOutputSoundHeader.channels = 1;

    if( !(pOutputSNDStream = sf_open (pChOutputSoundFile, SFM_WRITE, &sfOutputSoundHeader)))
    {
        sms_error("cannot open file for writing (sf_open).");
        return -1;
    }
    return 0;
}

/*! \brief write to the sound file data 
 *
 * \param pFBuffer    data to write to file
 * \param sizeBuffer     size of data buffer
 */
void sms_writeSound (sfloat *pFBuffer, int sizeBuffer)
{
    sf_writef_float(pOutputSNDStream, pFBuffer, sizeBuffer);
}

/*! \brief function to write the output sound file to disk */
void sms_writeSF()
{
    sf_close(pOutputSNDStream);
}

/*! \brief fill the sound buffer
 *
 * \param sizeWaveform        size of input data
 * \param pWaveform           input data
 * \param pAnalParams        pointer to structure of analysis parameters
 */
void sms_fillSoundBuffer(int sizeWaveform, sfloat *pWaveform, SMS_AnalParams *pAnalParams)
{
    int i;
    long sizeNewData = (long)sizeWaveform;

    /* leave space for new data */
    memcpy(pAnalParams->soundBuffer.pFBuffer, pAnalParams->soundBuffer.pFBuffer+sizeNewData, 
           sizeof(sfloat) * (pAnalParams->soundBuffer.sizeBuffer - sizeNewData));

    pAnalParams->soundBuffer.iFirstGood = 
        MAX(0, pAnalParams->soundBuffer.iFirstGood - sizeNewData);
    pAnalParams->soundBuffer.iMarker += sizeNewData;   

    /* put the new data in, and do some pre-emphasis */
    if(pAnalParams->iAnalysisDirection == SMS_DIR_REV)
        for(i=0; i<sizeNewData; i++)
            pAnalParams->soundBuffer.pFBuffer[pAnalParams->soundBuffer.sizeBuffer - sizeNewData + i] = 
                sms_preEmphasis(pWaveform[sizeNewData - (1+ i)], pAnalParams);
    else
        for(i=0; i<sizeNewData; i++)
            pAnalParams->soundBuffer.pFBuffer[pAnalParams->soundBuffer.sizeBuffer - sizeNewData + i] = 
                sms_preEmphasis(pWaveform[i], pAnalParams);
}

