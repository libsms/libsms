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
/*! \file peakContinuation.c
 * \brief peak continuation algorithm and functions
 */

#include "sms.h"

/*! guide states */
#define GUIDE_BEG -2
#define GUIDE_DEAD -1
#define GUIDE_ACTIVE 0

/*!< maximum number of peak continuation candidates */
#define MAX_CONT_CANDIDATES 5

/*! \brief function to get the next closest peak from a guide
 *
 * \param fGuideFreq        guide's frequency
 * \param pFFreqDistance    distance of last best peak from guide
 * \param pSpectralPeaks    array of peaks
 * \param pAnalParams           analysis parameters
 * \param fFreqDev              maximum deviation from guide
 * \return peak number or -1 if nothing is good
 */
static int GetNextClosestPeak(sfloat fGuideFreq, sfloat *pFFreqDistance,
                              const SMS_Peak *pSpectralPeaks, const SMS_AnalParams *pAnalParams,
                              sfloat fFreqDev)
{
    int iInitialPeak = SMS_MAX_NPEAKS * fGuideFreq / (pAnalParams->iSamplingRate * .5);
    int iLowPeak, iHighPeak, iChosenPeak = -1;
    sfloat fLowDistance, fHighDistance, fFreq;

    if(iInitialPeak >= pAnalParams->peakParams.iMaxPeaks)
        iInitialPeak = 0;
    else if(pSpectralPeaks[iInitialPeak].fFreq <= 0)
        iInitialPeak = 0;

    /* find a low peak to start */
    fLowDistance = fGuideFreq - pSpectralPeaks[iInitialPeak].fFreq;
    if(floor(fLowDistance) < floor(*pFFreqDistance))
    {
        while(floor(fLowDistance) <= floor(*pFFreqDistance) &&
              iInitialPeak > 0)
        {
            iInitialPeak--;
            fLowDistance = fGuideFreq - pSpectralPeaks[iInitialPeak].fFreq;
        }
    }
    else
    {
        while(floor(fLowDistance) >= floor(*pFFreqDistance) &&
              iInitialPeak < (pAnalParams->peakParams.iMaxPeaks - 1))
        {
            iInitialPeak++;
            if((fFreq = pSpectralPeaks[iInitialPeak].fFreq) == 0)
                return -1;
            fLowDistance = fGuideFreq - fFreq;
        }
        if(iInitialPeak > 0)
            iInitialPeak--;
        fLowDistance = fGuideFreq - pSpectralPeaks[iInitialPeak].fFreq;
    }

    if(floor(fLowDistance) <= floor(*pFFreqDistance) ||
       fLowDistance > fFreqDev)
        iLowPeak = -1;
    else
        iLowPeak = iInitialPeak;

    /* find a high peak to finish */
    iHighPeak = iInitialPeak;
    fHighDistance = fGuideFreq - pSpectralPeaks[iHighPeak].fFreq;
    while(floor(fHighDistance) >= floor(-*pFFreqDistance) &&
          iHighPeak < (pAnalParams->peakParams.iMaxPeaks - 1))
    {
        iHighPeak++;
        if((fFreq = pSpectralPeaks[iHighPeak].fFreq) == 0)
        {
            iHighPeak = -1;
            break;
        }
        fHighDistance = fGuideFreq - fFreq;
    }
    if(fHighDistance > 0 || fabs(fHighDistance) > fFreqDev ||
       floor(fabs(fHighDistance)) <= floor(*pFFreqDistance))
        iHighPeak = -1;

    /* chose between the two extrema */
    if(iHighPeak >= 0 && iLowPeak >= 0)
    {
        if(fabs(fHighDistance) > fLowDistance)
            iChosenPeak = iLowPeak;
        else
            iChosenPeak = iHighPeak;
    }
    else if(iHighPeak < 0 && iLowPeak >= 0)
        iChosenPeak = iLowPeak;
    else if(iHighPeak >= 0 && iLowPeak < 0)
        iChosenPeak = iHighPeak;
    else
        return -1;

    *pFFreqDistance = fabs(fGuideFreq - pSpectralPeaks[iChosenPeak].fFreq);
    return iChosenPeak;
}

