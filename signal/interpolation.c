/*
 *   Copyright 2002 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Pierre Kestener.
 *
 *  The author may be reached (Email) at the address
 *      kestener@crpp.u-bordeaux.fr
 *
 */

#include "signal_int.h"

/*
 */
Signal *
sig_to_pol_int_derivative (Signal *sig_in,
			   int order)
{
  Signal *result;
  int    i,n;
  float  *Y1, *Y2, *dY1, *dY2;
  float  deltax, *datainY, *dataoutY, *datainX, *dataoutX;

  datainY  = sig_in->dataY;
  n = sig_in->n;

  if (sig_in->dataX!=NULL) {
    datainX  = sig_in->dataX;
    deltax = (sig_in->dataX[1]-sig_in->dataX[0])/100.0;
  } else {
    deltax = 1/100.0;
  }

  Y1  = (float *) malloc((size_t) (1*sizeof(float)));
  dY1 = (float *) malloc((size_t) (1*sizeof(float)));
  Y2  = (float *) malloc((size_t) (1*sizeof(float)));
  dY2 = (float *) malloc((size_t) (1*sizeof(float)));

  result = sig_new (sig_in->type, 0, sig_in->n - 1);
  result->dx = sig_in->dx;
  result->x0 = sig_in->x0;

  dataoutX = result->dataX;
  dataoutY = result->dataY;

   if (!result)
    return 0;
   
   if (order==1) {
     /*for (i = 0; i < 4; i++){
       dataoutY[i] = 0;
       dataoutX[i] = datainX[i];
       }
       for (i = 4; i < n-4; i++){
       polint(datainX+i-4, datainY+i-4, 9, datainX[i]-deltax, Y1, dY1);
       polint(datainX+i-4, datainY+i-4, 9, datainX[i]+deltax, Y2, dY2);
       dataoutY[i] = (*Y2 - *Y1)/2/deltax;
	   dataoutX[i] = datainX[i];
	   }
	 for (i = n-4; i < n; i++){
	 dataoutY[i] = 0;
	 dataoutX[i] = datainX[i];
	 }*/
     for (i = 1; i < n-2; i++){
       if (i==1) {
	 *Y1 = CubicInterpolate(datainY[0],datainY[1],datainY[2],datainY[3],-1.01);
	 *Y2 = CubicInterpolate(datainY[0],datainY[1],datainY[2],datainY[3],-0.99);
	 dataoutY[0] = (*Y2 - *Y1)/2/0.01;
       }
       *Y1=CubicInterpolate(datainY[i-1],datainY[i],datainY[i+1],datainY[i+2],-0.01);
       *Y2=CubicInterpolate(datainY[i-1],datainY[i],datainY[i+1],datainY[i+2],0.01);
       dataoutY[i] = (*Y2 - *Y1)/2/0.01;
       if (i==n-3) {
	 *Y1 = CubicInterpolate(datainY[n-4],datainY[n-3],datainY[n-2],datainY[n-1],0.99);
	 *Y2 = CubicInterpolate(datainY[n-4],datainY[n-3],datainY[n-2],datainY[n-1],1.01);
	 dataoutY[n-2] = (*Y2 - *Y1)/2/0.01;
	 *Y1 = CubicInterpolate(datainY[n-4],datainY[n-3],datainY[n-2],datainY[n-1],1.99);
	 *Y2 = CubicInterpolate(datainY[n-4],datainY[n-3],datainY[n-2],datainY[n-1],2.01);
	 dataoutY[n-1] = (*Y2 - *Y1)/2/0.01;
       }
     }
   } else {
     /*for (i = 0; i < 1; i++){
       dataoutY[i] = 0;
       dataoutX[i] = datainX[i];
       }
       for (i = 1; i < n-1; i++){
       polint(datainX+i-1, datainY+i-1, 3, datainX[i]-deltax, Y1, dY1);
       polint(datainX+i-1, datainY+i-1, 3, datainX[i]+deltax, Y2, dY2);
       dataoutY[i] = 2*(*Y2 - 2*datainY[i] + *Y1)/deltax/deltax;
       dataoutX[i] = datainX[i];
       }
       for (i = n-1; i < n; i++){
       dataoutY[i] = 0;
       dataoutX[i] = datainX[i];
       }*/
     for (i = 1; i < n-2; i++){
       if (i==1) {
	 *Y1 = CubicInterpolate(datainY[0],datainY[1],datainY[2],datainY[3],-1.05);
	 *Y2 = CubicInterpolate(datainY[0],datainY[1],datainY[2],datainY[3],-0.95);
	 dataoutY[0] = (*Y2 -2*datainY[i] + *Y1)/0.05/0.05;
       }
       *Y1=CubicInterpolate(datainY[i-1],datainY[i],datainY[i+1],datainY[i+2],-0.05);
       *Y2=CubicInterpolate(datainY[i-1],datainY[i],datainY[i+1],datainY[i+2],0.05);
       dataoutY[i] = (*Y2 -2*datainY[i] + *Y1)/0.05/0.05;
       if (i==n-3) {
	 *Y1 = CubicInterpolate(datainY[n-4],datainY[n-3],datainY[n-2],datainY[n-1],0.95);
	 *Y2 = CubicInterpolate(datainY[n-4],datainY[n-3],datainY[n-2],datainY[n-1],1.05);
	 dataoutY[n-2] = (*Y2 -2*datainY[i] + *Y1)/0.05/0.05;
	 dataoutY[n-1] = (*Y2 -2*datainY[i] + *Y1)/0.05/0.05;
       }
     }  
   }
 
   if (sig_in->dataX!=NULL) {
     for (i=0;i<n;i++) {
       dataoutX[i] = datainX[i];
     }
   }
  
   free(Y1);
   free(Y2);
   free(dY1);
   free(dY2);
   
   return result;
}

