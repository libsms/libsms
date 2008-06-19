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
 *    main program for calcCorr
 *
 *    calculate the correlation of a sound
 *     when the type is set to 0 it prints the average correlation of
 *       the correlation function and when it is set to 1 it print the 
 *        correlation function 
 *
 */
#include "../sms.h"

#define USAGE "Usage: calcCorr [-t time] [-l length] [-n number] [-o type] <inputSnd2File>"

//short MaxDelayFrames;
float FResidualPerc;
SOUND_BUFFER soundBuffer, synthBuffer;
ANAL_FRAME **ppFrames, *pFrames;

void main (int argc, char *argv[])
{
  char *pChSndFile = NULL;
  SNDSoundStruct *pSndHeader;
  short *pSData;
  int iError, i, j, sizeData, iLength = 1024, iFirstSample = 0, iType = 0, 
    iNumber= 1;
  float fTime = 0, *pFData, *pFOut;
  double *pFOutput;
  void correl ();
  
  if (argc > 2) 
    {
      for (i=1; i<argc-1; i++) 
        if (*(argv[i]++) == '-') 
          switch (*(argv[i]++)) 
            {
            case 't':  if (sscanf(argv[i],"%f",&fTime) < 0)
              quit("Invalid time");
              break;
            case 'l':  if (sscanf(argv[i],"%d",&iLength) < 0)
              quit("Invalid length");
              break;
            case 'o':  if (sscanf(argv[i],"%d",&iType) < 0)
              quit("Invalid type");
              break;
            case 'n':  if (sscanf(argv[i],"%d",&iNumber) < 1)
              quit("Invalid number");
              break;
            default:   quit(USAGE);
            }
    }
  else if (argc < 2)
    quit (USAGE);
  
  pChSndFile = argv[argc-1];
  
  if (iError = SNDReadSoundfile (pChSndFile, &pSndHeader))
    {
      printf("Sound error %s on %s\n",
	     SNDSoundError(iError), pChSndFile);
      exit(1);
    }
  

  
  pSData = (short *) ((char *) pSndHeader + 
                      pSndHeader->dataLocation);
  
  pFData = (float *) calloc (iLength+1, sizeof (float));
  pFOut = (float *) calloc ((iLength+1)*2, sizeof (float));
  pFOutput = (double *) calloc ((iLength+1)/2, sizeof (double));

  sizeData = pSndHeader->dataSize / sizeof(short);
  iFirstSample = pSndHeader->samplingRate * fTime;
  if ((iLength * iNumber) + iFirstSample >= sizeData)
    quit ("Data out of bounds");

  for (j = 0; j <= iNumber; j++)
    {
      for(i = 1; i <= iLength; i++)
        pFData[i] = pSData[i + iFirstSample];
  
      correl (pFData, pFData, iLength, pFOut);
  
      for(i = 1; i <= iLength/2; i++)
        pFOutput[i] += pFOut[i];

      iFirstSample += iLength;
    }	

  for(i = 2; i <= iLength/2; i++)
    pFOutput[i] = pFOutput[i] / pFOutput[1];

  pFOutput[1] = 1;

  if (iType > 0)
    {
      double fTotal = 0, fAverage = 0, fDev = 0;

      for (i = 2; i <= iLength/2; i++)
        fTotal += pFOutput[i];
      fAverage = fTotal / (iLength/2 - 2);
      for (i = 2; i <= iLength/2; i++)
        fDev +=  ((pFOutput[i] - fAverage) * (pFOutput[i] - fAverage)) / 
          (iLength/2 - 2);

      fprintf (stdout, "%f\n", fDev);
    }
  else
    for (i = 1; i <= iLength/2; i++)
      fprintf (stdout, "%d %f\n", i, pFOutput[i]);

  free (pFData);
  free (pFOut);
  SNDFree(pSndHeader);
  exit(0);
}
