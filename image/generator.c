/*
 * generator.c --
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: generator.c,v 1.13 1998/07/27 14:45:34 decoster Exp $
 *  Modified by Pierre Kestener.(june 2000)
 */

#include "image_int.h"

/* The largest number rand will return (same as INT_MAX).  */
/* normally defined in stdlib.h */
#ifndef RAND_MAX
#define RAND_MAX        2147483647
#endif

/*
 * I know nearly nothing of the following functions...
 */

#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

enum {RAN1, RAN3, GAUSS, EXPO};

real 
_Ran1_(long *idum)
{
  int         j;
  long        k;
  static long iy = 0;
  static long iv[NTAB];
  real       temp;

  if (*idum <= 0 || !iy) {
    if (-(*idum) < 1)
      *idum = 1;
    else
      *idum = -(*idum);
    for (j = NTAB+7; j >= 0; j--) {
      k = (*idum)/IQ;
      *idum = IA*(*idum-k*IQ)-IR*k;
      if (*idum < 0)
	*idum += IM;
      if (j < NTAB)
	iv[j] = *idum;
    }
    iy = iv[0];
  }
  k = (*idum)/IQ;
  *idum = IA*(*idum-k*IQ)-IR*k;
  if (*idum < 0)
    *idum += IM;
  j = iy/NDIV;
  iy = iv[j];
  iv[j] = *idum;
  if ((temp = AM*iy) > RNMX)
    return (RNMX);
  else
    return (temp);
}


#define IM1 2147483563
#define IM2 2147483399
#define AM (1.0/IM1)
#define IMM1 (IM1-1)
#define IA1 40014
#define IA2 40692
#define IQ1 53668
#define IQ2 52774
#define IR1 12211
#define IR2 3791
#define NTAB 32
#define NDIV (1+IMM1/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)
real _Ran2_(long *idum)
//       Long period (> 2 * 10 puiss 18) random number generator of LEcuyer with Bays-Durham shuffle and added safeguards. Returns a uniform random deviate between 0.0 and 1.0 (exclusive of the endpoint values). Call with idum a negative integer to initialize; thereafter, do not alter idum between successive deviates in a sequence. RNMX should approximate the largest floating value that is less than 1.
{ 
  int j;
  long k;
  static long idum2=123456789;
  static long iy=0;
  static long iv[NTAB];
  float temp;
  if (*idum <= 0) {    //Initialize.
    if (-(*idum) < 1) *idum=1;    //Be sure to prevent idum = 0.
    else *idum = -(*idum);
    idum2=(*idum);
    for (j=NTAB+7;j>=0;j--) {    //Load the shuffle table (after 8 warm-ups).
      k=(*idum)/IQ1;
      *idum=IA1*(*idum-k*IQ1)-k*IR1;
      if (*idum < 0) *idum += IM1;
      if (j < NTAB) iv[j] = *idum;
    }iy=iv[0];
  }k=(*idum)/IQ1;    //Start here when not initializing.
  *idum=IA1*(*idum-k*IQ1)-k*IR1;    //Compute idum=(IA1*idum) % IM1 without
  if (*idum < 0) *idum += IM1;    //overflows by Schrage s method.
  k=idum2/IQ2;
  idum2=IA2*(idum2-k*IQ2)-k*IR2;    //Compute idum2=(IA2*idum) % IM2 likewise.
  if (idum2 < 0) idum2 += IM2;
  j=iy/NDIV;    //Will be in the range 0..NTAB-1.
  iy=iv[j]-idum2;    //Here idum is shuffled, idum and idum2 are
  iv[j] = *idum;    //combined to generate output.
  if (iy < 1) iy += IMM1;
  if ((temp=AM*iy) > RNMX) return RNMX; //Because users dont expect endpoint values.
  else return temp;
}


#define MBIG 1000000000
#define MSEED 161803398
#define MZ 0
#define FAC (1.0/MBIG)

real 
_Ran3_(long *idum)
{
  static int  inext, inextp;
  static long ma[56];
  static int  iff = 0;
  real       mj, mk;
  int         i, ii, k;

  if (*idum <= 0 || iff == 0) {
    iff = 1;
    mj = MSEED-(*idum < 0 ? -*idum : *idum);
    mj = (int)mj%MBIG;
    ma[55] = mj;
    mk = 1;
    for (i = 1; i <= 54; i++) {
      ii = (21*i)%55;
      ma[ii] = mk;
      mk = mj-mk;
      if (mk < MZ)
	mk += MBIG;
      mj = ma[ii];
    }
    for (k = 1; k <= 4; k++) {
      for (i = 1; i <= 55; i++) {
	ma[i] -= ma[1+(i+30)%55];
	if (ma[i] < MZ)
	  ma[i] += MBIG;
      }
      inext = 0;
      inextp = 31;
      *idum = 1;
    }
  }
  if (++inext == 56)
    inext = 1;
  if (++inextp == 56)
    inextp = 1;
  mj = ma[inext]-ma[inextp];
  if (mj < MZ)
    mj += MBIG;
  ma[inext] = mj;
  return (mj*FAC);
}

/* based on RAN2 (see numerical recipes) */

real
_Gasdev_(long *idum)
{
  static int iset = 0;
  static real gset;
  real fac, rsq, v1, v2;

  if (iset == 0)
    {
      do
	{
	  v1 = 2.0 * _Ran2_ (idum) - 1.0;
	  v2 = 2.0 * _Ran2_ (idum) - 1.0;
	  rsq = v1 * v1 + v2 * v2;
	}
      while (rsq >= 1.0 || rsq == 0.0);
      fac = sqrt (-2.0 * log (rsq) / rsq);
      gset = v1 * fac;
      iset = 1;
      return (v2 * fac);
    }
  else
    {
      iset = 0;
      return (gset);
    }
}

float Gammln (float xx)
{
  double x,y,tmp,ser,sinus;
  static double cof[6]={76.18009172947146,-86.50532032941677,
			24.01409824083091,-1.231739572450155,
			0.1208650973866179e-2,-0.5395239384953e-5};
  int j;
 
  y=x=xx;
  tmp=x+5.5;
  tmp -= (x+0.5)*log(tmp);
  ser=1.000000000190015;
  for (j=0;j<=5;j++) ser += cof[j]/++y;
  return -tmp+log(2.5066282746310005*ser/x);
}

real poidev(float xm, long *idum) {

  static float sq,alxm,g,oldm=(-1.0);
  float em,t,y;
  if (xm < 12.0) { 
    if (xm != oldm) {
      oldm=xm;
      g=exp(-xm);
    }
    em = -1;
    t=1.0;
    do { 
      ++em;
      t *= _Ran1_(idum);
    } while (t > g);
  } else { 
    if (xm != oldm) { 
      oldm=xm; 
      sq=sqrt(2.0*xm);
      alxm=log(xm);
      g=xm*alxm-Gammln(xm+1.0);
    }
    do {
      do {
	y=tan(M_PI*_Ran1_(idum)); 
	em=sq*y+xm; 
      } while (em < 0.0); 
      em=floor(em); 
      t=0.9*(1.0+y*y)*exp(em*alxm-Gammln(em+1.0)-g);
    } while (_Ran1_(idum) > t);
  }
  return em;
}

real myRandUnif () {

  return rand()/(RAND_MAX+1.0); /* between 0 and 1 */

}

int
_Puiss2_(int a)
{
  int i = 1;
  while (i < a)
    i *= 2;
  // vrai retour
  //return (i <= 2048 ? 2 * i : i);
  // faux retour; utile pour faire moins de 
  return i;
}


/*
 */
Image *
im_diamond (int dim)
{ 
  Image * image;
  int   i,j;

  image = im_new (dim, dim, dim*dim, PHYSICAL);

  if (!image)
    /*    return GenErrorMemoryAlloc(interp);*/
    return 0;

  im_set_0 (image);
  for (i = 0; i < dim; i++)
    for (j = 0; j < dim; j++)
      image -> data[i * dim + j] = (256.0
				    *(fabs ((double) (i + j - dim)) < dim / 4)
				    *(fabs ((double) (i - j)) < dim / 4));

  return image;
}

/*
 * Create an Image with 1, 2 or 3 "diracs".
 *   dimx, dimy : Image dimensions.
 *   dist : if dist is non null the Image contains 2 diracs with
 *          2*dist pixel along x's between them.
 *   distx : non null means 3 diracs.
 *   disty : ...
 */
Image *
im_dirac (int dimx,
	  int dimy,
	  int dist,
	  int distx,
	  int disty)
{ 
  Image * image;
 
  image = im_new (dimx, dimy, dimx*dimy, PHYSICAL);

  if (!image)
    /*    return GenErrorMemoryAlloc(interp);*/
    return 0;

  im_set_0 (image);

  if (dist > 0)
    {
      image -> data[dimx*(dimy/2) + dimx/2 + dist] = 100.0;
      image -> data[dimx*(dimy/2) + dimx/2 - dist] = 100.0;
    }
  else if (distx > 0)
    {
      image -> data[dimx*(dimy/2 + disty) + dimx/2 + distx] = 100.0;
      image -> data[dimx*(dimy/2 + disty) + dimx/2 - distx] = 100.0;
      image -> data[dimx*(dimy/2 - disty) + dimx/2] = 100.0;
    }
  else
    image -> data[dimx*(dimy/2) + dimx/2] = 100.0;

  return image;
}

/*
  recall : type = 0 is for gaussian
  type = 1 is for x derivative of a gaussian
  type = 2 is for y derivative of a gaussian

*/
Image *
im_gauss (int size,
	  int x0,
	  int y0,
	  real scale,
	  int type,
	  real factor)
{ 
  Image * image;
  int i, j;

  image = im_new (size, size, size*size, PHYSICAL);

  /*if (!image)
    return GenErrorMemoryAlloc(interp);*/

  im_set_0 (image);

  if (type==0) {
    for (i=0; i<size; i++) {
      for (j=0; j<size; j++) {
	image -> data[i+size*j] = factor*scale*exp( -((i-x0)*(i-x0)+(j-y0)*(j-y0))/scale/scale);
      }
    }
  } else if (type==1) {
    for (i=0; i<size; i++) {
      for (j=0; j<size; j++) {
	image -> data[i+size*j] = factor*(i-x0)/scale * exp( -((i-x0)*(i-x0)+(j-y0)*(j-y0))/scale/scale);
      }
    }
  } else if (type==2) {
    for (i=0; i<size; i++) {
      for (j=0; j<size; j++) {
	image -> data[i+size*j] = factor*(j-y0)/scale * exp( -((i-x0)*(i-x0)+(j-y0)*(j-y0))/scale/scale);
      }
    }
  }

  return image;
}

/*
  Modification of im_gauss to have ellipsoidal gaussian distributions
  AK 2005 10 05
*/
Image *
im_gauss_ellipse (int size,
		  int x0,
		  int y0,
		  real scale,
		  real a,
		  real b)
{ 
  Image * image;
  int i, j;

  image = im_new (size, size, size*size, PHYSICAL);

  /*if (!image)
    return GenErrorMemoryAlloc(interp);*/

  im_set_0 (image);

  for (i=0; i<size; i++) {
    for (j=0; j<size; j++) {
      image -> data[i+size*j] = scale*exp( -(a*(i-x0)*(i-x0)+b*(j-y0)*(j-y0))/scale/scale);
    }
  }
  
  return image;
}


/*
  Cell nuclei simulation based on Joerg's preliminary attempts with poisson dev
  The user specifies the size of the image (512, 1024, etc), the radius of the cell nucleus,
  the radius of objects A (chromosome territory) and B, the density (given by a poisson dist) 
  of the background (dens_nuc), and the density of the two objects (dens_A, dens_B).
  AK 2005 10 17
*/
/* Editions de Pierre 2005 12 15
 */

Image *
im_cell (int size,
	 int rad_nuc,
	 int rad_A,
	 int rad_B,
	 real dens_nuc,
	 real dens_A,
	 real dens_B)
{
  Image * image;
  int i, j;
  long iseed;
  real x;
  real A[2], B[2];
  real *data;
 
  /* initialize random generator */
  //srand ( time(0) );
  struct timeval tv; // C requires "struct timval" instead of just "timeval"
  gettimeofday(&tv, 0);
    
  // use BOTH microsecond precision AND pid as seed
  long int n = tv.tv_usec * getpid(); 
  srand(n);
 
  /* initiate seed with time */
  //srand(time(NULL));
  iseed = rand();

  /* modify iseed */
  x = _Ran1_(&iseed);
  x = _Ran3_(&iseed);
 
  image = im_new (size, size, size*size, PHYSICAL);
 
  if (!image)
    return 0;
 
  im_set_0 (image);
 
  /* Randomly determine the positions of the two objects, A & B
     1. Make sure that their whole area is within the nucleus.
     2. Make sure that their areas do not overlap.
  */
  A[0]=0;
  A[1]=0;
  B[0]=0;
  B[1]=0;
  while ( ( sqrt( (A[0]-size/2)*(A[0]-size/2) + (A[1]-size/2)*(A[1]-size/2) ) > (rad_nuc - rad_A)) || ( sqrt((B[0]-size/2)*(B[0]-size/2) + (B[1]-size/2)*(B[1]-size/2) ) > (rad_nuc - rad_B)) || sqrt( (A[0]-B[0])*(A[0]-B[0]) + (A[1]-B[1])*(A[1]-B[1]) ) <= (rad_A + rad_B) ) {
    A[0] = myRandUnif()*(2*rad_nuc) + size/2-rad_nuc;
    A[1] = myRandUnif()*(2*rad_nuc) + size/2-rad_nuc;
    B[0] = myRandUnif()*(2*rad_nuc) + size/2-rad_nuc;
    B[1] = myRandUnif()*(2*rad_nuc) + size/2-rad_nuc;
  }
  //    printf("%f /t %f /t %f /t %f /n", A[0], A[1], B[0], B[1]);
  /* data pointer */
  printf("%p \n", &iseed);
  printf("%f \t %f \t %f \n", dens_A, dens_B, dens_nuc);

  data = image->data;
  for (i=0; i<size; i++) {
    for (j=0; j<size; j++) {
      if ( ( sqrt( (i-size/2)*(i-size/2) + (j-size/2)*(j-size/2)) <= 
	     rad_nuc) && ( sqrt( (i-A[0])*(i-A[0]) + (j-A[1])*(j-A[1])) > rad_A)){
	data[i+size*j] = poidev(dens_nuc,&iseed);
      }
      if ( ( sqrt( (i-size/2)*(i-size/2) + (j-size/2)*(j-size/2)) <= 
	     rad_nuc) && ( sqrt( (i-B[0])*(i-B[0]) + (j-B[1])*(j-B[1])) > rad_B)){
	data[i+size*j] = poidev(dens_nuc,&iseed);
      }
      if ( ( sqrt( (i-size/2)*(i-size/2) + (j-size/2)*(j-size/2)) <= 
	     rad_nuc) && ( sqrt( (i-A[0])*(i-A[0]) + (j-A[1])*(j-A[1])) <= rad_A)){
	data[i+size*j] = poidev(dens_A,&iseed);
      }
      if ( ( sqrt( (i-size/2)*(i-size/2) + (j-size/2)*(j-size/2)) <= 
	     rad_nuc) && ( sqrt( (i-B[0])*(i-B[0]) + (j-B[1])*(j-B[1])) <= rad_B)){
	data[i+size*j] = poidev(dens_B,&iseed);
      }
      if ( sqrt( (i-size/2)*(i-size/2) + (j-size/2)*(j-size/2)) > rad_nuc){
	data[i+size*j] = 0;
      }
    }
  }
  
 
  /* Filter image with point-spread function (PSF).
     For Widefield, PSF = Airy Disc.
     For Confocal, PSF = (Airy Disc)^2.
  */
    
  /* We assume that the radius of the cell nucleus is
     8 microns (diameter = 16 microns). So let's say 8192 nm.
     And the pixel size is therefore 8192 nm / rad_nuc (nm/pixel).
     We will often use rad_nuc = 256, so that we have 32 nm/pixel.
       
     Now, in what follows, r is in nm. We need to translate it into pixels.
     To do so, we multiply r by (8192 / rad_nuc).
  */
    
  /* Do nothing here for now, just write out the image.
     See if Pierre can help.
     2005 12 14
  */
    
  return image;
}





/*
  Cell nuclei simulation.
  Same as above (im_cell) but with ellipses instead of circles
  AK 2006 09 14
*/

