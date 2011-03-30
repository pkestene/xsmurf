#include <math.h>
#include <stdio.h>

#include "nr_lib.h"
#define NRANSI
#include "nrutil.h"

/* See Numerical Recipes in C 15.5 for a description of the Levnberg-Marquardt
   method */

/* mrqmin() DESCRIPTION:
   
   Non linear fitting routine for ndata points.
   Array's index goes from 1 to ndata: x[1..ndata], y[1..ndata], sigmaY[1..ndata].

   The non linear function depends on ma parameters a[1..ma], ia[1..ma] is non zero
   for those parameter that should be fitted for (zero indicates constant parameter).
   The routine returns current best fit estimation in a[] and chi square in chiqs.

   The arrays covar[1..ndata][1..ndata], alpha[1..ndata][1..ndata] are used as 
   working space during most iterations.

   funcs(float x, float a[], float *yfit, float dyda[], int ma) is the fiting function
   that evaluates the fitting function yfit and its derivatives dyda[1..ma] with
   respect to parameters a[1..ma] at point x.

   USAGE: On first call provide an initial guess for the parameters a[] and set 
          alamda<0 for initialisation (which set alamda = 0.001).
	  If a step succeeds chisq becomes smaller and alamda decreases by a 
	  factor of 10.
	  If a step fails alamda grows by a factor of 10.
	  You must call this routine repeatedly until convergence is achieved.
	  Then make one final call with alamda = 0, so that covar[][] returns 
	  the covariance matrix, and alpha the curvature matrix.
          
   */

void NonLinFitInit(float **a,int **ia,int ma,float ***covar, float ***alpha)
{
  *a = vector(1,ma);
  *ia = ivector(1,ma);
  *covar = matrix(1,ma,1,ma);
  *alpha = matrix(1,ma,1,ma);
}

void NonLinFitDelete(float *a,int *ia,int ma,float **covar, float **alpha)
{
  free_vector(a,1,ma);
  free_ivector(ia,1,ma);
  free_matrix(covar,1,ma,1,ma);
  free_matrix(alpha,1,ma,1,ma);
}

#define DELTACHIQ 0.001

#ifdef USED_WITH_LW
extern void Printf(char *format,...);
extern void Flush(void);
#endif

void NonLinFit(float x[],float y[],float sigmaY[], int ndata, float a[], int ia[],
	int ma, float **covar, float **alpha, float *chisq,
	void (*funcs)(float, float [], float *, float [], int))
{
  float alamda,alamdaprev;
  float chisqprev;

  //int i;

  /* Initialisation */
  alamda = -1.0;
  mrqmin(x,y,sigmaY,ndata,a,ia,ma,covar,alpha,chisq,funcs,&alamda);
  
  /* Main loop */
  while(1)
    {
#ifdef USED_WITH_LW
      /*      
	 for(i=1;i<=ma;i++)
	 Printf("%f ",a[i]);
	 Printf("\n");
	 */
      Printf("*");
      Flush();
#endif
      alamdaprev = alamda;
      chisqprev = *chisq;
      mrqmin(x,y,sigmaY,ndata,a,ia,ma,covar,alpha,chisq,funcs,&alamda);
      if(alamda > alamdaprev)
	continue;
      if( (chisqprev - *chisq) < DELTACHIQ )
	break;
    }
#ifdef USED_WITH_LW
  Printf("\n");
#endif
  /* Get the result */
  alamda = 0.0;
  mrqmin(x,y,sigmaY,ndata,a,ia,ma,covar,alpha,chisq,funcs,&alamda);

  return;
}
#undef DELTACHIQ

/* Here are the different models for fitting */

/* Scale invariance broken by min and max scale */

void ScaleInvariance(float lnMass,float a[],float *pLnRadius,float dLnR_da[],int ma)
{
  /* a[1] = dzeta
     a[2] = beta
     a[3] = lnMo
     */
  float one_beta,lnM_Mo;
  float tempLn,temp;

  if(ma != 3)
    nrerror("Scaleinvariance(): is a 3 parameters model !");

  one_beta = 1/a[2];
  lnM_Mo = lnMass-a[3];

  /* ln(beta/dzeta*ln(M/Mo) +1) */
  tempLn = (float) log((double) (a[2]*lnM_Mo/a[1] + 1.0));
  /* 1/(beta*ln(M/Mo) + dzeta) */
  temp = 1.0/(a[2]*lnM_Mo+a[1]);

  *pLnRadius = one_beta*tempLn;

  if(dLnR_da != NULL)
    {
      /*dLnR_ddzeta */
      dLnR_da[1] = -lnM_Mo*temp/a[1];
      /* dLnR_dBeta */
      dLnR_da[2] = one_beta*(lnM_Mo*temp - one_beta*tempLn);
      /* dLnR_dBeta */
      dLnR_da[3] = -temp;
    }

}

/* polynomial fitting: to test the routines */
void PolynomeFit(float x,float a[],float *y,float dyda[],int monome_number)
{
  float xn =1.0;
  int i;

  *y = a[1];
  if(dyda != NULL) dyda[1] = 1.0;

  for(i=2;i<=monome_number;i++)
    {
      xn *= x;
      *y += a[i]*xn;
      if(dyda != NULL) dyda[i] = xn;
    }
}

