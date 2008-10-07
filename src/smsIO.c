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
/*! \file smsIO.c
 * \brief SMS file input and output
 */

#include "sms.h"

/*! \brief file identification constant
 * 
 * constant number that is first within SMS_Header, in order to correctly
 * identify an SMS file when read.  
 */
#define SMS_MAGIC 767  

/*! \brief initialize the header structure of an SMS file
 *
 * \param pSmsHeader	header for SMS file
 */
void sms_initHeader (SMS_Header *pSmsHeader)
{    
	pSmsHeader->iSmsMagic = SMS_MAGIC;
	pSmsHeader->iHeadBSize =  sizeof(SMS_Header);
	pSmsHeader->nFrames = 0;
	pSmsHeader->iRecordBSize = 0;
	pSmsHeader->iFormat = SMS_FORMAT_H;
	pSmsHeader->iFrameRate = 0;
	pSmsHeader->iStochasticType = SMS_STOC_APPROX;
	pSmsHeader->nTracks = 0;
	pSmsHeader->nStochasticCoeff = 0;
	pSmsHeader->fAmplitude = 0;
	pSmsHeader->fFrequency = 0;
	pSmsHeader->iBegSteadyState = 0;
	pSmsHeader->iEndSteadyState = 0;
	pSmsHeader->fResidualPerc = 0;
	pSmsHeader->nLoopRecords = 0;
	pSmsHeader->nSpecEnvelopePoints = 0;
	pSmsHeader->nTextCharacters = 0;
	pSmsHeader->pILoopRecords = NULL;
	pSmsHeader->pFSpectralEnvelope = NULL;
	pSmsHeader->pChTextCharacters = NULL;    
}

/*! \brief fill an SMS header with necessary information for storage
 * 
 * copies parameters from SMS_AnalParams, along with other values
 * so an SMS file can be stored and correctly synthesized at a later
 * time. This is somewhat of a convenience function.
 *
 * \param pSmsHeader    header for SMS file (to be stored)
 * \param nFrames           number of frames in analysis
 * \param pAnalParams   structure of analysis parameters
 * \param iOriginalSRate  samplerate of original input sound file
 * \param nTracks           number of sinusoidal tracks in the analysis
 */
void sms_fillHeader (SMS_Header *pSmsHeader, 
                          int nFrames, SMS_AnalParams *pAnalParams,
                    int iOriginalSRate, int nTracks)
{
        sms_initHeader (pSmsHeader);

        pSmsHeader->nFrames = nFrames;
        pSmsHeader->iFormat = pAnalParams->iFormat;
        pSmsHeader->iFrameRate = pAnalParams->iFrameRate;
        pSmsHeader->iStochasticType = pAnalParams->iStochasticType;
        pSmsHeader->nTracks = nTracks;
	if(pAnalParams->iStochasticType != SMS_STOC_APPROX)
		pSmsHeader->nStochasticCoeff = 0;
        else
                pSmsHeader->nStochasticCoeff = pAnalParams->nStochasticCoeff;
        pSmsHeader->iOriginalSRate = iOriginalSRate;
        pSmsHeader->iRecordBSize = sms_recordSizeB(pSmsHeader);
}

/*! \brief initialize an SMS data record
 *
 * \param pSmsRecord	pointer to a frame of SMS data
 */
void sms_initRecord (SMS_Data *pSmsRecord)
{
	pSmsRecord->pSmsData = NULL;
	pSmsRecord->pFFreqTraj = NULL;
	pSmsRecord->pFMagTraj = NULL;
	pSmsRecord->pFPhaTraj = NULL;
	pSmsRecord->pFStocCoeff = NULL;
	pSmsRecord->pFStocGain = NULL;
	pSmsRecord->nTracks = 0;
	pSmsRecord->nCoeff = 0;
	pSmsRecord->sizeData = 0;
}