void polint(float *xa, float *ya, int n, float x, float *y, float *dy)
     /*Given arrays xa[1..n] and ya[1..n], and given a value x, this 
       routine returns a value y, and an error estimate dy.
       If P(x)is the polynomial of degree N - 1 such that P(xa_i) = ya_i ;
       i= 1..n, then the returned value y = P(x). */
{
  int i,m,ns=1;
  double den,dif,dift,ho,hp,w;
  double *c,*d; 
  
  dif=fabs(x-xa[0]);
  c = (double *) malloc((size_t) (n*sizeof(double)));
  d = (double *) malloc((size_t) (n*sizeof(double)));
  for (i=0;i<=n-1;i++) { 
    /*Here we find the index ns of the closest table entry*/
    if ( (dift=fabs(x-xa[i])) < dif) {
      ns=i;
      dif=dift;
    }
    c[i]=ya[i];    /* and initialize the tableau of c's and d's.*/
    d[i]=ya[i];
  }
  *y=ya[ns--];

  /*This is the initial approximation to y.*/
  for (m=0;m<n-1;m++) {      /* For each column of the tableau,*/
    for (i=0;i<=n-m-2;i++) { /*we loop over the current c's and d's
			     and update them.*/
      ho=xa[i]-x;
      hp=xa[i+m+1]-x;
      w=c[i+1]-d[i];
      if ( (den=ho-hp) == 0.0) printf("Error in routine polint located in signal/interpolation.c\n");
      /*This error can occur only if two input xa's are 
	(to within roundo ) identical.*/
      den=w/den;
      d[i]=hp*den; 
      /*Here the c's and d's are updated.*/
      c[i]=ho*den;
    }
    
    *y += (*dy=(2*ns < (n-m) ? c[ns+1] : d[ns--]));
    /*After each column in the tableau is completed, we decide which
      correction, c or d, we want to add to our accumulating value of y,
      i.e., which path to take through the tableau|forking up or down. 
      We do this in such a way as to take the most straight line route
      through the tableau to its apex, updating ns accordingly to keep 
      track of where we are. This route keeps the partial approximations
      centered (insofar as possible) on the target x. Thelastdy added is
      thus the error indication.*/
  }
  free(c); 
  free(d); 
}


double CubicInterpolate(double y0, double y1, double y2, double y3, double mu)
{
  double a0,a1,a2,a3,mu2;
  
  mu2 = mu*mu;
  a0 = y3 - y2 - y0 + y1;
  a1 = y0 - y1 - a0;
  a2 = y2 - y0;
  a3 = y1;
  
  return(a0*mu*mu2+a1*mu2+a2*mu+a3);
}


/*
 */
