/*************************************************************************
 * extrema.c - tools for non-maxima gradient point suppression
 *
 *
 * Copyright©ENS LYON 2002-2003
 * Copyright@CEA      2003-2006
 *
 * DESCRIPTION: 
 *
 *
 * AUTHOR:
 * Pierre Kestener (pierre.kestener@ens-lyon.fr)
 * pierre.kestener@cea.fr 
 *
 * CREATION DATE: 
 * December, 13 2002
 *
 *
 * Copyright Pierre Kestener, ENS Lyon and CEA
 *
 * ADDITIONS, CHANGES
 * May, 20 2003
 * September, 13 2005
 *
 */

/* !!!! RECALL !!!! */
// typedef float fftw_real;
// typedef fftw_complex FFTW_COMPLEX; 
// typedef fftw_real FFTW_REAL;
// typedef struct {
//     fftw_real re, im;
// } fftw_complex;

#include <string.h>

#include <extrema_core.h>
#include "misc.h"

static int _VERBOSE_ = 0;


#define EXIT_ON_FAILURE 0 
#define EXIT_ON_SUCCESS 1 

/* /\* */
/*  * These routines are usefull to fit data into the  */
/*  * format used by rfftwnd_one_real_to_complex  */
/*  *\/  */
/* /\* recall memmove prototype : */
/*  *   void *memmove(void *dest, const void *src, size_t n); */
/*  *\/ */
/* static void Shift_Buffer_2D(fftw_real *indata, int M, int N)  */
/* {  */
/*   int i;  */
  
/*   for (i=M-1; i>=0; i--) */
/*     memmove(indata+i*2*(N/2+1),indata+i*N,N*sizeof(fftw_real)); */
/* } */

/* static void Shift_Inv_Buffer_2D(fftw_real *indata, int M, int N) */
/* { */
/*   int i; */
  
/*   for (i=0; i<M; i++) */
/*     memmove(indata+i*N,indata+i*2*(N/2+1),N*sizeof(fftw_real)); */
/*   //for (i=M-1; i<=0; i--) */
/*   // memmove(indata+i*N,indata+i*2*(N/2+1),N*sizeof(fftw_real)); */
/* } */


/* static fftw_real _tmp_r_; */
/* #define _swap_r_(a,b) _tmp_r_=(a);(a)=(b);(b)=_tmp_r_ */

/* static void Swap_Buffer_2D(fftw_real *indata, int M, int N)  */
/* {  */
/*   int i,j;  */
  
/*   fftw_real *tmp; */
  
/*   tmp = (fftw_real *) malloc(N*sizeof(fftw_real)); */

/*   for (i = 0; i < M/2; i++) { */
/*     for (j = 0; j < N/2; j++) { */
/*       /\* Swap upper-left with lower-right. *\/ */
/*       _swap_r_(indata[j+i*N], indata[(j+N/2)+(i+M/2)*N]); */
/*       /\* Swap upper-right with lower-left. *\/ */
/*       _swap_r_(indata[(j+N/2)+i*N], indata[j+(i+M/2)*N]); */
/*     } */
/*   } */
/* /\*   for (i = 0; i < M/2; i++) { *\/ */
/* /\*     memmove(tmp                 ,indata+(i+M/2)*N,N*sizeof(fftw_real)); *\/ */
/* /\*     memmove(indata+(i+M/2)*N    ,indata+i*N      ,N*sizeof(fftw_real)); *\/ */
/* /\*     memmove(indata+i*N          ,tmp             ,N*sizeof(fftw_real)); *\/ */
/* /\*   } *\/ */
/* /\*   free(tmp); *\/ */
/* }  */


/* static void Shift_Buffer_3D(fftw_real *indata, int L, int M, int N)  */
/* {  */
/*   int i,j;  */
  
/*   for (i=L-1; i>=0; i--) */
/*     for (j=M-1; j>=0; j--)  */
/*       memmove(indata+(j+i*M)*2*(N/2+1),indata+(j+i*M)*N,N*sizeof(fftw_real));  */
/* } */

/* static void Shift_Inv_Buffer_3D(fftw_real *indata, int L, int M, int N) */
/* {  */
/*   int i,j;  */
  
