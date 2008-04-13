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

#define PHI(I_,J_)	(pFPhi+(I_)*(nMax)+(J_))

/* subroutine: CovLatticeHarm
 * covariance lattice routine for harmonic mean method; it computes the
 * reflection coefficient of stage m, given the covariance matrix of the
 * signal and predictor coefficients up to stage m-1.
 *-----------------------------------------------------------------------
 */
void CovLatticeHarm (float *pFPhi, int nMax, int m, float *pFPredCoeff, 
                     float *pFReflexCoeff, float *pFError, float *pFScr)
{
  int _do0, _do1, _do2, _do3, i, i_, ip1, j, j_, mm1, mm2, mmi, mp1, 
      mp1mi, mp1mj;
  float c, fplusb, sum1, sum3, sum4, sum6, sum7, sum9, y;

  if( m > 1 )
    goto L_20;

  /* explicit computation of the first stage reflection coefficient */
  fplusb = *PHI(0,0) + *PHI(1,1);
  c = *PHI(1,0);
  *pFReflexCoeff = 0;
  if( c == 0.0 )
    goto L_10;
  *pFReflexCoeff = -2.*c/fplusb;
L_10:
  pFPredCoeff[0] = *pFReflexCoeff;
  goto L_90;

  /* recursive computation of the m-th stage (m.ge.2) reflection coefficient */
L_20:
  mp1 = m + 1;
  mm1 = m - 1;
  sum1 = 0;
  sum3 = 0;
  sum4 = 0;
  sum6 = 0;
  for( i = 1, _do0 = mm1; i <= _do0; i++ )
  {
    i_ = i - 1;
    ip1 = i + 1;
    pFScr[i_] = pFPredCoeff[i_];
    mp1mi = mp1 - i;
    sum1 = sum1 + pFPredCoeff[i_]*(*PHI(ip1 - 1,0) + *PHI(mp1mi - 1,mp1 - 1));
    sum3 = sum3 + pFPredCoeff[i_]*(*PHI(mp1mi - 1,0) + *PHI(mp1 - 1,ip1 - 1));
    y = pow(pFPredCoeff[i_], 2.0);
    sum4 = sum4 + y*(*PHI(ip1 - 1,ip1 - 1) + *PHI(mp1mi - 1,mp1mi - 1));
    sum6 = sum6 + y**PHI(mp1mi - 1,ip1 - 1);
  }
  sum7 = 0;
  sum9 = 0;
  if( m == 2 )
    goto L_60;
  mm2 = m - 2;
  for( i = 1, _do1 = mm2; i <= _do1; i++ )
  {
    i_ = i - 1;
    ip1 = i + 1;
    mp1mi = mp1 - i;
    for( j = ip1, _do2 = mm1; j <= _do2; j++ )
    {
      j_ = j - 1;
      y = pFPredCoeff[i_]*pFPredCoeff[j_];
      mp1mj = mp1 - j;
      sum7 = sum7 + y*(*PHI(j_ + 1,ip1 - 1) + *PHI(mp1mj - 1,mp1mi - 1));
      sum9 = sum9 + y*(*PHI(mp1mj - 1,ip1 - 1) + *PHI(mp1mi - 1,j_ + 1));
    }
  }
L_60:
  fplusb = *PHI(0,0) + *PHI(mp1 - 1,mp1 - 1) + 2.*(sum1 + sum7) + sum4;
  c = *PHI(mp1 - 1,0) + sum3 + sum6 + sum9;
  *pFReflexCoeff = 0;
  if( c == 0.0 )
    goto L_70;
  *pFReflexCoeff = -2.*c/fplusb;
L_70:
	;

  /* insert call to user's own subroutine to quantize the reflection
   * coefficient pFReflexCoeff
   *
   * recursion to convert reflection coefficients to predictor
   * coefficients
   */
  for( i = 1, _do3 = mm1; i <= _do3; i++ )
  {
    i_ = i - 1;
    mmi = m - i;
    pFPredCoeff[i_] = pFScr[i_] + *pFReflexCoeff*pFScr[mmi - 1];
  }
  pFPredCoeff[m - 1] = *pFReflexCoeff;

  /* compute minimum forward-plus-backward error at the output of
   * the m-th lattice stage
   */
L_90:
  *pFError = fplusb*(1. - pow(*pFReflexCoeff, 2.0));

  return;
} /* end of function */


/*-----------------------------------------------------------------------
 * subroutine: Covariance
 * given the signal, this routine generates its covariance matrix
 * using the relation:   
 * phi(i,j)=sum[sig(pFReflexCoeff+1-i)*sig(pFReflexCoeff+1-j)] 
 * over the range from 
 * pFReflexCoeff=nstage+1 to pFReflexCoeff=nsig.
 *-----------------------------------------------------------------------
 */
void Covariance (float *pFSig, int nSig, int nStage, float *pFPhi, int nMax)
{
  int _do0, _do1, _do2, _do3, _do4, i, i_, im1, j, j_, k, k_, kp1mj, 
	 np1, np2, np2mi, np2mj, nsp2, nsp2mi, nsp2mj;
  float temp;


  np1 = nStage + 1;
  np2 = nStage + 2;
  nsp2 = nSig + 2;
  for( j = 1, _do0 = np1; j <= _do0; j++ )
  {
    j_ = j - 1;
    temp = 0;
    for( k = np1, _do1 = nSig; k <= _do1; k++ )
    {
      k_ = k - 1;
      kp1mj = k + 1 - j;
      temp = temp + pFSig[k_] * pFSig[kp1mj - 1];
    }
    *PHI(j_,0) = temp;
  }
  for( i = 2, _do2 = np1; i <= _do2; i++ )
  {
    i_ = i - 1;
    im1 = i - 1;
    for( j = 1, _do3 = im1; j <= _do3; j++ )
    {
      j_ = j - 1;
      *PHI(j_,i_) = *PHI(i_,j_);
    }
    np2mi = np2 - i;
    nsp2mi = nsp2 - i;
    for( j = i, _do4 = np1; j <= _do4; j++ )
    {
      j_ = j - 1;
      np2mj = np2 - j;
      nsp2mj = nsp2 - j;
      *PHI(j_,i_) = *PHI(j_ - 1,i_ - 1) + pFSig[np2mi - 1] * pFSig[np2mj - 1] - 
                    pFSig[nsp2mi - 1] * pFSig[nsp2mj - 1];
    }
  }
  return;
} /* end of function */
