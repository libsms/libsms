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



/* function to generate a sinusoid given two peaks, current and last
 * it interpolation between phase values and magnitudes   
 * float fFreq;              current frequency
 * float fMag;               current magnitude
 * float fPhase;             current phase
 * SMS_Data                  values from last frame
 * float *pFWaveform;	     pointer to output waveform
 * int sizeBuffer;	     size of the synthesis buffer 
 */
static void SinePhaSynth (float fFreq, float fMag, float fPhase,
                          SMS_Data *pLastFrame, float *pFWaveform, 
                          int sizeBuffer, int iTraj)
{
  float  fMagIncr, fInstMag, fInstPhase, fTmp;
  int iM, i;
  float fAlpha, fBeta, fTmp1, fTmp2;
   
  /* if no mag in last frame copy freq from current and make phase */
  if (pLastFrame->pFMagTraj[iTraj] <= 0)
  {
    pLastFrame->pFFreqTraj[iTraj] = fFreq;
    fTmp = fPhase - (fFreq * sizeBuffer);
    pLastFrame->pFPhaTraj[iTraj] = fTmp - floor(fTmp / TWO_PI) * TWO_PI;
  }
  /* and the other way */
  else if (fMag <= 0)
  {
    fFreq = pLastFrame->pFFreqTraj[iTraj];
    fTmp = pLastFrame->pFPhaTraj[iTraj] + 
            (pLastFrame->pFFreqTraj[iTraj] * sizeBuffer);
    fPhase = fTmp - floor(fTmp / TWO_PI) * TWO_PI;
  }
  
  /* caculate the instantaneous amplitude */
  fMagIncr = (fMag - pLastFrame->pFMagTraj[iTraj]) / sizeBuffer;
  fInstMag = pLastFrame->pFMagTraj[iTraj];
  
  /* create instantaneous phase from freq. and phase values */
  fTmp1 = fFreq - pLastFrame->pFFreqTraj[iTraj];
  fTmp2 = ((pLastFrame->pFPhaTraj[iTraj] + 
		pLastFrame->pFFreqTraj[iTraj] * sizeBuffer - fPhase) +
		fTmp1 * sizeBuffer / 2.0) / TWO_PI;
  iM = (int) (fTmp2 + .5);
  fTmp2 = fPhase - pLastFrame->pFPhaTraj[iTraj] - 
		pLastFrame->pFFreqTraj[iTraj] * sizeBuffer +
		TWO_PI * iM;
  fAlpha = (3.0 / (float)(sizeBuffer * sizeBuffer)) * 
    fTmp2 - fTmp1 / sizeBuffer;
  fBeta = (-2.0 / ((float) (sizeBuffer * sizeBuffer * sizeBuffer))) * 
    fTmp2 + fTmp1 / ((float) (sizeBuffer * sizeBuffer));
  
  for(i=0; i<sizeBuffer; i++)
  {
    fInstMag += fMagIncr;
    fInstPhase = pLastFrame->pFPhaTraj[iTraj] + 
			pLastFrame->pFFreqTraj[iTraj] * i + 
			fAlpha * i * i + fBeta * i * i * i;

//why is sms_sine causing a seg fault here?
    pFWaveform[i] += TO_MAG(fInstMag) * sms_sine(fInstPhase + PI_2);
//    pFWaveform[i] += TO_MAG(fInstMag) * sin(fInstPhase + PI_2);
  }
  /* save current values into buffer */
  pLastFrame->pFFreqTraj[iTraj] = fFreq;
  pLastFrame->pFMagTraj[iTraj] = fMag;
  pLastFrame->pFPhaTraj[iTraj] = fPhase;
}

/*
 * function to generate a sinusoid given two frames, current and last
 * 
 * float fFreq;	         current frequency 
 * float fMag;              current magnitude  
 * SMS_Data *pLastFrame;   values from last frame 
 * float *pFBuffer;	 pointer to output waveform 
 * int sizeBuffer;		 size of the synthesis buffer 
 * int iTraj;               current trajectory 
 */