Signal *
sig_to_spline_int_derivative (Signal *sig_in,
			   int order)
{
  Signal *result;
  int    i,n;
  float  *Y1, *Y2, *dY1, *dY2;
  float  deltax, *datainY, *dataoutY, *datainX, *dataoutX;
  float  yp0, ypn_1;
  float  *deriv2;

  datainY  = sig_in->dataY;
  datainX  = sig_in->dataX;

  n = sig_in->n;

  deltax = (sig_in->dataX[1]-sig_in->dataX[0])/100.0;
  Y1  = (float *) malloc((size_t) (1*sizeof(float)));
  dY1 = (float *) malloc((size_t) (1*sizeof(float)));
  Y2  = (float *) malloc((size_t) (1*sizeof(float)));
  dY2 = (float *) malloc((size_t) (1*sizeof(float)));

  result = sig_new (sig_in->type, 0, sig_in->n - 1);
  result->dx = sig_in->dx;
  result->x0 = sig_in->x0;

  dataoutX = result->dataX;
  dataoutY = result->dataY;

  deriv2 = (float *) malloc((size_t) (n*sizeof(float)));

  if (!result)
    return 0;
  
  /* we begin with second derivative computation */
  yp0   = datainY[1]-datainY[0];
  ypn_1 = datainY[n-1]-datainY[n-2];
  spline(datainX, datainY, n, yp0, ypn_1, deriv2);
  
  if (order==1) {
    for (i=0;i<n;i++) {
      splint(datainX, datainY, deriv2, n, datainX[i]-0.05, Y1);
      splint(datainX, datainY, deriv2, n, datainX[i]+0.05, Y2);
      dataoutY[i] = (*Y2 - *Y1)/2/0.05;      
    }
  } else {
    for (i=0;i<n;i++) {
      dataoutY[i]=deriv2[i];
    }
    
  }
  

  for (i=0;i<n;i++) {
    dataoutX[i] = datainX[i];
  }

  
  free(Y1);
  free(Y2);
  free(dY1);
  free(dY2);
  
  return result;
}

void spline(float *x, float *y, int n, float yp0, float ypn_1, float *y2)
     /*Given arrays x[0..n-1] and y[0..n-1] containing a tabulated function, 
       i.e., y i = f(xi), with x0<x1< :::<xN-1 , and given values yp0 and 
       ypn_1 for the  rst derivative of the interpolating function at points
       1 and n, respectively, this routine returns an array y2[0..n-1] that
       contains the second derivatives of the interpolating function at the
       tabulated points xi. If yp0 and/or ypn_1 are equal to 1x10^30 or larger,
       the routine is signaled to set the corresponding boundary condition 
       for a natural spline, with zero second derivative on that boundary.*/
{
  int i,k; 
  float p,qn,sig,un,*u;
  u = (float *) malloc((size_t) ((n-1)*sizeof(float)));

  if (yp0 > 0.99e30)  /*The lower boundary condition is set either to be
			natural"*/
    y2[0]=u[0]=0.0;
  else {              /*or else to have a speci ed  rst derivative.*/
    y2[0] = -0.5; 
    u[0]=(3.0/(x[1]-x[0]))*((y[1]-y[0])/(x[1]-x[0])-yp0);
  }

  for (i=1;i<=n-2;i++) { /*This is the decomposition loop of the tridiagonal
			   algorithm. y2 and u are used for temporary storage
			   of the decomposed factors.*/
    sig=(x[i]-x[i-1])/(x[i+1]-x[i-1]);
    p=sig*y2[i-1]+2.0;
    y2[i]=(sig-1.0)/p;
    u[i]=(y[i+1]-y[i])/(x[i+1]-x[i]) - (y[i]-y[i-1])/(x[i]-x[i-1]);
    u[i]=(6.0*u[i]/(x[i+1]-x[i-1])-sig*u[i-1])/p;
  }
  if (ypn_1 > 0.99e30)  /*The upper boundary condition is set either to be
			"natural"*/
    qn=un=0.0;
  else {              /*or else to have a specied  frst derivative.*/
    qn=0.5;
    un=(3.0/(x[n-1]-x[n-2]))*(ypn_1-(y[n-1]-y[n-2])/(x[n-1]-x[n-2])); 
  }
  y2[n-1]=(un-qn*u[n-2])/(qn*y2[n-2]+1.0);

  for (k=n-2;k>=0;k--)    /* This is the backsubstitution loop of the
			     tridiagonal algorithm.*/
    y2[k]=y2[k]*y2[k+1]+u[k];

  free(u);
}


