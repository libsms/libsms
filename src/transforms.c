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
/*! \file fourier.c 
 * \brief routines for different Fast Fourier Transform Algorithms
 *
 * look at the very bottom of this file for the global fft call, sms_rdft()
 */

#include "sms.h"
#include "OOURA.h"

/* -_-_-_-_-_-_-_-_-_-_-_- FFTW3 Floating Point -_-_-_-_-_-_-_-_-_-_-_- */
/* FFTW3 is currently working for terminal-based programs, but not in the pd externals. */
#ifdef FFTW
/* hmm.. still crashing.. */
int sms_allocFourierForward( float *pWaveform, fftwf_complex *pSpectrum, int sizeFft ) 
{
        if((pWaveform = fftwf_malloc(sizeof(float) * sizeFft)) == NULL)
        {
                printf("\n sms_allocFourierForward: could not allocate spectrum array for fftw \n");
                return (SMS_FFTWERR);
        }
        if((pSpectrum = fftwf_malloc(sizeof(fftwf_complex) * (sizeFft/2 + 1))) == NULL)
        {
                printf("\n sms_allocFourierForward: could not allocate spectrum array for fftw \n");
                return (SMS_FFTWERR);
        }
        return(SMS_OK);
}
#endif /* FFTW */



/*! \brief main call to Fast Fourier Transform
 *
 * Currently, this function calls the OOURA routines to calculate
 * either a forward or backward FFT.
 *
 * \param sizeFft         size of the FFT in samples (must be a power of 2 >= 2)
 * \param pFReal pointer to real array (n >= 2, n = power of 2)
 * \param direction    direction of transform. 1 for forward, -1 for reverse
 */
void sms_rdft(  int sizeFft, float *pFReal, int direction )
{ 
        static int ip[NMAXSQRT +2];
        static float w[NMAX * 5 / 4];

        rdft( sizeFft, direction, pFReal, ip, w);
}