static void SineSynth (float fFreq, float fMag, SMS_Data *pLastFrame,
                       float *pFBuffer, int sizeBuffer, int iTraj)
{
  float  fMagIncr, fInstMag, fFreqIncr, fInstPhase, fInstFreq;
  int i;
  
  /* if no mag in last frame copy freq from current */
  if (pLastFrame->pFMagTraj[iTraj] <= 0)
  {
    pLastFrame->pFFreqTraj[iTraj] = fFreq;
    pLastFrame->pFPhaTraj[iTraj] = 
			TWO_PI * ((random() - HALF_MAX) / HALF_MAX);
  }
  /* and the other way */
  else if (fMag <= 0)
    fFreq = pLastFrame->pFFreqTraj[iTraj];
  
  /* calculate the instantaneous amplitude */
  fMagIncr = (fMag - pLastFrame->pFMagTraj[iTraj]) / sizeBuffer;
  fInstMag = pLastFrame->pFMagTraj[iTraj];
  /* calculate instantaneous frequency */
  fFreqIncr = (fFreq - pLastFrame->pFFreqTraj[iTraj]) / sizeBuffer;
  fInstFreq = pLastFrame->pFFreqTraj[iTraj];
  fInstPhase = pLastFrame->pFPhaTraj[iTraj];
  
  /* generate all the samples */    
  for (i = 0; i < sizeBuffer; i++)
  {
    fInstMag += fMagIncr;
    fInstFreq += fFreqIncr;
    fInstPhase += fInstFreq;
      
    pFBuffer[i] += TO_MAG (fInstMag) * sms_sine (fInstPhase);
  }
  
  /* save current values into last values */
  pLastFrame->pFFreqTraj[iTraj] = fFreq;
  pLastFrame->pFMagTraj[iTraj] = fMag;
  pLastFrame->pFPhaTraj[iTraj] = fInstPhase - 
		floor(fInstPhase / TWO_PI) * TWO_PI;
}

/*
 * function to generate all the sinusoids for a given frame
 * 
 * SMS_Data *pSmsData;     SMS data for current frame 
 * int nTraj;     	   number of partial trajectories 
 * float *pFBuffer;	   pointer to output waveform 
 * int sizeBuffer;	   size of the synthesis buffer
 * SMS_Data *pLastFrame;  SMS data from last frame 
 */
int sms_sineSynthFrame (SMS_Data *pSmsData, float *pFBuffer, 
                    int sizeBuffer, SMS_Data *pLastFrame, 
                    int iSamplingRate)
{


        float fMag, fFreq;
        int i, nTraj = pSmsData->nTraj, iHalfSamplingRate = iSamplingRate >> 1;
        
        /// RTE DEBUG //////////////
//        static int fc = 0;
//        printf(" # %d :::::::::::\n",fc++);
        ///////////////////////////////////////

        /* go through all the trajectories */    
        for (i = 0; i < nTraj; i++)
        {
                /* get magnitude */
                fMag = pSmsData->pFMagTraj[i];
      
                fFreq = pSmsData->pFFreqTraj[i];

                /* gaurd so transposed frequencies don't alias */
                if (fFreq > iHalfSamplingRate || fFreq < 0) 
                        fMag = 0;
      
                /* generate sines if there are magnitude values */
                if ((fMag > 0) || (pLastFrame->pFMagTraj[i] > 0))
                {
                        /* frequency from Hz to radians */
                        fFreq = (fFreq == 0) ? 0 : TWO_PI * fFreq / iSamplingRate;


                        // RTE TODO: make seperate function for SineSynth /wo phase
                        if (pSmsData->pFPhaTraj == NULL)
                        {                
                                SineSynth(fFreq, fMag, pLastFrame, pFBuffer, sizeBuffer, i);
                        }
                        else
                        {
                                SinePhaSynth(fFreq, fMag, pSmsData->pFPhaTraj[i], pLastFrame, 
                                             pFBuffer, sizeBuffer, i);
                        }
                }
//                printf(" [%d] %f, ",i, pSmsData->pFPhaTraj[i]);
        }
//        printf("\n");
        return 1;
}     