void splint(float *xa, float *ya, float *y2a, int n, float x, float *y)
     /*Given the arrays xa[0..n-1] and ya[0..n-1], which tabulate a function
       (with the xai's in order), and given the array y2a[0..n-1], which is
       the output from spline above, and given a value of x, this routine
       returns a cubic-spline interpolated value y.*/
{
  
  int klo,khi,k; 
  float h,b,a;
  klo=0; 
  
  /*We will find the right place in the table by means of bisection. 
    This is optimal if sequential calls to this routine are at random
    values of x. If sequential calls are in order, and closely spaced,
    one would do better tostoreprevious values ofklo and khi and test
    if they remain appropriate on the next call.*/

  khi=n-1;
  while (khi-klo > 1) { 
    k=(khi+klo) >> 1;
    if (xa[k] > x) khi=k; 
    else klo=k;
  }

  /*klo and khi now bracket the input value of x.*/
  
  h=xa[khi]-xa[klo];
  if (h == 0.0) printf("Bad xa input to routine splint"); 
  /*The xa's must be distinct.*/

  a=(xa[khi]-x)/h;
  b=(x-xa[klo])/h;

  /*Cubic spline polynomial is now evaluated.*/

  *y=a*ya[klo]+b*ya[khi]+((a*a*a-a)*y2a[klo]+(b*b*b-b)*y2a[khi])*(h*h)/6.0;
}



/*
  the following code is taken from web site (march 19th 2002):
  http://astronomy.swin.edu.au/~pbourke/curves/spline/
  by Paul Bourke (nov 1996)
  Interpolation by B-Splines !!!

  Take care that N = nb_points-1 !

 */
Signal *
sig_to_spline2_int_derivative (Signal *sig_in,
			       int order, int direction)
{
  Signal *result;
  int    i,n;
  float  deltax, *datainY, *dataoutY, *datainX, *dataoutX;

  int T = 4; //ordre du spline
  int *knots;
  int resolution;
  XY  *input, *output;


  datainY  = sig_in->dataY;
  datainX  = sig_in->dataX;
  n = sig_in->n;

  deltax = (sig_in->dataX[1]-sig_in->dataX[0])/100.0;

  result = sig_new (sig_in->type, 0, sig_in->n - 1);
  if (!result)
    return 0;
  result->dx = sig_in->dx;
  result->x0 = sig_in->x0;

  dataoutX = result->dataX;
  dataoutY = result->dataY;

  knots  = (int *) malloc((size_t) ((n+T)*sizeof(int)));
  input  = (XY *) malloc((size_t) ((n)*sizeof(XY)));
  resolution = 20*n;
  output = (XY *) malloc((size_t) ((resolution)*sizeof(XY)));

  // on remplit input
  for (i=0;i<n;i++) {
    input[i].x = (double)datainX[i];
    input[i].y = (double)datainY[i];
  }
  
  // on calcule les noeuds
  SplineKnots(knots,n-1,T);
  // on determine la courbe !!!
  SplineCurve(input,n-1,knots,T,output,resolution);
  

  if (direction==ALONGY) {
    if (order==1) {
      for (i=0;i<n;i++) {
	dataoutY[i] = (float) (output[20*i+11].y-output[20*i+9].y)/2/(1/20.0);
      }
    } else if (order==2){
      for (i=0;i<n;i++) {
	dataoutY[i] = (float) (output[20*i+11].y -2*output[20*i+10].y +output[20*i+9].y)/(1/20.0)/(1/20.0);      
      }
    }
  } else if (direction==ALONGX) {
    if (order==1) {
      for (i=0;i<n;i++) {
	dataoutY[i] = (float) (output[i].x)/2/(1/20.0);      
      }
    } else if (order==2){
      for (i=0;i<n;i++) {
	dataoutY[i] = (float) (output[20*i+11].x -2*output[20*i+10].x +output[20*i+9].x)/(1/20.0)/(1/20.0);      
      }
    }
  }
    

  for (i=0;i<n;i++) {
    dataoutX[i] = datainX[i];
    //dataoutX[i] = i;
  }

  free(knots);
  free(input);
  free(output);

  return result;
}

