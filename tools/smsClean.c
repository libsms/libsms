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
 *
 *    main program for smsClean
 *
 */
#include "sms.h"
//#define USAGE "Usage: smsClean <inputSmsFile> <outputSmsFile>"

short MaxDelayFrames;
float FResidualPerc;
SMS_SndBuffer soundBuffer, synthBuffer;
SMS_AnalFrame **ppFrames, *pFrames;

void usage (void)
{
        fprintf (stderr, "\n"
                 "Usage: smsClean <inputSmsFile> <outputSmsFile>"
                 "\n\n");
        exit(1);
}

/*
 * search over all the data of a record for empy slots
 *
 * SMS_Data smsData     SMS data record
 * float *pFFreq
 * int *pIGoodRecords
 *
 */
void SearchSms (SMS_Data smsData, float *pFFreq, int *pIGoodRecords)
{
	int i;

	for (i = 0; i < smsData.nTracks; i++)
		if (smsData.pFSinAmp[i] > 0)
		{		
			pFFreq[i] += smsData.pFSinFreq[i];
			pIGoodRecords[i]++;
		}
}	    

/*
 *   calculate size of record in bytes
 */
int CalcRecordBSize (SMS_Header *pSmsHeader, int nTrajectories)
{
	int iSize = 0, nGain = 1, nComp = 2;
    
	if (pSmsHeader->iStochasticType == SMS_STOC_NONE)
		nGain = 0;
    
	if (pSmsHeader->iFormat == SMS_FORMAT_HP ||
	    pSmsHeader->iFormat == SMS_FORMAT_IHP)
		nComp = 3;
     
	iSize = sizeof (float) * (nComp * nTrajectories + 
		pSmsHeader->nStochasticCoeff + nGain);
	return(iSize);
}	     

void SetTraj (float *pFFreq, int inNTraj, 
               int *pITrajOrder, int outNTraj)
{
	int i, j;
        int iTrack = 0;
	float fTmp = 22000, fLow = 0;

	for (i = 0; i < outNTraj; i++)
	{
		for (j = 0; j < inNTraj; j++)
		{
			if (pFFreq[j] > fLow)
				fTmp = MIN (pFFreq[j], fTmp);
			if (fTmp == pFFreq[j])
				iTrack = j;
		}
		fLow = fTmp;
		fTmp = 22000;
		pITrajOrder[i] = iTrack;
	}
}

void CleanSms (SMS_Data inSmsData, SMS_Data *pOutSmsData, int *pITrajOrder)
{
	int iTrack, iCoeff;

	for (iTrack = 0; iTrack < pOutSmsData->nTracks; iTrack++)
	{
		pOutSmsData->pFSinFreq[iTrack] = 
			inSmsData.pFSinFreq[pITrajOrder[iTrack]];
		pOutSmsData->pFSinAmp[iTrack] = 
			inSmsData.pFSinAmp[pITrajOrder[iTrack]];
	}

	if (inSmsData.nCoeff > 0)
	{
		pOutSmsData->nCoeff = inSmsData.nCoeff;
			*(pOutSmsData->pFStocGain) = *(inSmsData.pFStocGain);

		for (iCoeff = 0; iCoeff < pOutSmsData->nCoeff; iCoeff++)
			pOutSmsData->pFStocCoeff[iCoeff] = 
				inSmsData.pFStocCoeff[iCoeff];
	}
}

