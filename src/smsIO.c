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
#include "sms.h"


/* initialize the header structure of an SMS file
 *
 * SMSHeader *pSmsHeader	header for SMS file
 */
int InitSmsHeader (SMSHeader *pSmsHeader)
{    
	pSmsHeader->iSmsMagic = SMS_MAGIC;
	pSmsHeader->iHeadBSize =  sizeof(SMSHeader);
	pSmsHeader->nRecords = 0;
	pSmsHeader->iRecordBSize = 0;
	pSmsHeader->iFormat = FORMAT_HARMONIC;
	pSmsHeader->iFrameRate = 0;
	pSmsHeader->iStochasticType = STOC_APPROX;
	pSmsHeader->nTrajectories = 0;
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
	return (1);
}
 
/* initialize an SMS data record
 *
 * SMS_DATA *pSmsRecord;	pointer to a frame of SMS data
 */
void InitSmsRecord (SMS_DATA *pSmsRecord)
{
	pSmsRecord->pSmsData = NULL;
	pSmsRecord->pFFreqTraj = NULL;
	pSmsRecord->pFMagTraj = NULL;
	pSmsRecord->pFPhaTraj = NULL;
	pSmsRecord->pFStocCoeff = NULL;
	pSmsRecord->pFStocGain = NULL;
	pSmsRecord->nTraj = 0;
	pSmsRecord->nCoeff = 0;
	pSmsRecord->sizeData = 0;
}

/* write SMS header to file
 *
 * char *pChFileName	   file name for SMS file
 * SMSHeader *pSmsHeader header for SMS file
 * FILE **ppSmsFile      file to be created
 *
 */
int WriteSmsHeader (char *pChFileName, SMSHeader *pSmsHeader, 
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

	pSmsHeader->iHeadBSize = sizeof(SMSHeader) + iVariableSize;

	/* write header */
	if (fwrite((void *)pSmsHeader, (size_t)1, (size_t)sizeof(SMSHeader),
	    *ppSmsFile) < (size_t)sizeof(SMSHeader))
		quit ("Error: Cannot write SMS header");
	
	/* write variable part of header */
	if (pSmsHeader->nLoopRecords > 0)
	{
		char *pChStart = (char *) pSmsHeader->pILoopRecords;
		int iSize = sizeof (int) * pSmsHeader->nLoopRecords;
    
		if (fwrite ((void *)pChStart, (size_t)1, (size_t)iSize, *ppSmsFile) < 
		    (size_t)iSize)
			quit ("Error: Cannot write SMS header");
	}
	if (pSmsHeader->nSpecEnvelopePoints > 0)
	{
		char *pChStart = (char *) pSmsHeader->pFSpectralEnvelope;
		int iSize = sizeof (float) * pSmsHeader->nSpecEnvelopePoints;
    
		if (fwrite ((void *)pChStart, (size_t)1, (size_t)iSize, *ppSmsFile) < 
		    (size_t)iSize)
			quit ("Error: Cannot write SMS header");
	}
	if (pSmsHeader->nTextCharacters > 0)
	{
		char *pChStart = (char *) pSmsHeader->pChTextCharacters;
		int iSize = sizeof(char) * pSmsHeader->nTextCharacters;
    
		if (fwrite ((void *)pChStart, (size_t)1, (size_t)iSize, *ppSmsFile) < 
		    (size_t)iSize)
			quit ("Error: Cannot write SMS header");
	}
	return (SMS_OK);
}

/* rewrite SMS header and close file
 *
 * FILE *pChFileName	pointer to SMS file
 * SMSHeader *pSmsHeader header for SMS file
 *
 */
