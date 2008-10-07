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

/* command-line arguments */

typedef struct 
{
	int iDebugMode;	      /* 0 no debug,
                         	 1 debug initialitzation functions,
                           2 debug peak detection function,
                           3 debug harmonic detection function,
                           4 debug peak continuation function,
                           5 debug clean trajectories function,
	                         6 debug sine synthesis function,
	                         7 debug stochastic analysis function,
	                         8 debug stochastic synthesis function,
	                         9 debug top level analysis function,
	                         10 debug everything 
	                         11 write residual sound into file 
							             12 write original, synthesis and residual
							                to a text file */
	float fWindowSize;    /* window size in number of periods */
	int iFrameRate;	      /* number of frames per second */
	float fFreqDeviation; /* maximum frequency deviation for peak 
	                         continuation as a multiplicative factor */
	float fLowestFund;    /* lowest posible fundamental */
	float fHighestFund;   /* highest posible fundamental */
	float fDefaultFund;   /* default fundamental */
	float fPeakContToGuide; /* contribution of the continuation peak into
	                           the value of the guide */
	float fFundContToGuide; /* contribution of current fundamental to
	                           current guide (between 0 and 1) */
	int nGuides;	         /* number of guides to use */
	int nTrajectories;	   /* number of trajectories to use */
	int nStochasticCoeff;  /* number of filter coefficients */
	int iFormat;	         /* 1 for monophonic harmonic sounds, 
	                          2 for others */
	int iStochasticType;   /* 1 for IIR filter, 2 for line-segments,
	                          3 for no stochastic analysis */
	int iCleanTracks;        /* 1 if we want to clean trajectories,
	                          0 if not */
	float fMinRefHarmMag;  /* minimum magnitude in dB of reference harmonic 
	                          peak */
	float fRefHarmMagDiffFromMax;/* maximum magnitude difference 
	                                from reference harmonic peak to the maximum
	                                magnitude peak */
	int iRefHarmonic;	      /* reference harmonic to use in the harmonic 
	                           detection */	
	float fMinTrajLength;   /* minimum length of trajectory in seconds */
	float fMaxSleepingTime; /* maximum sleeping time for the trajectories
	                           in seconds */		
	float fHighestFreq;     /* highest frequency to be searched */
	float fMinPeakMag;      /* minimum magnitude in dB for a good peak */	
	int iSoundType;         /* type of sound to be analyzed */		
	int iAnalysisDirection;    /* analysis direction, direct or reverse */
	int iWindowType;        /* type of analysis window */		 				  	 			 
	 				  	 
} ARGUMENTS;