int main (int argc, char *argv[])
{
	char *pChInputSmsFile = NULL, *pChOutputSmsFile = NULL;
	SMS_Header *pInSmsHeader, OutSmsHeader;
	FILE *pInSmsFile, *pOutSmsFile;
	float *pFFreq;
	SMS_Data inSmsData, outSmsData;
	int iError, *pIGoodRecords, *pITrajOrder, iRecord, iGoodTraj = 0, iTrack,
		iFrameBSize, iHeadBSize, iDataBSize;
  
	/* get user arguments */
	if (argc != 3) usage();
		
    
	pChInputSmsFile = argv[1];
	pChOutputSmsFile = argv[2];

	/* open SMS file and read the header */
	if ((iError = sms_getHeader (pChInputSmsFile, &pInSmsHeader, 
	                            &pInSmsFile)) < 0)
	{
                printf("error in sms_getHeader: %s", sms_errorString());
                exit(EXIT_FAILURE);
	}	    
  
	if ((pFFreq =  
	     (float *) calloc (pInSmsHeader->nTracks, sizeof (float))) == 
		 NULL)
        {
                printf("error allocating memory for pFFreq: %s", sms_errorString());
                exit(EXIT_FAILURE);
        }
	if ((pIGoodRecords =  
	     (int *) calloc (pInSmsHeader->nTracks, sizeof (int))) == NULL)
        {
                printf("error allocating memory for plGoodRecords: %s", sms_errorString());
                exit(EXIT_FAILURE);
        }

	sms_allocFrameH (pInSmsHeader, &inSmsData);

	for (iRecord = 1; iRecord < pInSmsHeader->nFrames; iRecord++)
	{
		sms_getFrame (pInSmsFile, pInSmsHeader, iRecord, &inSmsData);
		SearchSms (inSmsData, pFFreq, pIGoodRecords);	
	}
  
	for (iTrack = 0; iTrack < pInSmsHeader->nTracks; iTrack++)
		if (pIGoodRecords[iTrack] > 0)
		{
			iGoodTraj++;
			pFFreq[iTrack] /= pIGoodRecords[iTrack];
		}
	
	iFrameBSize = CalcRecordBSize (pInSmsHeader, iGoodTraj);
	iHeadBSize = sizeof (SMS_Header);
	iDataBSize = iFrameBSize * pInSmsHeader->nFrames;
	
	sms_initHeader (&OutSmsHeader);
	OutSmsHeader.iFrameBSize = iFrameBSize;
	OutSmsHeader.nFrames = pInSmsHeader->nFrames;
	OutSmsHeader.iFormat = pInSmsHeader->iFormat;
	OutSmsHeader.iFrameRate = pInSmsHeader->iFrameRate;
	OutSmsHeader.iStochasticType = pInSmsHeader->iStochasticType;
	OutSmsHeader.nTracks = iGoodTraj;
	OutSmsHeader.nStochasticCoeff = pInSmsHeader->nStochasticCoeff;
	OutSmsHeader.iSamplingRate = pInSmsHeader->iSamplingRate;
	OutSmsHeader.nTextCharacters = pInSmsHeader->nTextCharacters;
	OutSmsHeader.pChTextCharacters = pInSmsHeader->pChTextCharacters;
	OutSmsHeader.iEnvType = pInSmsHeader->iEnvType;
	OutSmsHeader.nEnvCoeff = pInSmsHeader->nEnvCoeff;
	OutSmsHeader.iMaxFreq = pInSmsHeader->iMaxFreq;


	sms_allocFrameH (&OutSmsHeader, &outSmsData);
	if ((pITrajOrder =  
	     (int *) calloc (OutSmsHeader.nTracks, sizeof (int))) == NULL)
        {
                printf("error allocating memory for pITrajOrder: %s", sms_errorString());
                exit(EXIT_FAILURE);
        }

	SetTraj (pFFreq, pInSmsHeader->nTracks, pITrajOrder, 
	         OutSmsHeader.nTracks);

	/* create output SMS file and write the header */
	sms_writeHeader (pChOutputSmsFile, &OutSmsHeader, &pOutSmsFile);
	
	/* iterate over the input file and clean it */
	for (iRecord = 1; iRecord < OutSmsHeader.nFrames; iRecord++)
	{
		sms_getFrame (pInSmsFile, pInSmsHeader, iRecord, &inSmsData);
		CleanSms (inSmsData, &outSmsData, pITrajOrder);
		sms_writeFrame (pOutSmsFile, &OutSmsHeader, &outSmsData);
	}
	
	/* rewrite the header and close the output SMS file */
	sms_writeFile (pOutSmsFile, &OutSmsHeader);

	free (pInSmsHeader);
	free (pFFreq);
	free (pIGoodRecords);
	free (pITrajOrder);
	fclose (pInSmsFile);

	return 0;
}