Image *
im_cell_elli (int size,
	      int rad_nuc,
	      int a_A,
	      int b_A,
	      int a_B,
	      int b_B,
	      real dens_nuc,
	      real dens_A,
	      real dens_B)
{
  Image * image;
  int i, j;
  long iseed;
  real x;
  real A[2], B[2];
  real *data;
    
  /* initialize random generator */
  //srand ( time(0) );
  struct timeval tv; // C requires "struct timval" instead of just "timeval"
  gettimeofday(&tv, 0);
    
  // use BOTH microsecond precision AND pid as seed
  long int n = tv.tv_usec * getpid(); 
  srand(n);
 
  /* initiate seed with time */
  //srand(time(NULL));
  iseed = rand();
  /* modify iseed */
  x = _Ran1_(&iseed);
  x = _Ran3_(&iseed);
 
  image = im_new (size, size, size*size, PHYSICAL);
 
  if (!image)
    return 0;
 
  im_set_0 (image);
 
  /* Randomly determine the positions of the two objects, A & B
     1. Make sure that their whole area is within the nucleus.
     2. Make sure that their areas do not overlap.
  */


  A[0]=0;
  A[1]=0;
  B[0]=0;
  B[1]=0;
  while ( ( sqrt( (A[0]-size/2)*(A[0]-size/2) + (A[1]-size/2)*(A[1]-size/2) ) > (rad_nuc - a_A)) || ( sqrt((B[0]-size/2)*(B[0]-size/2) + (B[1]-size/2)*(B[1]-size/2) ) > (rad_nuc - a_B)) || sqrt( (A[0]-B[0])*(A[0]-B[0]) + (A[1]-B[1])*(A[1]-B[1]) ) < (a_A + a_B) ) {
    A[0] = myRandUnif()*(2*rad_nuc) + size/2-rad_nuc;
    A[1] = myRandUnif()*(2*rad_nuc) + size/2-rad_nuc;
    B[0] = myRandUnif()*(2*rad_nuc) + size/2-rad_nuc;
    B[1] = myRandUnif()*(2*rad_nuc) + size/2-rad_nuc;
  }
  //    printf("%f \t %f \t %f \t %f \n", A[0], A[1], B[0], B[1]);

  real theta_A = myRandUnif()*(360);
  theta_A = 3.14159265/180*theta_A; /* on transforme theta en radians */
  real cost_A = cos(theta_A);
  real sint_A = sin(theta_A);
  real a_Af = (float) a_A;
  real b_Af = (float) b_A;
  real e_A = sqrt(1-(b_Af*b_Af)/(a_Af*a_Af));
  real xFocus_A = A[0] + a_A*e_A*cost_A;
  real yFocus_A = A[1] + a_A*e_A*sint_A;
  real xFocusp_A = A[0] - a_A*e_A*cost_A;
  real yFocusp_A = A[1] - a_A*e_A*sint_A;

  real theta_B = myRandUnif()*(360);
  theta_B = 3.14159265/180*theta_B; /* on transforme theta en radians */
  real cost_B = cos(theta_B);
  real sint_B = sin(theta_B);
  real a_Bf = (float) a_B;
  real b_Bf = (float) b_B;
  real e_B = sqrt(1-(b_Bf*b_Bf)/(a_Bf*a_Bf));
  real xFocus_B = B[0] + a_B*e_B*cost_B;
  real yFocus_B = B[1] + a_B*e_B*sint_B;
  real xFocusp_B = B[0] - a_B*e_B*cost_B;
  real yFocusp_B = B[1] - a_B*e_B*sint_B;
    
  //    printf("%d \n", rad_nuc);
  //    printf("%d \t %d \t %d \t %d \n", a_A, b_A, a_B, b_B);
  //    printf("%f \t %f \n", a_Af, b_Af);
  //    printf("%f \n", e_A);
  //    printf("%f \n", e_B);
 
  //    printf("%f \t %f \t %f \t %f \n", xFocus_A, yFocus_A, xFocusp_A, yFocusp_A);
  //    printf("%f \t %f \t %f \t %f \n", xFocus_B, yFocus_B, xFocusp_B, yFocusp_B);

  //    printf("%f \n", &iseed);
  //    printf("%f \t %f \t %f \n", dens_A, dens_B, dens_nuc);

  data = image->data;
  for (i = 0; i < size; i++) {
    for (j = 0; j < size; j++) {
      if ( sqrt( (i-size/2)*(i-size/2) + (j-size/2)*(j-size/2)) < rad_nuc ) {
	real dist_A = sqrt((i - xFocus_A)*(i - xFocus_A) + (j - yFocus_A)*(j - yFocus_A)) + sqrt((i - xFocusp_A)*(i - xFocusp_A) + (j - yFocusp_A)*(j - yFocusp_A));
	real dist_B = sqrt((i - xFocus_B)*(i - xFocus_B) + (j - yFocus_B)*(j - yFocus_B)) + sqrt((i - xFocusp_B)*(i - xFocusp_B) + (j - yFocusp_B)*(j - yFocusp_B));
	if ( dist_A < 2*a_A) {
	  data[i + j*size] = poidev(dens_A,&iseed);
	  //image -> data[i + j*size] = 15;
	}
	else if ( dist_B < 2*a_B) {
	  data[i + j*size] = poidev(dens_B,&iseed);
	  //image -> data[i + j*size] = 20;
	}
	else {
	  data[i + j*size] = poidev(dens_nuc,&iseed);
	  //image -> data[i + j*size] = 10;
	}
      }
    }
  }


  /*
  for (i=0; i<size; i++) {
    for (j=0; j<size; j++) {
      image -> data[i+size*j];
    }
  }
  */

  return image;
}

/*
  Same things as im_cell, but in 3D. 
  TO DO: icell3D

*/



/*
  Same things as im_cell, but in 3D (Max projection). The final output is still a 2D image
  since we are only considering the projections.
  
  Last modified: AK 2007 07 18

*/

Image *
im_cell3Dproj (int size,
	       int rad_nuc,
	       int rad_A,
	       int rad_B,
	       real dens_nuc,
	       real dens_A,
	       real dens_B)
{
  Image * image;
  int i, j, k;
  long iseed;
  real x;
  real A[3], B[3];
  real *data;
  double f[512][512][512];
 
  /* initiate seed with time */
  srand(time(NULL));
  iseed = rand();
  /* modify iseed */
  x = _Ran1_(&iseed);
  x = _Ran3_(&iseed);
 
  image = im_new (size, size, size*size, PHYSICAL);
 
  if (!image)
    return 0;
 
  im_set_0 (image);
 
  /* Randomly determine the positions of the two objects, A & B
     1. Make sure that their whole area is within the nucleus.
     2. Make sure that their areas do not overlap.
  */
  A[0]=0;
  A[1]=0;
  A[2]=0;
  B[0]=0;
  B[1]=0;
  B[2]=0;
  while ( ( sqrt( (A[0]-size/2)*(A[0]-size/2) + (A[1]-size/2)*(A[1]-size/2) + (A[2]-size/2)*(A[2]-size/2) ) > (rad_nuc - rad_A)) || ( sqrt((B[0]-size/2)*(B[0]-size/2) + (B[1]-size/2)*(B[1]-size/2) + (B[2]-size/2)*(B[2]-size/2) ) > (rad_nuc - rad_B)) || sqrt( (A[0]-B[0])*(A[0]-B[0]) + (A[1]-B[1])*(A[1]-B[1]) + (A[2]-B[2])*(A[2]-B[2]) ) <= (rad_A + rad_B) ) {
    A[0] = myRandUnif()*(2*rad_nuc) + size/2-rad_nuc;
    A[1] = myRandUnif()*(2*rad_nuc) + size/2-rad_nuc;
    A[2] = myRandUnif()*(2*rad_nuc) + size/2-rad_nuc;
    B[0] = myRandUnif()*(2*rad_nuc) + size/2-rad_nuc;
    B[1] = myRandUnif()*(2*rad_nuc) + size/2-rad_nuc;
    B[2] = myRandUnif()*(2*rad_nuc) + size/2-rad_nuc;
  }

  /* data pointer */
  data = image->data;
  for (i=0; i<size; i++) {
    for (j=0; j<size; j++) {
      for (k=0; k<size; k++) {
	if ( ( sqrt( (i-size/2)*(i-size/2) + (j-size/2)*(j-size/2) + (k-size/2)*(k-size/2)) <= 
	       rad_nuc) && ( sqrt( (i-A[0])*(i-A[0]) + (j-A[1])*(j-A[1]) + (k-A[2])*(k-A[2])) > rad_A)){
	  //data[i+size*j] = poidev(dens_nuc,&iseed);
	  f[i][j][k] = poidev(dens_nuc,&iseed);
	}
	if ( ( sqrt( (i-size/2)*(i-size/2) + (j-size/2)*(j-size/2) + (k-size/2)*(k-size/2)) <= 
	       rad_nuc) && ( sqrt( (i-B[0])*(i-B[0]) + (j-B[1])*(j-B[1]) + (k-B[2])*(k-B[2])) > rad_B)){
	  //data[i+size*j] = poidev(dens_nuc,&iseed);
	  f[i][j][k] = poidev(dens_nuc,&iseed);
	}
	if ( ( sqrt( (i-size/2)*(i-size/2) + (j-size/2)*(j-size/2) + (k-size/2)*(k-size/2)) <= 
	       rad_nuc) && ( sqrt( (i-A[0])*(i-A[0]) + (j-A[1])*(j-A[1]) + (k-A[2])*(k-A[2])) <= rad_A)){
	  //data[i+size*j] = poidev(dens_A,&iseed);
	  f[i][j][k] = poidev(dens_A,&iseed);
	}
	if ( ( sqrt( (i-size/2)*(i-size/2) + (j-size/2)*(j-size/2) + (k-size/2)*(k-size/2)) <= 
	       rad_nuc) && ( sqrt( (i-B[0])*(i-B[0]) + (j-B[1])*(j-B[1]) + (k-B[2])*(k-B[2])) <= rad_B)){
	  //data[i+size*j] = poidev(dens_B,&iseed);
	  f[i][j][k] = poidev(dens_B,&iseed);
	}
	if ( sqrt( (i-size/2)*(i-size/2) + (j-size/2)*(j-size/2) + (k-size/2)*(k-size/2)) > rad_nuc){
	  //data[i+size*j] = 0;
	  f[i][j][k] = 0;
	}
      }
    }
  }
    

  /* 
     Do the 3D --> 2D MAXIMUM projection
  */
  for (i=0; i<size; i++) {
    for (j=0; j<size; j++) {
      for (k=1; k<size; k++) {
	if (data[i+size*j] < f[i][j][k-1]) {
	  data[i+size*j] = f[i][j][k-1];
	}
      }
    }
  }
    
  return image;
}


/*
 */
Image *
im_step (int dim)
{ 
  Image * image;
  int i;
  
  image = im_new (dim ,dim ,dim *dim , PHYSICAL);

  if (!image)
    return 0;
    
  im_set_0 (image);  
  
  for (i = 0; i < dim*(dim/2); i++)
    image -> data[i] = 100.0;
  
  return image;
}

/*
 */
Image *
im_circle (int lx, int ly, int cx, int cy, int radius)
{
  Image * image;
  int i,j;

  image = im_new (lx, ly, lx*ly, PHYSICAL);

  if (!image)
    return 0;

  im_set_0 (image);

  for (i = 0; i < lx; i++)
    for (j = 0; j < ly; j++)
      if ((i - cx)*(i - cx) + (j - cy)*(j - cy) < radius*radius )
	image -> data[i + j*lx] = 1.0;
  
  return image;
}

/* ***********************************************************************
   generateur d'une image contenant une ellipse pleine au centre

   ********************************************************************  */

Image *
im_ellipse (int dim, real theta)
{
  Image * image;
  int i,j;
  real a2, b2;
  real cost, sint;

  image = im_new (dim, dim, dim*dim, PHYSICAL);

  if (!image)
    return 0;
  im_set_0 (image);

  theta = 3.14159265/180*theta; /* on transforme theta en radians */
  im_set_0 (image);
  a2 = 1.0/9;
  b2 = 1.0;
  cost = cos(theta);
  sint = sin(theta);

  for (i = 0; i < dim; i++)
    for (j = 0; j < dim; j++)
      if ((i - dim/2)*(i - dim/2)*(cost*cost/a2+sint*sint) + (j - dim/2)*(j - dim/2)*(sint*sint/a2+cost*cost) + (i - dim/2)*(j - dim/2)*2*cost*sint*(1/a2-1/b2) < dim*dim/256 )
	image -> data[i + j*dim] = 1.0;
  
  return image;
}

Image *
my_im_ellipse (int dim, int x, int y, real a, real b, real theta)
{
  Image * image;
  int i,j;
  real cost, sint;

  /*
    See Carroll & Ostlie, p. 28-29
    Here, b is a and a is b.
    So a must be less than b
  */
  theta = 3.14159265/180*theta; /* on transforme theta en radians */
  cost = cos(theta);
  sint = sin(theta);
  real e = sqrt(1-(a*a)/(b*b));
  real xFocus = x + b*e*cost;
  real yFocus = y + b*e*sint;
  real xFocusp = x - b*e*cost;
  real yFocusp = y - b*e*sint;

  image = im_new (dim, dim, dim*dim, PHYSICAL);

  if (!image)
    return 0;
  im_set_0 (image);

  for (i = 0; i < dim; i++) {
    for (j = 0; j < dim; j++) {
      real dist = sqrt((i - xFocus)*(i - xFocus) + (j - yFocus)*(j - yFocus)) + sqrt((i - xFocusp)*(i - xFocusp) + (j - yFocusp)*(j - yFocusp));
      if ( dist < 2*b) {
	//if ( (dist < 2*b+1 ) && (dist > 2*b-1) ) {
	image -> data[i + j*dim] = 1.0;
      }
    }
  }
  return image;
}



Image *
my_im_ellipse_3Dproj (int dim, int x, int y, int z, real a, real b, real theta, real phi)
{
  Image * image;
  int i,j,k;
  real cost, sint, cosp, sinp, dist;

  theta = 3.14159265/180*theta;
  phi   = 3.14159265/180*phi;
  cost = cos(theta);
  sint = sin(theta);
  cosp = cos(phi);
  sinp = sin(phi);

  real e = sqrt(1-(a*a)/(b*b));
  real xFocus = x + b*e*cost*cosp;
  real yFocus = y + b*e*sint*cosp;
  real zFocus = z + b*e*sinp;
  real xFocusp = x - b*e*cost*cosp;
  real yFocusp = y - b*e*sint*cosp;
  real zFocusp = z - b*e*sinp;

  image = im_new (dim, dim, dim*dim, PHYSICAL);

  if (!image)
    return 0;
  im_set_0 (image);

  for (i = 0; i < dim; i++) {
    for (j = 0; j < dim; j++) {
      for (k = 0; k < dim; k++) {
	dist = sqrt((i - xFocus)*(i - xFocus) + (j - yFocus)*(j - yFocus) + (k - zFocus)*(k - zFocus)) + sqrt((i - xFocusp)*(i - xFocusp) + (j - yFocusp)*(j - yFocusp) + (k - zFocusp)*(k - zFocusp));
	if ( dist < 2*b) {
	  //if ( (dist < 2*b+1 ) && (dist > 2*b-1) ) {
	  image -> data[i + j*dim] = 1.0;
	}
      }
    }
  }

  return image;
}


/*
  Cell nuclei simulation (3D) with two ellipsoidal-shaped CTs with poisson dev
  The user specifies the size of the image (512, 1024, etc), the radius of the cell nucleus,
  the axis lengths of objects A and B, the density (given by a poisson dist) 
  of the two objects (dens_A, dens_B), and the density of the background. 
  The number of z slices of also given.

  Note that for now, num_slices is not implemented.

  AK 2007 07 24
*/

Image *
im_cell_ellipsoid_proj (int size, 
			int rad_nuc,
			int a_A, 
			int b_A, 
			int c_A, 
			int a_B, 
			int b_B, 
			int c_B, 
			real dens_A,
			real dens_B,
			real dens_nuc,
			int num_slices)
{
  Image * image;
  int i,j,k;
  long iseed;
  real *data;
  real f[512][512][512];
  real g_A[512][512];
  real g_B[512][512];
  real h[512][512];
  real M_A[512][512][512];
  real M_B[512][512][512];
  int area_A=0, area_B=0;

  real i_A, j_A, k_A, i_B, j_B, k_B;
  real urand_A, vrand_A;
  real urand_B, vrand_B;
  real theta_x_A, theta_y_A;
  real cosx_A, sinx_A, cosy_A, siny_A, dist_A;
  real theta_x_B, theta_y_B;
  real cosx_B, sinx_B, cosy_B, siny_B, dist_B;
  real x_A=0, y_A=0, z_A=0;
  real x_B=0, y_B=0, z_B=0;

  /* initiate seed with time */
  srand(time(NULL));
  iseed = rand();
  /* modify iseed */
 
  image = im_new (size, size, size*size, PHYSICAL);
  if (!image)
    return 0;
    
  im_set_0 (image);
    
  /* Randomly determine the positions of the two objects, A & B
     1. Make sure that their whole area is within the nucleus.
     2. Make sure that their areas do not overlap.
  */
  int overlap = 1;
  int outside = 1;
  while ( (overlap == 1) || (outside == 1) ) {
    x_A = myRandUnif()*(2*(rad_nuc-c_A)) + size/2-(rad_nuc-c_A);
    y_A = myRandUnif()*(2*(rad_nuc-c_A)) + size/2-(rad_nuc-c_A);
    z_A = myRandUnif()*(2*(rad_nuc-c_A)) + size/2-(rad_nuc-c_A);
    x_B = myRandUnif()*(2*(rad_nuc-c_B)) + size/2-(rad_nuc-c_B);
    y_B = myRandUnif()*(2*(rad_nuc-c_B)) + size/2-(rad_nuc-c_B);
    z_B = myRandUnif()*(2*(rad_nuc-c_B)) + size/2-(rad_nuc-c_B);

    if ( (sqrt( (x_A-size/2)*(x_A-size/2) + (y_A-size/2)*(y_A-size/2) + (z_A-size/2)*(z_A-size/2)) < (rad_nuc - c_A)) && (sqrt((x_B-size/2)*(x_B-size/2) + (y_B-size/2)*(y_B-size/2) + (z_B-size/2)*(z_B-size/2) ) < (rad_nuc - c_B))) { 

      urand_A = myRandUnif();
      vrand_A = myRandUnif();
      theta_x_A = 2*3.14159265*urand_A;
      theta_y_A = acos(2*vrand_A-1);
      cosx_A = cos(theta_x_A);
      sinx_A = sin(theta_x_A);
      cosy_A = cos(theta_y_A);
      siny_A = sin(theta_y_A);

      urand_B = myRandUnif();
      vrand_B = myRandUnif();
      theta_x_B = 2*3.14159265*urand_B;
      theta_y_B = acos(2*vrand_B-1);
      cosx_B = cos(theta_x_B);
      sinx_B = sin(theta_x_B);
      cosy_B = cos(theta_y_B);
      siny_B = sin(theta_y_B);
      area_A=0;
      area_B=0;
      for (i = 0; i < size; i++) {
	for (j = 0; j < size; j++) {
	  /*
	    g_A and g_B tell us whether we are inside (=1) or outside the 2D projected ellipsoid
	    M_A and M_B tell us whether we are inside (=1) or outside the 3D ellipsoid
	  */
	  g_A[i][j] = 0;
	  g_B[i][j] = 0;
	  h[i][j]   = 0;
	  overlap = 0;
	  outside = 0;
	  for (k = 0; k < size; k++) {
	    M_A[i][j][k] = 0;
	    M_B[i][j][k] = 0;
	    i_A    =  (cosx_A*cosy_A)*(i-x_A) + (-sinx_A)*(j-y_A) + (cosx_A*siny_A)*(k-z_A);
	    j_A    =  (sinx_A*cosy_A)*(i-x_A) + (cosx_A)*(j-y_A) + (sinx_A*siny_A)*(k-z_A);
	    k_A    =  (-siny_A)*(i-x_A) + (cosy_A)*(k-z_A);
	    dist_A = i_A*i_A/a_A/a_A + j_A*j_A/b_A/b_A + k_A*k_A/c_A/c_A;
	    if (dist_A <= 1 ) {
	      g_A[i][j]=1;
	      M_A[i][j][k]=1;  
	    }
	    i_B    =  (cosx_B*cosy_B)*(i-x_B) + (-sinx_B)*(j-y_B) + (cosx_B*siny_B)*(k-z_B);
	    j_B    =  (sinx_B*cosy_B)*(i-x_B) + (cosx_B)*(j-y_B) + (sinx_B*siny_B)*(k-z_B);
	    k_B    =  (-siny_B)*(i-x_B) + (cosy_B)*(k-z_B);
	    dist_B = i_B*i_B/a_B/a_B + j_B*j_B/b_B/b_B + k_B*k_B/c_B/c_B;
	    if (dist_B <= 1 ) {
	      g_B[i][j]=1;
	      M_B[i][j][k]=1;
	    }
	  }
	  h[i][j]=g_A[i][j] + g_B[i][j];
	  if (h[i][j] > 1) {
	    overlap = 1;
	    //printf("No good, overlap\n");
	    i = size;
	    j = size;
	  }
	  if ( (g_A[i][j] == 1) || (g_B[i][j] == 1) ) {
	    if (sqrt((i-size/2)*(i-size/2) + (j-size/2)*(j-size/2)) >= rad_nuc) {
	      outside = 1;
	      //printf("No good, outside\n");
	      i = size;
	      j = size;
	    }
	  }
	  if (g_A[i][j] == 1) {
	    area_A++;
	  }
	  if (g_B[i][j] == 1) {
	    area_B++;
	  }

	}
      }   
    }
  }

  printf("A: %f \t %f \t %d \n", x_A, y_A, area_A);
  printf("B: %f \t %f \t %d \n", x_B, y_B, area_B);
  printf("\n");

  for (i=0; i<size; i++) {
    for (j=0; j<size; j++) {
      for (k=0; k<size; k++) {
	if (M_A[i][j][k]==1 ){
	  f[i][j][k] = poidev(dens_A,&iseed);
	}
	if (M_B[i][j][k]==1 ){
	  f[i][j][k] = poidev(dens_B,&iseed);
	}
	if ((M_A[i][j][k]!=1 ) && (M_B[i][j][k]!=1 )) {
	  f[i][j][k] = poidev(dens_nuc,&iseed);
	}
	if ( sqrt( (i-size/2)*(i-size/2) + (j-size/2)*(j-size/2) + (k-size/2)*(k-size/2)) > rad_nuc){
	  f[i][j][k] = 0;
	}		
      }
    }
  }
    
  /* data pointer */
  data = image->data;
    
  /* 
     Do the 3D --> 2D MAXIMUM projection
  */
  for (i=0; i<size; i++) {
    for (j=0; j<size; j++) {
      for (k=1; k<size; k++) {
	//		if (data[i+size*j] < f[i][j][k-1]) {
	//		    data[i+size*j] = f[i][j][k-1];
	data[i+size*j] = data[i+size*j] + f[i][j][k-1];
	//	    }
      }
    }
  }
  /* 
     Just for temporary console display
  */
    
  /* 
     for (j = 0; j < size; j++) {
     printf("\n");
     for (i = 0; i < size; i++) {
     if (sqrt((i-size/2)*(i-size/2) + (j-size/2)*(j-size/2)) < rad_nuc) {
     if (g_B[i][j]==1) {
     printf(" B");
     }
     if (g_A[i][j]==1) {
     printf(" A");
     }
     if ((g_B[i][j]==1)&&(g_A[i][j]==1)) {
     printf("BA");
     }
     if ((g_B[i][j]==0)&&(g_A[i][j]==0)) {
     printf(" -");
     }
       
       
     }
     else {
     printf(" ");
     printf(" ");
     }
     }
     }
     printf("\n");
       
  */
    
  return image;
  free(data);
}


 

