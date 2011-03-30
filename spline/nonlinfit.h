extern void NonLinFitInit(float **a,int **ia,int ma,float ***covar, float ***alpha);
extern void NonLinFitDelete(float *a,int *ia,int ma,float **covar, float **alpha);
extern void NonLinFit(float x[],float y[],float sigmaY[], int ndata, float a[], int ia[],
	int ma, float **covar, float **alpha, float *chisq,
	void (*funcs)(float, float [], float *, float [], int));
extern void ScaleInvariance(float lnMass,float a[],float *pLnRadius,float dLnR_da[],int ma);
extern void PolynomeFit(float x,float a[],float *y,float dyda[],int monome_number);
extern float NonLinFitConfidence(float chisq,int ndata,int ma);
extern void MWC(float h,float a[],float *F,float dFda[],int ma);
extern void BrowIncrCorr(float t,float a[],float *C,float dCda[],int ma);
extern void DNQ(float x,float a[],float *f,float dfda[],int ma);
extern void Gauss(float x,float a[],float *g,float dgda[],int ma);