/*! \brief write SMS header to file
 *
 * \param pChFileName	   file name for SMS file
 * \param pSmsHeader header for SMS file
 * \param ppSmsFile     (double pointer to)  file to be created
 * \return error code \see SMS_WRERR in SMS_ERRORS 
 */
int sms_writeHeader (char *pChFileName, SMS_Header *pSmsHeader, 
                    FILE **ppSmsFile)
{
	int iVariableSize = 0;

	if (pSmsHeader->iSmsMagic != SMS_MAGIC)
		return(SMS_NSMS);
	if ((*ppSmsFile = fopen (pChFileName, "w+")) == NULL)
		return(SMS_NOPEN);
	
	/* check variable size of header */
	iVariableSize = sizeof (int) * pSmsHeader->nLoopRecords +
		sizeof (float) * pSmsHeader->nSpecEnvelopePoints +
		sizeof(char) * pSmsHeader->nTextCharacters;

	pSmsHeader->iHeadBSize = sizeof(SMS_Header) + iVariableSize;

	/* write header */
	if (fwrite((void *)pSmsHeader, (size_t)1, (size_t)sizeof(SMS_Header),
	    *ppSmsFile) < (size_t)sizeof(SMS_Header))
                return(SMS_WRERR);
	
	/* write variable part of header */
	if (pSmsHeader->nLoopRecords > 0)
	{
		char *pChStart = (char *) pSmsHeader->pILoopRecords;
		int iSize = sizeof (int) * pSmsHeader->nLoopRecords;
    
		if (fwrite ((void *)pChStart, (size_t)1, (size_t)iSize, *ppSmsFile) < 
		    (size_t)iSize)
                        return(SMS_WRERR);

	}
	if (pSmsHeader->nSpecEnvelopePoints > 0)
	{
		char *pChStart = (char *) pSmsHeader->pFSpectralEnvelope;
		int iSize = sizeof (float) * pSmsHeader->nSpecEnvelopePoints;
    
		if (fwrite ((void *)pChStart, (size_t)1, (size_t)iSize, *ppSmsFile) < 
		    (size_t)iSize)
                        return(SMS_WRERR);
	}
	if (pSmsHeader->nTextCharacters > 0)
	{
		char *pChStart = (char *) pSmsHeader->pChTextCharacters;
		int iSize = sizeof(char) * pSmsHeader->nTextCharacters;
    
		if (fwrite ((void *)pChStart, (size_t)1, (size_t)iSize, *ppSmsFile) < 
		    (size_t)iSize)
                        return(SMS_WRERR);
	}
	return (SMS_OK);
}

/*! \brief rewrite SMS header and close file
 *
 * \param pSmsFile	     pointer to SMS file
 * \param pSmsHeader pointer to header for SMS file
 * \return error code \see SMS_WRERR in SMS_ERRORS 
 */
int sms_writeFile (FILE *pSmsFile, SMS_Header *pSmsHeader)
{
	int iVariableSize;

	rewind(pSmsFile);

	/* check variable size of header */
	iVariableSize = sizeof (int) * pSmsHeader->nLoopRecords +
		sizeof (float) * pSmsHeader->nSpecEnvelopePoints +
		sizeof(char) * pSmsHeader->nTextCharacters;

	pSmsHeader->iHeadBSize = sizeof(SMS_Header) + iVariableSize;

	/* write header */
	if (fwrite((void *)pSmsHeader, (size_t)1, (size_t)sizeof(SMS_Header),
	    pSmsFile) < (size_t)sizeof(SMS_Header))
		return(SMS_WRERR);
	
	/* write variable part of header */
	if (pSmsHeader->nLoopRecords > 0)
	{
		char *pChStart = (char *) pSmsHeader->pILoopRecords;
		int iSize = sizeof (int) * pSmsHeader->nLoopRecords;
    
		if (fwrite ((void *)pChStart, (size_t)1, (size_t)iSize, pSmsFile) < 
		    (size_t)iSize)
			return(SMS_WRERR);
	}
	if (pSmsHeader->nSpecEnvelopePoints > 0)
	{
		char *pChStart = (char *) pSmsHeader->pFSpectralEnvelope;
		int iSize = sizeof (float) * pSmsHeader->nSpecEnvelopePoints;
    
		if (fwrite ((void *)pChStart, (size_t)1, (size_t)iSize, pSmsFile) < 
		    (size_t)iSize)
			return(SMS_WRERR);
	}
	if (pSmsHeader->nTextCharacters > 0)
	{
		char *pChStart = (char *) pSmsHeader->pChTextCharacters;
		int iSize = sizeof(char) * pSmsHeader->nTextCharacters;
    
		if (fwrite ((void *)pChStart, (size_t)1, (size_t)iSize, pSmsFile) < 
		    (size_t)iSize)
			return(SMS_WRERR);
	}

	fclose(pSmsFile);
	return (SMS_OK);
}