/*
  Cell nuclei simulation (3D) with two ellipsoidal-shaped CTs with poisson dev
  Same as above, but here the user gives the (x,y,z) coordinates and the rotation
  angles (theta_x, theta_y) for both objects, as well as the slice number.
  So this command produces a single 2D slice and should be used in a loop
  to create a stack of 2D images that can be filtered with a PSF BEFORE doing
  the maximum projection.

  IMPORTANT: The user has to make sure that the ellipsoids are inside the nucleus!!
  Make sure that (x,y,z) is at least a distance c (the longest axis length)
  from the edge of the nucleus.

  AK 2007 07 24
*/

Image *
im_cell_ellipsoid_slice (int size, 
			 int rad_nuc,
			 int x_A,
			 int y_A,
			 int z_A,
			 int x_B,
			 int y_B,
			 int z_B,
			 int a_A, 
			 int b_A, 
			 int c_A, 
			 int a_B, 
			 int b_B, 
			 int c_B,
			 real theta_x_A,
			 real theta_y_A,
			 real theta_x_B,
			 real theta_y_B,
			 real dens_A,
			 real dens_B,
			 real dens_nuc,
			 int slice_number)
{
  Image * image;
  int i,j,k;
  long iseed;
  real *data;
  real f[512][512][512];
  real g_A[512][512];
  real g_B[512][512];
  real M_A[512][512][512];
  real M_B[512][512][512];
  int area_A=0, area_B=0;
  real fil_A=0, fil_B=0;
  real diam_A=0, diam_B=0;
  real i_A, j_A, k_A, i_B, j_B, k_B;
  real cosx_A, sinx_A, cosy_A, siny_A, dist_A;
  real cosx_B, sinx_B, cosy_B, siny_B, dist_B;
 
  image = im_new (size, size, size*size, PHYSICAL);
  if (!image)
    return 0;
    
  im_set_0 (image);
    
  cosx_A = cos(theta_x_A);
  sinx_A = sin(theta_x_A);
  cosy_A = cos(theta_y_A);
  siny_A = sin(theta_y_A);
  cosx_B = cos(theta_x_B);
  sinx_B = sin(theta_x_B);
  cosy_B = cos(theta_y_B);
  siny_B = sin(theta_y_B);
  for (i = 0; i < size; i++) {
    for (j = 0; j < size; j++) {
      /*
	g_A and g_B tell us whether we are inside (=1) or outside the 2D projected ellipsoid
	M_A and M_B tell us whether we are inside (=1) or outside the 3D ellipsoid
      */
      g_A[i][j] = 0;
      g_B[i][j] = 0;
      for (k = 0; k < size; k++) {
	M_A[i][j][k] = 0;
	M_B[i][j][k] = 0;
	i_A    =  (cosx_A*cosy_A)*(i-x_A) + (-sinx_A)*(j-y_A) + (cosx_A*siny_A)*(k-z_A);
	j_A    =  (sinx_A*cosy_A)*(i-x_A) + (cosx_A)*(j-y_A) + (sinx_A*siny_A)*(k-z_A);
	k_A    =  (-siny_A)*(i-x_A) + (cosy_A)*(k-z_A);
	dist_A = i_A*i_A/a_A/a_A + j_A*j_A/b_A/b_A + k_A*k_A/c_A/c_A;
	if (dist_A <= 1 ) {
	  g_A[i][j]=1;
	  M_A[i][j][k]=1;  
	}
	i_B    =  (cosx_B*cosy_B)*(i-x_B) + (-sinx_B)*(j-y_B) + (cosx_B*siny_B)*(k-z_B);
	j_B    =  (sinx_B*cosy_B)*(i-x_B) + (cosx_B)*(j-y_B) + (sinx_B*siny_B)*(k-z_B);
	k_B    =  (-siny_B)*(i-x_B) + (cosy_B)*(k-z_B);
	dist_B = i_B*i_B/a_B/a_B + j_B*j_B/b_B/b_B + k_B*k_B/c_B/c_B;
	if (dist_B <= 1 ) {
	  g_B[i][j]=1;
	  M_B[i][j][k]=1;
	}
      }
      if (g_A[i][j] == 1) {
	area_A++;
      }
      if (g_B[i][j] == 1) {
	area_B++;
      }
	    
    }
  }   
    
  for (i=0; i<size; i++) {
    for (j=0; j<size; j++) {
      for (k=0; k<size; k++) {
	if (M_A[i][j][k]==1 ){
	  f[i][j][k] = poidev(dens_A,&iseed);
	}
	if (M_B[i][j][k]==1 ){
	  f[i][j][k] = poidev(dens_B,&iseed);
	}
	if ((M_A[i][j][k]!=1 ) && (M_B[i][j][k]!=1 )) {
	  f[i][j][k] = poidev(dens_nuc,&iseed);
	}
	if ( sqrt( (i-size/2)*(i-size/2) + (j-size/2)*(j-size/2) + (k-size/2)*(k-size/2)) > rad_nuc){
	  f[i][j][k] = 0;
	}		
      }
    }
  }
    
  /* data pointer */
  data = image->data;
    
  /* 
     Give us a single slice
     While you're doing that, calculate the diameter
     for objects A and B.
  */
    
  real Dtemp_A=0, Dtemp_B=0;
  int m, n;

  for (i=0; i<size; i++) {
    for (j=0; j<size; j++) {
      data[i+size*j] = f[i][j][slice_number];
      if (g_A[i][j] == 1) {
	for (m=0; m<size; m++) {
	  for (n=0; n<size; n++) {
	    if (g_A[m][n] == 1) {
	      Dtemp_A = sqrt( (n-j)*(n-j) + (m-i)*(m-i) );
	      if (Dtemp_A > diam_A) {
		diam_A = Dtemp_A;
	      }
	    }
	  }
	}
      }
      if (g_B[i][j] == 1) {
	for (m=0; m<size; m++) {
	  for (n=0; n<size; n++) {
	    if (g_B[m][n] == 1) {
	      Dtemp_B = sqrt( (n-j)*(n-j) + (m-i)*(m-i) );
	      if (Dtemp_B > diam_B) {
		diam_B = Dtemp_B;
	      }
	    }
	  }
	}
      }
    }
  }
    
  fil_A = 3.14159*diam_A*diam_A/4/area_A;
  fil_B = 3.14159*diam_B*diam_B/4/area_B;
    
  printf("A: %d \t %f \n", area_A, fil_A);
  printf("B: %d \t %f \n", area_B, fil_B);
  printf("\n");
  
  /* 
     Just for temporary console display
  */
    
  /* 
     for (j = 0; j < size; j++) {
     printf("\n");
     for (i = 0; i < size; i++) {
     if (sqrt((i-size/2)*(i-size/2) + (j-size/2)*(j-size/2)) < rad_nuc) {
     if (g_B[i][j]==1) {
     printf(" B");
     }
     if (g_A[i][j]==1) {
     printf(" A");
     }
     if ((g_B[i][j]==1)&&(g_A[i][j]==1)) {
     printf("BA");
     }
     if ((g_B[i][j]==0)&&(g_A[i][j]==0)) {
     printf(" -");
     }
       
       
     }
     else {
     printf(" ");
     printf(" ");
     }
     }
     }
     printf("\n");
       
  */
    
  return image;
  free(data);
}


 


/*
 * White noise, brownians, etc...
 */

static real * FftData (n)	/* n >0, allocation wanted for n reals */
     int n;
{
  static real *buf1;
  static int size = 0;
  real *curs;
  int i;

  size = n;
  buf1 = (real *) malloc (size * sizeof (real));
  if(!buf1){
    /* Probleme d'allocation memoire dans FftData() */
    exit(0);
  }
  curs = buf1;
  for (i = 0; i < size; i++, curs++)
    *curs = 0.0;
  return buf1;
}

/*
 * Fonction a modifier pour qu'elle renvoie une image ! Cf la commande
 * xsmurf associee.
 */

/***************************************************************************/
/*-------------------------------------------------------------------------*/
/* autre brownien selon "the science of fractal images"                    */
/*-------------------------------------------------------------------------*/
static int N;
static real * Tableau;
static long graine;
/*-------------fonctions utilisees dans la suite---------------------------*/
static void setX(x,y,val)
     int x,y;
     real val;
{
  Tableau[x+(N+1)*y]=val;
}

static real getX(x,y)
     int x,y;
{
  return  Tableau[x+(N+1)*y];
}

static void setX3D(x,y,z,val)
     int x,y,z;
     real val;
{
  Tableau[x+(N+1)*y+(N+1)*(N+1)*z]=val;
}

static real getX3D(x,y,z)
     int x,y,z;
{
  return  Tableau[x+(N+1)*y+(N+1)*(N+1)*z];
}


real f3(delta,x0,x1,x2)
     real delta,x0,x1,x2;
{
  return (x0+x1+x2)/3.+delta*_Gasdev_(&graine);
}

real f4(delta,x0,x1,x2,x3)
     real delta,x0,x1,x2,x3;
{
  return (x0+x1+x2+x3)/4.+delta*_Gasdev_(&graine);
}

real f5(delta,x0,x1,x2,x3,x4)
     real delta,x0,x1,x2,x3,x4;
{
  return (x0+x1+x2+x3+x4)/5.+delta*_Gasdev_(&graine);
}

real f6(delta,x0,x1,x2,x3,x4,x5)
     real delta,x0,x1,x2,x3,x4,x5;
{
  return (x0+x1+x2+x3+x4+x5)/6.+delta*_Gasdev_(&graine);
}

real f8(delta,x0,x1,x2,x3,x4,x5,x6,x7)
     real delta,x0,x1,x2,x3,x4,x5,x6,x7;
{
  return (x0+x1+x2+x3+x4+x5+x6+x7)/8.+delta*_Gasdev_(&graine);
}

void im_construit_brownien_mid_point(real *data,
				     int larg,
				     real h_bro,
				     int alea,
				     real sigma)
{
  real delta;
  int D,d,stage,x,y;
  int maxlevel;

  /* Brownien fractionnaire par midpoint-diplacement */
  
  /* initialisation du generateur de nombres aleatoires */
  graine=-1;
  _Gasdev_(&graine);
  _Ran1_(&graine);
  graine = 653535+alea*4;
  _Gasdev_(&graine);
  _Ran1_(&graine);

  srand((unsigned int)  _Ran1_(&graine));

  /* calcul de la dimension de l'image */
  N=1;
  maxlevel=0;
  while (N<larg)
    {
      maxlevel++;
      N*=2;
    }

  Tableau=(real*) malloc ((N+1)*(N+1)*sizeof(real));
  for (x=0;x<(N+1)*(N+1);x++)
    Tableau[x]=0;
  
  /* debut de l'algorithme, on fixe la valeur des coins */
  delta = sigma;
  setX(0,0,delta * _Gasdev_(&graine));
  setX(0,N,delta * _Gasdev_(&graine));
  setX(N,0,delta * _Gasdev_(&graine));
  setX(N,N,delta * _Gasdev_(&graine));
  D = N;
  d = N/2;
  
  for (stage=1;stage <= maxlevel; stage ++)
    { 
      graine = rand();
      delta*=pow(0.5,0.5*h_bro);
      for (x=d;x<=N-d;x+=D)
	for (y=d;y<=N-d;y+=D)
	  setX(x,y,f4(delta,getX(x+d,y+d),getX(x+d,y-d),
		      getX(x-d,y+d),getX(x-d,y-d)));
      /* addition */
      for (x=0;x<=N;x+=D)
	for (y=0;y<=N;y+=D)
	  setX(x,y,getX(x,y) + delta * _Gasdev_(&graine));
      
      delta*=pow(0.5,0.5*h_bro);
      for (x=d;x<=N-d;x+=D)
	{
	  setX(x, 0, f3(delta,getX(x+d,0),getX(x-d,0),getX(x,d))   );
	  setX(x, N, f3(delta,getX(x+d,N),getX(x-d,N),getX(x,N-d)) );
	  setX(0, x, f3(delta,getX(0,x+d),getX(0,x-d),getX(d,x))   );
	  setX(N, x, f3(delta,getX(N,x+d),getX(N,x-d),getX(N-d,x)) );
	}

      for (x=d;x<=N-d;x+=D)
	for (y=D;y<=N-d;y+=D)
	  setX(x,y,f4(delta,getX(x,y+d),getX(x,y-d),
		      getX(x+d,y),getX(x-d,y))); 
      for (x=D;x<=N-d;x+=D)
	for (y=d;y<=N-d;y+=D)
	  setX(x,y,f4(delta,getX(x,y+d),getX(x,y-d),
		      getX(x+d,y),getX(x-d,y))); 
      
      /* addition */
      for (x=0;x<=N;x+=D)
	for (y=0;y<=N;y+=D)
	  setX(x,y,getX(x,y) + delta * _Gasdev_(&graine));
      for (x=d;x<=N-d;x+=D)
	for (y=d;y<=N-d;y+=D)
	  setX(x,y,getX(x,y) + delta * _Gasdev_(&graine)); 
      D/=2;
      d/=2;
    
    }
  /* on recopie tableau dans image */
  for (x=0;x<larg;x++)
    for (y=0;y<larg;y++)
      data[x+larg*y]=getX(x,y);
  
  free(Tableau);
}

