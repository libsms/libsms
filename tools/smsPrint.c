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


#define PRINT_ALL 1
#define PRINT_DET 2
#define PRINT_STOC 3
#define PRINT_HDR 4  //TODO: print only header
#define USAGE "Usage: smsPrint [-t type-format][-i initial-time][-e end-time][-f first-trajectory][-l last-trajectory] <smsFile>"



short MaxDelayFrames;
float FResidualPerc;
SOUND_BUFFER soundBuffer, synthBuffer;
ANAL_FRAME **ppFrames, *pFrames;

int main (int argc, char *argv[])
{
	SMSHeader *pSmsHeader;
	FILE *pSmsFile;
	SMS_DATA smsData;
	int iError, i, j, iFormat = 1, iFirstFrame = 0, iLastFrame = -1, 
		iFirstTraj = 0, iLastTraj = -1;
    float fInitialTime = 0, fEndTime = 0;

	for (i=1; i<argc-1; i++) 
	{
		if (*(argv[i]++) == '-') 
		{
			switch (*(argv[i]++)) 
			{
				case 't': if (sscanf(argv[i],"%d", 
				              &iFormat) < 1)
					quit("Invalid format");
					break;
				case 'i': if (sscanf(argv[i],"%f", 
				              &fInitialTime) < 0)
					quit("Invalid initialTime");
					break;
				case 'e': if (sscanf(argv[i],"%f", 
				              &fEndTime) < 0)
					quit("Invalid EndTime");
					break;
				case 'f': if (sscanf(argv[i],"%d", 
				              &iFirstTraj) < 0)
					quit("Invalid FirstTraj");
					break;
				case 'l': if (sscanf(argv[i],"%d", 
				              &iLastTraj) < 0)
					quit("Invalid LastTraj");
					break;

				default:   quit(USAGE);
			}
		}
	}

	if (argc <= 1) 
		quit(USAGE);

	if((iError = GetSmsHeader (argv[argc-1], &pSmsHeader, &pSmsFile)) < 0)
	{
		if(iError == SMS_NOPEN)
			quit("cannot open file");
		if(iError == SMS_RDERR)
			quit("read error");
		if(iError == SMS_NSMS)
			quit("not an SMS file");
		if(iError == SMS_MALLOC)
			quit("cannot allocate memory");
		quit("error");
	}	    
    
	AllocSmsRecord (pSmsHeader, &smsData);

	printf("\nHEADER INFORMATION:\n");
	printf("Number of records = %d\n", pSmsHeader->nRecords);
	printf("Frame rate (Hz) = %d\n", pSmsHeader->iFrameRate);
	printf("Number of trajectories = %d\n", pSmsHeader->nTrajectories);
	printf("Number of stochastic coefficients = %d\n",
    	   pSmsHeader->nStochasticCoeff);
        if(pSmsHeader->iFormat == 1) printf("Format = harmonic\n");
        else if(pSmsHeader->iFormat == 2) printf("Format = inharmonic\n");
        else if(pSmsHeader->iFormat == 3) printf("Format = harmonic with phase\n");
        else if(pSmsHeader->iFormat == 4) printf("Format = inharmonic with phase\n");
	if(pSmsHeader->iStochasticType == 0) printf("Stochastic type = waveform\n");
	else if(pSmsHeader->iStochasticType == 1) printf("Stochastic type = STFT\n");
	else if(pSmsHeader->iStochasticType == 2)
                printf("Stochastic type = line segment magnitude spectrum approximation \n");
	else if(pSmsHeader->iStochasticType == 3) printf("Stochastic type = none\n");
	printf("Original sampling rate = %d\n", pSmsHeader->iOriginalSRate);  

	if (pSmsHeader->nTextCharacters > 0)
	{
		printf("\nANALISIS ARGUMENTS:\n");
		printf("%s\n", pSmsHeader->pChTextCharacters);
	} 

	iFirstFrame = 
		MIN (pSmsHeader->nRecords - 1, fInitialTime * pSmsHeader->iFrameRate);
	if (fEndTime > 0) 
		iLastFrame = 
		MIN (fEndTime * pSmsHeader->iFrameRate, pSmsHeader->nRecords);
	else
		iLastFrame = pSmsHeader->nRecords; 

	if (iFirstTraj > 0)
		iFirstTraj = MIN (pSmsHeader->nTrajectories, iFirstTraj);

	if (iLastTraj >= 0)
		iLastTraj = MIN (pSmsHeader->nTrajectories, iLastTraj);
	else
		iLastTraj = pSmsHeader->nTrajectories;

        if(iFormat != PRINT_HDR)
        {
                for(i = iFirstFrame; i < iLastFrame; i++)
                {
                                GetSmsRecord (pSmsFile, pSmsHeader, i, &smsData);
                                printf("\nFrame #%d {%1.3fs}: ", i, (float) i / pSmsHeader->iFrameRate);
                                if (iFormat != PRINT_STOC) 
                                {
                                        printf("\n    det:\n");
                                        if((pSmsHeader->iFormat == FORMAT_HARMONIC ||
                                            pSmsHeader->iFormat == FORMAT_INHARMONIC))
                                        {
                                                for(j = iFirstTraj; j < iLastTraj; j++)
                                                        printf("%5.2f[%2.4f]  ", smsData.pFFreqTraj[j], smsData.pFMagTraj[j]);
                                        }
                                        else 
                                        {
                                                for(j = iFirstTraj; j < iLastTraj; j++)
                                                        printf("%5.2f[%2.4f, %2.4f]  ", smsData.pFFreqTraj[j], 
                                                               smsData.pFMagTraj[j], smsData.pFPhaTraj[j]);
                                        }
                                }
                                if(iFormat != PRINT_DET && pSmsHeader->iStochasticType != STOC_NONE)
                                        {	
                                                if(pSmsHeader->iStochasticType == STOC_WAVEFORM)
                                                {
                                                        printf("\n    stoc_wave:\n");
                                                        for( j = 0; j < ( pSmsHeader->iOriginalSRate / pSmsHeader->iFrameRate); j++)
                                                                printf("%f, ", *(smsData.pFStocWave));
                                                }
                                                else if(pSmsHeader->iStochasticType == STOC_STFT)
                                                 {

                                                 }
                                                else if( pSmsHeader->iStochasticType == STOC_APPROX )
                                                {
                                                        printf("\n    stoc_gain: %f\n", *(smsData.pFStocGain));
                                                        printf("stoc_coefficients: ");
                                                        for(j = 0; j < smsData.nCoeff; j++)
                                                                printf("%1.3f  ", smsData.pFStocCoeff[j]);
                                                }   
                                        }
                                printf("\n");
                }

        }

        free (pSmsHeader);
        fclose (pSmsFile);
        return(1);
}