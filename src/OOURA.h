#ifndef _OOURA_H
#define _OOURA_H

#ifndef sfloat
# ifdef DOUBLE_PRECISION
#  define sfloat double
# else
#  define sfloat float
# endif
#endif

#define NMAX 8192
#define NMAXSQRT 64

#define DECORATE(x) _sms_ooura_##x

#define rdft DECORATE(rdft)
#define makewt DECORATE(makewt)
#define makect DECORATE(makect)
#define bitrv2 DECORATE(bitrv2)
#define cftfsub DECORATE(cftfsub)
#define cftbsub DECORATE(cftbsub)
#define rftfsub DECORATE(rftfsub)
#define rftbsub DECORATE(rftbsub)
#define cft1st DECORATE(cft1st)
#define cftmdl DECORATE(cftmdl)


void rdft(int n, int isgn, sfloat *a, int *ip, sfloat *w);

void makewt(int nw, int *ip, sfloat *w);
void makect(int nc, int *ip, sfloat *c);
void bitrv2(int n, int *ip, sfloat *a);
void cftfsub(int n, sfloat *a, sfloat *w);
void cftbsub(int n, sfloat *a, sfloat *w);
void rftfsub(int n, sfloat *a, int nc, sfloat *c);
void rftbsub(int n, sfloat *a, int nc, sfloat *c);

void cft1st(int n, sfloat *a, sfloat *w);
void cftmdl(int n, int l, sfloat *a, sfloat *w);

#endif /* _OURA_H */
