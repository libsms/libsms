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
#define PRINT_HDR 4

#define USAGE "Usage: smsToYaml [-t type-format][-i initial-time][-e end-time] <smsFile> <yamlFile>"

void usage (void)
{
        fprintf (stderr, "\n"
                 "Usage: smsToYaml [options]  <inputSmsFile> <outputYamlFile>\n"
                 "\n"
                 "Options:\n"
                 " -t      type-format (default 1) \n"
                 "                    1: print all the information \n"
                 "                    2: print only the deterministic component \n"
                 "                    3: print only the stochastic component. \n"
                 "                    4: print only the header. \n"
                 "-i       initial time (default: 0) \n"
                 "-e       end time (default: end of file) \n"
                 "\n"
                 "Converts an SMS analysis file (.sms) made with smsAnal to a YAML formatted text file."
                 "\n\n");
        
        exit(1);
}


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
     int iError, i, j, nSamples, iFormat = 1, iFirstFrame = 0, iLastFrame = -1, 
          iFirstTraj = 0, iLastTraj = -1;
     float fInitialTime = 0, fEndTime = 0;
        
     if (argc <= 2) usage();
     pChOutputYamlFile = argv[argc-1];
     pChInputSmsFile = argv[argc-2];
     for (i=1; i<argc-1; i++) 
     {
          if (*(argv[i]++) == '-') 
          {
               switch (*(argv[i]++)) 
               {
               case 't': if (sscanf(argv[i],"%d", &iFormat) < 1)
                         quit("Invalid format");
                    break;
               case 'i': if (sscanf(argv[i],"%f", 
                                    &fInitialTime) < 0)
                         quit("Invalid initialTime");
                    else printf("initialTime: %f\n", fInitialTime);
                    break;
               case 'e': if (sscanf(argv[i],"%f", 
                                    &fEndTime) < 0)
                         quit("Invalid EndTime");
                    break;				
               default:   usage();
               }
          }
     }
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
     fprintf(fp,"smsHeader:\n");
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
     else if(pSmsHeader->iStochasticType == STOC_IFFT) 
          fprintf(fp,"stft\n");
     else if(pSmsHeader->iStochasticType == STOC_APPROX) 
          fprintf(fp,"approx\n");
     else if(pSmsHeader->iStochasticType == STOC_NONE) 
          fprintf(fp,"none\n");
     fprintf(fp,"    iOriginalSRate   : %d\n", pSmsHeader->iOriginalSRate);  


     //:::::::::::: Write Analysis Arguments :::::::::::::::
     if (pSmsHeader->nTextCharacters > 0)
     {
          char cOneLine[100];
          int t, iStrStart, nBytes;
          fprintf(fp,"\nanalysis_arguments: >\n");
          iStrStart = 0;
          for(t = 0; t < pSmsHeader->nTextCharacters; t++)
          {
               if(pSmsHeader->pChTextCharacters[t] == ',' && (t - iStrStart) > 60)
               {   
                    memset(cOneLine, 0, 100);
                    nBytes = t + 1 - iStrStart;
                    strncpy (cOneLine, pSmsHeader->pChTextCharacters+ iStrStart, nBytes);
                    fprintf(fp,"    %s\n", cOneLine);
                    iStrStart = t+2;
               }
               else if(pSmsHeader->nTextCharacters == t - 1)
               {
                    memset(cOneLine, 0, 100);
                    nBytes = t + 1 - iStrStart;
                    strncpy (cOneLine, pSmsHeader->pChTextCharacters+ iStrStart, nBytes);
                    fprintf(fp,"    %s\n", cOneLine);
               }
          }
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
          //:::::::::::: Write Data :::::::::::::::
          fprintf(fp, "\nsmsData:\n");
          for(i = iFirstFrame; i < iLastFrame; i++)
          {
               GetSmsRecord (pSmsFile, pSmsHeader, i, &smsData);
               fprintf(fp,"\n  - frame    : %d \n", i);
               fprintf(fp,"    timetag  : %f \n", (float) i / pSmsHeader->iFrameRate);
               if(iFormat != 3)
               {
                    if(pSmsHeader->iFormat == FORMAT_HARMONIC ||
                       pSmsHeader->iFormat == FORMAT_HARMONIC_WITH_PHASE)
                         fprintf(fp,"    harmonics:\n");
                    if(pSmsHeader->iFormat == FORMAT_INHARMONIC ||
                       pSmsHeader->iFormat == FORMAT_INHARMONIC_WITH_PHASE)
                         fprintf(fp,"    tracks:\n");
                    if (pSmsHeader->iFormat == FORMAT_HARMONIC ||
                        pSmsHeader->iFormat == FORMAT_INHARMONIC)
                    {	
                         for(j = iFirstTraj; j < iLastTraj; j++)
                         {
                              if(smsData.pFMagTraj[j] > 0.00000001 )
                                   fprintf(fp, "       %-4d: [%12f, %12f]  \n", j,
                                           smsData.pFFreqTraj[j], smsData.pFMagTraj[j]);
                         }
                    }
                    else
                    {
                         for(j = iFirstTraj; j < iLastTraj; j++)
                         {
                              if(smsData.pFMagTraj[j] > 0.00000001 )
                                   fprintf(fp,"        %-4d: [%12f, %12f, %12f]  \n", j,
                                           smsData.pFFreqTraj[j], smsData.pFMagTraj[j], smsData.pFPhaTraj[j]);	
                         }
                    }
               }
		
               if (iFormat != PRINT_DET && pSmsHeader->iStochasticType != STOC_NONE)
               {	
                    if(pSmsHeader->iStochasticType == STOC_WAVEFORM)
                    {
                         nSamples = pSmsHeader->iOriginalSRate / pSmsHeader->iFrameRate;
                         fprintf(fp,"    stocWave: [ ");
                         for( j = 0; j < nSamples; j++)
                         {
                              //print 4 values a line
                              if( j !=0 && j%4 == 0) fprintf(fp, ",\n                        ");
                              else if( j != 0) fprintf(fp, ", ");
                              fprintf(fp,"%12f", smsData.pFStocWave[j]);
                         }
                         fprintf(fp," ]\n");
                    }
                    else if(pSmsHeader->iStochasticType == STOC_IFFT)
                    {

                    }
                    else if( pSmsHeader->iStochasticType == STOC_APPROX )
                    {
                         fprintf(fp,"    stocGain: %f\n", *(smsData.pFStocGain));
                         fprintf(fp,"    stocCoefficients: [");
                         for(j = 0; j < smsData.nCoeff; j++)
                         {
                              if(j%4 == 0 && j !=0)
                                   fprintf(fp,",\n                       ");
                              else if( j !=0)
                                   fprintf(fp,", ");
                              fprintf(fp,"%9f", smsData.pFStocCoeff[j]);
                         }
                         fprintf(fp," ]\n");
                    }   
               }
          }
     }
     free (pSmsHeader);
     fclose (fp);
     fclose (pSmsFile);
     return(1);
}