/*   for (i=0; i<L; i++) */
/*     for (j=0; j<M; j++)  */
/*       memmove(indata+(j+i*M)*N,indata+(j+i*M)*2*(N/2+1),N*sizeof(fftw_real));  */
/* } */

/* static void Swap_Buffer_3D(fftw_real *indata, int L, int M, int N)  */
/* {  */
/*   int i;  */
/*   fftw_real *tmp; */
  
/*   tmp = (fftw_real *) malloc(M*N*sizeof(fftw_real)); */
  
/*   for (i = 0; i < L/2; i++) { */
/*     memmove(tmp                 ,indata+(i+L/2)*M*N,M*N*sizeof(fftw_real)); */
/*     memmove(indata+(i+L/2)*M*N  ,indata+i*M*N      ,M*N*sizeof(fftw_real)); */
/*     memmove(indata+i*M*N        ,tmp               ,M*N*sizeof(fftw_real)); */
/*   } */
/*   free(tmp); */
/* } */

/* static void Swap_Buffer_3D_bis(fftw_real *indata, int L, int M, int N)  */
/* {  */
/*   int i,j,k;  */
  
/*   for (i = 0; i < L/2; i++) {  */
/*     for (j = 0; j < M/2; j++) {  */
/*       for (k = 0; k < N/2; k++) {  */
/* 	_swap_r_(indata[k+j*N+i*N*M], indata[(k+N/2)+(j+M/2)*N+(i+L/2)*N*M]);  */
/* 	_swap_r_(indata[(k+N/2)+j*N+i*N*M], indata[k+(j+M/2)*N+(i+L/2)*N*M]);  */
/* 	_swap_r_(indata[k+(j+M/2)*N+i*N*M], indata[(k+N/2)+j*N+(i+L/2)*N*M]);  */
/* 	_swap_r_(indata[(k+N/2)+(j+M/2)*N+i*N*M], indata[k+j*N+(i+L/2)*N*M]);  */
/*       }  */
/*     }    */
/*   }  */
/* } */


/***************************************************** 
 ***************************************************** 
 *****************************************************/ 
int Extract_Gradient_Maxima_2D( Image *GradX,
				Image *GradY,
				float *mod,
				float *arg,
				float *maxima,
				float scale)
{
  char *proc="Extract_Gradient_Maxima_2D";
  
  /* 
   * Pointers 
   */ 
  float *gx = GradX->data;
  float *gy = GradY->data;
  
  int sliceDims[3];
  
  int dimxXdimy;
  int lx,ly;
  

  lx = GradX->lx;
  ly = GradX->ly;
  
  dimxXdimy  = lx * ly;
  sliceDims[0] = lx;
  sliceDims[1] = ly;
  sliceDims[2] = 1;
  
  /*
   * Modulus and argument of the gradient
   */ 
  GradientModulus2D (mod, gx, gy, dimxXdimy );
  GradientArgument2D(arg, gx, gy, dimxXdimy );
  
  Remove_Gradient_NonMaxima_Slice_2D( maxima, gx ,gy,
				      mod, sliceDims );
  
  return( EXIT_ON_SUCCESS );
}

/***********************************************
 ***********************************************
 ***********************************************/
int Extract_Gradient_Maxima_2D_vectorfield( Image *GradX1,
					    Image *GradY1,
					    Image *GradX2,
					    Image *GradY2,
					    float *mod,
					    float *arg,
					    float *maxima,
					    float scale, 
					    int type_singular_value)
{
  char *proc="Extract_Gradient_Maxima_2D_vectorfield";

  /*
   * Pointers
   */
  float *gx1 = GradX1->data;
  float *gx2 = GradX2->data;
  float *gy1 = GradY1->data;
  float *gy2 = GradY2->data;

  int sliceDims[3];
  
  int dimxXdimy;
  int lx,ly;
 
  lx = GradX1->lx;
  ly = GradX1->ly;
  
  dimxXdimy  = lx * ly;
  sliceDims[0] = lx;
  sliceDims[1] = ly;
  sliceDims[2] = 1;

  /*
   * Modulus and argument of the gradient vector (obtained by the SVD
   * of the gradient tensor)
   */ 
  GradientModulus2D_tensor2D( mod, arg, gx1, gy1, gx2, gy2, dimxXdimy, type_singular_value );
  
  /*
   * Suppression of the non maxima of the gradient
   * in the direction of the gradient.
   *
   */
  Remove_Gradient_NonMaxima_Slice_2D( maxima, gx1 ,gy1,
				      mod, sliceDims );
  
  
  return( EXIT_ON_SUCCESS );
}

