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
const char *pChResidualFile = "residual.aiff";

/*! \brief open a sound file and check its header
 *
 * \param pChInputSoundFile    name of soundfile
 * \param pSoundHeader    information of the sound
 * \return error code \see SMS_SNDERR in SMS_ERRORS
 */
int sms_openSF (char *pChInputSoundFile, SMS_SndHeader *pSoundHeader)
{
    memset (&sfSoundHeader, 0, sizeof (sfSoundHeader)) ;

    if(!(pSNDStream = sf_open (pChInputSoundFile, SFM_READ, &sfSoundHeader)))
    {
        printf("sms_openSF: can't open %s \n", pChInputSoundFile);  
        return(SMS_SNDERR);
    }

    if (sfSoundHeader.channels > 1)
    {
	  printf ("sms_openSF: multi-channel soundfile  unsupported\n");
          return(SMS_SNDERR);
    }
   pSoundHeader->channelCount = sfSoundHeader.channels;
   pSoundHeader->iSamplingRate = sfSoundHeader.samplerate;
   pSoundHeader->nSamples = sfSoundHeader.frames; 
   pSoundHeader->sizeHeader = 0; 
   return (SMS_OK);
}

/*! \brief get a chunk of sound from input file 
 *
 * \param pSoundHeader       sound header information to hold extracted information
 * \param pSoundData             buffer for samples read
 * \param sizeSound               number of samples read
 * \param offset                      where to start reading in the file
 * \return error code \see SMS_SNDERR in SMS_ERRORS
 */
int sms_getSound (SMS_SndHeader *pSoundHeader, float *pSoundData, long sizeSound,
                  long offset) 
{
	int nSamples;
	if (sf_seek(pSNDStream,  offset , SEEK_SET) < 0)
	{
		printf ("sms_getSound: could not seek to the sound location %ld\n", 
		         offset);
		return (SMS_SNDERR);
	}
	if ((nSamples = sf_readf_float( pSNDStream, pSoundData, sizeSound ))
	    != sizeSound)
	{
	    printf ("sms_getSound: could not read %ld samples, it read %d\n", 
			 sizeSound, nSamples);
	    return (SMS_SNDERR);
	}
	return (SMS_OK);
}

/*! \brief function to create an output sound file 
 *
 * \param pChOutputSoundFile   name of output file
 * \param iSamplingRate  sampling rate of synthesis
 * \param iType output file format (0 is wav, 1 is aiff, or other is libsndfile specific)
 * \return error code \see SMS_SNDERR in SMS_ERRORS
 */
int sms_createSF ( char *pChOutputSoundFile, int iSamplingRate, int iType)
{
    memset (&sfOutputSoundHeader, 0, sizeof (sfOutputSoundHeader)) ;
   
    if(iType == 1)
            sfOutputSoundHeader.format = SF_FORMAT_AIFF | SF_FORMAT_FLOAT;
    else if(iType == 0)
            sfOutputSoundHeader.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    else sfOutputSoundHeader.format = iType; /* untested.. */
    sfOutputSoundHeader.samplerate = iSamplingRate;
    sfOutputSoundHeader.channels = 1;
    
    if( !(pOutputSNDStream = sf_open (pChOutputSoundFile, SFM_WRITE, &sfOutputSoundHeader)))
    {
        printf("CreatOutputSoundFile: can't open %s for writing.\n", pChOutputSoundFile);
        return(SMS_SNDERR);
    }
    return (SMS_OK);
}

/*! \brief write to the sound file data 
 *
 * \param pFBuffer    data to write to file
 * \param sizeBuffer     size of data buffer
 */
void sms_writeSound (float *pFBuffer, int sizeBuffer)
{
    sf_writef_float( pOutputSNDStream, pFBuffer, sizeBuffer);
}

/*! \brief function to write the output sound file to disk */
void sms_writeSF ()
{
    sf_close(pOutputSNDStream);
}

/*! \brief function to create the residual sound file 
 *
 *  \param iSamplingRate samplerate of sound file
 * \return error code \see SMS_SNDERR in SMS_ERRORS
 */
int sms_createResSF (int iSamplingRate)
{
    memset (&sfResidualHeader, 0, sizeof (sfResidualHeader)) ;
    sfResidualHeader.format = SF_FORMAT_AIFF | SF_FORMAT_FLOAT;
    sfResidualHeader.samplerate = iSamplingRate;
    sfResidualHeader.channels = 1;

    int err = sf_format_check (&sfResidualHeader);
    if(err == 0)
    {
        printf("sms_createResSF: invalid file format.");
        return(SMS_SNDERR);
    }
    if( !(pResidualSNDStream = sf_open (pChResidualFile, SFM_WRITE, &sfResidualHeader)))
    {
        err = sf_error(pResidualSNDStream);
        printf("sms_createResSF: can't open %s for writing; SNDFILE ERR: %d, %s.\n", pChResidualFile, err,  sf_error_number(err));  
        return(SMS_SNDERR);;
    }
    return (SMS_OK);
}

/*! \brief function to write to the residual sound file 
 *
 * \param pBuffer    data to write to residual file
 * \param sizeBuffer     size of data buffer
 * \return error code \see SMS_SNDERR in SMS_ERRORS
 */
int sms_writeResSound (float *pBuffer, int sizeBuffer)
{
	int i;
	float *pFResidual;

	if ((pFResidual = (float *) calloc(sizeBuffer, sizeof(float))) == NULL)
		return(SMS_MALLOC);

	for (i = 0; i < sizeBuffer; i++)
		pFResidual[i] = sms_deEmphasis (pBuffer[i]);

        sf_writef_float( pResidualSNDStream, pBuffer, sizeBuffer);
	
	free (pFResidual);
	return(SMS_OK);
}

/*! \brief write the residual sound file to disk */
void sms_writeResSF ()
{
    sf_close(pOutputSNDStream);
}

/*! \brief fill the sound buffer
 *
 * \param pWaveform           input data
 * \param sizeNewData        size of input data
 * \param pAnalParams        pointer to structure of analysis parameters
 */
void sms_fillSndBuffer (float *pWaveform, long sizeNewData, SMS_AnalParams *pAnalParams)
{
	int i;
  
	/* leave space for new data */
	memcpy ( pAnalParams->soundBuffer.pFBuffer,  pAnalParams->soundBuffer.pFBuffer+sizeNewData, 
                 sizeof(float) * (pAnalParams->soundBuffer.sizeBuffer - sizeNewData));
  
	pAnalParams->soundBuffer.iFirstGood = 
		MAX (0, pAnalParams->soundBuffer.iFirstGood - sizeNewData);
	pAnalParams->soundBuffer.iMarker += sizeNewData;   
  
	/* put the new data in, and do some pre-emphasis */
	if (pAnalParams->iAnalysisDirection == SMS_DIR_REV)
		for (i=0; i<sizeNewData; i++)
			pAnalParams->soundBuffer.pFBuffer[pAnalParams->soundBuffer.sizeBuffer - sizeNewData + i] = 
				sms_preEmphasis( pWaveform[sizeNewData - (1+ i)]);
	else
		for (i=0; i<sizeNewData; i++)
			pAnalParams->soundBuffer.pFBuffer[pAnalParams->soundBuffer.sizeBuffer - sizeNewData + i] = 
				sms_preEmphasis(pWaveform[i]);

}