int WriteSmsFile (FILE *pSmsFile, SMSHeader *pSmsHeader)
{
	int iVariableSize;
	rewind(pSmsFile);

	/* check variable size of header */
	iVariableSize = sizeof (int) * pSmsHeader->nLoopRecords +
		sizeof (float) * pSmsHeader->nSpecEnvelopePoints +
		sizeof(char) * pSmsHeader->nTextCharacters;

	pSmsHeader->iHeadBSize = sizeof(SMSHeader) + iVariableSize;

	/* write header */
	if (fwrite((void *)pSmsHeader, (size_t)1, (size_t)sizeof(SMSHeader),
	    pSmsFile) < (size_t)sizeof(SMSHeader))
		quit ("Error: Cannot write SMS header");
	
	/* write variable part of header */
	if (pSmsHeader->nLoopRecords > 0)
	{
		char *pChStart = (char *) pSmsHeader->pILoopRecords;
		int iSize = sizeof (int) * pSmsHeader->nLoopRecords;
    
		if (fwrite ((void *)pChStart, (size_t)1, (size_t)iSize, pSmsFile) < 
		    (size_t)iSize)
			quit ("Error: Cannot write SMS header");
	}
	if (pSmsHeader->nSpecEnvelopePoints > 0)
	{
		char *pChStart = (char *) pSmsHeader->pFSpectralEnvelope;
		int iSize = sizeof (float) * pSmsHeader->nSpecEnvelopePoints;
    
		if (fwrite ((void *)pChStart, (size_t)1, (size_t)iSize, pSmsFile) < 
		    (size_t)iSize)
			quit ("Error: Cannot write SMS header");
	}
	if (pSmsHeader->nTextCharacters > 0)
	{
		char *pChStart = (char *) pSmsHeader->pChTextCharacters;
		int iSize = sizeof(char) * pSmsHeader->nTextCharacters;
    
		if (fwrite ((void *)pChStart, (size_t)1, (size_t)iSize, pSmsFile) < 
		    (size_t)iSize)
			quit ("Error: Cannot write SMS header");
	}

	fclose(pSmsFile);
	return (SMS_OK);
}

/* write SMS record
 *
 * FILE *pSmsFile	        pointer to SMS file
 * SMSHeader *pSmsHeader  pointer to SMS header
 * SMS_DATA *pSmsRecord   pointer to SMS data record
 *
 */
int WriteSmsRecord (FILE *pSmsFile, SMSHeader *pSmsHeader, 
                    SMS_DATA *pSmsRecord)
{  
	if (fwrite ((void *)pSmsRecord->pSmsData, 1, pSmsHeader->iRecordBSize, 
	            pSmsFile) < pSmsHeader->iRecordBSize)
			quit ("Error: Cannot write SMS record");
	return (SMS_OK);			
}


/* function return the size in bytes of the record in a SMS file 
 *
 * SMSHeader *pSmsHeader;    pointer to SMS header
 *
 */
int GetRecordBSize (SMSHeader *pSmsHeader)
{
	int iSize = 0, nGain = 1, nComp = 2;
    
	if (pSmsHeader->iStochasticType == STOC_NONE)
		nGain = 0;
    
	if (pSmsHeader->iFormat == FORMAT_HARMONIC_WITH_PHASE ||
	    pSmsHeader->iFormat == FORMAT_INHARMONIC_WITH_PHASE)
		nComp = 3;
     
	iSize = sizeof (float) * (nComp * pSmsHeader->nTrajectories + 
		pSmsHeader->nStochasticCoeff + nGain);
	return(iSize);
}	     
     

/* function to read SMS header
 *
 * char *pChFileName;		      file name for SMS file
 * SMSHeader  *pSmsHeader;	SMS header
 * FILE **ppSmsFile         pointer to inputfile
 *
 * returns SMS_NOPEN   if can't open
 *         SMS_NSMS    if not a SMS file
 *        SMS_RDERR   if reads fail
 *        SMS_MALLOC  if can't get memory
 *        SMS_OK      otherwise	
 */
int GetSmsHeader (char *pChFileName, SMSHeader **ppSmsHeader,
                  FILE **ppSmsFile)
{
	int iHeadBSize, iRecordBSize, nRecords;
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
  if (fread ((void *) &nRecords, (size_t) sizeof(int), (size_t)1, 
	         *ppSmsFile) < (size_t)1)
		return (SMS_RDERR);

	if (nRecords <= 0)
		return (SMS_RDERR);
	
  /* read size of data Records */
	if (fread ((void *) &iRecordBSize, (size_t) sizeof(int), (size_t)1, 
	           *ppSmsFile) < (size_t)1)
		return (SMS_RDERR);

	if (iRecordBSize <= 0)
		return (SMS_RDERR);

	/* allocate memory for header */
	if (((*ppSmsHeader) = (SMSHeader *)malloc (iHeadBSize)) == NULL)
		return (SMS_MALLOC);

	/* read header */
	rewind (*ppSmsFile);
	if (fread ((void *) (*ppSmsHeader), 1, iHeadBSize, *ppSmsFile) < iHeadBSize)
		return (SMS_RDERR);

	/* set pointers to variable part of header */
	if ((*ppSmsHeader)->nLoopRecords > 0)
		(*ppSmsHeader)->pILoopRecords = (int *) ((char *)(*ppSmsHeader) + 
			sizeof(SMSHeader));
						
	if ((*ppSmsHeader)->nSpecEnvelopePoints > 0)
		(*ppSmsHeader)->pFSpectralEnvelope = 
			(float *) ((char *)(*ppSmsHeader) + sizeof(SMSHeader) + 
			           sizeof(int) * (*ppSmsHeader)->nLoopRecords);
			
	if ((*ppSmsHeader)->nTextCharacters > 0)
		(*ppSmsHeader)->pChTextCharacters = 
			(char *) ((char *)(*ppSmsHeader) + sizeof(SMSHeader) + 
			sizeof(int) * (*ppSmsHeader)->nLoopRecords +
			sizeof(float) * (*ppSmsHeader)->nSpecEnvelopePoints);

	return (SMS_OK);			
}