Signal *
sig_to_spline3_int_derivative (Signal *sig_in,
			       int order, int direction)
{
  Signal *result;
  int    i,j,n;
  float  deltax, *datainY, *dataoutY, *datainX, *dataoutX;

  int T = 7; //ordre du spline
  int N = 6;
  int *knots;
  int resolution;
  XY  *input, *output;


  datainY  = sig_in->dataY;
  datainX  = sig_in->dataX;
  n = sig_in->n;

  deltax = (sig_in->dataX[1]-sig_in->dataX[0])/100.0;

  result = sig_new (sig_in->type, 0, sig_in->n - 1);
  if (!result)
    return 0;
  result->dx = sig_in->dx;
  result->x0 = sig_in->x0;

  dataoutX = result->dataX;
  dataoutY = result->dataY;

  knots  = (int *) malloc((size_t) ((N+1+T)*sizeof(int)));
  input  = (XY *) malloc((size_t) ((n)*sizeof(XY)));
  resolution = 300;
  output = (XY *) malloc((size_t) (resolution*sizeof(XY)));

  // on remplit input
  for (i=0;i<n;i++) {
    input[i].x = (double)datainX[i];
    input[i].y = (double)datainY[i];
  }
  

  SplineKnots(knots,N,T);
  //SplineCurve(input,n-1,knots,T,output,resolution);
  

  if (direction==ALONGY) {
    if (order==1) {
      for (i=N/2;i<n-N/2;i++) {
	if (i==N/2) {
	  SplineCurve(input,N,knots,T,output,resolution);
	  dataoutY[0] = (float) (output[2].y-output[0].y)/2;
	  dataoutY[1] = (float) (output[51].y-output[49].y)/2;
	  dataoutY[2] = (float) (output[101].y-output[99].y)/2;
	}
	SplineCurve(input+i-3,N,knots,T,output,resolution);
	//dataoutY[i] =0;
	/*for (j=1;j<=10;j++) {
	  dataoutY[i] += (float) (output[resolution/2+j].y-output[resolution/2-j].y);
	  }*/
	dataoutY[i] = (float) (output[resolution/2+2].y-1*output[resolution/2+1].y+1*output[resolution/2-1].y-output[resolution/2-2].y)/2;
	//dataoutY[i] = (float) (output[resolution/2].y);
	if (i==n-4) {
	  SplineCurve(input+n-7,N,knots,T,output,resolution);
	  dataoutY[n-3] = (float) (output[201].y-output[199].y);
	  dataoutY[n-2] = (float) (output[251].y-output[249].y);
	  dataoutY[n-1] = (float) (output[299].y-output[297].y);
	}
      }
    } else if (order==2){
      for (i=N/2;i<n-N/2;i++) {
	if (i==N/2) {
	  SplineCurve(input,N,knots,T,output,resolution);
	  dataoutY[0] = (float) (output[2].y-2*output[1].y+output[0].y)/2;
	  dataoutY[1] = (float) (output[51].y-2*output[50].y+output[49].y)/2;
	  dataoutY[2] = (float) (output[101].y-2*output[100].y+output[99].y)/2;
	}
	SplineCurve(input+i-3,N,knots,T,output,resolution);
	/*dataoutY[i] =0;
	  for (j=1;j<=10;j++) {
	  dataoutY[i] += (float) (output[resolution/2+j].y+output[resolution/2-j].y);
	  }
	  dataoutY[i] -= (float) (20*output[resolution/2].y);*/
	dataoutY[i] = (float) (output[resolution/2+2].y-2*output[resolution/2+1].y+2*output[resolution/2-1].y-output[resolution/2-2].y);
	//dataoutY[i] = (float) (output[resolution/2+30].y+output[resolution/2+10].y-4*output[resolution/2].y+output[resolution/2-10].y+output[resolution/2-30].y)/2;
	//dataoutY[i] = (float) (output[resolution/2].y);
	if (i==n-4) {
	  SplineCurve(input+n-7,N,knots,T,output,resolution);
	  dataoutY[n-3] = (float) (output[201].y-output[199].y);
	  dataoutY[n-2] = (float) (output[251].y-output[249].y);
	  dataoutY[n-1] = (float) (output[299].y-output[297].y);
	}
      }
    }
  } else if (direction==ALONGX) {
    if (order==1) {
      
    } else if (order==2){
      
    }
  }
    

  for (i=0;i<n;i++) {
    dataoutX[i] = datainX[i];
    //dataoutX[i] = i;
  }

  free(knots);
  free(input);
  free(output);

  return result;
}



