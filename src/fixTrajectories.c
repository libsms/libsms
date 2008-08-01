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

//extern SMS_AnalFrame **ppFrames;

/* function to fill a gap in a given trajectory 
 *
 * int iCurrentFrame;      currrent frame number 
 * int iTraj;              trajectory to be filled
 * int *pIState;           state of trajectories
 * SMS_AnalParams *pAnalParams; analysis parameters
 *
 */
static void FillGap (int iCurrentFrame, int iTraj, int *pIState, 
                     SMS_AnalParams *pAnalParams)
{
	int iFrame, iLastFrame = - (pIState[iTraj] - 1);
	float fConstant = TWO_PI / pAnalParams->iSamplingRate;
	float fFirstMag, fFirstFreq, fLastMag, fLastFreq, fIncrMag, fIncrFreq,
		fMag, fTmpPha, fFreq;
  
	if(iCurrentFrame - iLastFrame < 0)
		return;
  
	/* if firstMag is 0 it means that there is no Gap, just the begining */
	/*   of a trajectory                                                 */
	if (pAnalParams->ppFrames[iCurrentFrame - 
	    iLastFrame]->deterministic.pFMagTraj[iTraj] == 0)
	{
		pIState[iTraj] = 1;
		return;
	}
  
	fFirstMag = 
		pAnalParams->ppFrames[iCurrentFrame - iLastFrame]->deterministic.pFMagTraj[iTraj];
	fFirstFreq = 
		pAnalParams->ppFrames[iCurrentFrame - iLastFrame]->deterministic.pFFreqTraj[iTraj];
	fLastMag = pAnalParams->ppFrames[iCurrentFrame]->deterministic.pFMagTraj[iTraj];
	fLastFreq = pAnalParams->ppFrames[iCurrentFrame]->deterministic.pFFreqTraj[iTraj];
	fIncrMag =  (fLastMag - fFirstMag) / iLastFrame;
	fIncrFreq =  (fLastFreq - fFirstFreq) / iLastFrame;
  
	/* if inharmonic format and the two extremes are very different  */
	/* do not interpolate, it means that they are different trajectories */
	if ((pAnalParams->iFormat == SMS_FORMAT_IH ||
	     pAnalParams->iFormat == SMS_FORMAT_IHP) &&
		(MIN (fFirstFreq, fLastFreq) * .5 * pAnalParams->fFreqDeviation <
	     fabs ((double) fLastFreq - fFirstFreq)))
	{
		pIState[iTraj] = 1;
		return;		
	}

	fMag = fFirstMag;
	fFreq = fFirstFreq;
	/* fill the gap by interpolating values */
	/* if the gap is too long it should consider the lower partials */
	for (iFrame = iCurrentFrame - iLastFrame + 1; iFrame < iCurrentFrame; 
	     iFrame++)
	{
		/* interpolate magnitude */
		fMag += fIncrMag;
		pAnalParams->ppFrames[iFrame]->deterministic.pFMagTraj[iTraj] = fMag;
		/* interpolate frequency */
		fFreq += fIncrFreq;
		pAnalParams->ppFrames[iFrame]->deterministic.pFFreqTraj[iTraj] = fFreq;
		/*interpolate phase (this may not be the right way) */
		fTmpPha = 
			pAnalParams->ppFrames[iFrame-1]->deterministic.pFPhaTraj[iTraj] -
				(pAnalParams->ppFrames[iFrame-1]->deterministic.pFFreqTraj[iTraj] * 
				fConstant) * pAnalParams->sizeHop;
		pAnalParams->ppFrames[iFrame]->deterministic.pFPhaTraj[iTraj] = 
			fTmpPha - floor(fTmpPha/ TWO_PI) * TWO_PI;
	}
  
	if(pAnalParams->iDebugMode == SMS_DBG_CLEAN_TRAJ || 
	   pAnalParams->iDebugMode == SMS_DBG_ALL)
	{
		fprintf (stdout, "fillGap: traj %d, frames %d to %d filled\n",
		        iTraj, pAnalParams->ppFrames[iCurrentFrame-iLastFrame + 1]->iFrameNum, 
		        pAnalParams->ppFrames[iCurrentFrame-1]->iFrameNum);
		fprintf (stdout, "firstFreq %f lastFreq %f, firstMag %f lastMag %f\n",
		        fFirstFreq, fLastFreq, fFirstMag, fLastMag);

  	}

	/* reset status */
	pIState[iTraj] = pAnalParams->iMinTrajLength;
}


/* function to delete a short trajectory 
 *
 * int iCurrentFrame;    current frame
 * int iTraj;            trajectory to be deleted
 * int *pIState;         state of trajectories
 * SMS_AnalParams *pAnalParams; analysis parameters
 *
 */
static void DeleteShortTraj (int iCurrentFrame, int iTraj, int *pIState,
                             SMS_AnalParams *pAnalParams)
{
	int iFrame, frame;
  
	for (iFrame = 1; iFrame <= pIState[iTraj]; iFrame++)
	{
		frame = iCurrentFrame - iFrame;
      
		if (frame <= 0)
			return;
      
		pAnalParams->ppFrames[frame]->deterministic.pFMagTraj[iTraj] = 0;
		pAnalParams->ppFrames[frame]->deterministic.pFFreqTraj[iTraj] = 0;
		pAnalParams->ppFrames[frame]->deterministic.pFPhaTraj[iTraj] = 0;
	}
  
	if (pAnalParams->iDebugMode == SMS_DBG_CLEAN_TRAJ ||
	    pAnalParams->iDebugMode == SMS_DBG_ALL)
		fprintf (stdout, "deleteShortTraj: traj %d, frames %d to %d deleted\n",
		         iTraj, pAnalParams->ppFrames[iCurrentFrame - pIState[iTraj]]->iFrameNum, 
		         pAnalParams->ppFrames[iCurrentFrame-1]->iFrameNum);
  
	/* reset state */
	pIState[iTraj] = -pAnalParams->iMaxSleepingTime;
}