/* function to read SMS data record
 *
 * FILE *pSmsFile;		     pointer to SMS file
 * SMSHeader *pSmsHeader;	   pointer to SMS header
 * int iRecord;              record number
 * SMS_DATA *pSmsRecord;       pointer to SMS record
 *
 * returns
 *        SMS_OK      if it could read the data
 *	
 */
int GetSmsRecord (FILE *pSmsFile, SMSHeader *pSmsHeader, int iRecord,
                  SMS_DATA *pSmsRecord)
{    
	if (fseek (pSmsFile, pSmsHeader->iHeadBSize + iRecord * 
	                     pSmsHeader->iRecordBSize, SEEK_SET) < 0)
	{
		fprintf (stderr,"GetSmsRecord: could not seek to the sms record %d\n", 
		         iRecord);
		return (-1);
	}
	if ((pSmsHeader->iRecordBSize = 
	       fread ((void *)pSmsRecord->pSmsData, (size_t)1, 
	              (size_t)pSmsHeader->iRecordBSize, pSmsFile))
	    != pSmsHeader->iRecordBSize)
	{
		fprintf (stderr,"GetSmsRecord: could not read sms record %d\n", 
		         iRecord);
		return (-1);
	}
	return (SMS_OK);			
}

/* function to allocate an SMS data record
 *
 * SMSHeader *pSmsHeader;	   pointer to SMS header
 * SMS_DATA *pSmsRecord;     pointer to SMS record
 *
 * returns
 *        SMS_OK      if it could read the data
 *	      SMS_MALLOC  if it could not allocate record
 */
int AllocSmsRecord (SMSHeader *pSmsHeader, SMS_DATA *pSmsRecord)
{
	int iPhase = (pSmsHeader->iFormat == FORMAT_HARMONIC_WITH_PHASE ||
	              pSmsHeader->iFormat == FORMAT_INHARMONIC_WITH_PHASE) ? 1 : 0;

	return (AllocateSmsRecord (pSmsRecord, pSmsHeader->nTrajectories, 
                                   pSmsHeader->nStochasticCoeff, iPhase, pSmsHeader->sizeHop,
                                   pSmsHeader->iStochasticType));
}


/* allocate memory for a frame of SMS data
 *
 * SMS_DATA *pSmsRecord;	pointer to a frame of SMS data
 * int nTraj;		        number of trajectories in frame
 * int nCoeff;		      number of stochastic coefficients in frame
 * int iPhase;		      whether phase information is in the frame
 * int sizeHop;               the hopsize used for residual resynthesis
 * int stochType;           stochastic resynthesis type
 */
int AllocateSmsRecord (SMS_DATA *pSmsRecord, int nTraj, int nCoeff, int iPhase,
                                       int sizeHop, int stochType)
{
//        printf("sizeHop: %d, stochType: %d \n", sizeHop, stochType);
        int dataPos = nTraj;
	/* calculate size of record */
	int sizeData = 2 * nTraj * sizeof(float);
	if (iPhase > 0) sizeData += nTraj * sizeof(float);
	if (nCoeff > 0) sizeData += (nCoeff + 1) * sizeof(float);
        if( stochType == STOC_WAVEFORM)
                {
//                        printf("allocating %d bytes for audio data \n", sizeHop * sizeof(float));
                        sizeData += sizeHop * sizeof(float);
                }
	/* allocate memory for data */
	if ((pSmsRecord->pSmsData = (float *) malloc (sizeData)) == NULL)
		return (SMS_MALLOC);

	/* set the rest of the variables in the structure */
	pSmsRecord->nTraj = nTraj;
	pSmsRecord->nCoeff = nCoeff;
	pSmsRecord->sizeData = sizeData;
	pSmsRecord->pFFreqTraj = pSmsRecord->pSmsData;
	pSmsRecord->pFMagTraj = (float *) (pSmsRecord->pSmsData + dataPos);
        dataPos += nTraj; // after Magnitude
	if (iPhase > 0)
	{
		pSmsRecord->pFPhaTraj = (float *) (pSmsRecord->pSmsData + dataPos);
                dataPos += nTraj; //after Phase
		if (nCoeff > 0)
		{
			pSmsRecord->pFStocCoeff = (float *) (pSmsRecord->pSmsData + dataPos);
                        dataPos += nCoeff; //after stochastic coefficients
                        //todo: verify StocGain# is same as Coeff
			pSmsRecord->pFStocGain = (float *) (pSmsRecord->pSmsData + dataPos);
                        dataPos += nCoeff;
		}
		else
		{
			pSmsRecord->pFStocCoeff = NULL;
			pSmsRecord->pFStocGain = NULL;
		}
	}
	else
	{
		pSmsRecord->pFPhaTraj = NULL;
		if (nCoeff > 0)
		{
			pSmsRecord->pFStocCoeff = (float *) (pSmsRecord->pSmsData + dataPos);
                        dataPos += nCoeff;
			pSmsRecord->pFStocGain = (float *) (pSmsRecord->pSmsData + dataPos);
                        dataPos += nCoeff;
		}
		else
		{
			pSmsRecord->pFStocCoeff = NULL;
			pSmsRecord->pFStocGain = NULL;
		}
	}
        if( stochType == STOC_WAVEFORM)
        {
                        pSmsRecord->pFStocAudio = (float *)(pSmsRecord->pSmsData + dataPos);
                        dataPos += sizeHop;
        }
	return (SMS_OK);			
}

