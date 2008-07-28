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
/* program written by Celso Mendonca de Aguiar */

#include "sms.h"
#define USAGE "Usage: smsUnDb <inputsmsFile> <outputsmsFile>"

int main (int ac, char *av[])
{
	char *pData, *inputFileName, *outputFileName;
	int j, i, NRec, Ntrajet, counter=0;
	float freqs, printfreqs, freqsm1, freqsp1, Sum=0.0;
	SMS_Header *pSmsHeader;
	SMS_DATA smsData;
  
        initSms(&smsData);
	if (ac != 3) quit(USAGE);
	inputFileName  = av[1];
	outputFileName = av[2];
	if(readSmsFile (inputFileName, &pSmsHeader) < 0) 
	  printf("error");

	Ntrajet = pSmsHeader->nTrajectories;
	NRec    = pSmsHeader->nRecords;
	for(j = 0; j < Ntrajet; j++) 
           {
	    pData = (char *) pSmsHeader + pSmsHeader->iHeadBSize;
	    pData += pSmsHeader->iRecordBSize;
	    for(i = 0; i < NRec; i++) 
               {
          	setSmsRecord (pSmsHeader, pData, &smsData);
          	smsData.pFMagTraj[j] = TO_MAG(smsData.pFMagTraj[j]);
          	pData += pSmsHeader->iRecordBSize;
	       }
	   }
        
        for(j = 0; j < Ntrajet; j++) 
	   {
	    pData = (char *) pSmsHeader + pSmsHeader->iHeadBSize;
	    pData += pSmsHeader->iRecordBSize;
	    freqsm1 = 0.0;
	    setSmsRecord(pSmsHeader, pData, &smsData);
	    freqs = smsData.pFFreqTraj[j];
	    for(i = 0; i < NRec-2; i++) 
	       {
          	pData += pSmsHeader->iRecordBSize;
          	setSmsRecord(pSmsHeader, pData, &smsData);
          	freqsp1 = smsData.pFFreqTraj[j];
          	printfreqs = freqs;
          	if (freqs<0.000001 && freqsm1>0.0) printfreqs = freqsm1;
          	if (freqs<0.000001 && freqsp1>0.0) printfreqs = freqsp1;
          	if (freqs<0.000001) 
          	     smsData.pFFreqTraj[j] = .0;
          	else smsData.pFFreqTraj[j] = printfreqs;
          	freqsm1 = freqs;
          	freqs = freqsp1;			
                }
           }
	
	pData = (char *) pSmsHeader + pSmsHeader->iHeadBSize;
	pData += pSmsHeader->iRecordBSize;
        setSmsRecord(pSmsHeader, pData, &smsData);
	for(i = 0; i < NRec; i++) 
	   {
            freqs = smsData.pFFreqTraj[0];
            Sum += freqs;
            if (freqs>0) counter++;
            pData += pSmsHeader->iRecordBSize;
            setSmsRecord(pSmsHeader, pData, &smsData);
	   }
	pSmsHeader->fFrequency =  Sum / counter;
	writeSmsFile (outputFileName, pSmsHeader);
        free(pSmsHeader);
        return(1);
}