/*! \brief write SMS record
 *
 * \param pSmsFile	        pointer to SMS file
 * \param pSmsHeader  pointer to SMS header
 * \param pSmsRecord   pointer to SMS data record
 * \return error code \see SMS_WRERR in SMS_ERRORS 
 */
int sms_writeRecord (FILE *pSmsFile, SMS_Header *pSmsHeader, 
                    SMS_Data *pSmsRecord)
{  
	if (fwrite ((void *)pSmsRecord->pSmsData, 1, pSmsHeader->iRecordBSize, 
	            pSmsFile) < pSmsHeader->iRecordBSize)
                return(SMS_WRERR);
	else return (SMS_OK);			
}


/*! \brief get the size in bytes of the record in a SMS file 
 *
 * \param pSmsHeader    pointer to SMS header
 * \return the size in bytes of the record
 */
int sms_recordSizeB (SMS_Header *pSmsHeader)
{
	int iSize, nDet;
  
	if (pSmsHeader->iFormat == SMS_FORMAT_H ||
	    pSmsHeader->iFormat == SMS_FORMAT_IH)
		nDet = 2;// freq, mag
        else nDet = 3; // freq, mag, phase

	iSize = sizeof (float) * (nDet * pSmsHeader->nTracks);

        if(pSmsHeader->iStochasticType == SMS_STOC_APPROX)
        {       //stocCoeff + 1 (gain)
                iSize += sizeof(float) * (pSmsHeader->nStochasticCoeff + 1);
        }
        else if(pSmsHeader->iStochasticType == SMS_STOC_IFFT)
        {
                //sizeFFT*2
        }

	return(iSize);
}	     
   

/*! \brief function to read SMS header
 *
 * \param pChFileName		      file name for SMS file
 * \param ppSmsHeader	(double pointer to) SMS header
 * \param ppSmsFile        (double pointer to) inputfile
 * \return error code \see SMS_ERRORS
 */