void im_construit_brownien3D_mid_point(FILE *fileOut,
				       int larg,
				       real h_bro,
				       int alea,
				       real sigma)
{
  real delta;
  int D,d,stage,x,y,z;
  int maxlevel;
  
  /* Brownien fractionnaire par midpoint-diplacement */
  
  /* initialisation du generateur de nombres aleatoires */
  graine=-1;
  _Gasdev_(&graine);
  _Ran1_(&graine);
  graine = 653535+alea*4;
  _Gasdev_(&graine);
  _Ran1_(&graine);

  /* calcul de la dimension de l'image */
  N=1;
  maxlevel=0;
  while (N<larg)
    {
      maxlevel++;
      N*=2;
    }

  Tableau=(real*) malloc ((N+1)*(N+1)*(N+1)*sizeof(real));
  for (x=0;x<(N+1)*(N+1)*(N+1);x++)
    Tableau[x]=0;
  
  /* debut de l'algorithme, on fixe la valeur des 8 coins */
  delta = sigma;
  setX3D(0,0,0,delta * _Gasdev_(&graine));
  setX3D(0,N,0,delta * _Gasdev_(&graine));
  setX3D(N,0,0,delta * _Gasdev_(&graine));
  setX3D(N,N,0,delta * _Gasdev_(&graine));
  setX3D(0,0,N,delta * _Gasdev_(&graine));
  setX3D(0,N,N,delta * _Gasdev_(&graine));
  setX3D(N,0,N,delta * _Gasdev_(&graine));
  setX3D(N,N,N,delta * _Gasdev_(&graine));
  D = N;
  d = N/2;
  
  for (stage=1;stage <= maxlevel; stage ++)
    { 
      /* les centres des cubes */
      delta*=pow(0.5,0.5*h_bro);
      for (x=d;x<=N-d;x+=D)
	for (y=d;y<=N-d;y+=D)
	  for (z=d;z<=N-d;z+=D)
	    setX3D(x,y,z,f8(delta,getX3D(x+d,y+d,z+d),getX3D(x+d,y-d,z+d),
			    getX3D(x-d,y+d,z+d),getX3D(x-d,y-d,z+d),
			    getX3D(x+d,y+d,z-d),getX3D(x+d,y-d,z-d),
			    getX3D(x-d,y+d,z-d),getX3D(x-d,y-d,z-d)));
      /* addition deplacement aleatoire */
      for (x=0;x<=N;x+=D)
	for (y=0;y<=N;y+=D)
	  for (z=0;z<=N;z+=D)
	    setX3D(x,y,z,getX3D(x,y,z) + delta * _Gasdev_(&graine));
      
      delta*=pow(0.5,0.5*h_bro);
      /* les 6 faces externes */
      /* d'abord les centres */
      for (x=d;x<=N-d;x+=D) 
	for (y=d;y<=N-d;y+=D) {
	  setX3D(x, y, 0, f5(delta,getX3D(x+d,y+d,0),getX3D(x+d,y-d,0),
			     getX3D(x-d,y-d,0),getX3D(x-d,y+d,0),
			     getX3D(x,y,d) )   );
	  setX3D(x, y, N, f5(delta,getX3D(x+d,y+d,N),getX3D(x+d,y-d,N),
			     getX3D(x-d,y-d,N),getX3D(x-d,y+d,N),
			     getX3D(x,y,N-d) )   );
	}
      for (x=d;x<=N-d;x+=D) 
	for (z=d;z<=N-d;z+=D) {
	  setX3D(x, 0, z, f5(delta,getX3D(x+d,0,z+d),getX3D(x+d,0,z+d),
			     getX3D(x+d,0,z-d),getX3D(x+d,0,z-d),
			     getX3D(x,d,z) )   );
	  setX3D(x, N, z, f5(delta,getX3D(x-d,N,z+d),getX3D(x-d,N,z+d),
			     getX3D(x-d,N,z-d),getX3D(x-d,N,z-d),
			     getX3D(x,N-d,z) )   );
	}
      for (y=d;y<=N-d;y+=D) 
	for (z=d;z<=N-d;z+=D) {
	  setX3D(0, y, z, f5(delta,getX3D(0,y+d,z+d),getX3D(0,y+d,z+d),
			     getX3D(0,y+d,z-d),getX3D(0,y+d,z-d),
			     getX3D(d,y,z) )   );
	  setX3D(N, y, z, f5(delta,getX3D(N,y-d,z+d),getX3D(N,y-d,z+d),
			     getX3D(N,y-d,z-d),getX3D(N,y-d,z-d),
			     getX3D(N-d,y,z) )   );
	}



      /* ensuite les milieux des 12 arretes */
      for (x=d;x<=N-d;x+=D) {
	setX3D(x, 0, 0, f4(delta,getX3D(x+d,0,0),getX3D(x-d,0,0),
			   getX3D(x,0,d),getX3D(x,d,0)));
	setX3D(x, 0, N, f4(delta,getX3D(x+d,0,N),getX3D(x-d,0,N),
			   getX3D(x,0,N-d),getX3D(x,d,N)));
	setX3D(x, N, N, f4(delta,getX3D(x+d,N,N),getX3D(x-d,N,N),
			   getX3D(x,N,N-d),getX3D(x,N-d,N)));
	setX3D(x, N, 0, f4(delta,getX3D(x+d,N,0),getX3D(x-d,N,0),
			   getX3D(x,N,d),getX3D(x,N-d,0)));
      }
      for (y=d;y<=N-d;y+=D) {
	setX3D(0, y, 0, f4(delta,getX3D(0,y+d,0),getX3D(0,y-d,0),
			   getX3D(0,y,d),getX3D(d,y,0)));
	setX3D(0, y, N, f4(delta,getX3D(0,y+d,N),getX3D(0,y-d,N),
			   getX3D(0,y,N-d),getX3D(d,y,N)));
	setX3D(N, y, N, f4(delta,getX3D(N,y+d,N),getX3D(N,y-d,N),
			   getX3D(N,y,N-d),getX3D(N-d,y,N)));
	setX3D(N, y, 0, f4(delta,getX3D(N,y+d,0),getX3D(N,y-d,0),
			   getX3D(N,y,d),getX3D(N-d,y,0)));
      }
      for (z=d;z<=N-d;z+=D) {
	setX3D(0, 0, z, f4(delta,getX3D(0,0,z+d),getX3D(0,0,z-d),
			   getX3D(0,d,z),getX3D(d,0,z)));
	setX3D(0, N, z, f4(delta,getX3D(0,N,z+d),getX3D(0,N,z-d),
			   getX3D(0,N-d,z),getX3D(d,N,z)));
	setX3D(N, N, z, f4(delta,getX3D(N,N,z+d),getX3D(N,N,z-d),
			   getX3D(N,N-d,z),getX3D(N-d,N,z)));
	setX3D(N, 0, z, f4(delta,getX3D(N,0,z+d),getX3D(N,0,z-d),
			   getX3D(N,d,z),getX3D(N-d,0,z)));
      }
      /* ensuite les faces en elles-meme */
      //delta*=pow(0.5,0.5*h_bro);
      for (x=d;x<=N-d;x+=D)
	for (y=D;y<=N-d;y+=D) {
	  setX3D(x,y,0, f5(delta,getX3D(x,y+d,0),getX3D(x,y-d,0),
			   getX3D(x+d,y,0),getX3D(x-d,y,0),getX3D(x,y,d)));
	  setX3D(x,y,N, f5(delta,getX3D(x,y+d,N),getX3D(x,y-d,N),
			   getX3D(x+d,y,N),getX3D(x-d,y,N),getX3D(x,y,N-d)));
	}
      for (x=D;x<=N-d;x+=D)
	for (y=d;y<=N-d;y+=D) {
	  setX3D(x,y,0, f5(delta,getX3D(x,y+d,0),getX3D(x,y-d,0),
			   getX3D(x+d,y,0),getX3D(x-d,y,0),getX3D(x,y,d)));
	  setX3D(x,y,N, f5(delta,getX3D(x,y+d,N),getX3D(x,y-d,N),
			   getX3D(x+d,y,N),getX3D(x-d,y,N),getX3D(x,y,N-d)));
	}
	 
      for (y=d;y<=N-d;y+=D)
	for (z=D;z<=N-d;z+=D) {
	  setX3D(0,y,z, f5(delta,getX3D(0,y+d,z),getX3D(0,y-d,z),
			   getX3D(0,y,z+d),getX3D(0,y,z-d),getX3D(d,y,z)));
	  setX3D(N,y,z, f5(delta,getX3D(N,y+d,z),getX3D(N,y-d,z),
			   getX3D(N,y,z+d),getX3D(N,y,z-d),getX3D(N-d,y,z)));
	}
      for (y=D;y<=N-d;y+=D)
	for (z=d;z<=N-d;z+=D) {
	  setX3D(0,y,z, f5(delta,getX3D(0,y+d,z),getX3D(0,y-d,z),
			   getX3D(0,y,z+d),getX3D(0,y,z-d),getX3D(d,y,z)));
	  setX3D(N,y,z, f5(delta,getX3D(N,y+d,z),getX3D(N,y-d,z),
			   getX3D(N,y,z+d),getX3D(N,y,z-d),getX3D(N-d,y,z)));
	}

      for (z=d;z<=N-d;z+=D)
	for (x=D;x<=N-d;x+=D) {
	  setX3D(x,0,z, f5(delta,getX3D(x+d,0,z),getX3D(x-d,0,z),
			   getX3D(x,0,z+d),getX3D(x,0,z-d),getX3D(x,d,z)));
	  setX3D(x,N,z, f5(delta,getX3D(x+d,N,z),getX3D(x-d,N,z),
			   getX3D(x,N,z+d),getX3D(x,N,z-d),getX3D(x,N-d,z)));
	}
      for (z=D;z<=N-d;z+=D)
	for (x=d;x<=N-d;x+=D) {
	  setX3D(x,0,z, f5(delta,getX3D(x+d,0,z),getX3D(x-d,0,z),
			   getX3D(x,0,z+d),getX3D(x,0,z-d),getX3D(x,d,z)));
	  setX3D(x,N,z, f5(delta,getX3D(x+d,N,z),getX3D(x-d,N,z),
			   getX3D(x,N,z+d),getX3D(x,N,z-d),getX3D(x,N-d,z)));
	}
  
      /* addition deplacement aleatoire */
      /*
	for (x=d;x<=N-d;x+=D)
	for (y=D;y<=N-d;y+=D) {
	setX3D(x,y,0, getX3D(x,y,0) + delta * _Gasdev_(&graine));
	setX3D(x,y,N, getX3D(x,y,N) + delta * _Gasdev_(&graine));
	}
	for (x=D;x<=N-d;x+=D)
	for (y=d;y<=N-d;y+=D) {
	setX3D(x,y,0, getX3D(x,y,0) + delta * _Gasdev_(&graine));
	setX3D(x,y,N, getX3D(x,y,N) + delta * _Gasdev_(&graine));
	}
	 
	for (y=d;y<=N-d;y+=D)
	for (z=D;z<=N-d;z+=D) {
	setX3D(0,y,z, getX3D(0,y,z) + delta * _Gasdev_(&graine));
	setX3D(N,y,z, getX3D(N,y,z) + delta * _Gasdev_(&graine));
	}
	for (y=D;y<=N-d;y+=D)
	for (z=d;z<=N-d;z+=D) {
	setX3D(0,y,z, getX3D(0,y,z) + delta * _Gasdev_(&graine));
	setX3D(N,y,z, getX3D(N,y,z) + delta * _Gasdev_(&graine));
	}

	for (z=d;z<=N-d;z+=D)
	for (x=D;x<=N-d;x+=D) {
	setX3D(x,y,0, getX3D(x,y,0) + delta * _Gasdev_(&graine));
	setX3D(x,y,N, getX3D(x,y,N) + delta * _Gasdev_(&graine));
	}
	for (z=D;z<=N-d;z+=D)
	for (x=d;x<=N-d;x+=D) {
	setX3D(x,y,0, getX3D(x,y,0) + delta * _Gasdev_(&graine));
	setX3D(x,y,N, getX3D(x,y,N) + delta * _Gasdev_(&graine));	 
	}
      */

      /* and now ladies and petomen, l'interieur du cubes */
      /* d'abord les faces interieures 
	 il y en a trois types : paralleles xy, yz, zx */
      

      for (x=d;x<=N-d;x+=D)
	for (y=D;y<=N-d;y+=D)
	  for (z=d;z<=N-d;z+=D)
	    setX3D(x,y,z, f6(delta,getX3D(x,y+d,z),getX3D(x,y-d,z),
			     getX3D(x+d,y,z+d),getX3D(x-d,y,z-d),
			     getX3D(x-d,y,z+d),getX3D(x+d,y,z-d))); 
      for (x=D;x<=N-d;x+=D)
	for (y=d;y<=N-d;y+=D)
	  for (z=d;z<=N-d;z+=D)
	    setX3D(x,y,z, f6(delta,getX3D(x,y+d,z+d),getX3D(x,y-d,z-d),
			     getX3D(x+d,y,z),getX3D(x-d,y,z),
			     getX3D(x,y-d,z+d),getX3D(x,y+d,z-d))); 
      for (x=d;x<=N-d;x+=D)
	for (y=d;y<=N-d;y+=D)
	  for (z=D;z<=N-d;z+=D)
	    setX3D(x,y,z, f6(delta,getX3D(x+d,y+d,z),getX3D(x-d,y-d,z),
			     getX3D(x+d,y-d,z),getX3D(x-d,y+d,z),
			     getX3D(x,y,z+d),getX3D(x,y,z-d))); 
      
      /* ensuite arretes interieures */
      for (x=D;x<=N-d;x+=D)
	for (y=D;y<=N-d;y+=D)
	  for (z=d;z<=N-d;z+=D)
	    setX3D(x,y,z, f6(delta,getX3D(x,y+d,z),getX3D(x,y-d,z),
			     getX3D(x+d,y,z),getX3D(x-d,y,z),
			     getX3D(x,y,z+d),getX3D(x,y,z-d))); 
      for (x=D;x<=N-d;x+=D)
	for (y=d;y<=N-d;y+=D)
	  for (z=D;z<=N-d;z+=D)
	    setX3D(x,y,z, f6(delta,getX3D(x,y+d,z),getX3D(x,y-d,z),
			     getX3D(x+d,y,z),getX3D(x-d,y,z),
			     getX3D(x,y,z+d),getX3D(x,y,z-d))); 
      for (x=d;x<=N-d;x+=D)
	for (y=D;y<=N-d;y+=D)
	  for (z=D;z<=N-d;z+=D)
	    setX3D(x,y,z, f6(delta,getX3D(x,y+d,z),getX3D(x,y-d,z),
			     getX3D(x+d,y,z),getX3D(x-d,y,z),
			     getX3D(x,y,z+d),getX3D(x,y,z-d))); 
      
      
      /* addition */
      for (x=0;x<=N;x+=D)
	for (y=0;y<=N;y+=D)
	  for (z=0;z<=N;z+=D)
	    setX3D(x,y,z,getX3D(x,y,z) + delta * _Gasdev_(&graine));
      for (x=d;x<=N-d;x+=D)
	for (y=d;y<=N-d;y+=D)
	  for (z=d;z<=N-d;z+=D)
	    setX3D(x,y,z,getX3D(x,y,z) + delta * _Gasdev_(&graine)); 
      D/=2;
      d/=2;
    
    }
  /* on recopie tableau dans image */
  {
    float tmp;
    for (x=0;x<larg;x++)
      for (y=0;y<larg;y++)
	for (z=0;z<larg;z++) {
	  tmp = getX3D(x,y,z);
	  fwrite(&tmp, sizeof(float), 1, fileOut);
	}
  }

  /* on ecrit les donnees */
  //fwrite(Tableau, sizeof(float), larg*larg*larg, fileOut);
  
  free(Tableau);
}



/*----------------------------------------------------------------------
  _QuickSort_
  
  Fonction qui classe dans l'ordre croissant le tableau x de taille n.
  --------------------------------------------------------------------*/
static void 
_QuickSort_(double *x,
	    int    n)
{
  int    l, j, ir, i;
  double xx;
  
  l = (n>>1) + 1;
  ir = n;

  /*  L'index l est decremente de sa valeur initiale vers 1 pendant
   *  la periode de creation de l'arbre, une fois 1 atteint, l'index ir
   *  decremente de sa valeur initiale vers 1 pendant la phase de
   *  selection entre branches */

  for(;;) {
    if(l > 1)
      xx = x[--l];
    else {
      xx = x[ir];
      x[ir] = x[1];
      if(--ir == 1) {
	x[1] = xx;
	return;
      }
    }
    i = l;
    j = l<<1;
    while(j <= ir) {
      if(j < ir && x[j] < x[j+1])
	++j;
      if(xx < x[j]) {
	x[i] = x[j];
	j += (i = j);
      }
      else j = ir + 1;
    }
    x[i] = xx;
  }
}


real special4(delta,x0,x1,x2,x3)
     real delta,x0,x1,x2,x3;
{
  double *tab;
  real res;

  tab = (double *) malloc(4*sizeof(double));
  tab[0] = x0; 
  tab[1] = x1; 
  tab[2] = x2; 
  tab[3] = x3; 
  _QuickSort_(tab-1, 4);
  res = (real)(tab[3])+delta*_Gasdev_(&graine);
  free(tab);
  return res;
}

void im_construit_brownien_mid_point_special(real *data,
					     int larg,
					     real h_bro,
					     int alea,
					     real sigma)
{
  real delta;
  int D,d,stage,x,y;
  int maxlevel;

  /* Brownien fractionnaire par midpoint-diplacement */
  
  /* initialisation du generateur de nombres aleatoires */
  graine=-1;
  _Gasdev_(&graine);
  _Ran1_(&graine);
  graine = 653535+alea*4;
  _Gasdev_(&graine);
  _Ran1_(&graine);

  /* calcul de la dimension de l'image */
  N=1;
  maxlevel=0;
  while (N<larg)
    {
      maxlevel++;
      N*=2;
    }

  Tableau=(real*) malloc ((N+1)*(N+1)*sizeof(real));
  for (x=0;x<(N+1)*(N+1);x++)
    Tableau[x]=0;
  
  /* debut de l'algorithme, on fixe la valeur des coins */
  delta = sigma;
  setX(0,0,delta * _Gasdev_(&graine));
  setX(0,N,delta * _Gasdev_(&graine));
  setX(N,0,delta * _Gasdev_(&graine));
  setX(N,N,delta * _Gasdev_(&graine));
  D = N;
  d = N/2;
  
  for (stage=1;stage <= maxlevel; stage ++)
    { 
      delta*=pow(0.5,0.5*h_bro);
      for (x=d;x<=N-d;x+=D)
	for (y=d;y<=N-d;y+=D)
	  setX(x,y,special4(delta,getX(x+d,y+d),getX(x+d,y-d),
			    getX(x-d,y+d),getX(x-d,y-d)));
      /* addition */
      for (x=0;x<=N;x+=D)
	for (y=0;y<=N;y+=D)
	  setX(x,y,getX(x,y) + delta * _Gasdev_(&graine));
      
      delta*=pow(0.5,0.5*h_bro);
      for (x=d;x<=N-d;x+=D)
	{
	  setX(x, 0, f3(delta,getX(x+d,0),getX(x-d,0),getX(x,d))   );
	  setX(x, N, f3(delta,getX(x+d,N),getX(x-d,N),getX(x,N-d)) );
	  setX(0, x, f3(delta,getX(0,x+d),getX(0,x-d),getX(d,x))   );
	  setX(N, x, f3(delta,getX(N,x+d),getX(N,x-d),getX(N-d,x)) );
	}
      
      for (x=d;x<=N-d;x+=D)
	for (y=D;y<=N-d;y+=D)
	  setX(x,y,special4(delta,getX(x,y+d),getX(x,y-d),
			    getX(x+d,y),getX(x-d,y))); 
      for (x=D;x<=N-d;x+=D)
	for (y=d;y<=N-d;y+=D)
	  setX(x,y,special4(delta,getX(x,y+d),getX(x,y-d),
			    getX(x+d,y),getX(x-d,y))); 
      
      /* addition */
      for (x=0;x<=N;x+=D)
	for (y=0;y<=N;y+=D)
	  setX(x,y,getX(x,y) + delta * _Gasdev_(&graine));
      for (x=d;x<=N-d;x+=D)
	for (y=d;y<=N-d;y+=D)
	  setX(x,y,getX(x,y) + delta * _Gasdev_(&graine)); 
      D/=2;
      d/=2;
    
    }
  /* on recopie tableau dans image */
  for (x=0;x<larg;x++)
    for (y=0;y<larg;y++)
      data[x+larg*y]=getX(x,y);
  
  free(Tableau);
}


void
im_construit_brownien_fft(real *data,
			  int   nrow,
			  real h_bro,
			  int   alea,
			  int   fft_dim,
			  void *h_fct)