/*! \brief choose the best candidate out of all
 *
 * \param pCandidate         pointer to all the continuation candidates
 * \param nCandidates       number of candidates
 * \param fFreqDev             maximum frequency deviation allowed
 * \return the peak number of the best candidate
 */
static int ChooseBestCand(SMS_ContCandidate *pCandidate, int nCandidates,
                          sfloat fFreqDev)
{
    int i, iHighestCand, iClosestCand, iBestCand = 0;
    sfloat fMaxMag, fClosestFreq;

    /* intial guess */
    iClosestCand = 0;
    fClosestFreq = pCandidate[iClosestCand].fFreqDev;
    iHighestCand = 0;
    fMaxMag = pCandidate[iHighestCand].fMagDev;

    /* get the best candidate */
    for(i = 1; i < nCandidates; i++)
    {
        /* look for the one with highest magnitude */
        if(pCandidate[i].fMagDev > fMaxMag)
        {
            fMaxMag = pCandidate[i].fMagDev;
            iHighestCand = i;
        }
        /* look for the closest one to the guide */
        if(pCandidate[i].fFreqDev < fClosestFreq)
        {
            fClosestFreq = pCandidate[i].fFreqDev;
            iClosestCand = i;
        }
    }
    iBestCand = iHighestCand;

    /* reconcile the two results */
    if(iBestCand != iClosestCand &&
       fabs(pCandidate[iHighestCand].fFreqDev - fClosestFreq) > fFreqDev / 2)
        iBestCand = iClosestCand;

    return pCandidate[iBestCand].iPeak;
}

/*! \brief check for one guide that has choosen iBestPeak
 *
 * \param iBestPeak choosen peak for a guide
 * \param pGuides       array of guides
 * \param nGuides       total number of guides
 * \return number of guide that chose the peak, or -1 if none
 */
static int CheckForConflict(int iBestPeak, const SMS_Guide *pGuides, int nGuides)
{
    int iGuide;

    for(iGuide = 0; iGuide < nGuides; iGuide++)
    {
        if(pGuides[iGuide].iPeakChosen == iBestPeak)
            return iGuide;
    }

    return -1;
}

/*! \brief chose the best of the two guides for the conflicting peak
 *
 * \param iConflictingGuide conflicting guide number
 * \param iGuide            guide number
 * \param pGuides               array of guides
 * \param pSpectralPeaks    array of peaks
 * \return number of guide
 */
static int BestGuide(int iConflictingGuide, int iGuide, const SMS_Guide *pGuides,
                     const SMS_Peak *pSpectralPeaks)
{
    int iConflictingPeak = pGuides[iConflictingGuide].iPeakChosen;
    sfloat fGuideDistance = fabs(pSpectralPeaks[iConflictingPeak].fFreq -
                                 pGuides[iGuide].fFreq);
    sfloat fConfGuideDistance = fabs(pSpectralPeaks[iConflictingPeak].fFreq -
                                     pGuides[iConflictingGuide].fFreq);

    if(fGuideDistance > fConfGuideDistance)
        return iConflictingGuide;
    else
        return iGuide;
}

/*! \brief function to find the best continuation peak for a given guide
 * \param pGuides       guide attributes
 * \param iGuide        number of guide
 * \param pSpectralPeaks    peak values at the current frame
 * \param pAnalParams           analysis parameters
 * \param fFreqDev                  frequency deviation allowed
 * \return the peak number
 */