int sms_getHeader (char *pChFileName, SMS_Header **ppSmsHeader,
                  FILE **ppSmsFile)
{
	int iHeadBSize, iRecordBSize, nFrames;
	int iMagicNumber;
    
	/* open file for reading */
	if ((*ppSmsFile = fopen (pChFileName, "r")) == NULL)
		return (SMS_NOPEN);
            
	/* read magic number */
	if (fread ((void *) &iMagicNumber, (size_t) sizeof(int), (size_t)1, 
	           *ppSmsFile) < (size_t)1)
		return (SMS_RDERR);
	
	if (iMagicNumber != SMS_MAGIC)
		return (SMS_NSMS);

	/* read size of of header */
	if (fread ((void *) &iHeadBSize, (size_t) sizeof(int), (size_t)1, 
		*ppSmsFile) < (size_t)1)
		return (SMS_RDERR);
	
	if (iHeadBSize <= 0)
		return (SMS_RDERR);
     
        /* read number of data Records */
        if (fread ((void *) &nFrames, (size_t) sizeof(int), (size_t)1, 
                   *ppSmsFile) < (size_t)1)
		return (SMS_RDERR);
        
	if (nFrames <= 0)
		return (SMS_RDERR);
        
        /* read size of data Records */
	if (fread ((void *) &iRecordBSize, (size_t) sizeof(int), (size_t)1, 
	           *ppSmsFile) < (size_t)1)
		return (SMS_RDERR);
        
	if (iRecordBSize <= 0)
		return (SMS_RDERR);

	/* allocate memory for header */
	if (((*ppSmsHeader) = (SMS_Header *)malloc (iHeadBSize)) == NULL)
		return (SMS_MALLOC);

	/* read header */
	rewind (*ppSmsFile);
	if (fread ((void *) (*ppSmsHeader), 1, iHeadBSize, *ppSmsFile) < iHeadBSize)
		return (SMS_RDERR);

	/* set pointers to variable part of header */
	if ((*ppSmsHeader)->nLoopRecords > 0)
		(*ppSmsHeader)->pILoopRecords = (int *) ((char *)(*ppSmsHeader) + 
			sizeof(SMS_Header));
						
	if ((*ppSmsHeader)->nSpecEnvelopePoints > 0)
		(*ppSmsHeader)->pFSpectralEnvelope = 
			(float *) ((char *)(*ppSmsHeader) + sizeof(SMS_Header) + 
			           sizeof(int) * (*ppSmsHeader)->nLoopRecords);
			
	if ((*ppSmsHeader)->nTextCharacters > 0)
		(*ppSmsHeader)->pChTextCharacters = 
			(char *) ((char *)(*ppSmsHeader) + sizeof(SMS_Header) + 
			sizeof(int) * (*ppSmsHeader)->nLoopRecords +
			sizeof(float) * (*ppSmsHeader)->nSpecEnvelopePoints);

	return (SMS_OK);			
}

/*! \brief read an SMS data record
 *
 * \param pSmsFile		   pointer to SMS file
 * \param pSmsHeader	   pointer to SMS header
 * \param iRecord               record number
 * \param pSmsRecord       pointer to SMS record
 * \return  SMS_OK if it could read the data, -1 if not
 *	
 */
int sms_getRecord (FILE *pSmsFile, SMS_Header *pSmsHeader, int iRecord,
                  SMS_Data *pSmsRecord)
{    
	if (fseek (pSmsFile, pSmsHeader->iHeadBSize + iRecord * 
	                     pSmsHeader->iRecordBSize, SEEK_SET) < 0)
	{
		printf ("sms_getRecord: could not seek to the sms record %d\n", 
		         iRecord);
		return (-1);
	}
	if ((pSmsHeader->iRecordBSize = 
	       fread ((void *)pSmsRecord->pSmsData, (size_t)1, 
	              (size_t)pSmsHeader->iRecordBSize, pSmsFile))
	    != pSmsHeader->iRecordBSize)
	{
		printf ("sms_getRecord: could not read sms record %d\n", 
		         iRecord);
		return (-1);
	}
	return (SMS_OK);			
}



/*! \brief  allocate memory for a frame of SMS data
 *
 * \param pSmsRecord	     pointer to a frame of SMS data
 * \param nTracks		      number of sinusoidal tracks in frame
 * \param nCoeff		      number of stochastic coefficients in frame
 * \param iPhase		      whether phase information is in the frame
 * \param stochType           stochastic resynthesis type
 * \return error code \see SMS_MALLOC in SMS_ERRORS
 */