{
  unsigned long nn[3];
  int           i, j;
  int           ni, nj, ni_2, nj_2, di, dj, i0, j0;
  real         phase, rad, x;
  long          iseed, is2;
  real         *fftData;
  real h;
  real theta;
  int compt=0;

  if(fft_dim == 0) {
    /* retourne la plus proche puissance de 2 superieure a nrow */
    ni = _Puiss2_(nrow); 
    nj = _Puiss2_(nrow);
  }
  else {
    ni = fft_dim;
    nj = fft_dim;
  }

  ni_2 = (int)(ni/2);
  nj_2 = (int)(nj/2);

  /* allocation memoire */
  fftData = FftData(2*nj*ni+2);
  
  (iseed) = -1;
  is2 = -1;
  x = _Ran1_(&iseed);
  x = _Ran3_(&iseed);
  x = _Gasdev_(&is2);
  iseed = 4135234;
  is2 = 3654635;
  x = _Ran1_(&iseed);
  x = _Ran3_(&iseed);
  x = _Gasdev_(&is2);

  iseed = 366555+alea*4;
  is2 = 7543526+alea*5;
  for (i = 0; i <= ni_2; i++) {
    for (j = 0; j <= nj_2; j++) {
      compt++;
      phase = (real) (2*M_PI*_Ran3_(&iseed));
      if ((!(i == 0)) && (!(j == 0))) {
	theta = atan2((double)i,(double)j);
	h = h_bro*evaluator_evaluate_x(h_fct,theta);
	rad = 1e6*pow((double)(i*i+j*j), (double)(-(2*h+2)/4))
	  *_Gasdev_(&is2);
      }
      else
	rad = 0.0;
      fftData[2*(i*nj+j)+1] = rad*cos(phase);
      fftData[2*(i*nj+j)+2] = rad*sin(phase);
      
      if (i == 0)
	i0 = 0;
      else
	i0 = ni-i;
      if (j == 0)
	j0 = 0;
      else
	j0 = nj-j;
      fftData[2*(i0*nj+j0)+1] =  rad*cos(phase);
      fftData[2*(i0*nj+j0)+2] = -rad*sin(phase);
    }
  }


  fftData[2*nj_2+2] = 0.0;
  fftData[2*ni_2*nj+2] = 0.0;
  fftData[2*(ni_2*nj+nj_2)+2] = 0.0;
  
  iseed = 524455+alea*8;
  is2 = 878774676+alea*2;

  for (i = 1; i < ni_2; i++) {
    for (j = 1; j < nj_2; j++) {
      compt++;
      phase = 2*M_PI*_Ran3_(&iseed);
      theta = atan2((double)i,(double)j);
      h = h_bro*evaluator_evaluate_x(h_fct,theta);
      rad = 1e6*pow((double)(i*i+j*j), (double)(-(2*h+2)/4))*_Gasdev_(&is2);
      fftData[2*(nj*(ni-i)+j)+1] =  rad*cos(phase);
      fftData[2*(nj*(ni-i)+j)+2] =  rad*sin(phase);
      fftData[2*(i*nj+nj-j)+1]   =  rad*cos(phase);
      fftData[2*(i*nj+nj-j)+2]   = -rad*sin(phase);
    }
  }
  
  nn[1] = (unsigned long) ni;
  nn[2] = (unsigned long) nj;

  _Fourn_(fftData, nn, 2, 1);
  di = (ni-nrow)/2;
  dj = (nj-nrow)/2;

  for (i = 0; i < nrow; i++)
    for (j = 0; j < nrow; j++)
      data[i*nrow+j] = 1e3*fftData[2*(nj*(j+dj)+i+di)+1]/((real)(ni*nj));
      
  free(fftData);
}

void
im_construit_brownien_fft2(real *data,
			   real *data2,
			   int   nrow,
			   real h_bro,
			   int   alea,
			   int   fft_dim,
			   void *h_fct)
     
{
  unsigned long nn[3];
  int           i, j;
  int           ni, nj, ni_2, nj_2, di, dj, i0, j0;
  real         phase, rad, x;
  long          iseed, is2;
  real         *fftData, *fftData2;
  real h;
  real theta;
  int compt=0;

  if(fft_dim == 0) {
    /* retourne la plus proche puissance de 2 superieure a nrow */
    ni = _Puiss2_(nrow); 
    nj = _Puiss2_(nrow);
  }
  else {
    ni = fft_dim;
    nj = fft_dim;
  }

  ni_2 = (int)(ni/2);
  nj_2 = (int)(nj/2);

  /* allocation memoire */
  fftData  = FftData(2*nj*ni+2);
  fftData2 = FftData(2*nj*ni+2);
  
  (iseed) = -1;
  is2 = -1;
  x = _Ran1_(&iseed);
  x = _Ran3_(&iseed);
  x = _Gasdev_(&is2);
  iseed = 4135234;
  is2 = 3654635;
  x = _Ran1_(&iseed);
  x = _Ran3_(&iseed);
  x = _Gasdev_(&is2);

  iseed = 366555+alea*4;
  is2 = 7543526+alea*5;
  for (i = 0; i <= ni_2; i++) {
    for (j = 0; j <= nj_2; j++) {
      compt++;
      phase = (real) (2*M_PI*_Ran3_(&iseed));
      if ((!(i == 0)) || (!(j == 0))) {
	theta = atan2((double)i,(double)j);
	h = h_bro*evaluator_evaluate_x(h_fct,theta);
	rad = 1e6*pow((double)(i*i+j*j), (double)(-(2*h+2)/4))
	  *_Gasdev_(&is2);
      }
      else
	rad = 0.0;
      fftData[2*(i*nj+j)+1]  = rad*cos(phase);
      fftData[2*(i*nj+j)+2]  = rad*sin(phase);
      fftData2[2*(i*nj+j)+1] = rad*cos(phase);
      fftData2[2*(i*nj+j)+2] = rad*sin(phase);
      
      if ((i == 0) || (j == 0)) {
	fftData2[2*(i*nj+j)+1] = 0.0;
	fftData2[2*(i*nj+j)+2] = 0.0;
      }

      if (i == 0)
	i0 = 0;
      else
	i0 = ni-i;
      if (j == 0)
	j0 = 0;
      else
	j0 = nj-j;
      fftData[2*(i0*nj+j0)+1] =  rad*cos(phase);
      fftData[2*(i0*nj+j0)+2] = -rad*sin(phase);
      fftData2[2*(i0*nj+j0)+1] =  rad*cos(phase);
      fftData2[2*(i0*nj+j0)+2] = -rad*sin(phase);
    }
  }


  fftData[2*nj_2+2] = 0.0;
  fftData[2*ni_2*nj+2] = 0.0;
  fftData[2*(ni_2*nj+nj_2)+2] = 0.0;
  fftData2[2*nj_2+2] = 0.0;
  fftData2[2*ni_2*nj+2] = 0.0;
  fftData2[2*(ni_2*nj+nj_2)+2] = 0.0;
  
  iseed = 524455+alea*8;
  is2 = 878774676+alea*2;

  for (i = 1; i < ni_2; i++) {
    for (j = 1; j < nj_2; j++) {
      compt++;
      phase = 2*M_PI*_Ran3_(&iseed);
      theta = atan2((double)i,(double)j);
      h = h_bro*evaluator_evaluate_x(h_fct,theta);
      rad = 1e6*pow((double)(i*i+j*j), (double)(-(2*h+2)/4))*_Gasdev_(&is2);
      fftData[2*(nj*(ni-i)+j)+1] =  rad*cos(phase);
      fftData[2*(nj*(ni-i)+j)+2] =  rad*sin(phase);
      fftData[2*(i*nj+nj-j)+1]   =  rad*cos(phase);
      fftData[2*(i*nj+nj-j)+2]   = -rad*sin(phase);
      fftData2[2*(nj*(ni-i)+j)+1] =  rad*cos(phase);
      fftData2[2*(nj*(ni-i)+j)+2] =  rad*sin(phase);
      fftData2[2*(i*nj+nj-j)+1]   =  rad*cos(phase);
      fftData2[2*(i*nj+nj-j)+2]   = -rad*sin(phase);
    }
  }
  
  nn[1] = (unsigned long) ni;
  nn[2] = (unsigned long) nj;

  _Fourn_(fftData, nn, 2, 1);
  _Fourn_(fftData2, nn, 2, 1);
  di = (ni-nrow)/2;
  dj = (nj-nrow)/2;

  for (i = 0; i < nrow; i++)
    for (j = 0; j < nrow; j++) {
      data[i*nrow+j] = 1e3*fftData[2*(nj*(j+dj)+i+di)+1]/((real)(ni*nj));
      data2[i*nrow+j] = 1e3*fftData2[2*(nj*(j+dj)+i+di)+1]/((real)(ni*nj));
    }      
  free(fftData);
  free(fftData2);
}

void
im_construit_brownien_fft_anis(real *data,
			       int   nrow,
			       real h_bro,
			       int   alea,
			       int   fft_dim,
			       void *h_fct,
			       float deltaH)
     
{
  unsigned long nn[3];
  int           i, j;
  int           ni, nj, ni_2, nj_2, di, dj, i0, j0;
  real         phase, rad, rad2, x;
  long          iseed, is2;
  real         *fftData;
  real h;
  real theta;
  int compt=0;

  if(fft_dim == 0) {
    /* retourne la plus proche puissance de 2 superieure a nrow */
    ni = _Puiss2_(nrow); 
    nj = _Puiss2_(nrow);
  }
  else {
    ni = fft_dim;
    nj = fft_dim;
  }

  ni_2 = (int)(ni/2);
  nj_2 = (int)(nj/2);

  /* allocation memoire */
  fftData = FftData(2*nj*ni+2);
  
  (iseed) = -1;
  is2 = -1;
  x = _Ran1_(&iseed);
  x = _Ran3_(&iseed);
  x = _Gasdev_(&is2);
  iseed = 4135234;
  is2 = 3654635;
  x = _Ran1_(&iseed);
  x = _Ran3_(&iseed);
  x = _Gasdev_(&is2);

  iseed = 366555+alea*4;
  is2 = 7543526+alea*5;
  for (i = 1; i <= ni_2; i++) {
    for (j = 1; j <= nj_2; j++) {
      compt++;
      phase = (real) (2*M_PI*_Ran3_(&iseed));
      if ((!(i == 0)) && (!(j == 0))) {
	theta = atan2((double)i,(double)j);
	h = h_bro*evaluator_evaluate_x(h_fct,theta);
	rad = 1e6*pow((double)(i*i+j*j), (double)(-(2*h+2)/4))
	  *_Gasdev_(&is2);
	rad2 = pow((double)(i*i), (double)(-(deltaH)/2));
	//rad = _Gasdev_(&is2);
      }
      else
	rad = 0.0;
      fftData[2*(i*nj+j)+1] = rad*rad2*cos(phase);
      fftData[2*(i*nj+j)+2] = rad*rad2*sin(phase);
      
      if (i == 0)
	i0 = 0;
      else
	i0 = ni-i;
      if (j == 0)
	j0 = 0;
      else
	j0 = nj-j;
      fftData[2*(i0*nj+j0)+1] =  rad*rad2*cos(phase);
      fftData[2*(i0*nj+j0)+2] = -rad*rad2*sin(phase);
    }
  }


  fftData[2*nj_2+2] = 0.0;
  fftData[2*ni_2*nj+2] = 0.0;
  fftData[2*(ni_2*nj+nj_2)+2] = 0.0;
  
  iseed = 524455+alea*8;
  is2 = 878774676+alea*2;

  for (i = 1; i < ni_2; i++) {
    for (j = 1; j < nj_2; j++) {
      compt++;
      phase = 2*M_PI*_Ran3_(&iseed);
      theta = atan2((double)i,(double)j);
      h = h_bro*evaluator_evaluate_x(h_fct,theta);
      rad = 1e6*pow((double)(i*i+j*j), (double)(-(2*h+2)/4))*_Gasdev_(&is2);
      rad2 = pow((double)(i*i), (double)(-(deltaH)/2));
      fftData[2*(nj*(ni-i)+j)+1] =  rad*rad2*cos(phase);
      fftData[2*(nj*(ni-i)+j)+2] =  rad*rad2*sin(phase);
      fftData[2*(i*nj+nj-j)+1]   =  rad*rad2*cos(phase);
      fftData[2*(i*nj+nj-j)+2]   = -rad*rad2*sin(phase);
    }
  }
  
  nn[1] = (unsigned long) ni;
  nn[2] = (unsigned long) nj;

  _Fourn_(fftData, nn, 2, 1);
  di = (ni-nrow)/2;
  dj = (nj-nrow)/2;

  for (i = 0; i < nrow; i++)
    for (j = 0; j < nrow; j++)
      data[i*nrow+j] = 1e3*fftData[2*(nj*(j+dj)+i+di)+1]/((real)(ni*nj));
      
  free(fftData);
}

void
im_construit_brownien_fft_anis2(real *data,
				int   nrow,
				real  h1,
				real  h2,
				int   alea,
				int   fft_dim)
{
  unsigned long nn[3];
  int           i, j;
  int           ni, nj, ni_2, nj_2, di, dj, i0, j0;
  real         phase, rad, rad2, x;
  long          iseed, is2;
  real         *fftData;
  real beta1, beta2;
  real theta;
  int compt=0;


  beta1 = - (h1+1);
  beta2 = - (h2+1);
  if(fft_dim == 0) {
    /* retourne la plus proche puissance de 2 superieure a nrow */
    ni = _Puiss2_(nrow); 
    nj = _Puiss2_(nrow);
  }
  else {
    ni = fft_dim;
    nj = fft_dim;
  }

  ni_2 = (int)(ni/2);
  nj_2 = (int)(nj/2);

  /* allocation memoire */
  fftData = FftData(2*nj*ni+2);
  
  (iseed) = -1;
  is2 = -1;
  x = _Ran1_(&iseed);
  x = _Ran3_(&iseed);
  x = _Gasdev_(&is2);
  iseed = 4135234;
  is2 = 3654635;
  x = _Ran1_(&iseed);
  x = _Ran3_(&iseed);
  x = _Gasdev_(&is2);

  iseed = 366555+alea*4;
  is2 = 7543526+alea*5;
  for (i = 0; i <= ni_2; i++) {
    for (j = 0; j <= nj_2; j++) {
      compt++;
      phase = (real) (2*M_PI*_Ran3_(&iseed));
      if ((!(i == 0)) && (!(j == 0))) {
	theta = atan2((double)i,(double)j);
	rad =  lgamma(1+beta1/2)/pow(2,1-beta1)/lgamma(2-beta1/2)*cos(theta)*cos(theta)*pow((double)(i*i+j*j), (double)(-(2*h1+2)/4));
	rad += lgamma(1+beta2/2)/pow(2,1-beta2)/lgamma(2-beta2/2)*sin(theta)*sin(theta)*pow((double)(i*i+j*j), (double)(-(2*h2+2)/4));
	rad *= 1e6*_Gasdev_(&is2);
      }
      else
	rad = 0.0;
      fftData[2*(i*nj+j)+1] = rad*cos(phase);
      fftData[2*(i*nj+j)+2] = rad*sin(phase);
      
      if (i == 0)
	i0 = 0;
      else
	i0 = ni-i;
      if (j == 0)
	j0 = 0;
      else
	j0 = nj-j;
      fftData[2*(i0*nj+j0)+1] =  rad*cos(phase);
      fftData[2*(i0*nj+j0)+2] = -rad*sin(phase);
    }
  }


  fftData[2*nj_2+2] = 0.0;
  fftData[2*ni_2*nj+2] = 0.0;
  fftData[2*(ni_2*nj+nj_2)+2] = 0.0;
  
  iseed = 524455+alea*8;
  is2 = 878774676+alea*2;

  for (i = 1; i < ni_2; i++) {
    for (j = 1; j < nj_2; j++) {
      compt++;
      phase = 2*M_PI*_Ran3_(&iseed);
      if ((!(i == 0)) && (!(j == 0))) {
	theta = atan2((double)i,(double)j);
      }
      rad =  lgamma(1+beta1/2)/pow(2,1-beta1)/lgamma(2-beta1/2)*cos(theta)*cos(theta)*pow((double)(i*i+j*j), (double)(-(2*h1+2)/4));
      rad += lgamma(1+beta2/2)/pow(2,1-beta2)/lgamma(2-beta2/2)*sin(theta)*sin(theta)*pow((double)(i*i+j*j), (double)(-(2*h2+2)/4));
      rad *= 1e6*_Gasdev_(&is2);
      fftData[2*(nj*(ni-i)+j)+1] =  rad*cos(phase);
      fftData[2*(nj*(ni-i)+j)+2] =  rad*sin(phase);
      fftData[2*(i*nj+nj-j)+1]   =  rad*cos(phase);
      fftData[2*(i*nj+nj-j)+2]   = -rad*sin(phase);
    }
  }
  
  nn[1] = (unsigned long) ni;
  nn[2] = (unsigned long) nj;

  _Fourn_(fftData, nn, 2, 1);
  di = (ni-nrow)/2;
  dj = (nj-nrow)/2;

  for (i = 0; i < nrow; i++)
    for (j = 0; j < nrow; j++)
      data[i*nrow+j] = 1e3*fftData[2*(nj*(j+dj)+i+di)+1]/((real)(ni*nj));
      
  free(fftData);
}

void
im_construit_brownien_fft_bsheet(real *data,
				 int   nrow,
				 real  h1,
				 real  h2,
				 int   alea,
				 int   fft_dim)
{
  unsigned long nn[3];
  int           i, j;
  int           ni, nj, ni_2, nj_2, di, dj, i0, j0;
  real          phase, rad, x;
  long          iseed, is2;
  real         *fftData;
  
  
  if(fft_dim == 0) {
    /* retourne la plus proche puissance de 2 superieure a nrow */
    ni = _Puiss2_(nrow); 
    nj = _Puiss2_(nrow);
  }
  else {
    ni = fft_dim;
    nj = fft_dim;
  }
  
  ni_2 = (int)(ni/2);
  nj_2 = (int)(nj/2);

  /* allocation memoire */
  fftData = FftData(2*nj*ni+2);
  
  (iseed) = -1;
  is2 = -1;
  x = _Ran1_(&iseed);
  x = _Ran3_(&iseed);
  x = _Gasdev_(&is2);
  iseed = 4135234;
  is2 = 3654635;
  x = _Ran1_(&iseed);
  x = _Ran3_(&iseed);
  x = _Gasdev_(&is2);

  iseed = 366555+alea*4;
  is2 = 7543526+alea*5;
  for (i = 0; i <= ni_2; i++) {
    for (j = 0; j <= nj_2; j++) {
      phase = (real) (2*M_PI*_Ran3_(&iseed));
      if ((!(i == 0)) && (!(j == 0))) {
	rad =  pow((double)(i*i), (double)(-(2*h1+1)/4));
	rad *= pow((double)(j*j), (double)(-(2*h2+1)/4));
	rad *= 1e6*_Gasdev_(&is2);
      }
      else
	rad = 0.0;
      fftData[2*(i*nj+j)+1] = rad*cos(phase);
      fftData[2*(i*nj+j)+2] = rad*sin(phase);
      
      if (i == 0)
	i0 = 0;
      else
	i0 = ni-i;
      if (j == 0)
	j0 = 0;
      else
	j0 = nj-j;
      fftData[2*(i0*nj+j0)+1] =  rad*cos(phase);
      fftData[2*(i0*nj+j0)+2] = -rad*sin(phase);
    }
  }


  fftData[2*nj_2+2] = 0.0;
  fftData[2*ni_2*nj+2] = 0.0;
  fftData[2*(ni_2*nj+nj_2)+2] = 0.0;
  
  iseed = 524455+alea*8;
  is2 = 878774676+alea*2;

  for (i = 1; i < ni_2; i++) {
    for (j = 1; j < nj_2; j++) { 
      phase = 2*M_PI*_Ran3_(&iseed);
      rad =  pow((double)(i*i), (double)(-(2*h1+1)/4));
      rad *= pow((double)(j*j), (double)(-(2*h2+1)/4));
      rad *= 1e6*_Gasdev_(&is2);
      fftData[2*(nj*(ni-i)+j)+1] =  rad*cos(phase);
      fftData[2*(nj*(ni-i)+j)+2] =  rad*sin(phase);
      fftData[2*(i*nj+nj-j)+1]   =  rad*cos(phase);
      fftData[2*(i*nj+nj-j)+2]   = -rad*sin(phase);
    }
  }
  
  nn[1] = (unsigned long) ni;
  nn[2] = (unsigned long) nj;

  _Fourn_(fftData, nn, 2, 1);
  di = (ni-nrow)/2;
  dj = (nj-nrow)/2;

  for (i = 0; i < nrow; i++)
    for (j = 0; j < nrow; j++)
      data[i*nrow+j] = 1e3*fftData[2*(nj*(j+dj)+i+di)+1]/((real)(ni*nj));
      
  free(fftData);
}

