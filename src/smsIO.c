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

/*! \brief file identification constant
 * 
 * constant number that is first within SMS_Header, in order to correctly
 * identify an SMS file when read.  */
#define SMS_MAGIC 767  

/* initialize the header structure of an SMS file
 *
 * SMS_Header *pSmsHeader	header for SMS file
 */
int sms_initHeader (SMS_Header *pSmsHeader)
{    
	pSmsHeader->iSmsMagic = SMS_MAGIC;
	pSmsHeader->iHeadBSize =  sizeof(SMS_Header);
	pSmsHeader->nFrames = 0;
	pSmsHeader->iRecordBSize = 0;
	pSmsHeader->iFormat = SMS_FORMAT_H;
	pSmsHeader->iFrameRate = 0;
	pSmsHeader->iStochasticType = SMS_STOC_APPROX;
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
 * SMS_Data *pSmsRecord;	pointer to a frame of SMS data
 */
void sms_initRecord (SMS_Data *pSmsRecord)
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
 * SMS_Header *pSmsHeader header for SMS file
 * FILE **ppSmsFile      file to be created
 *
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
		//quit ("Error: Cannot write SMS header");
                return(SMS_WRERR);
	
	/* write variable part of header */
	if (pSmsHeader->nLoopRecords > 0)
	{
		char *pChStart = (char *) pSmsHeader->pILoopRecords;
		int iSize = sizeof (int) * pSmsHeader->nLoopRecords;
    
		if (fwrite ((void *)pChStart, (size_t)1, (size_t)iSize, *ppSmsFile) < 
		    (size_t)iSize)
			//quit ("Error: Cannot write SMS header");
                        return(SMS_WRERR);

	}
	if (pSmsHeader->nSpecEnvelopePoints > 0)
	{
		char *pChStart = (char *) pSmsHeader->pFSpectralEnvelope;
		int iSize = sizeof (float) * pSmsHeader->nSpecEnvelopePoints;
    
		if (fwrite ((void *)pChStart, (size_t)1, (size_t)iSize, *ppSmsFile) < 
		    (size_t)iSize)
                        return(SMS_WRERR);
                        //quit ("Error: Cannot write SMS header");
	}
	if (pSmsHeader->nTextCharacters > 0)
	{
		char *pChStart = (char *) pSmsHeader->pChTextCharacters;
		int iSize = sizeof(char) * pSmsHeader->nTextCharacters;
    
		if (fwrite ((void *)pChStart, (size_t)1, (size_t)iSize, *ppSmsFile) < 
		    (size_t)iSize)
                        return(SMS_WRERR);
                        //quit ("Error: Cannot write SMS header");
	}
	return (SMS_OK);
}

/* rewrite SMS header and close file
 *
 * FILE *pChFileName	pointer to SMS file
 * SMS_Header *pSmsHeader header for SMS file
 *
 */