/***************************************************** 
 ***************************************************** 
 *****************************************************/ 
int Extract_Gradient_Maxima_3D( Image3D *GradX,
				Image3D *GradY,
				Image3D *GradZ,
				float *mod,
				float *maxima,
				float scale)
{
  char *proc="Extract_Gradient_Maxima_3D";
  
  /* 
   * Pointers 
   */ 
  float *Gx,*gx[2];
  float *Gy,*gy[2];
  float *Gz,*gz;

  float *norme[3], *pt;
  float *max = maxima;
  
  int sliceDims[3];
  
  int dimxXdimy, z;
  int lx,ly,lz;
  int Lx,Ly,Lz;
  

  /* column major sizes */
  lx = GradX->lx;
  ly = GradX->ly;
  lz = GradZ->lz;

  /* row major sizes */
  /*Lx = lz;
  Ly = ly;
  Lz = 2 * (lx/2+1);*/
  Lx = lx;
  Ly = ly;
  Lz = lz;

  dimxXdimy  = Lx * Ly;
  
  sliceDims[0] = Lx;
  sliceDims[1] = Ly;
  sliceDims[2] = Lz;
  
  /* compute gradient modulus */
  Gx = GradX->data;
  Gy = GradY->data;
  Gz = GradZ->data;
  GradientModulus3D (mod, Gx, Gy, Gz, Lx*Ly*Lz );

  /*printf("mod[%i]=%f\n",0,mod[0]);*/

  /* norme pointers */
  /*norme[0] = mod;
  norme[1] = norme[0] + dimxXdimy;
  norme[2] = norme[1] + dimxXdimy;*/

  /*
   * First slice: extraction of 2D edges.
   */
  gx[0] = Gx;
  gy[0] = Gy;
  norme[1] = mod;
  Remove_Gradient_NonMaxima_Slice_2D( max, gx[0] ,gy[0],
				      norme[1], sliceDims );

  max += dimxXdimy;
  gx[1] = gx[0] + dimxXdimy;
  gy[1] = gy[0] + dimxXdimy;
  norme[2] = mod + dimxXdimy;

  /*
   * slice by slice processing 
   */
  for ( z=1; z<Lz-1; z++ ) {
    /*
     * slices permutations
     */
    pt = gx[0]; gx[0] = gx[1]; gx[1] = pt;
    pt = gy[0]; gy[0] = gy[1]; gy[1] = pt;
    pt = norme[0]; norme[0] = norme[1]; 
    norme[1] = norme[2]; norme[2] = pt;
    /*
     * gx[0] and gy[0] are the X and Y components
     * of the gradient of the current slice.
     * gx[1] and gy[1] are the X and Y components
     * of the gradient of the next slice.
     * norme[0] is the gradient modulus of the previous slice,
     * norme[1] is the gradient modulus of the current slice,
     * norme[2] is the gradient modulus of the next slice.
     */
    /*
     * Processing of the next slice.
     * - computation of the X component of the gradient
     *   for that slice
     * - idem for the Y component of the gradient
     * - computation of the modulus
     */

    gx[1] = Gx + (z+1)*dimxXdimy;
    gy[1] = Gy + (z+1)*dimxXdimy;
    gz    = Gz + z*dimxXdimy;
    norme[2] = mod + (z+1)*dimxXdimy;

    /*
     * suppression of the 3D non maxima of the gradient.
     */
    Remove_Gradient_NonMaxima_Slice_3D( max, gx[0], gy[0], gz, norme, sliceDims );

    max += dimxXdimy;
  }

  /*
   * last slice 
   * 
   * Components and modulus of the gradient are 
   * already computed.
   *
   * - 2D suppression of the non maxima
   */
  Remove_Gradient_NonMaxima_Slice_2D( max, gx[1], gy[1],
				      norme[2], sliceDims );
  
  return( EXIT_ON_SUCCESS );
}

