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

/*
 * soundIO.c: Functions for soundfile input and output.
 *
 * January 2008 - now using libsndfile for Linux/Mac/Windows compatibility - rte
 */

#include "sms.h"

SNDFILE *pSNDStream, *pOutputSNDStream, *pResidualSNDStream;
SF_INFO sfSoundHeader, sfResidualHeader, sfOutputSoundHeader;
const char *pChResidualFile = "residual.aiff";

/*  open a sound file and check its header
 *
 * char *pChInputSoundFile;    name of soundfile
 * SMS_SndHeader *pSoundHeader;    information of the sound
 */
int sms_openSF (char *pChInputSoundFile, SMS_SndHeader *pSoundHeader)
{
    memset (&sfSoundHeader, 0, sizeof (sfSoundHeader)) ;
    // read sound file
    if( !(pSNDStream = sf_open (pChInputSoundFile, SFM_READ, &sfSoundHeader)))
    {
        printf("sms_openSF: can't open %s \n", pChInputSoundFile);  
        exit(1);
    }
    // Abort if multi-channel sound
    if (sfSoundHeader.channels > 1)
    {
	  fprintf (stderr,"sms_openSF: multi-channel soundfile  unsupported\n");
	  exit(1);
    }
   pSoundHeader->channelCount = sfSoundHeader.channels;
   pSoundHeader->iSamplingRate = sfSoundHeader.samplerate;
   pSoundHeader->nSamples = sfSoundHeader.frames; 
   pSoundHeader->sizeHeader = 0; 
   return (1);
}

/* get a chunk of sound from input file 
 *
 * SMS_SndHeader *pSoundHeader;       sound header information
 * float *pSoundData;             buffer for samples read
 * short sizeSound;               number of samples read
 *
 */
int sms_getSound (SMS_SndHeader *pSoundHeader, float *pSoundData, long sizeSound,
                  long offset) 
{
	long nSamples;

	if (sf_seek(pSNDStream,  offset , SEEK_SET) < 0)
	{
		fprintf (stderr,"sms_getSound: could not seek to the sound location %ld\n", 
		         offset);
		return (-1);
	}
	if ((nSamples = (long) sf_readf_float( pSNDStream, pSoundData, sizeSound ))
	    != sizeSound)
	{
	    fprintf (stderr,"sms_getSound: could not read %ld samples, it read %ld\n", 
			 sizeSound, nSamples);
	    return (-1);
	}
/*         ///// RTE DEBUG */
/*         int i; */
/*         static float max = 0.; */
        
/*         //printf("\nsms_getSound: size %d \n", (int) sizeSound); */
/*         for( i = 0; i < (int) sizeSound; i++) */
/*         { */
/*                 //printf("[%d] %f, ", i, pSoundData[i]); */
/*                 if(pSoundData[i] < max) */
/*                 { */
/*                         max = pSoundData[i]; */
/*                         printf("\n New Min: %f", max); */
/*                 } */
/*         }  */
/*         ///////////////////////////////// */
	return (1);
}

/* function to create the residual sound file 
 *
 *  SMS_SynthParams synthParams;   synthesis paramenters
 *  char *pChOutputSoundFile;   name of output file
 *
 */
int sms_createSF (SMS_SynthParams synthParams, char *pChOutputSoundFile)
{
    memset (&sfOutputSoundHeader, 0, sizeof (sfOutputSoundHeader)) ;
   
    sfOutputSoundHeader.format = SF_FORMAT_AIFF | SF_FORMAT_FLOAT;
    sfOutputSoundHeader.samplerate = synthParams.iSamplingRate;
    sfOutputSoundHeader.channels = 1;
    
    if( !(pOutputSNDStream = sf_open (pChOutputSoundFile, SFM_WRITE, &sfOutputSoundHeader)))
    {
        printf("CreatOutputSoundFile: can't open %s for writing.\n", pChOutputSoundFile);
        exit(1); //TODO: add return (-1)  makes parent function print error statement
    }
    return 1;
}

/* function to write to the residual sound file 
 *
 * float *pFBuffer;    data to write to residual file
 * int sizeBuffer;     size of data buffer
 *
 */
int sms_writeSound (float *pFBuffer, int sizeBuffer)
{
    sf_writef_float( pOutputSNDStream, pFBuffer, sizeBuffer);
    return 1;
}

/* function to write the output sound file to disk */
int sms_writeSF ()
{
    //   sf_writef_short( pOutputSNDStream, pSBuffer, sizeBuffer);
    sf_close(pOutputSNDStream);
    return 1;		  
}

/* function to create the residual sound file 
 *
 *  SMS_AnalParams *pAnalParams;   analysis paramenters
 *
 */
int sms_createResSF (SMS_AnalParams *pAnalParams)
{
    memset (&sfResidualHeader, 0, sizeof (sfResidualHeader)) ;
    sfResidualHeader.format = SF_FORMAT_AIFF | SF_FORMAT_PCM_16;
    sfResidualHeader.samplerate = pAnalParams->iSamplingRate;
    sfResidualHeader.channels = 1;

    int err = sf_format_check (&sfResidualHeader);
    if(err == 0)
    {
        printf("sms_createResSF: invalid file format.");
        exit(1);
    }
    if( !(pResidualSNDStream = sf_open (pChResidualFile, SFM_WRITE, &sfResidualHeader)))
    {
        err = sf_error(pResidualSNDStream);
        printf("sms_createResSF: can't open %s for writing; SNDFILE ERR: %d, %s.\n", pChResidualFile, err,  sf_error_number(err));  
        exit(1);
    }
 
    return 1;
}

/* function to write to the residual sound file 
 *
 * float *pFBuffer;    data to write to residual file
 * int sizeBuffer;     size of data buffer
 *
 */
int sms_writeResSound (float *pBuffer, int sizeBuffer)
{
	int i;
	short *pSResidual;

	if ((pSResidual = (short *) calloc(sizeBuffer, sizeof(short))) == NULL)
		return -1;

	for (i = 0; i < sizeBuffer; i++)
		pSResidual[i] = (short) sms_deEmphasis (pBuffer[i]);

                sf_writef_float( pResidualSNDStream, pBuffer, sizeBuffer);
	
	free (pSResidual);
	return 1;
}

/* function to write the residual sound file to disk */
int sms_writeResSF ()
{
    sf_close(pOutputSNDStream);
    return 1;		  
}

/* fill the sound buffer
 *
 * float *pWaveform           input data
 * int sizeNewData             size of input data
 * int sizeHop                 analysis hop size
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
/*         ///// RTE DEBUG */
/*         static float max = 0.; */
        
/*         //printf("\nsms_getSound: size %d \n", (int) sizeSound); */
/*         for( i = 0; i < (int) sizeNewData; i++) */
/*         { */
/*                 //printf("[%d] %f, ", i, pSoundData[i]); */
/*                 if(pAnalParams->soundBuffer.pFBuffer[i] < max) */
/*                 { */
/*                         max = pAnalParams->soundBuffer.pFBuffer[i]; */
/*                         printf("\n New Min: %f", max); */
/*                 } */
/*         }  */
/*         ///////////////////////////////// */

}