float NonLinFitConfidence(float chisq,int ndata,int ma)
{
  return( gammq((float) (0.5*(ndata-ma)),0.5*chisq) );
}


/* Millner, Witten and Cates model for polymer brushes */
#define MWC_RADIUS 85
#define PREFACT 40.e-9*M_PI/9.
void MWC(float h,float a[],float *F,float dFda[],int ma)
{
  float f0;
  float h0,h02,h03,h05,h06;
  float h2,h3,h5,h6;
  float temp;

  if(ma != 2)
    nrerror("MWC(): is a 2 parameters model !");

  h0 = a[1];
  f0 = a[2];

  h2 = h*h;
  h3 = h2*h;
  h5 = h3*h2;
  h6 = h3*h3;
  h02 = h0*h0;
  h03 = h02*h0;
  h05 = h03*h02;
  h06 = h03*h03;

  temp = PREFACT*(MWC_RADIUS*(h0/h - 0.5 + 0.1 - h5/(h05*320.) - 0.5 + h2/(h02*8.)) + 0.5*(h0-h/2.+h0*log(2.*h0/h)) - (h0-h/2.)/10. + (64.*h06 - h6)/(3840.*h05) + (h0-h/2.)/2. - (8*h03-h3)/(48*h02));

  *F = f0*temp;

  if(dFda != NULL)
    {
      dFda[2] = temp;
      dFda[1] = PREFACT*f0*(MWC_RADIUS*(1./h + h5/(64.*h06) - 0.25*h2/h03) + 1. + 0.5*log(2*h0/h)-0.1+1/60.+h6/(768.*h06)+0.5-1/6.-h3/(24.*h03));
    }
}
#undef MWC_RADIUS
#undef PREFACT

/* To fit the correletion function of fractinnal brownian motion
   increments.
   Parameter are: sigma (a[1]), H (a[2]) and the gap delta (a[3])
   */
static float D_Dh(float x,float h)
{
  double xx;

  xx = fabs((double) x);
  
  if(x == 0.)
    return 0.;
  else 
    return (float) (2.*log(xx)*pow(xx,2.*h));
}
static float D_Dd(float x,float h)
{
  double xx;

  xx = fabs((double) x);
  
  if(x == 0.)
    return 0.;
  else if (x > 0.)
    return (float) (pow(xx,2.*h-1.));
  else 
    return (float) (-pow(xx,2.*h-1.));
}

void BrowIncrCorr(float t,float a[],float *C,float dCda[],int ma)
{
  float s,h,d;
  float temp;

  if(ma != 3)
    nrerror("BrowIncrCorr(): is a 2 parameters model !");

  s = a[1];
  h = a[2];
  d = a[3];

  temp = pow(fabs(t+d),2.*h)+pow(fabs(t-d),2.*h)-2.*pow(fabs(t),2.*h);

  *C = s*s*temp/2.;

  if(dCda != NULL)
    {
      dCda[1] = s*temp;
      dCda[2] = s*s/2.*(D_Dh(t+d,h)+D_Dh(t-d,h)-2.*D_Dh(t,h));
      dCda[3] = s*s/2.*(D_Dd(d+t,h)+D_Dd(d-t,h));
    }
}
/* To fit the density function of |X|^q when X is N(),sigma).
   Parameter are: sigma (a[1]) and q (a[2]).
   */
void DNQ(float x,float a[],float *f,float dfda[],int ma)
{
  double res,x1sq,xd,temp;
  double sigma,q;

  if(ma != 2)
    nrerror("DNQ(): is a 2 parameters model !");

  if(a[1] <= 0. || a[2] == 0.)
    nrerror("DNQ(): bag range for parameters");
  
  if(x <= 0.)
    nrerror("DNQ(): argument must be strickly positive");
  
  sigma = a[1];
  q = a[2];
  
  xd = x;
  x1sq = pow(xd,1/q);
  res = sqrt(M_2_PI)*x1sq*exp(-x1sq*x1sq/(2*sigma*sigma))/(sigma*fabs(q)*xd);
  
  *f = res;

  if(dfda!= NULL)
    {
      temp = x1sq*x1sq/(sigma*sigma)-1.;
      dfda[1] = res*temp/sigma;
      dfda[2] = -res*(1+log(x)*temp/q)/q;
    }
  
}

/* To fit the gauss function.
   Parameter are: sigma (a[1]), m (a[2]) and a prefactor norm (a[3]).
   */
void Gauss(float x,float a[],float *g,float dgda[],int ma)
{
  double res,xd,temp;
  double sigma,m,norm;

  if(ma != 3)
    nrerror("Gauss(): is a 2 parameters model !");

  if(a[1] == 0.)
    nrerror("Gauss(): bag range for sigma");
  /*
    if(a[3] <= 0.)
    nrerror("Gauss(): bag range for norm");  
    */
  sigma = a[1];
  m = a[2];
  norm = a[3];
  
  xd = x;
  temp = exp(-(xd-m)*(xd-m)/(sigma*sigma));
  res = norm*temp;

  *g = res;

  if(dgda != NULL)
    {
      dgda[3] = temp;
      temp = 2.*(xd-m)*res/(sigma*sigma);
      dgda[1] = (xd-m)*temp/sigma;
      dgda[2] = temp; 
    }
  
}
#undef NRANSI