/* free the SMS data structure
 * 
 * SMS_DATA *pSmsRecord;	pointer to frame of SMS data
 *
 */
void FreeSmsRecord (SMS_DATA *pSmsRecord)
{
	free(pSmsRecord->pSmsData);
	pSmsRecord->nTraj = 0;
	pSmsRecord->nCoeff = 0;
	pSmsRecord->sizeData = 0;
	pSmsRecord->pFFreqTraj = NULL;
	pSmsRecord->pFMagTraj = NULL;
	pSmsRecord->pFStocCoeff = NULL;
	pSmsRecord->pFStocGain = NULL;
}

/* clear the SMS data structure
 * 
 * SMS_DATA *pSmsRecord;	pointer to frame of SMS data
 *
 */
void ClearSmsRecord (SMS_DATA *pSmsRecord)
{
	memset ((char *) pSmsRecord->pSmsData, 0, pSmsRecord->sizeData);
}
  
/* copy a record of SMS data into another
 *
 * SMS_DATA *pCopySmsData;	copy of frame
 * SMS_DATA *pOriginalSmsData;	original frame
 *
 */
int CopySmsRecord (SMS_DATA *pCopySmsData, SMS_DATA *pOriginalSmsData)
{
	/* if the two records are the same size just copy data */
	if (pCopySmsData->sizeData == pOriginalSmsData->sizeData &&
	    pCopySmsData->nTraj == pOriginalSmsData->nTraj)
	{
		memcpy ((char *)pCopySmsData->pSmsData, 
	          (char *)pOriginalSmsData->pSmsData,
	          pCopySmsData->sizeData);
	}
	/* if records of different size copy the smallest */
	else
	{	
		int nTraj = MIN (pCopySmsData->nTraj, pOriginalSmsData->nTraj);
		int nCoeff = MIN (pCopySmsData->nCoeff, pOriginalSmsData->nCoeff);

		pCopySmsData->nTraj = nTraj;
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
	return (1);
}


/* function to interpolate two SMS records
 * it assumes that the two records are of the same size
 *
 * SMS_DATA *pSmsRecord1            sms record 1
 * SMS_DATA *pSmsRecord2            sms record 2
 * SMS_DATA *pSmsRecordOut          sms output record
 * float fInterpFactor              interpolation factor
 *
 */
int InterpolateSmsRecords (SMS_DATA *pSmsRecord1, SMS_DATA *pSmsRecord2,
                           SMS_DATA *pSmsRecordOut, float fInterpFactor)
{
	int i, nTraj = pSmsRecord1->nTraj, nCoeff = pSmsRecord1->nCoeff; 
	float fFreq1, fFreq2;
					 
	/* interpolate the deterministic part */
	for (i = 0; i < nTraj; i++)
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
	
	/* interpolate the stochastic part */
	if (pSmsRecordOut->pFStocGain)
		*(pSmsRecordOut->pFStocGain) = 
			*(pSmsRecord1->pFStocGain) + fInterpFactor *
			(*(pSmsRecord2->pFStocGain) - *(pSmsRecord1->pFStocGain));
		
	if (nCoeff)
		for (i = 0; i < pSmsRecord1->nCoeff; i++)
			pSmsRecordOut->pFStocCoeff[i] = 
				pSmsRecord1->pFStocCoeff[i] + fInterpFactor * 
				(pSmsRecord2->pFStocCoeff[i] - pSmsRecord1->pFStocCoeff[i]);

	return 1;
}