int sms_writeFile (FILE *pSmsFile, SMS_Header *pSmsHeader)
{
/*         fclose(debugFile); */

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

/* write SMS record
 *
 * FILE *pSmsFile	        pointer to SMS file
 * SMS_Header *pSmsHeader  pointer to SMS header
 * SMS_Data *pSmsRecord   pointer to SMS data record
 *
 */
int sms_writeRecord (FILE *pSmsFile, SMS_Header *pSmsHeader, 
                    SMS_Data *pSmsRecord)
{  
	if (fwrite ((void *)pSmsRecord->pSmsData, 1, pSmsHeader->iRecordBSize, 
	            pSmsFile) < pSmsHeader->iRecordBSize)
                return(SMS_WRERR);
	else return (SMS_OK);			
}


/* function return the size in bytes of the record in a SMS file 
 *
 * SMS_Header *pSmsHeader;    pointer to SMS header
 *
 */
int sms_recordSizeB (SMS_Header *pSmsHeader)
{
/* 	int iSize = 0, nGain = 1, nComp = 2; */
    
/* 	if (pSmsHeader->iStochasticType == SMS_STOC_NONE) */
/* 		nGain = 0; */
    
/* 	if (pSmsHeader->iFormat == SMS_FORMAT_HP || */
/* 	    pSmsHeader->iFormat == SMS_FORMAT_IHP) */
/* 		nComp = 3; */
     
/* 	iSize = sizeof (float) * (nComp * pSmsHeader->nTrajectories +  */
/* 		pSmsHeader->nStochasticCoeff + nGain); */

	int iSize, nDet;
  
	if (pSmsHeader->iFormat == SMS_FORMAT_H ||
	    pSmsHeader->iFormat == SMS_FORMAT_IH)
		nDet = 2;// freq, mag
        else nDet = 3; // freq, mag, phase

	iSize = sizeof (float) * (nDet * pSmsHeader->nTrajectories);

	if (pSmsHeader->iStochasticType == SMS_STOC_WAVE)
        {       //numSamples
//                iSize += sizeof(float) * iHopSize;
                iSize +=sizeof(float) * ( (int)(pSmsHeader->iOriginalSRate / 
                                                (float) pSmsHeader->iFrameRate));
        }
        else if(pSmsHeader->iStochasticType == SMS_STOC_IFFT)
        {
                //sizeFFT*2
        }
        else if(pSmsHeader->iStochasticType == SMS_STOC_APPROX)
        {       //stocCoeff + 1 (gain)
                iSize += sizeof(float) * (pSmsHeader->nStochasticCoeff + 1);
        }

//        printf("iSize: %d \n", iSize);
	return(iSize);
}	     
   

/* function to read SMS header
 *
 * char *pChFileName;		      file name for SMS file
 * SMS_Header  *pSmsHeader;	SMS header
 * FILE **ppSmsFile         pointer to inputfile
 *
 * returns SMS_NOPEN   if can't open
 *         SMS_NSMS    if not a SMS file
 *        SMS_RDERR   if reads fail
 *        SMS_MALLOC  if can't get memory
 *        SMS_OK      otherwise	
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

/* function to read SMS data record
 *
 * FILE *pSmsFile;		     pointer to SMS file
 * SMS_Header *pSmsHeader;	   pointer to SMS header
 * int iRecord;              record number
 * SMS_Data *pSmsRecord;       pointer to SMS record
 *
 * returns
 *        SMS_OK      if it could read the data
 *	
 */
int sms_getRecord (FILE *pSmsFile, SMS_Header *pSmsHeader, int iRecord,
                  SMS_Data *pSmsRecord)
{    
	if (fseek (pSmsFile, pSmsHeader->iHeadBSize + iRecord * 
	                     pSmsHeader->iRecordBSize, SEEK_SET) < 0)
	{
		fprintf (stderr,"sms_getRecord: could not seek to the sms record %d\n", 
		         iRecord);
		return (-1);
	}
	if ((pSmsHeader->iRecordBSize = 
	       fread ((void *)pSmsRecord->pSmsData, (size_t)1, 
	              (size_t)pSmsHeader->iRecordBSize, pSmsFile))
	    != pSmsHeader->iRecordBSize)
	{
		fprintf (stderr,"sms_getRecord: could not read sms record %d\n", 
		         iRecord);
		return (-1);
	}
	return (SMS_OK);			
}



/* allocate memory for a frame of SMS data
 *
 * SMS_Data *pSmsRecord;	pointer to a frame of SMS data
 * int nTraj;		        number of trajectories in frame
 * int nCoeff;		      number of stochastic coefficients in frame
 * int iPhase;		      whether phase information is in the frame
 * int sizeHop;               the hopsize used for residual resynthesis
 * int stochType;           stochastic resynthesis type
 */
int sms_allocRecord (SMS_Data *pSmsRecord, int nTraj, int nCoeff, int iPhase,
                                       int sizeHop, int stochType)
{
//        printf("sizeHop: %d, stochType: %d \n", sizeHop, stochType);
//        int dataPos = nTraj;
        float *dataPos;  // a marker to locate specific data witin smsData
	/* calculate size of record */
	int sizeData = 2 * nTraj * sizeof(float);
        sizeData += 1 * sizeof(float); //adding one for nSamples
	if (iPhase > 0) sizeData += nTraj * sizeof(float);
	if (nCoeff > 0) sizeData += (nCoeff + 1) * sizeof(float);
        if( stochType == SMS_STOC_WAVE)
        {
                sizeData += sizeHop * sizeof(float);
                pSmsRecord->nSamples = sizeHop;
        }
        else  pSmsRecord->nSamples = 0;
	/* allocate memory for data */
	if ((pSmsRecord->pSmsData = (float *) malloc (sizeData)) == NULL)
		return (SMS_MALLOC);

	/* set the variables in the structure */
	pSmsRecord->nTraj = nTraj;
	pSmsRecord->nCoeff = nCoeff;
	pSmsRecord->sizeData = sizeData; // this should be removed, it is in the header
        /* set pointers to data types within smsData array */
	pSmsRecord->pFFreqTraj = pSmsRecord->pSmsData;
        dataPos =  (float *)(pSmsRecord->pFFreqTraj + nTraj);
	pSmsRecord->pFMagTraj = dataPos;
        dataPos = (float *)(pSmsRecord->pFMagTraj + nTraj);
	if (iPhase > 0)
	{
		pSmsRecord->pFPhaTraj = dataPos;
                dataPos = (float *) (pSmsRecord->pFPhaTraj + nTraj);
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
        if( stochType == SMS_STOC_WAVE)
        {
                pSmsRecord->pFStocWave = dataPos;
                dataPos = (float *)(pSmsRecord->pFStocWave + sizeHop);
        }
        else         pSmsRecord->pFStocWave = NULL;

	return (SMS_OK);			
}

/* function to allocate an SMS data record
 * - this one is used when you have only read the header (opening a file)
 *
 * SMS_Header *pSmsHeader;	   pointer to SMS header
 * SMS_Data *pSmsRecord;     pointer to SMS record
 *
 * returns
 *        SMS_OK      if it could read the data
 *	      SMS_MALLOC  if it could not allocate record
 */
int sms_allocRecordH (SMS_Header *pSmsHeader, SMS_Data *pSmsRecord)
{
	int iPhase = (pSmsHeader->iFormat == SMS_FORMAT_HP ||
	              pSmsHeader->iFormat == SMS_FORMAT_IHP) ? 1 : 0;
        int sizeHop = pSmsHeader->iOriginalSRate / pSmsHeader->iFrameRate;

	return (sms_allocRecord (pSmsRecord, pSmsHeader->nTrajectories, 
                                   pSmsHeader->nStochasticCoeff, iPhase, sizeHop,
                                   pSmsHeader->iStochasticType));
}

/* free the SMS data structure
 * 
 * SMS_Data *pSmsRecord;	pointer to frame of SMS data
 *
 */
void sms_freeRecord (SMS_Data *pSmsRecord)
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
 * SMS_Data *pSmsRecord;	pointer to frame of SMS data
 *
 */
void sms_clearRecord (SMS_Data *pSmsRecord)
{
	memset ((char *) pSmsRecord->pSmsData, 0, pSmsRecord->sizeData);
}
  
/* copy a record of SMS data into another
 *
 * SMS_Data *pCopySmsData;	copy of frame
 * SMS_Data *pOriginalSmsData;	original frame
 *
 */
int sms_copyRecord (SMS_Data *pCopySmsData, SMS_Data *pOriginalSmsData)
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
 * SMS_Data *pSmsRecord1            sms record 1
 * SMS_Data *pSmsRecord2            sms record 2
 * SMS_Data *pSmsRecordOut          sms output record
 * float fInterpFactor              interpolation factor
 *
 */
int sms_interpolateRecords (SMS_Data *pSmsRecord1, SMS_Data *pSmsRecord2,
                           SMS_Data *pSmsRecordOut, float fInterpFactor)
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
	if( 	*(pSmsRecord1->pFStocGain) > 0.00001 )
        {
                ; //blaRg!
        }
	/* interpolate the stochastic part */
	if (pSmsRecordOut->pFStocGain)
        {
                *(pSmsRecordOut->pFStocGain) = 
			*(pSmsRecord1->pFStocGain) + fInterpFactor *
			(*(pSmsRecord2->pFStocGain) - *(pSmsRecord1->pFStocGain));
        }
	if (nCoeff)
        {
                for (i = 0; i < pSmsRecord1->nCoeff; i++)
			pSmsRecordOut->pFStocCoeff[i] = 
				pSmsRecord1->pFStocCoeff[i] + fInterpFactor * 
				(pSmsRecord2->pFStocCoeff[i] - pSmsRecord1->pFStocCoeff[i]);
        }

        if(pSmsRecordOut->pFStocWave)
        {
                memcpy(pSmsRecordOut->pFStocWave, pSmsRecord1->pFStocWave,
                       pSmsRecordOut->nSamples * sizeof(float));
        }
        return 1;
}

const char* sms_errorString(int iError)
{
        /*! \todo make this a switch statement */
        if (iError == SMS_NOPEN)
                return ("cannot open input file");
        if (iError == SMS_NSMS)
                return ("input file not an SMS file");
        if (iError == SMS_MALLOC)
                return ("cannot allocate memory for input file");
        if (iError == SMS_RDERR)
                return ("read error in input file");
        if (iError == SMS_WRERR)
                return ("cannot write output file");
        else 
                return ("error undefined"); 
}

 