void
im_construit_brownien_fft_anis_montseny(real *data,
					int   nrow,
					real  h1,
					real  h2,
					real  beta,
					int   alea,
					int   fft_dim)
{
  unsigned long nn[3];
  int           i, j;
  int           ni, nj, ni_2, nj_2, di, dj, i0, j0;
  real          phase, rad, x;
  long          iseed, is2;
  real         *fftData;
  
  
  if(fft_dim == 0) {
    /* retourne la plus proche puissance de 2 superieure a nrow */
    ni = _Puiss2_(nrow); 
    nj = _Puiss2_(nrow);
  }
  else {
    ni = fft_dim;
    nj = fft_dim;
  }
  
  ni_2 = (int)(ni/2);
  nj_2 = (int)(nj/2);

  /* allocation memoire */
  fftData = FftData(2*nj*ni+2);
  
  (iseed) = -1;
  is2 = -1;
  x = _Ran1_(&iseed);
  x = _Ran3_(&iseed);
  x = _Gasdev_(&is2);
  iseed = 4135234;
  is2 = 3654635;
  x = _Ran1_(&iseed);
  x = _Ran3_(&iseed);
  x = _Gasdev_(&is2);

  iseed = 366555+alea*4;
  is2 = 7543526+alea*5;
  for (i = 0; i <= ni_2; i++) {
    for (j = 0; j <= nj_2; j++) {
      phase = (real) (2*M_PI*_Ran3_(&iseed));
      if ((!(i == 0)) && (!(j == 0))) {
	rad =  pow((double) (i*i), (double)((2*h1+2)/4/beta)); 
	rad+=  pow((double) (j*j), (double)((2*h2+2)/4/beta)); 
	rad =  pow(rad, (double)(-beta));
	rad*= 1e6*_Gasdev_(&is2);
      }
      else
	rad = 0.0;
      fftData[2*(i*nj+j)+1] = rad*cos(phase);
      fftData[2*(i*nj+j)+2] = rad*sin(phase);
      
      if (i == 0)
	i0 = 0;
      else
	i0 = ni-i;
      if (j == 0)
	j0 = 0;
      else
	j0 = nj-j;
      fftData[2*(i0*nj+j0)+1] =  rad*cos(phase);
      fftData[2*(i0*nj+j0)+2] = -rad*sin(phase);
    }
  }


  fftData[2*nj_2+2] = 0.0;
  fftData[2*ni_2*nj+2] = 0.0;
  fftData[2*(ni_2*nj+nj_2)+2] = 0.0;
  
  iseed = 524455+alea*8;
  is2 = 878774676+alea*2;

  for (i = 1; i < ni_2; i++) {
    for (j = 1; j < nj_2; j++) { 
      phase = 2*M_PI*_Ran3_(&iseed);
      rad =  pow((double) (i*i), (double)((2*h1+2)/4/beta)); 
      rad+=  pow((double) (j*j), (double)((2*h2+2)/4/beta)); 
      rad =  pow(rad, (double)(-beta));
      rad*= 1e6*_Gasdev_(&is2);
      fftData[2*(nj*(ni-i)+j)+1] =  rad*cos(phase);
      fftData[2*(nj*(ni-i)+j)+2] =  rad*sin(phase);
      fftData[2*(i*nj+nj-j)+1]   =  rad*cos(phase);
      fftData[2*(i*nj+nj-j)+2]   = -rad*sin(phase);
    }
  }
  
  nn[1] = (unsigned long) ni;
  nn[2] = (unsigned long) nj;

  _Fourn_(fftData, nn, 2, 1);
  di = (ni-nrow)/2;
  dj = (nj-nrow)/2;

  for (i = 0; i < nrow; i++)
    for (j = 0; j < nrow; j++)
      data[i*nrow+j] = 1e3*fftData[2*(nj*(j+dj)+i+di)+1]/((real)(ni*nj));
      
  free(fftData);
}


void
im_construit_brownien_fft_anis_yafmont(real *data,
				       int   nrow,
				       real  h1,
				       real  h2,
				       real  mux,
				       real  muy,
				       real  beta,
				       int   alea,
				       int   fft_dim)
{
  unsigned long nn[3];
  int           i, j;
  int           ni, nj, ni_2, nj_2, di, dj, i0, j0;
  real          phase, rad, x;
  long          iseed, is2;
  real         *fftData;
  
  
  if(fft_dim == 0) {
    /* retourne la plus proche puissance de 2 superieure a nrow */
    ni = _Puiss2_(nrow); 
    nj = _Puiss2_(nrow);
  }
  else {
    ni = fft_dim;
    nj = fft_dim;
  }
  
  ni_2 = (int)(ni/2);
  nj_2 = (int)(nj/2);

  /* allocation memoire */
  fftData = FftData(2*nj*ni+2);
  
  (iseed) = -1;
  is2 = -1;
  x = _Ran1_(&iseed);
  x = _Ran3_(&iseed);
  x = _Gasdev_(&is2);
  iseed = 4135234;
  is2 = 3654635;
  x = _Ran1_(&iseed);
  x = _Ran3_(&iseed);
  x = _Gasdev_(&is2);

  iseed = 366555+alea*4;
  is2 = 7543526+alea*5;
  for (i = 0; i <= ni_2; i++) {
    for (j = 0; j <= nj_2; j++) {
      phase = (real) (2*M_PI*_Ran3_(&iseed));
      if ((!(i == 0)) && (!(j == 0))) {
	rad =  mux*pow((double) (i*i), (double)((2*h1+2)/4/beta)); 
	rad+=  muy*pow((double) (j*j), (double)((2*h2+2)/4/beta)); 
	rad =  pow(rad, (double)(-beta));
	rad*= 1e6*_Gasdev_(&is2);
      }
      else
	rad = 0.0;
      fftData[2*(i*nj+j)+1] = rad*cos(phase);
      fftData[2*(i*nj+j)+2] = rad*sin(phase);
      
      if (i == 0)
	i0 = 0;
      else
	i0 = ni-i;
      if (j == 0)
	j0 = 0;
      else
	j0 = nj-j;
      fftData[2*(i0*nj+j0)+1] =  rad*cos(phase);
      fftData[2*(i0*nj+j0)+2] = -rad*sin(phase);
    }
  }


  fftData[2*nj_2+2] = 0.0;
  fftData[2*ni_2*nj+2] = 0.0;
  fftData[2*(ni_2*nj+nj_2)+2] = 0.0;
  
  iseed = 524455+alea*8;
  is2 = 878774676+alea*2;

  for (i = 1; i < ni_2; i++) {
    for (j = 1; j < nj_2; j++) { 
      phase = 2*M_PI*_Ran3_(&iseed);
      rad =  mux*pow((double) (i*i), (double)((2*h1+2)/4/beta)); 
      rad+=  muy*pow((double) (j*j), (double)((2*h2+2)/4/beta)); 
      rad =  pow(rad, (double)(-beta));
      rad*= 1e6*_Gasdev_(&is2);
      fftData[2*(nj*(ni-i)+j)+1] =  rad*cos(phase);
      fftData[2*(nj*(ni-i)+j)+2] =  rad*sin(phase);
      fftData[2*(i*nj+nj-j)+1]   =  rad*cos(phase);
      fftData[2*(i*nj+nj-j)+2]   = -rad*sin(phase);
    }
  }
  
  nn[1] = (unsigned long) ni;
  nn[2] = (unsigned long) nj;

  _Fourn_(fftData, nn, 2, 1);
  di = (ni-nrow)/2;
  dj = (nj-nrow)/2;

  for (i = 0; i < nrow; i++)
    for (j = 0; j < nrow; j++)
      data[i*nrow+j] = 1e3*fftData[2*(nj*(j+dj)+i+di)+1]/((real)(ni*nj));
      
  free(fftData);
}

void
im_construit_brownien_2D_field_fft(real *data1, real *data2,
				   int   nrow,
				   real h_bro,
				   real h_brob,
				   int   alea1,
				   int   alea2,
				   int   alea3,
				   int   fft_dim,
				   void *h_fct)
     
{
  unsigned long nn[3];
  int           i, j;
  int           ni, nj, ni_2, nj_2, di, dj, i0, j0;
  real          phase, phase1, phase2, rad, radd, x;
  long          iseed1, iseed2, iseed3, is21, is22, is23;
  real         *fftData1,*fftData2;
  real h, hh;
  real theta, tmp;
  int compt=0;

  if(fft_dim == 0) {
    /* retourne la plus proche puissance de 2 superieure a nrow */
    ni = _Puiss2_(nrow); 
    nj = _Puiss2_(nrow);
  }
  else {
    ni = fft_dim;
    nj = fft_dim;
  }

  ni_2 = (int)(ni/2);
  nj_2 = (int)(nj/2);

  /* allocation memoire */
  fftData1 = FftData(2*nj*ni+2);
  fftData2 = FftData(2*nj*ni+2);
  
  (iseed1) = -1;
  is21 = -1;
  x = _Ran1_(&iseed1);
  x = _Ran3_(&iseed1);
  x = _Gasdev_(&is21);
  iseed1 = 4135234;
  is21 = 3654635;
  x = _Ran1_(&iseed1);
  x = _Ran3_(&iseed1);
  x = _Gasdev_(&is21);

  iseed1 = 366555+alea1*4;
  is21 = 7543526+alea1*5;

  (iseed2) = -1;
  is22 = -1;
  x = _Ran1_(&iseed2);
  x = _Ran3_(&iseed2);
  x = _Gasdev_(&is22);
  iseed2 = 4135234;
  is22 = 3654635;
  x = _Ran1_(&iseed2);
  x = _Ran3_(&iseed2);
  x = _Gasdev_(&is22);

  iseed2 = 366555+alea2*4;
  is22 = 7543526+alea2*5;

  (iseed3) = -1;
  is23 = -1;
  x = _Ran1_(&iseed3);
  x = _Ran3_(&iseed3);
  x = _Gasdev_(&is23);
  iseed3 = 4135234;
  is23 = 3654635;
  x = _Ran1_(&iseed3);
  x = _Ran3_(&iseed3);
  x = _Gasdev_(&is23);

  iseed3 = 366555+alea3*4;
  is23 = 7543526+alea3*5;

  for (i = 1; i <= ni_2; i++) {
    for (j = 1; j <= nj_2; j++) {
      compt++;
      //mod   = (real) (_Ran3_(&iseed));
      phase = (real) (2*M_PI*_Ran3_(&iseed3));
      phase1 = (real) (2*M_PI*_Ran3_(&iseed1));
      phase2 = (real) (2*M_PI*_Ran3_(&iseed2));
      
      if ((!(i == 0)) && (!(j == 0))) {
	theta = atan2((double)i,(double)j);
	h  = h_bro*evaluator_evaluate_x(h_fct,theta);
	hh = h_brob*evaluator_evaluate_x(h_fct,theta);
	tmp = _Gasdev_(&is23);
	rad = 1e6*pow((double)(i*i+j*j), (double)(-(2*h+2)/4))
	  *tmp;
	radd = 1e6*pow((double)(i*i+j*j), (double)(-(2*hh+2)/4))
	  *tmp;
	//rad = _Gasdev_(&is2);
      }
      else
	rad = 0.0;
      fftData1[2*(i*nj+j)+1] = rad*cos(phase1)*cos(phase);
      fftData1[2*(i*nj+j)+2] = rad*sin(phase1)*cos(phase);
      fftData2[2*(i*nj+j)+1] = radd*cos(phase2)*sin(phase);
      fftData2[2*(i*nj+j)+2] = radd*sin(phase2)*sin(phase);
       
      if (i == 0)
	i0 = 0;
      else
	i0 = ni-i;
      if (j == 0)
	j0 = 0;
      else
	j0 = nj-j;
      fftData1[2*(i0*nj+j0)+1] =  rad*cos(phase1)*cos(phase);
      fftData1[2*(i0*nj+j0)+2] = -rad*sin(phase1)*cos(phase);
      fftData2[2*(i0*nj+j0)+1] =  radd*cos(phase2)*sin(phase);
      fftData2[2*(i0*nj+j0)+2] = -radd*sin(phase2)*sin(phase);
    }
  }


  fftData1[2*nj_2+2] = 0.0;
  fftData1[2*ni_2*nj+2] = 0.0;
  fftData1[2*(ni_2*nj+nj_2)+2] = 0.0;
  fftData2[2*nj_2+2] = 0.0;
  fftData2[2*ni_2*nj+2] = 0.0;
  fftData2[2*(ni_2*nj+nj_2)+2] = 0.0;
  
  iseed1 = 524455+alea1*8;
  is21 = 878774676+alea1*2;
  iseed2 = 524455+alea2*8;
  is22 = 878774676+alea2*2;
  iseed3 = 524455+alea3*8;
  is23 = 878774676+alea3*2;

  for (i = 1; i < ni_2; i++) {
    for (j = 1; j < nj_2; j++) {
      compt++;
      phase = 2*M_PI*_Ran3_(&iseed3);
      phase1 = 2*M_PI*_Ran3_(&iseed1);
      phase2 = 2*M_PI*_Ran3_(&iseed2);
      theta = atan2((double)i,(double)j);
      h = h_bro*evaluator_evaluate_x(h_fct,theta);
      hh = h_brob*evaluator_evaluate_x(h_fct,theta);
      tmp = _Gasdev_(&is23);
      rad  = 1e6*pow((double)(i*i+j*j), (double)(-(2*h+2)/4))*tmp;
      radd = 1e6*pow((double)(i*i+j*j), (double)(-(2*hh+2)/4))*tmp;
      fftData1[2*(nj*(ni-i)+j)+1] =  rad*cos(phase1)*cos(phase);
      fftData1[2*(nj*(ni-i)+j)+2] =  rad*sin(phase1)*cos(phase);
      fftData1[2*(i*nj+nj-j)+1]   =  rad*cos(phase1)*cos(phase);
      fftData1[2*(i*nj+nj-j)+2]   = -rad*sin(phase1)*cos(phase);
      fftData2[2*(nj*(ni-i)+j)+1] =  radd*cos(phase2)*sin(phase);
      fftData2[2*(nj*(ni-i)+j)+2] =  radd*sin(phase2)*sin(phase);
      fftData2[2*(i*nj+nj-j)+1]   =  radd*cos(phase2)*sin(phase);
      fftData2[2*(i*nj+nj-j)+2]   = -radd*sin(phase2)*sin(phase);
    }
  }
  
  nn[1] = (unsigned long) ni;
  nn[2] = (unsigned long) nj;

  _Fourn_(fftData1, nn, 2, 1);
  _Fourn_(fftData2, nn, 2, 1);
  di = (ni-nrow)/2;
  dj = (nj-nrow)/2;

  for (i = 0; i < nrow; i++)
    for (j = 0; j < nrow; j++) {
      data1[i*nrow+j] = 1e3*fftData1[2*(nj*(j+dj)+i+di)+1]/((real)(ni*nj));
      data2[i*nrow+j] = 1e3*fftData2[2*(nj*(j+dj)+i+di)+1]/((real)(ni*nj));
    }
  free(fftData1);
  free(fftData2);
}

void
im_construit_brownien_2D_field_fft_pedagogic(real *data1, real *data2,
					     real *data3, real *data4,
					     real *data5, real *data6,
					     int   nrow,
					     real h_bro, real h_bro2, 
					     real h_bro3,
					     int   alea1,
					     int   alea2,
					     int   alea3,
					     int   fft_dim,
					     void *h_fct)
     