/* function to fill gaps and delete short trajectories 
 *
 * int iCurrentFrame;     current frame number
 * SMS_AnalParams *pAnalParams analysis parameters
 *
 */
int CleanTrajectories (int iCurrentFrame, SMS_AnalParams *pAnalParams)
{
	int iTraj, iLength, iFrame;
	static int *pIState = NULL;
  
	if (pIState == NULL)
		pIState = (int *) calloc (pAnalParams->nGuides, sizeof(int));
  
	/* if fundamental and first partial are short, delete everything */
	if ((pAnalParams->iFormat == SMS_FORMAT_H ||
	     pAnalParams->iFormat == SMS_FORMAT_HP) &&
	     pAnalParams->ppFrames[iCurrentFrame]->deterministic.pFMagTraj[0] == 0 &&
	     pIState[0] > 0 &&
	     pIState[0] < pAnalParams->iMinTrajLength &&
	     pAnalParams->ppFrames[iCurrentFrame]->deterministic.pFMagTraj[1] == 0 &&
	     pIState[1] > 0 &&
	     pIState[1] < pAnalParams->iMinTrajLength)
	{
		iLength = pIState[0];
		for (iTraj = 0; iTraj < pAnalParams->nGuides; iTraj++)
		{
			for (iFrame = 1; iFrame <= iLength; iFrame++)
			{
				pAnalParams->ppFrames[iCurrentFrame - 
					iFrame]->deterministic.pFMagTraj[iTraj] = 0;
				pAnalParams->ppFrames[iCurrentFrame - 
					iFrame]->deterministic.pFFreqTraj[iTraj] = 0;
				pAnalParams->ppFrames[iCurrentFrame - 
					iFrame]->deterministic.pFPhaTraj[iTraj] = 0;
			}
			pIState[iTraj] = -pAnalParams->iMaxSleepingTime;
		}
		if (pAnalParams->iDebugMode == SMS_DBG_CLEAN_TRAJ || 
		    pAnalParams->iDebugMode == SMS_DBG_ALL)
			fprintf(stdout, "cleanTraj: frame %d to frame %d deleted\n",
			        pAnalParams->ppFrames[iCurrentFrame-iLength]->iFrameNum, 
			        pAnalParams->ppFrames[iCurrentFrame-1]->iFrameNum);
		return (1);
	}
  
	/* check every partial individually */
	for (iTraj = 0; iTraj < pAnalParams->nGuides; iTraj++)
	{
		/* trajectory after gap */
		if(pAnalParams->ppFrames[iCurrentFrame]->deterministic.pFMagTraj[iTraj] != 0)
		{ 
			if(pIState[iTraj] < 0 && 
			   pIState[iTraj] > -pAnalParams->iMaxSleepingTime)
				FillGap (iCurrentFrame, iTraj, pIState, pAnalParams);
			else
				pIState[iTraj] = (pIState[iTraj]<0) ? 1 : pIState[iTraj]+1;
		}
		/* gap after trajectory */
		else
		{	   
			if(pIState[iTraj] > 0 &&  
			   pIState[iTraj] < pAnalParams->iMinTrajLength)
				DeleteShortTraj (iCurrentFrame, iTraj, pIState, pAnalParams);
			else 
				pIState[iTraj] = (pIState[iTraj]>0) ? -1 : pIState[iTraj]-1;
		}
	}
	return (1);
}

/* scale deterministic magnitude if synthesis is larger than original 
 *
 * float *pFSynthBuffer;      synthesis buffer
 * float *pFOriginalBuffer;   original sound
 * float *pFMagTraj;          magnitudes to be scaled
 * int sizeHop;               size of buffer
 * int nTraj;                    number of trajectories
 *
 */
void  ScaleDeterministic (float *pFSynthBuffer, float *pFOriginalBuffer, 
                          float *pFMagTraj, SMS_AnalParams *pAnalParams, int nTraj)
{
	float fOriginalMag = 0, fSynthesisMag = 0;
	float fCosScaleFactor;
	int iTraj, i;
  
	/* get sound energy */
	for (i = 0; i < pAnalParams->sizeHop; i++)
	{
		fOriginalMag += fabs((double) pFOriginalBuffer[i]);
		fSynthesisMag += fabs((double) pFSynthBuffer[i]);
	}
  
	/* if total energy of deterministic sound is larger than original,
	   scale deterministic representation */
	if (fSynthesisMag > (1.5 * fOriginalMag))
	{
		fCosScaleFactor = fOriginalMag / fSynthesisMag;
      
		if(pAnalParams->iDebugMode == SMS_DBG_CLEAN_TRAJ || 
		   pAnalParams->iDebugMode == SMS_DBG_ALL)
			fprintf (stdout, "Frame %d: magnitude scaled by %f\n",
			         pAnalParams->ppFrames[0]->iFrameNum, fCosScaleFactor);
      
		for (iTraj = 0; iTraj < nTraj; iTraj++)
			if (pFMagTraj[iTraj] > 0)
				pFMagTraj[iTraj] = 
					TO_DB (TO_MAG (pFMagTraj[iTraj]) * fCosScaleFactor);
	}
}
