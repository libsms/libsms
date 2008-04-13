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
#define USAGE "Usage: smsClean <inputSmsFile> <outputSmsFile>"

short MaxDelayFrames;
float FResidualPerc;
SOUND_BUFFER soundBuffer, synthBuffer;
ANAL_FRAME **ppFrames, *pFrames;

/*
 * search over all the data of a record for empy slots
 *
 * SMS_DATA smsData     SMS data record
 * float *pFFreq
 * int *pIGoodRecords
 *
 */
void SearchSms (SMS_DATA smsData, float *pFFreq, int *pIGoodRecords)
{
	int i;

	for (i = 0; i < smsData.nTraj; i++)
		if (smsData.pFMagTraj[i] > 0)
		{		
			pFFreq[i] += smsData.pFFreqTraj[i];
			pIGoodRecords[i]++;
		}
}	    

/*
 *   calculate size of record in bytes
 */
int CalcRecordBSize (SMSHeader *pSmsHeader, int nTrajectories)
{
	int iSize = 0, nGain = 1, nComp = 2;
    
	if (pSmsHeader->iStochasticType == STOC_NONE)
		nGain = 0;
    
	if (pSmsHeader->iFormat == FORMAT_HARMONIC_WITH_PHASE ||
	    pSmsHeader->iFormat == FORMAT_INHARMONIC_WITH_PHASE)
		nComp = 3;
     
	iSize = sizeof (float) * (nComp * nTrajectories + 
		pSmsHeader->nStochasticCoeff + nGain);
	return(iSize);
}	     

void SetTraj (float *pFFreq, int inNTraj, 
               int *pITrajOrder, int outNTraj)
{
	int i, j, iTraj;
	float fTmp = 22000, fLow = 0;

	for (i = 0; i < outNTraj; i++)
	{
		for (j = 0; j < inNTraj; j++)
		{
			if (pFFreq[j] > fLow)
				fTmp = MIN (pFFreq[j], fTmp);
			if (fTmp == pFFreq[j])
				iTraj = j;
		}
		fLow = fTmp;
		fTmp = 22000;
		pITrajOrder[i] = iTraj;
	}
}

void CleanSms (SMS_DATA inSmsData, SMS_DATA *pOutSmsData, int *pITrajOrder)
{
	int iTraj, iCoeff;

	for (iTraj = 0; iTraj < pOutSmsData->nTraj; iTraj++)
	{
		pOutSmsData->pFFreqTraj[iTraj] = 
			inSmsData.pFFreqTraj[pITrajOrder[iTraj]];
		pOutSmsData->pFMagTraj[iTraj] = 
			inSmsData.pFMagTraj[pITrajOrder[iTraj]];
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
	SMSHeader *pInSmsHeader, OutSmsHeader;
	FILE *pInSmsFile, *pOutSmsFile;
	float *pFFreq;
	SMS_DATA inSmsData, outSmsData;
	int iError, *pIGoodRecords, *pITrajOrder, iRecord, iGoodTraj = 0, iTraj,
		iRecordBSize, iHeadBSize, iDataBSize;
  
	/* get user arguments */
	if (argc != 3) 
		quit(USAGE);   
    
	pChInputSmsFile = argv[1];
	pChOutputSmsFile = argv[2];

	/* open SMS file and read the header */
	if ((iError = GetSmsHeader (pChInputSmsFile, &pInSmsHeader, 
	                            &pInSmsFile)) < 0)
	{
		if (iError == SMS_NOPEN)
			quit ("cannot open file");
		if (iError == SMS_RDERR)
			quit("read error");
		if (iError == SMS_NSMS)
			quit ("not an SMS file");
		if (iError == SMS_MALLOC)
			quit ("cannot allocate memory");
			quit ("error");
	}	    
  
	if ((pFFreq =  
	     (float *) calloc (pInSmsHeader->nTrajectories, sizeof (float))) == 
		 NULL)
		quit ("error allocating memory for pFFreq");
	if ((pIGoodRecords =  
	     (int *) calloc (pInSmsHeader->nTrajectories, sizeof (int))) == NULL)
		quit ("error allocating memory for pIGoodRecords");

	AllocSmsRecord (pInSmsHeader, &inSmsData);

	for (iRecord = 1; iRecord < pInSmsHeader->nRecords; iRecord++)
	{
		GetSmsRecord (pInSmsFile, pInSmsHeader, iRecord, &inSmsData);
		SearchSms (inSmsData, pFFreq, pIGoodRecords);	
	}
  
	for (iTraj = 0; iTraj < pInSmsHeader->nTrajectories; iTraj++)
		if (pIGoodRecords[iTraj] > 0)
		{
			iGoodTraj++;
			pFFreq[iTraj] /= pIGoodRecords[iTraj];
		}
	
	iRecordBSize = CalcRecordBSize (pInSmsHeader, iGoodTraj);
	iHeadBSize = sizeof (SMSHeader);
	iDataBSize = iRecordBSize * pInSmsHeader->nRecords;
	
	InitSmsHeader (&OutSmsHeader);
	OutSmsHeader.iRecordBSize = iRecordBSize;
	OutSmsHeader.nRecords = pInSmsHeader->nRecords;
	OutSmsHeader.iFormat = pInSmsHeader->iFormat;
	OutSmsHeader.iFrameRate = pInSmsHeader->iFrameRate;
	OutSmsHeader.iStochasticType = pInSmsHeader->iStochasticType;
	OutSmsHeader.nTrajectories = iGoodTraj;
	OutSmsHeader.nStochasticCoeff = pInSmsHeader->nStochasticCoeff;
	OutSmsHeader.iOriginalSRate = pInSmsHeader->iOriginalSRate;
	OutSmsHeader.nTextCharacters = pInSmsHeader->nTextCharacters;
	OutSmsHeader.pChTextCharacters = pInSmsHeader->pChTextCharacters;

	AllocSmsRecord (&OutSmsHeader, &outSmsData);
	if ((pITrajOrder =  
	     (int *) calloc (OutSmsHeader.nTrajectories, sizeof (int))) == NULL)
		quit ("error allocating memory for pITrajOrder");

	SetTraj (pFFreq, pInSmsHeader->nTrajectories, pITrajOrder, 
	         OutSmsHeader.nTrajectories);

	/* create output SMS file and write the header */
	WriteSmsHeader (pChOutputSmsFile, &OutSmsHeader, &pOutSmsFile);
	
	/* iterate over the input file and clean it */
	for (iRecord = 1; iRecord < OutSmsHeader.nRecords; iRecord++)
	{
		GetSmsRecord (pInSmsFile, pInSmsHeader, iRecord, &inSmsData);
		CleanSms (inSmsData, &outSmsData, pITrajOrder);
		WriteSmsRecord (pOutSmsFile, &OutSmsHeader, &outSmsData);
	}
	
	/* rewrite the header and close the output SMS file */
	WriteSmsFile (pOutSmsFile, &OutSmsHeader);

	free (pInSmsHeader);
	free (pFFreq);
	free (pIGoodRecords);
	free (pITrajOrder);
	fclose (pInSmsFile);

	return 0;
}