{
  unsigned long nn[3];
  int           i, j;
  int           ni, nj, ni_2, nj_2, di, dj, i0, j0;
  real          phase, phase1, phase2, rad,rad2,rad3, x;
  long          iseed1, iseed2, iseed3, is21, is22, is23;
  real         *fftData1,*fftData2,*fftData3,*fftData4,*fftData5,*fftData6;
  real h,h2,h3;
  real theta, tmp;
  int compt=0;

  if(fft_dim == 0) {
    /* retourne la plus proche puissance de 2 superieure a nrow */
    ni = _Puiss2_(nrow); 
    nj = _Puiss2_(nrow);
  }
  else {
    ni = fft_dim;
    nj = fft_dim;
  }

  ni_2 = (int)(ni/2);
  nj_2 = (int)(nj/2);

  /* allocation memoire */
  fftData1 = FftData(2*nj*ni+2);
  fftData2 = FftData(2*nj*ni+2);
  fftData3 = FftData(2*nj*ni+2);
  fftData4 = FftData(2*nj*ni+2);
  fftData5 = FftData(2*nj*ni+2);
  fftData6 = FftData(2*nj*ni+2);
    
  (iseed1) = -1;
  is21 = -1;
  x = _Ran1_(&iseed1);
  x = _Ran3_(&iseed1);
  x = _Gasdev_(&is21);
  iseed1 = 4135234;
  is21 = 3654635;
  x = _Ran1_(&iseed1);
  x = _Ran3_(&iseed1);
  x = _Gasdev_(&is21);

  iseed1 = 366555+alea1*4;
  is21 = 7543526+alea1*5;

  (iseed2) = -1;
  is22 = -1;
  x = _Ran1_(&iseed2);
  x = _Ran3_(&iseed2);
  x = _Gasdev_(&is22);
  iseed2 = 4135234;
  is22 = 3654635;
  x = _Ran1_(&iseed2);
  x = _Ran3_(&iseed2);
  x = _Gasdev_(&is22);

  iseed2 = 366555+alea2*4;
  is22 = 7543526+alea2*5;

  (iseed3) = -1;
  is23 = -1;
  x = _Ran1_(&iseed3);
  x = _Ran3_(&iseed3);
  x = _Gasdev_(&is23);
  iseed3 = 4135234;
  is23 = 3654635;
  x = _Ran1_(&iseed3);
  x = _Ran3_(&iseed3);
  x = _Gasdev_(&is23);

  iseed3 = 366555+alea3*4;
  is23 = 7543526+alea3*5;

  for (i = 1; i <= ni_2; i++) {
    for (j = 1; j <= nj_2; j++) {
      compt++;
      //mod   = (real) (_Ran3_(&iseed));
      phase  = (real) (2*M_PI*_Ran3_(&iseed3));
      phase1 = (real) (2*M_PI*_Ran3_(&iseed1));
      phase2 = (real) (2*M_PI*_Ran3_(&iseed2));
      
      if ((!(i == 0)) && (!(j == 0))) {
	theta = atan2((double)i,(double)j);
	h  = h_bro*evaluator_evaluate_x(h_fct,theta);
	h2 = h_bro2*evaluator_evaluate_x(h_fct,theta);
	h3 = h_bro3*evaluator_evaluate_x(h_fct,theta);
	tmp = _Gasdev_(&is23);
	rad = 1e6*pow((double)(i*i+j*j), (double)(-(2*h+2)/4)) * tmp;
	rad2 = 1e6*pow((double)(i*i+j*j), (double)(-(2*h2+2)/4)) * tmp;
	rad3 = 1e6*pow((double)(i*i+j*j), (double)(-(2*h3+2)/4)) * tmp;
	
	//rad = _Gasdev_(&is2);
      }
      else
	rad = 0.0;
      fftData1[2*(i*nj+j)+1] = rad*cos(phase1)*cos(phase);
      fftData1[2*(i*nj+j)+2] = rad*sin(phase1)*cos(phase);
      fftData2[2*(i*nj+j)+1] = rad*cos(phase2)*sin(phase);
      fftData2[2*(i*nj+j)+2] = rad*sin(phase2)*sin(phase);
       
      fftData3[2*(i*nj+j)+1] = rad2*cos(phase1)*cos(phase);
      fftData3[2*(i*nj+j)+2] = rad2*sin(phase1)*cos(phase);
      fftData4[2*(i*nj+j)+1] = rad2*cos(phase2)*sin(phase);
      fftData4[2*(i*nj+j)+2] = rad2*sin(phase2)*sin(phase);
       
      fftData5[2*(i*nj+j)+1] = rad3*cos(phase1)*cos(phase);
      fftData5[2*(i*nj+j)+2] = rad3*sin(phase1)*cos(phase);
      fftData6[2*(i*nj+j)+1] = rad3*cos(phase2)*sin(phase);
      fftData6[2*(i*nj+j)+2] = rad3*sin(phase2)*sin(phase);
       
      if (i == 0)
	i0 = 0;
      else
	i0 = ni-i;
      if (j == 0)
	j0 = 0;
      else
	j0 = nj-j;
      fftData1[2*(i0*nj+j0)+1] =  rad*cos(phase1)*cos(phase);
      fftData1[2*(i0*nj+j0)+2] = -rad*sin(phase1)*cos(phase);
      fftData2[2*(i0*nj+j0)+1] =  rad*cos(phase2)*sin(phase);
      fftData2[2*(i0*nj+j0)+2] = -rad*sin(phase2)*sin(phase);

      fftData3[2*(i0*nj+j0)+1] =  rad2*cos(phase1)*cos(phase);
      fftData3[2*(i0*nj+j0)+2] = -rad2*sin(phase1)*cos(phase);
      fftData4[2*(i0*nj+j0)+1] =  rad2*cos(phase2)*sin(phase);
      fftData4[2*(i0*nj+j0)+2] = -rad2*sin(phase2)*sin(phase);

      fftData5[2*(i0*nj+j0)+1] =  rad3*cos(phase1)*cos(phase);
      fftData5[2*(i0*nj+j0)+2] = -rad3*sin(phase1)*cos(phase);
      fftData6[2*(i0*nj+j0)+1] =  rad3*cos(phase2)*sin(phase);
      fftData6[2*(i0*nj+j0)+2] = -rad3*sin(phase2)*sin(phase);
    }
  }


  fftData1[2*nj_2+2] = 0.0;
  fftData1[2*ni_2*nj+2] = 0.0;
  fftData1[2*(ni_2*nj+nj_2)+2] = 0.0;
  fftData2[2*nj_2+2] = 0.0;
  fftData2[2*ni_2*nj+2] = 0.0;
  fftData2[2*(ni_2*nj+nj_2)+2] = 0.0;
  
  fftData3[2*nj_2+2] = 0.0;
  fftData3[2*ni_2*nj+2] = 0.0;
  fftData3[2*(ni_2*nj+nj_2)+2] = 0.0;
  fftData4[2*nj_2+2] = 0.0;
  fftData4[2*ni_2*nj+2] = 0.0;
  fftData4[2*(ni_2*nj+nj_2)+2] = 0.0;
  
  fftData5[2*nj_2+2] = 0.0;
  fftData5[2*ni_2*nj+2] = 0.0;
  fftData5[2*(ni_2*nj+nj_2)+2] = 0.0;
  fftData6[2*nj_2+2] = 0.0;
  fftData6[2*ni_2*nj+2] = 0.0;
  fftData6[2*(ni_2*nj+nj_2)+2] = 0.0;
  
  iseed1 = 524455+alea1*8;
  is21 = 878774676+alea1*2;
  iseed2 = 524455+alea2*8;
  is22 = 878774676+alea2*2;
  iseed3 = 524455+alea3*8;
  is23 = 878774676+alea3*2;

  for (i = 1; i < ni_2; i++) {
    for (j = 1; j < nj_2; j++) {
      compt++;
      phase = 2*M_PI*_Ran3_(&iseed3);
      phase1 = 2*M_PI*_Ran3_(&iseed1);
      phase2 = 2*M_PI*_Ran3_(&iseed2);
      theta = atan2((double)i,(double)j);
      h = h_bro*evaluator_evaluate_x(h_fct,theta);
      h2 = h_bro2*evaluator_evaluate_x(h_fct,theta);
      h3 = h_bro3*evaluator_evaluate_x(h_fct,theta);
      tmp = _Gasdev_(&is23);
      rad = 1e6*pow((double)(i*i+j*j), (double)(-(2*h+2)/4)) * tmp;
      rad2 = 1e6*pow((double)(i*i+j*j), (double)(-(2*h2+2)/4)) * tmp;
      rad3 = 1e6*pow((double)(i*i+j*j), (double)(-(2*h3+2)/4)) * tmp;
      fftData1[2*(nj*(ni-i)+j)+1] =  rad*cos(phase1)*cos(phase);
      fftData1[2*(nj*(ni-i)+j)+2] =  rad*sin(phase1)*cos(phase);
      fftData1[2*(i*nj+nj-j)+1]   =  rad*cos(phase1)*cos(phase);
      fftData1[2*(i*nj+nj-j)+2]   = -rad*sin(phase1)*cos(phase);
      fftData2[2*(nj*(ni-i)+j)+1] =  rad*cos(phase2)*sin(phase);
      fftData2[2*(nj*(ni-i)+j)+2] =  rad*sin(phase2)*sin(phase);
      fftData2[2*(i*nj+nj-j)+1]   =  rad*cos(phase2)*sin(phase);
      fftData2[2*(i*nj+nj-j)+2]   = -rad*sin(phase2)*sin(phase);

      fftData3[2*(nj*(ni-i)+j)+1] =  rad2*cos(phase1)*cos(phase);
      fftData3[2*(nj*(ni-i)+j)+2] =  rad2*sin(phase1)*cos(phase);
      fftData3[2*(i*nj+nj-j)+1]   =  rad2*cos(phase1)*cos(phase);
      fftData3[2*(i*nj+nj-j)+2]   = -rad2*sin(phase1)*cos(phase);
      fftData4[2*(nj*(ni-i)+j)+1] =  rad2*cos(phase2)*sin(phase);
      fftData4[2*(nj*(ni-i)+j)+2] =  rad2*sin(phase2)*sin(phase);
      fftData4[2*(i*nj+nj-j)+1]   =  rad2*cos(phase2)*sin(phase);
      fftData4[2*(i*nj+nj-j)+2]   = -rad2*sin(phase2)*sin(phase);

      fftData5[2*(nj*(ni-i)+j)+1] =  rad3*cos(phase1)*cos(phase);
      fftData5[2*(nj*(ni-i)+j)+2] =  rad3*sin(phase1)*cos(phase);
      fftData5[2*(i*nj+nj-j)+1]   =  rad3*cos(phase1)*cos(phase);
      fftData5[2*(i*nj+nj-j)+2]   = -rad3*sin(phase1)*cos(phase);
      fftData6[2*(nj*(ni-i)+j)+1] =  rad3*cos(phase2)*sin(phase);
      fftData6[2*(nj*(ni-i)+j)+2] =  rad3*sin(phase2)*sin(phase);
      fftData6[2*(i*nj+nj-j)+1]   =  rad3*cos(phase2)*sin(phase);
      fftData6[2*(i*nj+nj-j)+2]   = -rad3*sin(phase2)*sin(phase);
    }
  }
  
  nn[1] = (unsigned long) ni;
  nn[2] = (unsigned long) nj;

  _Fourn_(fftData1, nn, 2, 1);
  _Fourn_(fftData2, nn, 2, 1);
  _Fourn_(fftData3, nn, 2, 1);
  _Fourn_(fftData4, nn, 2, 1);
  _Fourn_(fftData5, nn, 2, 1);
  _Fourn_(fftData6, nn, 2, 1);
  di = (ni-nrow)/2;
  dj = (nj-nrow)/2;

  for (i = 0; i < nrow; i++)
    for (j = 0; j < nrow; j++) {
      data1[i*nrow+j] = 1e3*fftData1[2*(nj*(j+dj)+i+di)+1]/((real)(ni*nj));
      data2[i*nrow+j] = 1e3*fftData2[2*(nj*(j+dj)+i+di)+1]/((real)(ni*nj));
      data3[i*nrow+j] = 1e3*fftData3[2*(nj*(j+dj)+i+di)+1]/((real)(ni*nj));
      data4[i*nrow+j] = 1e3*fftData4[2*(nj*(j+dj)+i+di)+1]/((real)(ni*nj));
      data5[i*nrow+j] = 1e3*fftData5[2*(nj*(j+dj)+i+di)+1]/((real)(ni*nj));
      data6[i*nrow+j] = 1e3*fftData6[2*(nj*(j+dj)+i+di)+1]/((real)(ni*nj));
    }
  free(fftData1);
  free(fftData2);
  free(fftData3);
  free(fftData4);
  free(fftData5);
  free(fftData6);
}

void
im_construit_brownien_2D_field_fft_div_free_pedagogic(real *data1, real *data2,
						      real *data3, real *data4,
						      int   nrow,
						      real h_bro1, 
						      real h_bro2, 
						      int   alea1,
						      int   alea2,
						      int   alea3,
						      int   fft_dim,
						      void *h_fct)
     
{
  unsigned long nn[3];
  int           i, j;
  int           ni, nj, ni_2, nj_2, di, dj, i0, j0;
  real          phase, phase1, phase2, rad1, rad2, x;
  long          iseed1, iseed2, iseed3, is21, is22, is23;
  real         *fftData1,*fftData2,*fftData3,*fftData4;
  real          h1,h2;
  real          theta, tmp, dotprod_re, dotprod_im, norme2;
  int           compt=0;

  if(fft_dim == 0) {
    /* retourne la plus proche puissance de 2 superieure a nrow */
    ni = _Puiss2_(nrow); 
    nj = _Puiss2_(nrow);
  }
  else {
    ni = fft_dim;
    nj = fft_dim;
  }

  ni_2 = (int)(ni/2);
  nj_2 = (int)(nj/2);

  /* allocation memoire */
  fftData1 = FftData(2*nj*ni+2);
  fftData2 = FftData(2*nj*ni+2);
  fftData3 = FftData(2*nj*ni+2);
  fftData4 = FftData(2*nj*ni+2);
    
  (iseed1) = -1;
  is21 = -1;
  x = _Ran1_(&iseed1);
  x = _Ran3_(&iseed1);
  x = _Gasdev_(&is21);
  iseed1 = 4135234;
  is21 = 3654635;
  x = _Ran1_(&iseed1);
  x = _Ran3_(&iseed1);
  x = _Gasdev_(&is21);

  iseed1 = 366555+alea1*4;
  is21 = 7543526+alea1*5;

  (iseed2) = -1;
  is22 = -1;
  x = _Ran1_(&iseed2);
  x = _Ran3_(&iseed2);
  x = _Gasdev_(&is22);
  iseed2 = 4135234;
  is22 = 3654635;
  x = _Ran1_(&iseed2);
  x = _Ran3_(&iseed2);
  x = _Gasdev_(&is22);

  iseed2 = 366555+alea2*4;
  is22 = 7543526+alea2*5;

  (iseed3) = -1;
  is23 = -1;
  x = _Ran1_(&iseed3);
  x = _Ran3_(&iseed3);
  x = _Gasdev_(&is23);
  iseed3 = 4135234;
  is23 = 3654635;
  x = _Ran1_(&iseed3);
  x = _Ran3_(&iseed3);
  x = _Gasdev_(&is23);

  iseed3 = 366555+alea3*4;
  is23 = 7543526+alea3*5;

  for (i = 1; i <= ni_2; i++) {
    for (j = 1; j <= nj_2; j++) {
      compt++;
      phase  = (real) (2*M_PI*_Ran3_(&iseed3));
      phase1 = (real) (2*M_PI*_Ran3_(&iseed1));
      phase2 = (real) (2*M_PI*_Ran3_(&iseed2));
      
      if ((!(i == 0)) && (!(j == 0))) {
	theta = atan2((double)i,(double)j);
	h1 = h_bro1*evaluator_evaluate_x(h_fct,theta);
	h2 = h_bro2*evaluator_evaluate_x(h_fct,theta);
	tmp = _Gasdev_(&is23);
	rad1 = 1e6*pow((double)(i*i+j*j), (double)(-(2*h1+2)/4)) * tmp;
	rad2 = 1e6*pow((double)(i*i+j*j), (double)(-(2*h2+2)/4)) * tmp;
	//rad = _Gasdev_(&is2);
      }
      else {
	rad1 = 0.0;
	rad2 = 0.0;
      }
      
      fftData1[2*(i*nj+j)+1] = rad1*cos(phase1)*cos(phase);
      fftData1[2*(i*nj+j)+2] = rad1*sin(phase1)*cos(phase);
      fftData2[2*(i*nj+j)+1] = rad1*cos(phase2)*sin(phase);
      fftData2[2*(i*nj+j)+2] = rad1*sin(phase2)*sin(phase);
       
      norme2  = i*i+j*j;
      dotprod_re = i*rad2*cos(phase)*cos(phase1)+j*rad2*sin(phase)*cos(phase2);
      dotprod_im = i*rad2*cos(phase)*sin(phase1)+j*rad2*sin(phase)*sin(phase2);
      fftData3[2*(i*nj+j)+1] = rad2*cos(phase1)*cos(phase)-dotprod_re/norme2*i;
      fftData3[2*(i*nj+j)+2] = rad2*sin(phase1)*cos(phase)-dotprod_im/norme2*i;
      fftData4[2*(i*nj+j)+1] = rad2*cos(phase2)*sin(phase)-dotprod_re/norme2*j;
      fftData4[2*(i*nj+j)+2] = rad2*sin(phase2)*sin(phase)-dotprod_im/norme2*j;
      
      if (i == 0)
	i0 = 0;
      else
	i0 = ni-i;
      if (j == 0)
	j0 = 0;
      else
	j0 = nj-j;
      fftData1[2*(i0*nj+j0)+1] =  rad1*cos(phase1)*cos(phase);
      fftData1[2*(i0*nj+j0)+2] = -rad1*sin(phase1)*cos(phase);
      fftData2[2*(i0*nj+j0)+1] =  rad1*cos(phase2)*sin(phase);
      fftData2[2*(i0*nj+j0)+2] = -rad1*sin(phase2)*sin(phase);

      //norme2  = i*i+j*j;
      //dotprod_re = -i*rad2*cos(phase)*cos(phase1)-j*rad2*sin(phase)*cos(phase2);
      //dotprod_im = -i*rad2*cos(phase)*sin(phase1)-j*rad2*sin(phase)*sin(phase2);
      fftData3[2*(i0*nj+j0)+1] =  rad2*cos(phase1)*cos(phase)-dotprod_re/norme2*i;
      fftData3[2*(i0*nj+j0)+2] = -rad2*sin(phase1)*cos(phase)+dotprod_im/norme2*i;
      fftData4[2*(i0*nj+j0)+1] =  rad2*cos(phase2)*sin(phase)-dotprod_re/norme2*j;
      fftData4[2*(i0*nj+j0)+2] = -rad2*sin(phase2)*sin(phase)+dotprod_im/norme2*j;
    }
  }


  fftData1[2*nj_2+2] = 0.0;
  fftData1[2*ni_2*nj+2] = 0.0;
  fftData1[2*(ni_2*nj+nj_2)+2] = 0.0;
  fftData2[2*nj_2+2] = 0.0;
  fftData2[2*ni_2*nj+2] = 0.0;
  fftData2[2*(ni_2*nj+nj_2)+2] = 0.0;
  
  fftData3[2*nj_2+2] = 0.0;
  fftData3[2*ni_2*nj+2] = 0.0;
  fftData3[2*(ni_2*nj+nj_2)+2] = 0.0;
  fftData4[2*nj_2+2] = 0.0;
  fftData4[2*ni_2*nj+2] = 0.0;
  fftData4[2*(ni_2*nj+nj_2)+2] = 0.0;
  
  iseed1 = 524455+alea1*8;
  is21 = 878774676+alea1*2;
  iseed2 = 524455+alea2*8;
  is22 = 878774676+alea2*2;
  iseed3 = 524455+alea3*8;
  is23 = 878774676+alea3*2;

  for (i = 1; i < ni_2; i++) {
    for (j = 1; j < nj_2; j++) {
      compt++;
      phase  = 2*M_PI*_Ran3_(&iseed3);
      phase1 = 2*M_PI*_Ran3_(&iseed1);
      phase2 = 2*M_PI*_Ran3_(&iseed2);
      theta = atan2((double)i,(double)j);
      h1 = h_bro1*evaluator_evaluate_x(h_fct,theta);
      h2 = h_bro2*evaluator_evaluate_x(h_fct,theta);
      tmp = _Gasdev_(&is23);
      rad1 = 1e6*pow((double)(i*i+j*j), (double)(-(2*h1+2)/4)) * tmp;
      rad2 = 1e6*pow((double)(i*i+j*j), (double)(-(2*h2+2)/4)) * tmp;
      
      fftData1[2*(nj*(ni-i)+j)+1] =  rad1*cos(phase1)*cos(phase);
      fftData1[2*(nj*(ni-i)+j)+2] =  rad1*sin(phase1)*cos(phase);
      fftData1[2*(i*nj+nj-j)+1]   =  rad1*cos(phase1)*cos(phase);
      fftData1[2*(i*nj+nj-j)+2]   = -rad1*sin(phase1)*cos(phase);
      fftData2[2*(nj*(ni-i)+j)+1] =  rad1*cos(phase2)*sin(phase);
      fftData2[2*(nj*(ni-i)+j)+2] =  rad1*sin(phase2)*sin(phase);
      fftData2[2*(i*nj+nj-j)+1]   =  rad1*cos(phase2)*sin(phase);
      fftData2[2*(i*nj+nj-j)+2]   = -rad1*sin(phase2)*sin(phase);

      norme2  = i*i+j*j;
      //dotprod = -i*rad2*cos(phase)+j*rad2*sin(phase);
      dotprod_re = -i*rad2*cos(phase)*cos(phase1)+j*rad2*sin(phase)*cos(phase2);
      dotprod_im = -i*rad2*cos(phase)*sin(phase1)+j*rad2*sin(phase)*sin(phase2);
      fftData3[2*(nj*(ni-i)+j)+1] =  rad2*cos(phase1)*cos(phase)-dotprod_re/norme2*(-i);
      fftData3[2*(nj*(ni-i)+j)+2] =  rad2*sin(phase1)*cos(phase)-dotprod_im/norme2*(-i);
      //dotprod = i*rad2*cos(phase)-j*rad2*sin(phase);
      //dotprod_re = i*rad2*cos(phase)*cos(phase1)-j*rad2*sin(phase)*cos(phase2);
      //dotprod_im = i*rad2*cos(phase)*sin(phase1)-j*rad2*sin(phase)*sin(phase2);
      fftData3[2*(i*nj+nj-j)+1]   =  rad2*cos(phase1)*cos(phase)-dotprod_re/norme2*(-i);
      fftData3[2*(i*nj+nj-j)+2]   = -rad2*sin(phase1)*cos(phase)+dotprod_im/norme2*(-i);
      
      //norme2  = i*i+j*j;
      //dotprod = -i*rad2*cos(phase)+j*rad2*sin(phase);
      fftData4[2*(nj*(ni-i)+j)+1] =  rad2*cos(phase2)*sin(phase)-dotprod_re/norme2*j;
      fftData4[2*(nj*(ni-i)+j)+2] =  rad2*sin(phase2)*sin(phase)-dotprod_im/norme2*j;
      //dotprod = i*rad2*cos(phase)-j*rad2*sin(phase);
      fftData4[2*(i*nj+nj-j)+1]   =  rad2*cos(phase2)*sin(phase)-dotprod_re/norme2*j;
      fftData4[2*(i*nj+nj-j)+2]   = -rad2*sin(phase2)*sin(phase)+dotprod_im/norme2*j;
    }
  }
  
  nn[1] = (unsigned long) ni;
  nn[2] = (unsigned long) nj;

  _Fourn_(fftData1, nn, 2, 1);
  _Fourn_(fftData2, nn, 2, 1);
  _Fourn_(fftData3, nn, 2, 1);
  _Fourn_(fftData4, nn, 2, 1);
  di = (ni-nrow)/2;
  dj = (nj-nrow)/2;
  
  for (i = 0; i < nrow; i++)
    for (j = 0; j < nrow; j++) {
      data1[i*nrow+j] = 1e3*fftData1[2*(nj*(j+dj)+i+di)+1]/((real)(ni*nj));
      data2[i*nrow+j] = 1e3*fftData2[2*(nj*(j+dj)+i+di)+1]/((real)(ni*nj));
      data3[i*nrow+j] = 1e3*fftData3[2*(nj*(j+dj)+i+di)+1]/((real)(ni*nj));
      data4[i*nrow+j] = 1e3*fftData4[2*(nj*(j+dj)+i+di)+1]/((real)(ni*nj));
    }
  free(fftData1);
  free(fftData2);
  free(fftData3);
  free(fftData4);
}

