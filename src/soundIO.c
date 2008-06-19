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
 * SNDHeader *pSoundHeader;    information of the sound
 */
int OpenSound (char *pChInputSoundFile, SNDHeader *pSoundHeader)
{
    memset (&sfSoundHeader, 0, sizeof (sfSoundHeader)) ;
    // read sound file
    if( !(pSNDStream = sf_open (pChInputSoundFile, SFM_READ, &sfSoundHeader)))
    {
        printf("OpenSound: can't open %s \n", pChInputSoundFile);  
        exit(1);
    }
    // Abort if multi-channel sound
    if (sfSoundHeader.channels > 1)
    {
	  fprintf (stderr,"OpenSound: multi-channel soundfile  unsupported\n");
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
 * SNDHeader *pSoundHeader;       sound header information
 * short *pSoundData;             buffer for samples read
 * short sizeSound;               number of samples read
 *
 */
int GetSoundData (SNDHeader *pSoundHeader, short *pSoundData, long sizeSound,
                  long offset) 
{
	long nSamples;

	if (sf_seek(pSNDStream,  offset , SEEK_SET) < 0)
	{
		fprintf (stderr,"GetSoundData: could not seek to the sound location %ld\n", 
		         offset);
		return (-1);
	}
	if ((nSamples = (long) sf_readf_short( pSNDStream, pSoundData, sizeSound ))
	    != sizeSound)
	{
	    fprintf (stderr,"GetSoundData: could not read %ld samples, it read %ld\n", 
			 sizeSound, nSamples);
	    return (-1);
	}
        
	return (1);
}

/* function to create the residual sound file 
 *
 *  SYNTH_PARAMS synthParams;   synthesis paramenters
 *  char *pChOutputSoundFile;   name of output file
 *
 */
int CreateOutputSoundFile (SYNTH_PARAMS synthParams, char *pChOutputSoundFile)
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
int WriteToOutputFile (float *pFBuffer, int sizeBuffer)
{
    sf_writef_float( pOutputSNDStream, pFBuffer, sizeBuffer);
    return 1;
}

/* function to write the output sound file to disk */
int WriteOutputFile ()
{
    //   sf_writef_short( pOutputSNDStream, pSBuffer, sizeBuffer);
    sf_close(pOutputSNDStream);
    return 1;		  
}

/* function to create the residual sound file 
 *
 *  ANAL_PARAMS *pAnalParams;   analysis paramenters
 *
 */
int CreateResidualFile (ANAL_PARAMS *pAnalParams)
{
    memset (&sfResidualHeader, 0, sizeof (sfResidualHeader)) ;
    sfResidualHeader.format = SF_FORMAT_AIFF | SF_FORMAT_PCM_16;
    sfResidualHeader.samplerate = pAnalParams->iSamplingRate;
    sfResidualHeader.channels = 1;

    int err = sf_format_check (&sfResidualHeader);
    if(err == 0)
    {
        printf("CreateResidualFile: invalid file format.");
        exit(1);
    }
    if( !(pResidualSNDStream = sf_open (pChResidualFile, SFM_WRITE, &sfResidualHeader)))
    {
        err = sf_error(pResidualSNDStream);
        printf("CreateResidualFile: can't open %s for writing; SNDFILE ERR: %d, %s.\n", pChResidualFile, err,  sf_error_number(err));  
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
int WriteToResidualFile (float *pBuffer, int sizeBuffer)
{
	int i;
	short *pSResidual;

	if ((pSResidual = (short *) calloc(sizeBuffer, sizeof(short))) == NULL)
		return -1;

	for (i = 0; i < sizeBuffer; i++)
		pSResidual[i] = (short) DeEmphasis (pBuffer[i]);

                sf_writef_float( pResidualSNDStream, pBuffer, sizeBuffer);
	
	free (pSResidual);
	return 1;
}

/* function to write the residual sound file to disk */
int WriteResidualFile ()
{
    sf_close(pOutputSNDStream);
    return 1;		  
}

