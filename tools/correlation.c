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
#include "../sms.h"

#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr

static float *vector(nl,nh)
int nl,nh;
{
  float *v;

  v=(float *)malloc((unsigned) (nh-nl+1)*sizeof(float));
  if (!v) quit("allocation failure in vector()");
  return v-nl;
}

static void free_vector(v,nl,nh)
float *v;
int nl,nh;
{
  free((char*) (v+nl));
}

static void four1(float *data, int nn, int isign)
{
  int n,mmax,m,j,istep,i;
  double wtemp,wr,wpr,wpi,wi,theta;
  float tempr,tempi;

  n=nn << 1;
  j=1;
  for (i=1;i<n;i+=2) {
    if (j > i) {
      SWAP(data[j],data[i]);
      SWAP(data[j+1],data[i+1]);
    }
    m=n >> 1;
    while (m >= 2 && j > m) {
      j -= m;
      m >>= 1;
    }
    j += m;
  }
  mmax=2;
  while (n > mmax) {
    istep=2*mmax;
    theta=6.28318530717959/(isign*mmax);
    wtemp=sin(0.5*theta);
    wpr = -2.0*wtemp*wtemp;
    wpi=sin(theta);
    wr=1.0;
    wi=0.0;
    for (m=1;m<mmax;m+=2) {
      for (i=m;i<=n;i+=istep) {
        j=i+mmax;
        tempr=wr*data[j]-wi*data[j+1];
        tempi=wr*data[j+1]+wi*data[j];
        data[j]=data[i]-tempr;
        data[j+1]=data[i+1]-tempi;
        data[i] += tempr;
        data[i+1] += tempi;
      }
      wr=(wtemp=wr)*wpr-wi*wpi+wr;
      wi=wi*wpr+wtemp*wpi+wi;
    }
    mmax=istep;
  }
}

static void twofft(data1,data2,fft1,fft2,n)
float data1[],data2[],fft1[],fft2[];
int n;
{
  int nn3,nn2,jj,j;
  float rep,rem,aip,aim;
  void four1();

  nn3=1+(nn2=2+n+n);
  for (j=1,jj=2;j<=n;j++,jj+=2) {
    fft1[jj-1]=data1[j];
    fft1[jj]=data2[j];
  }
  four1(fft1,n,1);
  fft2[1]=fft1[2];
  fft1[2]=fft2[2]=0.0;
  for (j=3;j<=n+1;j+=2) {
    rep=0.5*(fft1[j]+fft1[nn2-j]);
    rem=0.5*(fft1[j]-fft1[nn2-j]);
    aip=0.5*(fft1[j+1]+fft1[nn3-j]);
    aim=0.5*(fft1[j+1]-fft1[nn3-j]);
    fft1[j]=rep;
    fft1[j+1]=aim;
    fft1[nn2-j]=rep;
    fft1[nn3-j] = -aim;
    fft2[j]=aip;
    fft2[j+1] = -rem;
    fft2[nn2-j]=aip;
    fft2[nn3-j]=rem;
  }
}

void correl(data1,data2,n,ans)
float data1[],data2[],ans[];
int n;
{
	int no2,i;
	float dum,*fft,*vector();
	void twofft(),realft(),free_vector();

	fft=vector(1,2*n);
	twofft(data1,data2,fft,ans,n);
	no2=n/2;
	for (i=2;i<=n+2;i+=2) {
		ans[i-1]=(fft[i-1]*(dum=ans[i-1])+fft[i]*ans[i])/no2;
		ans[i]=(fft[i]*dum-fft[i-1]*ans[i])/no2;
	}
	ans[2]=ans[n+1];
	realft(ans,no2,-1);
	free_vector(fft,1,2*n);
}

#undef SWAP