/* the following routine should be rewritten for using fftw and allow for
 * all type of buffer dimension
 */
void
im_construit_brownien3D_fft(/*FILE *fileOut,*/
			    float *data,
			    int   nrow,
			    real h_bro,
			    int   alea,
			    int   fft_dim,
			    void *h_fct)

{
  unsigned long nn[4];
  int           i, j, k, ii, jj;
  int           ni, nj, nk, ni_2, nj_2, nk_2, di, dj, dk;
  real         phase, rad, x;
  long          iseed, is2;
  real         *fftData;
  real h;
  real theta;

  int compt=0;
  
  if(fft_dim == 0) {
    ni = _Puiss2_(nrow);
    nj = ni;
    nk = ni;
  }
  else {
    ni = fft_dim;
    nj = fft_dim;
    nk = fft_dim;
  }

  ni_2 = (int)(ni/2);
  nj_2 = (int)(nj/2);
  nk_2 = (int)(nk/2);

  fftData = FftData(2*nk*nj*ni+2);

  (iseed) = -1;
  is2 = -1;
  x = _Ran1_(&iseed);
  x = _Ran3_(&iseed);
  x = _Gasdev_(&is2);
  iseed = 4135234;
  is2 = 3654635;
  x = _Ran1_(&iseed);
  x = _Ran3_(&iseed);
  x = _Gasdev_(&is2);

  iseed = 366555+alea*4;
  is2 = 7543526+alea*5;
  for (i = 1; i < ni; i++) {
    for (j = 1; j < nj; j++) {
      for (k = 1; k <= nk_2; k++) {
	compt++;

	if (i>ni_2)
	  ii=ni-i;
	else
	  ii=i;

	if (j>nj_2)
	  jj=nj-j;
	else
	  jj=j;


	phase = (real) (2*M_PI*_Ran3_(&iseed));

	theta = atan2((double)i,(double)j);
	h = h_bro*evaluator_evaluate_x(h_fct,theta);
	rad = 1e6*pow((double)(ii*ii+jj*jj+k*k), (double)(-(2*h+3)/4))
	  *_Gasdev_(&is2);
	
	fftData[2*(i*nj*nk+j*nk+k)+1] = rad*cos(phase);
	fftData[2*(i*nj*nk+j*nk+k)+2] = rad*sin(phase);
	
	fftData[2*((ni-i)*nj*nk+(nj-j)*nk+nk-k)+1] =  rad*cos(phase);
	fftData[2*((ni-i)*nj*nk+(nj-j)*nk+nk-k)+2] = -rad*sin(phase);
      }
    }
  }
  

  nn[1] = (unsigned long) ni;
  nn[2] = (unsigned long) nj;
  nn[3] = (unsigned long) nk;

  _Fourn_(fftData, nn, 3, 1);
  di = (ni-nrow)/2;
  dj = (nj-nrow)/2;
  dk = (nk-nrow)/2;

  {
    int index = 0;
    /*float * data;
      data = (float *) malloc(sizeof(float)*nrow*nrow);*/
    for (i = 0; i < nrow; i++) {
      for (j = 0; j < nrow; j++) {
	for (k = 0; k < nrow; k++) {
	  /*data[j*nrow+k] = 1e3*(float) fftData[2*((i+di)*nj*nk+(j+dj)*nk+k+dk)+1]/((real)(ni*nj*nk));*/
	  data[index] = 1e3*(float) fftData[2*((i+di)*nj*nk+(j+dj)*nk+k+dk)+1]/((real)(ni*nj*nk));
	  index++;
	}
      }
      /*fwrite(data, sizeof(float), nrow*nrow, fileOut);*/
    }
    /*free(data);*/
  }
  free(fftData);
}



/*
 * Fonction a modifier pour qu'elle renvoie une image ! Cf la commande
 * xsmurf associee.
 */

/*========================================================*/
/* DLA : jolie fonction qui charge un dla */
/*========================================================*/

void
im_Lit_dla_ (real * image,char * nom_fichier, int taille, 
	     int ncol, real angle)
{
  FILE *fin;
  int nrow;
  int i, j,flag_rot,t2;
  int rmax, deca=0,ind;
  real rmax2,p0;
  real p[2];			/* pour recevoir une particule */
  real cosa,sina;		/* sin et cos d'une eventuelle rotation */
  real rapport;		/* rapport d'une eventuelle homothetie */

  t2=ncol/2;
  if (angle==0.0)
    flag_rot=0;
  else
    flag_rot=1;
  
  /* cos et sin de la rotation */
  cosa=cos(angle);
  sina=sin(angle);

  /* si taille=0, on fait en sorte que tout l'amas tienne dans l'image sinon
     on coupe l'amas au rayon taille/2 */
  
  
  if (taille)
    {
      rmax = taille / 2; 
      deca = (ncol - taille) / 2;
    }
  else
    {
      rmax=ncol/2;
      deca=0;
    }
  rmax2 = (real) (rmax * rmax); 
  nrow = ncol;
  
  fin = fopen (nom_fichier, "r");
  if (fin == NULL)
    /* Attention, impossible d'ouvrir le fichier */
    return;
  else
    {
      /* si taille=0 , on fait une premiere lecture pour evaluer la taille */
      if (!taille)
	{			
	  real r2;
	  
	  /* calcul du rayon max de l'amas */
	  rmax2=0;
	  while (fread (p, sizeof (real), 2, fin) > 0)
	    {
	      r2 = p[0] * p[0] + p[1] * p[1];
	      if (r2>rmax2)
		rmax2=r2;
	    }
	  /* calcul du rapport de l'homothetie */
	  rapport=((real) rmax)*0.9/sqrt(rmax2);
          /* ("Rayon de l'amas %f \n",sqrt(rmax2)) */

	  /* repositionnement du pointeur de lecture au debut du fichier */
	  rewind(fin);
	}
      /*lecture des part. et mise a jour de l'image */
      while (fread (p, sizeof (real), 2, fin) > 0)
	if (p[0] * p[0] + p[1] * p[1] < rmax2)
	  {
	    if (flag_rot)
	      {
		p0=p[0];
		p[0]=p[0]*cosa-p[1]*sina;
		p[1]=p0*sina+p[1]*cosa;
	      }
	    if (!taille)
	      {
		p[0]*=rapport;
		p[1]*=rapport;
	      }
	    i = ((int) p[0]) + rmax + deca;
	    j = ((int) p[1]) + rmax + deca;
	    ind=i*ncol+j;
	    if ((ind>=0) && (ind<ncol*ncol))
	      image[i * ncol + j] = 1000.0;
	  }
      fclose (fin);
    }

}


/************************************************************************/

/*
 * Fonction a modifier pour qu'elle renvoie une image ! Cf la commande
 * xsmurf associee.
 */

void
im_bruit_blanc_(real *data,
		int   taille,
		real sigma,
		int   type,
		int   alea)
{
  int     i;
  real   x;
  long    iseed, is2;

  (iseed) = -1;
  is2 = -1;
  x = _Ran1_ (&iseed);
  x = _Ran3_ (&iseed);
  x = _Gasdev_ (&is2);
  iseed = 4135234+alea*3;
  is2 = 3654635+alea*2;
  x = _Ran1_ (&iseed);
  x = _Ran3_ (&iseed);
  x = _Gasdev_ (&is2);

  if (type==RAN1) /*  bruit blanc avec ran1 */
    for (i = 0; i < taille*taille; i++)
      data[i] = 1000.*(_Ran1_(&iseed)-0.5);

  if (type==RAN3) /*  bruit blanc avec ran3 */
    for (i = 0; i < taille*taille; i++)
      data[i] = 1000.*(_Ran3_(&iseed)-0.5);

  if (type==GAUSS) /*  bruit blanc gaussien avec gasdev */
    for (i = 0; i < taille*taille; i++)
      data[i] = sigma*_Gasdev_(&is2);

  if (type==EXPO) /*  bruit blanc exponetiel */
    for (i = 0; i < taille*taille; i++)
      data[i] = -sigma*log((double) (_Ran1_(&iseed)));
}


/*------------------------------------------------------------------------
  Cette fonction est integralement repompee de cwave2
  Elle cree un flocon de neige mathematique...
  ----------------------------------------------------------------------*/
void
_SFlake_(real * data,
	 int Lx,
	 real x1,
	 real x2,
	 real y1,
	 real y2,
	 real p,
	 real p1,
	 real p2,
	 real p3,
	 real p4,
	 real p5,
	 real p6,
	 real p7,
	 real p8,
	 real p9)
{
  real lx = (x2 - x1) / 3;
  real ly = (y2 - y1) / 3;

  if ((x2 - x1 >= 1 || y2 - y1 >= 1)) {
    _SFlake_(data, Lx, x1, x1+lx, y1, y1+ly, p*p1, p1,
	     p2, p3, p4, p5, p6, p7, p8, p9);
    _SFlake_(data, Lx, x1+lx, x1+2*lx, y1, y1+ly, p*p2,
	     p1, p2, p3, p4, p5, p6, p7, p8, p9);
    _SFlake_(data, Lx, x1+2*lx, x2, y1, y1+ly, p*p3, p1,
	     p2, p3, p4, p5, p6, p7, p8, p9);
    _SFlake_(data, Lx, x1, x1+lx, y1+ly, y1+2*ly, p*p4, p1,
	     p2, p3, p4, p5, p6, p7, p8, p9);
    _SFlake_(data, Lx, x1+lx, x1+2*lx, y1+ly, y1+2*ly,
	     p*p5, p1, p2, p3, p4, p5, p6, p7, p8, p9);
    _SFlake_(data, Lx, x1+2*lx, x2, y1+ly, y1+2*ly, p*p6,
	     p1, p2, p3, p4, p5, p6, p7, p8, p9);
    _SFlake_(data, Lx, x1, x1+lx, y1+2*ly, y2, p*p7, p1, p2,
	     p3, p4, p5, p6, p7, p8, p9);
    _SFlake_(data, Lx, x1+lx, x1+2*lx, y1+2*ly, y2, p*p8,
	     p1, p2, p3, p4, p5, p6, p7, p8, p9);
    _SFlake_(data, Lx, x1+2*lx, x2, y1+2*ly, y2, p*p9, p1,
	     p2, p3, p4, p5, p6, p7, p8, p9);
  }
  else {
    data[((int) y1)*Lx+((int) x1)] += p;
  }
}

/************************************************/
void
fl2 (real* values,
     int ncol,
     real x,
     real y,
     real lx,
     real ly,
     real p,
     int etape)
{
  int i, j;


  if (etape > 0)
    {
      fl2(values, ncol, x, y, lx / 4, ly / 4, p, etape - 1);
      fl2(values, ncol, x + 3. * lx, y, lx / 4., ly / 4., p, etape - 1);
      fl2(values, ncol, x, y + 3. * ly, lx / 4., ly / 4., p, etape - 1);
      fl2(values, ncol, x + 3. * lx, y + 3. * ly, lx / 4., ly / 4., p, etape - 1);
      fl2(values, ncol, x + lx, y + ly, lx / 2, ly / 2, p, etape - 1);
    }
  else
    {

      for (i = 0; i < lx * 4; i++)
        for (j = 0; j < ly * 4; j++)
          values[((int) (y + j)) * ncol + ((int) (x + i))] += 1;

      /*      values[((int) y)*ncol + ((int) x)] += p; */
    }
}
/* Ajouts de Andre Khalil -- option -yaf dans ibro
   2005 02 16
*/

void
im_construit_brownien_fft_anis_yaf(real *data,
				   int   nrow,
				   real  h1,
				   real  h2,
				   real  mux,
				   real  muy,
				   int   alea,
				   int   fft_dim)
{
  unsigned long nn[3];
  int           i, j;
  int           ni, nj, ni_2, nj_2, di, dj, i0, j0;
  real         phase, rad, x;
  long          iseed, is2;
  real         *fftData;
  real beta1, beta2;
  real theta;
  int compt=0;
    
  beta1 = - (h1+1);
  beta2 = - (h2+1);
  if(fft_dim == 0) {
    /* retourne la plus proche puissance de 2 superieure a nrow */
    ni = _Puiss2_(nrow); 
    nj = _Puiss2_(nrow);
  }
  else {
    ni = fft_dim;
    nj = fft_dim;
  }
    
  ni_2 = (int)(ni/2);
  nj_2 = (int)(nj/2);
    
  /* allocation memoire */
  fftData = FftData(2*nj*ni+2);
    
  (iseed) = -1;
  is2 = -1;
  x = _Ran1_(&iseed);
  x = _Ran3_(&iseed);
  x = _Gasdev_(&is2);
  iseed = 4135234;
  is2 = 3654635;
  x = _Ran1_(&iseed);
  x = _Ran3_(&iseed);
  x = _Gasdev_(&is2);
    
  iseed = 366555+alea*4;
  is2 = 7543526+alea*5;
  for (i = 0; i <= ni_2; i++) {
    for (j = 0; j <= nj_2; j++) {
      compt++;
      phase = (real) (2*M_PI*_Ran3_(&iseed));
      if ((!(i == 0)) && (!(j == 0))) {
	theta = atan2((double)i,(double)j);
	rad =  mux*lgamma(1+beta1/2)/pow(2,1-beta1)/lgamma(2-beta1/2)*cos(theta)*cos(theta)*pow((double)(i*i+j*j), (double)(-(2*h1+2)/4));
	rad += muy*lgamma(1+beta2/2)/pow(2,1-beta2)/lgamma(2-beta2/2)*sin(theta)*sin(theta)*pow((double)(i*i+j*j), (double)(-(2*h2+2)/4));
	rad *= 1e6*_Gasdev_(&is2);
      }
      else
	rad = 0.0;
      fftData[2*(i*nj+j)+1] = rad*cos(phase);
      fftData[2*(i*nj+j)+2] = rad*sin(phase);
	    
      if (i == 0)
	i0 = 0;
      else
	i0 = ni-i;
      if (j == 0)
	j0 = 0;
      else
	j0 = nj-j;
      fftData[2*(i0*nj+j0)+1] =  rad*cos(phase);
      fftData[2*(i0*nj+j0)+2] = -rad*sin(phase);
    }
  }
    
    
  fftData[2*nj_2+2] = 0.0;
  fftData[2*ni_2*nj+2] = 0.0;
  fftData[2*(ni_2*nj+nj_2)+2] = 0.0;
    
  iseed = 524455+alea*8;
  is2 = 878774676+alea*2;
    
  for (i = 1; i < ni_2; i++) {
    for (j = 1; j < nj_2; j++) {
      compt++;
      phase = 2*M_PI*_Ran3_(&iseed);
      if ((!(i == 0)) && (!(j == 0))) {
	theta = atan2((double)i,(double)j);
      }
      rad =  mux*lgamma(1+beta1/2)/pow(2,1-beta1)/lgamma(2-beta1/2)*cos(theta)*cos(theta)*pow((double)(i*i+j*j), (double)(-(2*h1+2)/4));
      rad += muy*lgamma(1+beta2/2)/pow(2,1-beta2)/lgamma(2-beta2/2)*sin(theta)*sin(theta)*pow((double)(i*i+j*j), (double)(-(2*h2+2)/4));
      rad *= 1e6*_Gasdev_(&is2);
      fftData[2*(nj*(ni-i)+j)+1] =  rad*cos(phase);
      fftData[2*(nj*(ni-i)+j)+2] =  rad*sin(phase);
      fftData[2*(i*nj+nj-j)+1]   =  rad*cos(phase);
      fftData[2*(i*nj+nj-j)+2]   = -rad*sin(phase);
    }
  }
    
  nn[1] = (unsigned long) ni;
  nn[2] = (unsigned long) nj;
    
  _Fourn_(fftData, nn, 2, 1);
  di = (ni-nrow)/2;
  dj = (nj-nrow)/2;
    
  for (i = 0; i < nrow; i++)
    for (j = 0; j < nrow; j++)
      data[i*nrow+j] = 1e3*fftData[2*(nj*(j+dj)+i+di)+1]/((real)(ni*nj));
    
  free(fftData);
}