static int GetBestPeak(SMS_Guide *pGuides, int iGuide, const SMS_Peak *pSpectralPeaks,
                       const SMS_AnalParams *pAnalParams, sfloat fFreqDev)
{
    int iCand = 0, iPeak, iBestPeak, iConflictingGuide, iWinnerGuide;
    sfloat fGuideFreq = pGuides[iGuide].fFreq,
           fGuideMag = pGuides[iGuide].fMag,
           fFreqDistance = -1,
           fMagDistance = 0;
    SMS_ContCandidate pCandidate[MAX_CONT_CANDIDATES];

    /* find all possible candidates */
    while(iCand < MAX_CONT_CANDIDATES)
    {
        /* find the next best peak */
        if((iPeak = GetNextClosestPeak(fGuideFreq, &fFreqDistance,
                                       pSpectralPeaks, pAnalParams,
                                       fFreqDev)) < 0)
        {
            break;
        }

        /* if the peak's magnitude is not too small accept it as
         * possible candidate */
        if((fMagDistance = pSpectralPeaks[iPeak].fMag  - fGuideMag) > -20.0)
        {
            pCandidate[iCand].fFreqDev = fabs(fFreqDistance);
            pCandidate[iCand].fMagDev = fMagDistance;
            pCandidate[iCand].iPeak = iPeak;

            if(pAnalParams->iDebugMode == SMS_DBG_PEAK_CONT ||
               pAnalParams->iDebugMode == SMS_DBG_ALL)
                fprintf(stdout, "candidate %d: freq %f mag %f\n",
                        iCand, pSpectralPeaks[iPeak].fFreq,
                        pSpectralPeaks[iPeak].fMag);
            iCand++;
        }
    }

    /* get best candidate */
    if(iCand < 1)
        return 0;
    else if(iCand == 1)
        iBestPeak = pCandidate[0].iPeak;
    else
        iBestPeak = ChooseBestCand(pCandidate, iCand,
                                   pAnalParams->fFreqDeviation);

    if(pAnalParams->iDebugMode == SMS_DBG_PEAK_CONT ||
       pAnalParams->iDebugMode == SMS_DBG_ALL)
        fprintf(stdout, "BestCandidate: freq %f\n",
                pSpectralPeaks[iBestPeak].fFreq);

    /* if peak taken by another guide resolve conflict */
    if((iConflictingGuide = CheckForConflict(iBestPeak, pGuides,
                                             pAnalParams->nGuides)) >= 0)
    {
        iWinnerGuide = BestGuide(iConflictingGuide, iGuide, pGuides,
                                 pSpectralPeaks);

        if(pAnalParams->iDebugMode == SMS_DBG_PEAK_CONT ||
           pAnalParams->iDebugMode == SMS_DBG_ALL)
            fprintf(stdout,
                    "Conflict: guide: %d (%f), and guide: %d (%f). best: %d\n",
                    iGuide, pGuides[iGuide].fFreq,
                    iConflictingGuide, pGuides[iConflictingGuide].fFreq,
                    iWinnerGuide);

        if(iGuide == iWinnerGuide)
        {
            pGuides[iGuide].iPeakChosen = iBestPeak;
            pGuides[iConflictingGuide].iPeakChosen = -1;
        }
    }
    else
        pGuides[iGuide].iPeakChosen = iBestPeak;

    return iBestPeak;
}

/*! \brief function to get the next maximum (magnitude) peak
 * \param pSpectralPeaks    array of peaks
 * \param pFCurrentMax      last peak maximum
 * \return the number of the maximum peak
 */
static int GetNextMax(const SMS_Peak *pSpectralPeaks, const SMS_PeakParams *pPeakParams,
                      sfloat *pFCurrentMax)
{
    sfloat fPeakMag;
    sfloat fMaxMag = 0.;
    int iPeak, iMaxPeak = -1;

    for(iPeak = 0; iPeak < pPeakParams->iMaxPeaks; iPeak++)
    {
        fPeakMag = pSpectralPeaks[iPeak].fMag;

        if(fPeakMag == 0)
            break;

        if(fPeakMag > fMaxMag && fPeakMag < *pFCurrentMax)
        {
            iMaxPeak = iPeak;
            fMaxMag = fPeakMag;
        }
    }
    *pFCurrentMax = fMaxMag;
    return iMaxPeak;
}

