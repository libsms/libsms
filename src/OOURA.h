#ifndef _OOURA_H
#define _OOURA_H



#define NMAX 8192
#define NMAXSQRT 64

void rdft(int n, int isgn, float *a, int *ip, float *w);

void makewt(int nw, int *ip, float *w);
void makect(int nc, int *ip, float *c);
void bitrv2(int n, int *ip, float *a);
void cftfsub(int n, float *a, float *w);
void cftbsub(int n, float *a, float *w);
void rftfsub(int n, float *a, int nc, float *c);
void rftbsub(int n, float *a, int nc, float *c);

void cft1st(int n, float *a, float *w);
void cftmdl(int n, int l, float *a, float *w);

#endif /* _OURA_H */
