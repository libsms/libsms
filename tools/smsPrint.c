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
#include <popt.h>

const char *help_header_text =
        "\n\n"
        "Prints an SMS analysis file made with libsms, specified as the last argument.\n"
        "If -o FILENAME or --output FILENAME is provided, the output will be written to a text file.\n"
        "The output is printed according to the YAML specification.\n"
        "\n [OPTIONS] \n";

enum  PRINT_TYPE
{
        PRINT_ALL,
        PRINT_DET,
        PRINT_STOC,
        PRINT_ENV,
        PRINT_HDR,
};

int main (int argc, const char *argv[])
{
	int optc;   /* switch */
        const char *pInName = NULL, *pOutName = NULL;
        SMS_Header *pSmsHeader = NULL;
        FILE *pSmsFile, *fp;
        SMS_Data smsData;
        int iError, i, j, iFormat = PRINT_ALL, iFirstFrame = 0, iLastFrame = -1, 
                iFirstTraj = 0, iLastTraj = -1, inDB = 0;
        float fInitialTime = 0, fEndTime = 0, mag;
	poptContext pc;

        struct poptOption	options[] =
                {
                        {"type", 't', POPT_ARG_INT, &iFormat, 0, 
                         "print type: 0=all (default), 1=deterministic, 2=residual, 3=envelope, 4=header", "int"},
                        {"initial-time", 'i', POPT_ARG_FLOAT, &fInitialTime, 0, 
                         "initial time", "float"},
                        {"end-time", 'e', POPT_ARG_FLOAT, &fEndTime, 0, 
                         "end time", "float"},
                        {"decibels", 'd', POPT_ARG_NONE, &inDB, 0, 
                         "make magnitude values printed i decibels", 0},
                        {"output", 'o', POPT_ARG_STRING, &pOutName, 0, 
                         "output text file name", "FILENAME"},
                        POPT_AUTOHELP
                        POPT_TABLEEND
                };

        pc = poptGetContext("smsPrint", argc, argv, options, 0);
        poptSetOtherOptionHelp(pc, help_header_text);

        if (argc <= 1)
        {
                poptPrintUsage(pc,stderr,0);
                return 1;
        }

        while ((optc = poptGetNextOpt(pc)) > 0) {
/*                 switch (optc) { */
/*                         /\* specific arguments are handled here *\/ */
/*                 } */
        }
        if (optc < -1) 
        {
                /* an error occurred during option processing */
                printf("%s: %s\n",
                       poptBadOption(pc, POPT_BADOPTION_NOALIAS),
                       poptStrerror(optc));
                return 1;
        }
        pInName = poptGetArg(pc);

        if(pOutName == NULL)
                fp = (FILE *) stdout;
        else
        {
                fp = fopen(pOutName, "w");
                printf("writing %s.\n", pOutName);
        }

        sms_getHeader ((char *) pInName, &pSmsHeader, &pSmsFile);
        if(sms_errorCheck())
        {
                printf("error in sms_getHeader: %s \n", sms_errorString());
                printf("failed when trying to open file %s \n", pInName );
                exit(EXIT_FAILURE);
        }	    
        sms_allocFrameH (pSmsHeader, &smsData);
        if(sms_errorCheck())
        {
                printf("error in sms_allocFrameH: %s \n", sms_errorString());
                exit(EXIT_FAILURE);
        }	    
	
        //:::::::::: write Header ::::::::::::::::
        fprintf(fp,"smsHeader:\n");
        fprintf(fp,"    nFrames         : %d\n", pSmsHeader->nFrames);
        fprintf(fp,"    iFrameRate       : %d\n", pSmsHeader->iFrameRate);
        fprintf(fp,"    nTracks    : %d\n", pSmsHeader->nTracks);
        fprintf(fp,"    nStochasticCoeff : %d\n", pSmsHeader->nStochasticCoeff);
        fprintf(fp,"    iFormat          : ");
        if(pSmsHeader->iFormat == SMS_FORMAT_H) 
                fprintf(fp,"harmonic\n");
        else if(pSmsHeader->iFormat == SMS_FORMAT_IH) 
                fprintf(fp,"inharmonic\n");
        else if(pSmsHeader->iFormat == SMS_FORMAT_HP) 
                fprintf(fp,"harmonic_with_phase\n");
        else if(pSmsHeader->iFormat == SMS_FORMAT_IHP) 
                fprintf(fp,"inharmonic_with_phase\n");
        fprintf(fp,"    iStochasticType  : ");
        if(pSmsHeader->iStochasticType == SMS_STOC_IFFT) 
                fprintf(fp,"stft\n");
        else if(pSmsHeader->iStochasticType == SMS_STOC_APPROX) 
                fprintf(fp,"approx\n");
        else if(pSmsHeader->iStochasticType == SMS_STOC_NONE) 
                fprintf(fp,"none\n");
        fprintf(fp,"    nEnvCoeff : %d\n", pSmsHeader->nEnvCoeff);
        fprintf(fp,"    iMaxFreq : %d\n", pSmsHeader->iMaxFreq);
        fprintf(fp,"    iEnvType  : ");
        if(pSmsHeader->iEnvType == SMS_ENV_CEP) 
                fprintf(fp,"cepstrum\n");
        else if(pSmsHeader->iEnvType == SMS_ENV_FBINS) 
                fprintf(fp,"fbins\n");
        else if(pSmsHeader->iEnvType == SMS_ENV_NONE) 
                fprintf(fp,"none\n");
        fprintf(fp,"    iSamplingRate   : %d\n", pSmsHeader->iSamplingRate);  

        //:::::::::::: Write Analysis Arguments :::::::::::::::
        if (pSmsHeader->nTextCharacters > 0)
        {
                char cOneLine[100];
                int t, iStrStart, nBytes;
                fprintf(fp,"\ntext_string: >\n");
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
                MIN (pSmsHeader->nFrames - 1, fInitialTime * pSmsHeader->iFrameRate);
        if (fEndTime > 0) 
                iLastFrame = 
                        MIN (fEndTime * pSmsHeader->iFrameRate, pSmsHeader->nFrames);
        else
                iLastFrame = pSmsHeader->nFrames; 

        if (iFirstTraj > 0)
                iFirstTraj = MIN (pSmsHeader->nTracks, iFirstTraj);

        if (iLastTraj >= 0)
                iLastTraj = MIN (pSmsHeader->nTracks, iLastTraj);
        else
                iLastTraj = pSmsHeader->nTracks;
        if(iFormat != PRINT_HDR)
        {
                //:::::::::::: Write Data :::::::::::::::
                fprintf(fp, "\nsmsData:\n");
                for(i = iFirstFrame; i < iLastFrame; i++)
                {
                        sms_getFrame (pSmsFile, pSmsHeader, i, &smsData);
                        if(sms_errorCheck())
                        {
                                printf("error in sms_getFrame: %s \n", sms_errorString());
                                exit(EXIT_FAILURE);
                        }	    
                        fprintf(fp,"\n  - frame    : %d \n", i);
                        fprintf(fp,"    timetag  : %f \n", (float) i / pSmsHeader->iFrameRate);
                        if(iFormat == PRINT_DET || iFormat == PRINT_ALL)
                        {
                                if(pSmsHeader->iFormat == SMS_FORMAT_H ||
                                   pSmsHeader->iFormat == SMS_FORMAT_HP)
                                        fprintf(fp,"    harmonics:\n");
                                if(pSmsHeader->iFormat == SMS_FORMAT_IH ||
                                   pSmsHeader->iFormat == SMS_FORMAT_IHP)
                                        fprintf(fp,"    tracks:\n");
                                if (pSmsHeader->iFormat == SMS_FORMAT_H ||
                                    pSmsHeader->iFormat == SMS_FORMAT_IH)
                                {	
                                        for(j = iFirstTraj; j < iLastTraj; j++)
                                        {
                                                if(smsData.pFSinAmp[j] > 0.00000001 )
                                                        fprintf(fp, "       %-4d: [%12f, %12f]  \n", j,
                                                                smsData.pFSinFreq[j], smsData.pFSinAmp[j]);
                                        }
                                }
                                else
                                {
                                        for(j = iFirstTraj; j < iLastTraj; j++)
                                        {
                                                if(smsData.pFSinAmp[j] > 0.00000001 )
                                                        fprintf(fp,"        %-4d: [%12f, %12f, %12f]  \n", j,
                                                                smsData.pFSinFreq[j], smsData.pFSinAmp[j], smsData.pFSinPha[j]);	
                                        }
                                }
                        }
		
                        if (iFormat == PRINT_STOC || iFormat == PRINT_ALL) /* \todo will this crash if no stoc component? */
                        {	
                                if( pSmsHeader->iStochasticType == SMS_STOC_APPROX )
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
                                else if(pSmsHeader->iStochasticType == SMS_STOC_IFFT)
                                {
                                        fprintf(fp,"    stocGain: %f\n", *(smsData.pFStocGain));
                                        fprintf(fp,"    residual: [");
                                        for(j = 0; j < smsData.nCoeff; j++)
                                        {
                                                if(j%4 == 0 && j !=0)
                                                        fprintf(fp,",\n                       ");
                                                else if( j !=0)
                                                        fprintf(fp,", ");
                                                fprintf(fp,"[%9f, %9f]", smsData.pFStocCoeff[j], smsData.pResPhase[j]);
                                        }
                                        fprintf(fp," ]\n");
                                }
                        }
                        if (iFormat == PRINT_ALL || iFormat == PRINT_ENV)
                        {
                                if (pSmsHeader->iEnvType == SMS_ENV_CEP)
                                {
                                        fprintf(fp,"    cepstrumCoefficients: [");
                                        for(j = 0; j < smsData.nEnvCoeff; j++)
                                        {
                                                if(j%4 == 0 && j !=0)
                                                        fprintf(fp,",\n                       ");
                                                else if( j !=0)
                                                        fprintf(fp,", ");
                                                fprintf(fp,"%9f", smsData.pSpecEnv[j]);
                                        }
                                        fprintf(fp," ]\n");
                                }
                                if (pSmsHeader->iEnvType == SMS_ENV_FBINS)
                                {
                                        fprintf(fp,"    specEnv: [");
                                        for(j = 0; j < smsData.nEnvCoeff; j++)
                                        {
                                                if(inDB) 
                                                        mag = sms_magToDB(smsData.pSpecEnv[j]);
                                                else 
                                                        mag = smsData.pSpecEnv[j];
                                                if(j%4 == 0 && j !=0)
                                                        fprintf(fp,",\n                       ");
                                                else if( j !=0)
                                                        fprintf(fp,", ");
                                                fprintf(fp,"%9f", mag);
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