/*! \brief function to get a good starting peak for a track
 *
 * \param iGuide            current guide
 * \param pGuides       array of guides
 * \param nGuides           total number of guides
 * \param pSpectralPeaks    array of peaks
 * \param pFCurrentMax      current peak maximum
 * \return \todo should this return something?
 */
static int GetStartingPeak(int iGuide, SMS_Guide *pGuides, int nGuides,
                           const SMS_Peak *pSpectralPeaks, const SMS_PeakParams *pPeakParams,
                           sfloat *pFCurrentMax)
{
    int iPeak = -1;
    short peakNotFound = 1;

    while(peakNotFound == 1 && *pFCurrentMax > 0)
    {
        /* \todo I don't think this ever returns -1, but check */
        if((iPeak = GetNextMax(pSpectralPeaks, pPeakParams, pFCurrentMax)) < 0)
            return -1;

        if(CheckForConflict(iPeak, pGuides, nGuides) < 0)
        {
            pGuides[iGuide].iPeakChosen = iPeak;
            pGuides[iGuide].iStatus = GUIDE_BEG;
            pGuides[iGuide].fFreq = pSpectralPeaks[iPeak].fFreq;
            peakNotFound = 0;
        }
    }
    return 1;
}

/*! \brief  function to advance the guides through the next frame
 *
 * the output is the frequency, magnitude, and phase tracks
 *
 * \param iFrame     current frame number
 * \param pAnalParams analysis parameters
 * \return error code \see SMS_ERRORS
 */