int sms_allocRecord (SMS_Data *pSmsRecord, int nTracks, int nCoeff, int iPhase,
                                       int stochType)
{
        float *dataPos;  /* a marker to locate specific data witin smsData */
	/* calculate size of record */
	int sizeData = 2 * nTracks * sizeof(float);
        sizeData += 1 * sizeof(float); //adding one for nSamples
	if (iPhase > 0) sizeData += nTracks * sizeof(float);
	if (nCoeff > 0) sizeData += (nCoeff + 1) * sizeof(float);
	/* allocate memory for data */
	if ((pSmsRecord->pSmsData = (float *) malloc (sizeData)) == NULL)
		return (SMS_MALLOC);

	/* set the variables in the structure */
	pSmsRecord->nTracks = nTracks;
	pSmsRecord->nCoeff = nCoeff;
	pSmsRecord->sizeData = sizeData; 
        /* set pointers to data types within smsData array */
	pSmsRecord->pFFreqTraj = pSmsRecord->pSmsData;
        dataPos =  (float *)(pSmsRecord->pFFreqTraj + nTracks);
	pSmsRecord->pFMagTraj = dataPos;
        dataPos = (float *)(pSmsRecord->pFMagTraj + nTracks);
	if (iPhase > 0)
	{
		pSmsRecord->pFPhaTraj = dataPos;
                dataPos = (float *) (pSmsRecord->pFPhaTraj + nTracks);
        }	
	else 	pSmsRecord->pFPhaTraj = NULL;
	if (nCoeff > 0)
	{
                pSmsRecord->pFStocCoeff = dataPos;
                dataPos = (float *) (pSmsRecord->pFStocCoeff + nCoeff);
                pSmsRecord->pFStocGain = dataPos; 
                dataPos = (float *) (pSmsRecord->pFStocGain + 1);
	}
        else
	{
                pSmsRecord->pFStocCoeff = NULL;
                pSmsRecord->pFStocGain = NULL;
        }
	return (SMS_OK);			
}

/*! \brief  function to allocate an SMS data record using an SMS_Header
 *
 * this one is used when you have only read the header, such as after 
 * opening a file.
 *
 * \param pSmsHeader	   pointer to SMS header
 * \param pSmsRecord     pointer to SMS record
 * \return  error code \see SMS_OK and SMS_MALLOC  in SMS_ERRORS 
 */
int sms_allocRecordH (SMS_Header *pSmsHeader, SMS_Data *pSmsRecord)
{
	int iPhase = (pSmsHeader->iFormat == SMS_FORMAT_HP ||
	              pSmsHeader->iFormat == SMS_FORMAT_IHP) ? 1 : 0;
	return (sms_allocRecord (pSmsRecord, pSmsHeader->nTracks, 
                                   pSmsHeader->nStochasticCoeff, iPhase,
                                   pSmsHeader->iStochasticType));
}

/*! \brief free the SMS data structure
 * 
 * \param pSmsRecord	pointer to frame of SMS data
 */
void sms_freeRecord (SMS_Data *pSmsRecord)
{
	free(pSmsRecord->pSmsData);
	pSmsRecord->nTracks = 0;
	pSmsRecord->nCoeff = 0;
	pSmsRecord->sizeData = 0;
	pSmsRecord->pFFreqTraj = NULL;
	pSmsRecord->pFMagTraj = NULL;
	pSmsRecord->pFStocCoeff = NULL;
	pSmsRecord->pFStocGain = NULL;
}

/*! \brief clear the SMS data structure
 * 
 * \param pSmsRecord	pointer to frame of SMS data
 */
void sms_clearRecord (SMS_Data *pSmsRecord)
{
	memset ((char *) pSmsRecord->pSmsData, 0, pSmsRecord->sizeData);
}
  
/*! \brief copy a frame of SMS_Data 
 *
 * \param pCopySmsData	copy of frame
 * \param pOriginalSmsData	original frame
 *
 */
