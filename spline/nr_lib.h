#ifndef NR_LIB_H
#define NR_LIB_H

#define NR_NO 0
#define NR_YES 1

extern void NRNewIdum(long *pIdum);
extern void NRInitIdum(long *pIdum,int *isInitIdum);

extern void covsrt(float **covar, int ma, int ia[], int mfit);
extern void gaussj(float **a, int n, float **b, int m);
extern void mrqcof(float x[],float y[],float sig[],int ndata,float a[],
		   int ia[],int ma, float **alpha, float beta[], float *chisq,
		   void (*funcs)(float, float [], float *, float [], int));
extern void mrqmin(float x[],float y[],float sig[],int ndata,float a[],
		   int ia[],int ma, float **covar, float **alpha, float *chisq,
		   void (*funcs)(float, float [], float *, float [], int),
		   float *alamda);
extern float gammp(float a, float x);
extern float gammq(float a, float x);
extern void gcf(float *gammcf, float a, float x, float *gln);
extern void gser(float *gamser, float a, float x, float *gln);
extern int nr_gamm_sign;
extern float gammln(float xx);
extern void medfit(float x[],float y[],int ndata,float *a,float *b,
		   float *abdev);
extern float rofunc(float b);

extern long ran0Idum;
extern int isInitRan0Idum;
extern float ran0(long *idum);

extern long ran1Idum;
extern int isInitRan1Idum;
extern float ran1(long *idum);

extern long ran2Idum;
extern int isInitRan2Idum;
extern float ran2(long *idum);

extern long ran3Idum;
extern int isInitRan3Idum;
extern float ran3(long *idum);

extern long ran4Idum;
extern int isInitRan4Idum;
extern float ran4(long *idum);

extern void psdes(unsigned long *lword, unsigned long *irword);
extern float gasdev(long *idum,float (*ran)(long *));
extern void fit(float x[],float y[],int ndata,float sig[],int mwt,float *a,
		float *b,float *siga,float *sigb,float *chi2,float *q);

#endif /* NR_LIB__H */