int Extract_Gradient_Maxima_3D_vectorfield( Image3D *GradX1,
					    Image3D *GradY1,
					    Image3D *GradZ1,
					    Image3D *GradX2,
					    Image3D *GradY2,
					    Image3D *GradZ2,
					    Image3D *GradX3,
					    Image3D *GradY3,
					    Image3D *GradZ3,
					    float *mod,
					    float *maxima,
					    float scale, 
					    int type_singular_value)
{
  char *proc="Extract_Gradient_Maxima_3D_vectorfield";
  
  /* 
   * Pointers 
   */ 
  float *Gx1;
  float *Gx2;
  float *Gx3;

  float *Gy1;
  float *Gy2;
  float *Gy3;

  float *Gz1;
  float *Gz2;
  float *Gz3;

  float *gx[2], *gy[2], *gz;

  float *norme[3], *pt;
  float *max = maxima;
  
  int sliceDims[3];
  
  int dimxXdimy, z;
  int lx,ly,lz;
  int Lx,Ly,Lz;
  

  /* column major sizes */
  lx = GradX1->lx;
  ly = GradX1->ly;
  lz = GradZ1->lz;

  /* row major sizes */
  Lx = lx;
  Ly = ly;
  Lz = lz;

  dimxXdimy  = Lx * Ly;
  
  sliceDims[0] = Lx;
  sliceDims[1] = Ly;
  sliceDims[2] = Lz;
  
  /* compute gradient modulus */
  Gx1 = GradX1->data;
  Gx2 = GradX2->data;
  Gx3 = GradX3->data;
  Gy1 = GradY1->data;
  Gy2 = GradY2->data;
  Gy3 = GradY3->data;
  Gz1 = GradZ1->data;
  Gz2 = GradZ2->data;
  Gz3 = GradZ3->data;

  /*
   * Modulus of the gradient vector (obtained by the SVD
   * of the gradient tensor); the results are overwriten in Gx1, Gy1 and Gz1
   */ 
  GradientModulus3D_tensor3D (mod, Gx1, Gy1, Gz1, Gx2, Gy2, Gz2, Gx3, Gy3, Gz3, Lx*Ly*Lz, type_singular_value, 0 );

  /*
   * First slice: extraction of 2D edges.
   */
  gx[0] = Gx1;
  gy[0] = Gy1;
  norme[1] = mod;
  Remove_Gradient_NonMaxima_Slice_2D( max, gx[0] ,gy[0], norme[1], sliceDims );

  max += dimxXdimy;
  gx[1] = gx[0] + dimxXdimy;
  gy[1] = gy[0] + dimxXdimy;
  norme[2] = mod + dimxXdimy;

  /*
   * slice by slice processing 
   */
  for ( z=1; z<Lz-1; z++ ) {
    /*
     * slices permutations
     */
    pt = gx[0]; gx[0] = gx[1]; gx[1] = pt;
    pt = gy[0]; gy[0] = gy[1]; gy[1] = pt;
    pt = norme[0]; norme[0] = norme[1]; 
    norme[1] = norme[2]; norme[2] = pt;
    /*
     * gx[0] and gy[0] are the X and Y components
     * of the gradient of the current slice.
     * gx[1] and gy[1] are the X and Y components
     * of the gradient of the next slice.
     * norme[0] is the gradient modulus of the previous slice,
     * norme[1] is the gradient modulus of the current slice,
     * norme[2] is the gradient modulus of the next slice.
     */
    /*
     * Processing of the next slice.
     * - computation of the X component of the gradient
     *   for that slice
     * - idem for the Y component of the gradient
     * - computation of the modulus
     */
    
    gx[1] = Gx1 + (z+1)*dimxXdimy;
    gy[1] = Gy1 + (z+1)*dimxXdimy;
    gz    = Gz1 + z*dimxXdimy;
    norme[2] = mod + (z+1)*dimxXdimy;
    
    /*
     * suppression of the 3D non maxima of the gradient.
     */
    Remove_Gradient_NonMaxima_Slice_3D( max, gx[0], gy[0], gz, norme, sliceDims );
    
    max += dimxXdimy;
  }
  
  /*
   * last slice 
   * 
   * Components and modulus of the gradient are 
   * already computed.
   *
   * - 2D suppression of the non maxima
   */
  Remove_Gradient_NonMaxima_Slice_2D( max, gx[1], gy[1],
				      norme[2], sliceDims );
  
  return( EXIT_ON_SUCCESS );
  
}
