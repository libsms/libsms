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

#define USAGE "Usage: smsToYaml [-t type-format][-i initial-time][-e end-time] <smsFile> <yamlFile>"

short MaxDelayFrames;
float FResidualPerc;
SOUND_BUFFER soundBuffer, synthBuffer;
ANAL_FRAME **ppFrames, *pFrames;

int main (int argc, char *argv[])
{
	char *pChInputSmsFile = NULL, *pChOutputYamlFile = NULL;
	SMSHeader *pSmsHeader;
	FILE *pSmsFile, *fp;
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
				default:   quit(USAGE);
			}
		}
	}

	if (argc <= 2) 
		quit(USAGE);
        
    pChInputSmsFile = argv[argc-1];
	pChOutputYamlFile = argv[argc-2];
      
	if((iError = GetSmsHeader (pChInputSmsFile, &pSmsHeader, &pSmsFile)) < 0)
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
	
	fp = fopen(pChOutputYamlFile, "w");

    //:::::::::: write Header ::::::::::::::::
    printf("writing %s.\n", pChOutputYamlFile);
	fprintf(fp,"Header:\n");
	fprintf(fp,"    nRecords         : %d\n", pSmsHeader->nRecords);
	fprintf(fp,"    iFrameRate       : %d\n", pSmsHeader->iFrameRate);
	fprintf(fp,"    nTrajectories    : %d\n", pSmsHeader->nTrajectories);
	fprintf(fp,"    nStochasticCoeff : %d\n", pSmsHeader->nStochasticCoeff);
    fprintf(fp,"    iFormat          : ");
    if(pSmsHeader->iFormat == FORMAT_HARMONIC) 
        fprintf(fp,"harmonic\n");
    else if(pSmsHeader->iFormat == FORMAT_INHARMONIC) 
        fprintf(fp,"inharmonic\n");
    else if(pSmsHeader->iFormat == FORMAT_HARMONIC_WITH_PHASE) 
        fprintf(fp,"harmonic_with_phase\n");
    else if(pSmsHeader->iFormat == FORMAT_INHARMONIC_WITH_PHASE) 
        fprintf(fp,"inharmonic_with_phase\n");
	fprintf(fp,"    iStochasticType  : ");
    if(pSmsHeader->iStochasticType == STOC_WAVEFORM) 
        fprintf(fp,"waveform\n");
	else if(pSmsHeader->iStochasticType == STOC_STFT) 
        fprintf(fp,"stft\n");
    else if(pSmsHeader->iStochasticType == STOC_APPROX) 
        fprintf(fp,"approx\n");
    else if(pSmsHeader->iStochasticType == STOC_NONE) 
        fprintf(fp,"none\n");
	fprintf(fp,"    iOriginalSRate   : %d\n", pSmsHeader->iOriginalSRate);  


    //:::::::::::: Write Analysis Arguments :::::::::::::::
	if (pSmsHeader->nTextCharacters > 0)
	{
		char *arg=NULL, *value=NULL;
        fprintf(fp,"\n\nanalysis_arguments:\n");
		
        printf("\nanalysis_arguments:\n");
        for(i = 0; i < pSmsHeader->nTextCharacters; i++)
        {
            if(pSmsHeader->pChTextCharacters[i] == ',')
            {
                printf("    arg: %s, value: %s \n", arg, value);
            }
        }
        //fprintf(fp,"    %s\n", pSmsHeader->pChTextCharacters);
	} 
    
    return 1;
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


	for(i = iFirstFrame; i < iLastFrame; i++)
	{
		GetSmsRecord (pSmsFile, pSmsHeader, i, &smsData);
		printf("\nFrame #%d {%1.3fs}:\n", i, (float) i / pSmsHeader->iFrameRate);
		printf("Deterministic:\n");
		if (iFormat != 3 && 
		    (pSmsHeader->iFormat == FORMAT_HARMONIC ||
		    pSmsHeader->iFormat == FORMAT_INHARMONIC))
		{	
			printf("Traj.:  Frequency  Magnitude\n");
			for(j = iFirstTraj; j < iLastTraj; j++)
			{
			    if(smsData.pFMagTraj[j] > 0.00000001 )
					printf("%d: %5.2f   %2.4f  \n", j, smsData.pFFreqTraj[j], smsData.pFMagTraj[j]);
			}
		}
		else
		{
			printf("Traj.:  Frequency  Magnitude  Phase\n");		
			for(j = iFirstTraj; j < iLastTraj; j++)
			{
				if(smsData.pFMagTraj[j] > 0.00000001 )
					printf("%d: %.0f  %.0f   %.0f  \n", j, smsData.pFFreqTraj[j], 
						smsData.pFMagTraj[j], smsData.pFPhaTraj[j]);	
			}
		}
		
		if (iFormat != 2 && pSmsHeader->iStochasticType != STOC_NONE)
		{	
			printf("\n");
			printf("stocg: %.0f\n", *(smsData.pFStocGain));
      		printf("stocc: ");
      		for(j = 0; j < smsData.nCoeff; j++)
        	printf("%1.3f  ", smsData.pFStocCoeff[j]);
    	}   
  	}

		free (pSmsHeader);
        fclose (fp);
		fclose (pSmsFile);
		return(1);
}