/*
   This returns the point "output" on the spline curve.
   The parameter "v" indicates the position, it ranges from 0 to n-t+2   
*/
void SplinePoint(int *u,int n,int t,double v,XY *control,XY *output)
{
   int k;
   double b;

   output->x = 0;
   output->y = 0;

   for (k=0;k<=n;k++) {
      b = SplineBlend(k,t,u,v);
      output->x += control[k].x * b;
      output->y += control[k].y * b;
   }
}

/*
   Calculate the blending value, this is done recursively.
   
   If the numerator and denominator are 0 the expression is 0.
   If the deonimator is 0 the expression is 0
*/
double SplineBlend(int k,int t,int *u,double v)
{
   double value;

   if (t == 1) {
      if ((u[k] <= v) && (v < u[k+1]))
         value = 1;
      else
         value = 0;
   } else {
      if ((u[k+t-1] == u[k]) && (u[k+t] == u[k+1]))
         value = 0;
      else if (u[k+t-1] == u[k]) 
         value = (u[k+t] - v) / (u[k+t] - u[k+1]) * SplineBlend(k+1,t-1,u,v);
      else if (u[k+t] == u[k+1])
         value = (v - u[k]) / (u[k+t-1] - u[k]) * SplineBlend(k,t-1,u,v);
     else
         value = (v - u[k]) / (u[k+t-1] - u[k]) * SplineBlend(k,t-1,u,v) + 
                 (u[k+t] - v) / (u[k+t] - u[k+1]) * SplineBlend(k+1,t-1,u,v);
   }
   return(value);
}

/*
   The positions of the subintervals of v and breakpoints, the position
   on the curve are called knots. Breakpoints can be uniformly defined
   by setting u[j] = j, a more useful series of breakpoints are defined
   by the function below. This set of breakpoints localises changes to
   the vicinity of the control point being modified.
*/
void SplineKnots(int *u,int n,int t)
{
   int j;

   for (j=0;j<=n+t;j++) {
     if (j < t)
       u[j] = 0;
     else if (j <= n)
       u[j] = j - t + 1;
     else if (j > n)
       u[j] = n - t + 2;
     //u[j]=j;
   }
}

/*-------------------------------------------------------------------------
   Create all the points along a spline curve
   Control points "input", "n" of them.
   Knots "knots", degree "t".
   Ouput curve "output", "res" of them.
*/
void SplineCurve(XY *input,int n,int *knots,int t,XY *output,int res)
{
   int i;
   double interval,increment;

   interval = 0;
   increment = (n - t + 2) / (double)(res - 1);
   for (i=0;i<res-1;i++) {
      SplinePoint(knots,n,t,interval,input,&(output[i]));
      interval += increment;
   }
   output[res-1] = input[n];
}

/*
   Example of how to call the spline functions
        Basically one needs to create the control points, then compute
   the knot positions, then calculate points along the curve.
*/

//int knots[N+T+1];
/*#define RESOLUTION 200
  XY output[RESOLUTION];

  int main(int argc,char **argv)
  {
  int i;
  
  SplineKnots(knots,N,T);
  SplineCurve(input,N,knots,T,output,RESOLUTION);
  
  
  printf("LIST\n");
  printf("{ = SKEL\n");
  printf("%d %d\n",RESOLUTION,RESOLUTION-1);
  for (i=0;i<RESOLUTION;i++)
  printf("%g %g %g\n",output[i].x,output[i].y,output[i].z);
  for (i=0;i<RESOLUTION-1;i++)
  printf("2 %d %d 1 1 1 1\n",i,i+1);
  printf("}\n");
  
  
  printf("{ = SKEL 3 2  0 0 4  0 0 0  4 0 0  2 0 1 0 0 1 1 2 1 2 0 0 1 1 }\n");
  
  
  printf("{ = SKEL\n");
  printf("%d %d\n",N+1,N);
  for (i=0;i<=N;i++)
  printf("%g %g %g\n",input[i].x,input[i].y,input[i].z);
  for(i=0;i<N;i++)
  printf("2 %d %d 0 1 0 1\n",i,i+1);
  printf("}\n");
  
  }
*/

