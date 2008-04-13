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

extern ANAL_FRAME **ppFrames;

/* function to fill a gap in a given trajectory 
 *
 * int iCurrentFrame;      currrent frame number 
 * int iTraj;              trajectory to be filled
 * int *pIState;           state of trajectories
 * ANAL_PARAMS analParams; analysis parameters
 *
 */
static void FillGap (int iCurrentFrame, int iTraj, int *pIState, 
                     ANAL_PARAMS analParams)
{
	int iFrame, iLastFrame = - (pIState[iTraj] - 1);
	float fConstant = TWO_PI / analParams.iSamplingRate;
	float fFirstMag, fFirstFreq, fLastMag, fLastFreq, fIncrMag, fIncrFreq,
		fMag, fTmpPha, fFreq;
  
	if(iCurrentFrame - iLastFrame < 0)
		return;
  
	/* if firstMag is 0 it means that there is no Gap, just the begining */
	/*   of a trajectory                                                 */
	if (ppFrames[iCurrentFrame - 
	    iLastFrame]->deterministic.pFMagTraj[iTraj] == 0)
	{
		pIState[iTraj] = 1;
		return;
	}
  
	fFirstMag = 
		ppFrames[iCurrentFrame - iLastFrame]->deterministic.pFMagTraj[iTraj];
	fFirstFreq = 
		ppFrames[iCurrentFrame - iLastFrame]->deterministic.pFFreqTraj[iTraj];
	fLastMag = ppFrames[iCurrentFrame]->deterministic.pFMagTraj[iTraj];
	fLastFreq = ppFrames[iCurrentFrame]->deterministic.pFFreqTraj[iTraj];
	fIncrMag =  (fLastMag - fFirstMag) / iLastFrame;
	fIncrFreq =  (fLastFreq - fFirstFreq) / iLastFrame;
  
	/* if inharmonic format and the two extremes are very different  */
	/* do not interpolate, it means that they are different trajectories */
	if ((analParams.iFormat == FORMAT_INHARMONIC ||
	     analParams.iFormat == FORMAT_INHARMONIC_WITH_PHASE) &&
		(MIN (fFirstFreq, fLastFreq) * .5 * analParams.fFreqDeviation <
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
		ppFrames[iFrame]->deterministic.pFMagTraj[iTraj] = fMag;
		/* interpolate frequency */
		fFreq += fIncrFreq;
		ppFrames[iFrame]->deterministic.pFFreqTraj[iTraj] = fFreq;
		/*interpolate phase (this may not be the right way) */
		fTmpPha = 
			ppFrames[iFrame-1]->deterministic.pFPhaTraj[iTraj] -
				(ppFrames[iFrame-1]->deterministic.pFFreqTraj[iTraj] * 
				fConstant) * analParams.sizeHop;
		ppFrames[iFrame]->deterministic.pFPhaTraj[iTraj] = 
			fTmpPha - floor(fTmpPha/ TWO_PI) * TWO_PI;
	}
  
	if(analParams.iDebugMode == DEBUG_CLEAN_TRAJ || 
	   analParams.iDebugMode == DEBUG_ALL)
	{
		fprintf (stdout, "fillGap: traj %d, frames %d to %d filled\n",
		        iTraj, ppFrames[iCurrentFrame-iLastFrame + 1]->iFrameNum, 
		        ppFrames[iCurrentFrame-1]->iFrameNum);
		fprintf (stdout, "firstFreq %f lastFreq %f, firstMag %f lastMag %f\n",
		        fFirstFreq, fLastFreq, fFirstMag, fLastMag);

  	}

	/* reset status */
	pIState[iTraj] = analParams.iMinTrajLength;
}


/* function to delete a short trajectory 
 *
 * int iCurrentFrame;    current frame
 * int iTraj;            trajectory to be deleted
 * int *pIState;         state of trajectories
 * ANAL_PARAMS analParams; analysis parameters
 *
 */
static void DeleteShortTraj (int iCurrentFrame, int iTraj, int *pIState,
                             ANAL_PARAMS analParams)
{
	int iFrame, frame;
  
	for (iFrame = 1; iFrame <= pIState[iTraj]; iFrame++)
	{
		frame = iCurrentFrame - iFrame;
      
		if (frame <= 0)
			return;
      
		ppFrames[frame]->deterministic.pFMagTraj[iTraj] = 0;
		ppFrames[frame]->deterministic.pFFreqTraj[iTraj] = 0;
		ppFrames[frame]->deterministic.pFPhaTraj[iTraj] = 0;
	}
  
	if (analParams.iDebugMode == DEBUG_CLEAN_TRAJ ||
	    analParams.iDebugMode == DEBUG_ALL)
		fprintf (stdout, "deleteShortTraj: traj %d, frames %d to %d deleted\n",
		         iTraj, ppFrames[iCurrentFrame - pIState[iTraj]]->iFrameNum, 
		         ppFrames[iCurrentFrame-1]->iFrameNum);
  
	/* reset state */
	pIState[iTraj] = -analParams.iMaxSleepingTime;
}

/* function to fill gaps and delete short trajectories 
 *
 * int iCurrentFrame;     current frame number
 * ANAL_PARAMS analParams analysis parameters
 *
 */
int CleanTrajectories (int iCurrentFrame, ANAL_PARAMS analParams)
{
	int iTraj, iLength, iFrame;
	static int *pIState = NULL;
  
	if (pIState == NULL)
		pIState = (int *) calloc (analParams.nGuides, sizeof(int));
  
	/* if fundamental and first partial are short, delete everything */
	if ((analParams.iFormat == FORMAT_HARMONIC ||
	     analParams.iFormat == FORMAT_HARMONIC_WITH_PHASE) &&
	     ppFrames[iCurrentFrame]->deterministic.pFMagTraj[0] == 0 &&
	     pIState[0] > 0 &&
	     pIState[0] < analParams.iMinTrajLength &&
	     ppFrames[iCurrentFrame]->deterministic.pFMagTraj[1] == 0 &&
	     pIState[1] > 0 &&
	     pIState[1] < analParams.iMinTrajLength)
	{
		iLength = pIState[0];
		for (iTraj = 0; iTraj < analParams.nGuides; iTraj++)
		{
			for (iFrame = 1; iFrame <= iLength; iFrame++)
			{
				ppFrames[iCurrentFrame - 
					iFrame]->deterministic.pFMagTraj[iTraj] = 0;
				ppFrames[iCurrentFrame - 
					iFrame]->deterministic.pFFreqTraj[iTraj] = 0;
				ppFrames[iCurrentFrame - 
					iFrame]->deterministic.pFPhaTraj[iTraj] = 0;
			}
			pIState[iTraj] = -analParams.iMaxSleepingTime;
		}
		if (analParams.iDebugMode == DEBUG_CLEAN_TRAJ || 
		    analParams.iDebugMode == DEBUG_ALL)
			fprintf(stdout, "cleanTraj: frame %d to frame %d deleted\n",
			        ppFrames[iCurrentFrame-iLength]->iFrameNum, 
			        ppFrames[iCurrentFrame-1]->iFrameNum);
		return (1);
	}
  
	/* check every partial individually */
	for (iTraj = 0; iTraj < analParams.nGuides; iTraj++)
	{
		/* trajectory after gap */
		if(ppFrames[iCurrentFrame]->deterministic.pFMagTraj[iTraj] != 0)
		{ 
			if(pIState[iTraj] < 0 && 
			   pIState[iTraj] > -analParams.iMaxSleepingTime)
				FillGap (iCurrentFrame, iTraj, pIState, analParams);
			else
				pIState[iTraj] = (pIState[iTraj]<0) ? 1 : pIState[iTraj]+1;
		}
		/* gap after trajectory */
		else
		{	   
			if(pIState[iTraj] > 0 &&  
			   pIState[iTraj] < analParams.iMinTrajLength)
				DeleteShortTraj (iCurrentFrame, iTraj, pIState, analParams);
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
                          float *pFMagTraj, ANAL_PARAMS analParams, int nTraj)
{
	float fOriginalMag = 0, fSynthesisMag = 0;
	float fCosScaleFactor;
	int iTraj, i;
  
	/* get sound energy */
	for (i = 0; i < analParams.sizeHop; i++)
	{
		fOriginalMag += fabs((double) pFOriginalBuffer[i]);
		fSynthesisMag += fabs((double) pFSynthBuffer[i]);
	}
  
	/* if total energy of deterministic sound is larger than original,
	   scale deterministic representation */
	if (fSynthesisMag > (1.5 * fOriginalMag))
	{
		fCosScaleFactor = fOriginalMag / fSynthesisMag;
      
		if(analParams.iDebugMode == DEBUG_CLEAN_TRAJ || 
		   analParams.iDebugMode == DEBUG_ALL)
			fprintf (stdout, "Frame %d: magnitude scaled by %f\n",
			         ppFrames[0]->iFrameNum, fCosScaleFactor);
      
		for (iTraj = 0; iTraj < nTraj; iTraj++)
			if (pFMagTraj[iTraj] > 0)
				pFMagTraj[iTraj] = 
					TO_DB (TO_MAG (pFMagTraj[iTraj]) * fCosScaleFactor);
	}
}