int sms_peakContinuation(int iFrame, const SMS_AnalParams *pAnalParams)
{
    int iGuide, iCurrentPeak = -1, iGoodPeak = -1;
    sfloat fFund = pAnalParams->ppFrames[iFrame]->fFundamental,
    fFreqDev = fFund * pAnalParams->fFreqDeviation, fCurrentMax = 1000;

    /* update guides with fundamental contribution */
    if(fFund > 0 && (pAnalParams->iFormat == SMS_FORMAT_H ||
                     pAnalParams->iFormat == SMS_FORMAT_HP))
        for(iGuide = 0; iGuide < pAnalParams->nGuides; iGuide++)
            pAnalParams->guides[iGuide].fFreq =
                (1 - pAnalParams->fFundContToGuide) * pAnalParams->guides[iGuide].fFreq +
                pAnalParams->fFundContToGuide * fFund * (iGuide + 1);

    if(pAnalParams->iDebugMode == SMS_DBG_PEAK_CONT ||
       pAnalParams->iDebugMode == SMS_DBG_ALL)
        fprintf(stdout, "Frame %d Peak Continuation: \n",
                pAnalParams->ppFrames[iFrame]->iFrameNum);

    /* continue all guides */
    for(iGuide = 0; iGuide < pAnalParams->nGuides; iGuide++)
    {
        sfloat fPreviousFreq = pAnalParams->ppFrames[iFrame-1]->deterministic.pFSinFreq[iGuide];

        /* get the guide value by upgrading the previous guide */
        if(fPreviousFreq > 0)
            pAnalParams->guides[iGuide].fFreq =
                (1 - pAnalParams->fPeakContToGuide) * pAnalParams->guides[iGuide].fFreq +
                pAnalParams->fPeakContToGuide * fPreviousFreq;

        if(pAnalParams->iDebugMode == SMS_DBG_PEAK_CONT ||
           pAnalParams->iDebugMode == SMS_DBG_ALL)
            fprintf(stdout, "Guide %d:  freq %f, mag %f\n",
                    iGuide, pAnalParams->guides[iGuide].fFreq, pAnalParams->guides[iGuide].fMag);

        if(pAnalParams->guides[iGuide].fFreq <= 0.0 ||
           pAnalParams->guides[iGuide].fFreq > pAnalParams->fHighestFreq)
        {
            pAnalParams->guides[iGuide].iStatus = GUIDE_DEAD;
            pAnalParams->guides[iGuide].fFreq = 0;
            continue;
        }

        pAnalParams->guides[iGuide].iPeakChosen = -1;

        if(pAnalParams->iFormat == SMS_FORMAT_IH || pAnalParams->iFormat == SMS_FORMAT_IHP)
            fFreqDev = pAnalParams->guides[iGuide].fFreq * pAnalParams->fFreqDeviation;

        /* get the best peak for the guide */
        iGoodPeak = GetBestPeak(pAnalParams->guides, iGuide,
                                pAnalParams->ppFrames[iFrame]->pSpectralPeaks,
                                pAnalParams, fFreqDev);
    }

    /* try to find good peaks for the GUIDE_DEAD guides */
    if(pAnalParams->iFormat == SMS_FORMAT_IH || pAnalParams->iFormat == SMS_FORMAT_IHP)
    {
        for(iGuide = 0; iGuide < pAnalParams->nGuides; iGuide++)
        {
            if(pAnalParams->guides[iGuide].iStatus != GUIDE_DEAD)
                continue;

            if(GetStartingPeak(iGuide, pAnalParams->guides, pAnalParams->nGuides,
                               pAnalParams->ppFrames[iFrame]->pSpectralPeaks,
                               &pAnalParams->peakParams, &fCurrentMax) == -1)
                break;
        }
    }

    /* save all the continuation values,
     * assume output tracks are already clear */
    for(iGuide = 0; iGuide < pAnalParams->nGuides; iGuide++)
    {
        if(pAnalParams->guides[iGuide].iStatus == GUIDE_DEAD)
            continue;

        if(pAnalParams->iFormat == SMS_FORMAT_IH || pAnalParams->iFormat == SMS_FORMAT_IHP)
        {
            if(pAnalParams->guides[iGuide].iStatus > 0 &&
               pAnalParams->guides[iGuide].iPeakChosen == -1)
            {
                if(pAnalParams->guides[iGuide].iStatus++ > pAnalParams->iMaxSleepingTime)
                {
                    pAnalParams->guides[iGuide].iStatus = GUIDE_DEAD;
                    pAnalParams->guides[iGuide].fFreq = 0;
                    pAnalParams->guides[iGuide].fMag = 0;
                    pAnalParams->guides[iGuide].iPeakChosen = -1;
                }
                else
                    pAnalParams->guides[iGuide].iStatus++;
                continue;
            }

            if(pAnalParams->guides[iGuide].iStatus == GUIDE_ACTIVE &&
               pAnalParams->guides[iGuide].iPeakChosen == -1)
            {
                pAnalParams->guides[iGuide].iStatus = 1;
                continue;
            }
        }

        /* if good continuation peak found, save it */
        if((iCurrentPeak = pAnalParams->guides[iGuide].iPeakChosen) >= 0)
        {
            pAnalParams->ppFrames[iFrame]->deterministic.pFSinFreq[iGuide] =
                pAnalParams->ppFrames[iFrame]->pSpectralPeaks[iCurrentPeak].fFreq;
            pAnalParams->ppFrames[iFrame]->deterministic.pFSinAmp[iGuide] =
                pAnalParams->ppFrames[iFrame]->pSpectralPeaks[iCurrentPeak].fMag;
            pAnalParams->ppFrames[iFrame]->deterministic.pFSinPha[iGuide] =
                pAnalParams->ppFrames[iFrame]->pSpectralPeaks[iCurrentPeak].fPhase;

            pAnalParams->guides[iGuide].iStatus = GUIDE_ACTIVE;
            pAnalParams->guides[iGuide].iPeakChosen = -1;
        }
    }
    return SMS_OK;
}