void sms_copyRecord (SMS_Data *pCopySmsData, SMS_Data *pOriginalSmsData)
{
	/* if the two records are the same size just copy data */
	if (pCopySmsData->sizeData == pOriginalSmsData->sizeData &&
	    pCopySmsData->nTracks == pOriginalSmsData->nTracks)
	{
		memcpy ((char *)pCopySmsData->pSmsData, 
	          (char *)pOriginalSmsData->pSmsData,
	          pCopySmsData->sizeData);
	}
	/* if records of different size copy the smallest */
	else
	{	
		int nTraj = MIN (pCopySmsData->nTracks, pOriginalSmsData->nTracks);
		int nCoeff = MIN (pCopySmsData->nCoeff, pOriginalSmsData->nCoeff);

		pCopySmsData->nTracks = nTraj;
		pCopySmsData->nCoeff = nCoeff;
		memcpy ((char *)pCopySmsData->pFFreqTraj, 
	          (char *)pOriginalSmsData->pFFreqTraj,
	          sizeof(float) * nTraj);
		memcpy ((char *)pCopySmsData->pFMagTraj, 
	          (char *)pOriginalSmsData->pFMagTraj,
	          sizeof(float) * nTraj);
		if (pOriginalSmsData->pFPhaTraj != NULL &&
	      pCopySmsData->pFPhaTraj != NULL)
			memcpy ((char *)pCopySmsData->pFPhaTraj, 
		          (char *)pOriginalSmsData->pFPhaTraj,
		          sizeof(float) * nTraj);
		if (pOriginalSmsData->pFStocCoeff != NULL &&
	      pCopySmsData->pFStocCoeff != NULL)
			memcpy ((char *)pCopySmsData->pFStocCoeff, 
	            (char *)pOriginalSmsData->pFStocCoeff,
	            sizeof(float) * nCoeff);
		if (pOriginalSmsData->pFStocGain != NULL &&
	      pCopySmsData->pFStocGain != NULL)
			memcpy ((char *)pCopySmsData->pFStocGain, 
	            (char *)pOriginalSmsData->pFStocGain,
	            sizeof(float));
	}
}

/*! \brief function to interpolate two SMS records
 *
 * this assumes that the two records are of the same size
 *
 * \param pSmsRecord1            sms frame 1
 * \param pSmsRecord2            sms frame 2
 * \param pSmsRecordOut        sms output record
 * \param fInterpFactor              interpolation factor
 */
void sms_interpolateRecords (SMS_Data *pSmsRecord1, SMS_Data *pSmsRecord2,
                           SMS_Data *pSmsRecordOut, float fInterpFactor)
{
	int i;
	float fFreq1, fFreq2;
					 
	/* interpolate the deterministic part */
	for (i = 0; i < pSmsRecord1->nTracks; i++)
	{
		fFreq1 = pSmsRecord1->pFFreqTraj[i];
		fFreq2 = pSmsRecord2->pFFreqTraj[i];
		if (fFreq1 == 0) fFreq1 = fFreq2;
		if (fFreq2 == 0) fFreq2 = fFreq1;
		pSmsRecordOut->pFFreqTraj[i] = 
			fFreq1 + fInterpFactor * (fFreq2 - fFreq1);
		pSmsRecordOut->pFMagTraj[i] = 
			pSmsRecord1->pFMagTraj[i] + fInterpFactor * 
			(pSmsRecord2->pFMagTraj[i] - pSmsRecord1->pFMagTraj[i]);
	}

	/* interpolate the stochastic part. The pointer is non-null when the frame contains
         stochastic coefficients */
	if (pSmsRecordOut->pFStocGain)
        {
                *(pSmsRecordOut->pFStocGain) = 
			*(pSmsRecord1->pFStocGain) + fInterpFactor *
			(*(pSmsRecord2->pFStocGain) - *(pSmsRecord1->pFStocGain));
        }
        for (i = 0; i < pSmsRecord1->nCoeff; i++)
                pSmsRecordOut->pFStocCoeff[i] = 
                        pSmsRecord1->pFStocCoeff[i] + fInterpFactor * 
                        (pSmsRecord2->pFStocCoeff[i] - pSmsRecord1->pFStocCoeff[i]);

}

 
