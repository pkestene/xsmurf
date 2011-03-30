/* mimgmanip.c  --  Image manipulation subroutines for morph */

/* contains: MakeByteImg(), MakeWordImg(), MoveByte2Word(),     */
/*           MoveByte2Byte(), ScaleWord2Byte(), ZeroPadByte(),  */
/*           ZeroPadWord(), SubtractImg(), SubtrByteImg()       */

/* dependencies: none */    

/* morph version 4.0   10 May 1993                 */
/* image morphology program                        */ 
/*                                                 */ 
/* by Richard Alan Peters II                       */
/* Department of Electrical Engineering            */
/* Vanderbilt University School of Engineering     */ 
/* Nashville, TN 37235                             */ 
/* rap2@vuse.vanderbilt.edu                        */ 
/*                                                 */ 
/* This software is freely redistributable if      */ 
/* the author's name and affiliation are included. */

/* See changes.doc for an explanation of the changes in v4.0. */

#include "morph_sub.h"


byte *MakeByteImg( x, y )
   int x,y;
   {
   byte *B;
   if ( !(B = (byte *)calloc( x*y, sizeof(byte) )) )
      {
      fprintf(stderr,"Unable to allocate space for byte image\n");
      exit(0);
      }
   return( B );
   }




word *MakeWordImg( x, y )
   int x,y;
   {
   word *W;

   if ( !(W = (word *)calloc( x*y, sizeof(word) )) )
      {
      fprintf(stderr,"Unable to allocate space for integer image\n");
      exit(0);
      }

   return( W );
   }


/* all of the following move routines permit the copying of */ 
/* one rectangular subimage to another rectangular subimage */


/* copy a byte image to a word image */

void MoveByte2Word( S, sX, sY, soX, soY, D, dX, dY, doX, doY, X, Y )
   byte *S;         /* source image */
   int  sX,sY;      /* source image dimensions */
   int  soX,soY;    /* source image offsets */
   word *D;         /* destination image */
   int  dX,dY;      /* destination image dimensions */
   int  doX,doY;    /* destination image offsets */
   int  X,Y;        /* number of bytes per row / number of rows to move */
   {
   int i,j;
   byte *s;
   word *d;

   for (j=0; j<Y; ++j)
      {
      s = S + ((soY+j) * sX) + soX;
      d = D + ((doY+j) * dX) + doX;
      for (i=0; i<X; ++i ) 
         *(d++) = (word)*(s++);
      }
   return;
   }
       


/* copy a byte image to a byte image */

void MoveByte2Byte( S, sX, sY, soX, soY, D, dX, dY, doX, doY, X, Y )
   byte *S;         /* source image */
   int  sX,sY;      /* source image dimensions */
   int  soX,soY;    /* source image offsets */
   byte *D;         /* destination image */
   int  dX,dY;      /* destination image dimensions */
   int  doX,doY;    /* destination image offsets */
   int  X,Y;        /* number of bytes per row / number of rows to move */
   {
   int i,j;
   byte *s;
   byte *d;

   for (j=0; j<Y; ++j)
      {
      s = S + ((soY+j) * sX) + soX;
      d = D + ((doY+j) * dX) + doX;
      for (i=0; i<X; ++i ) 
         *(d++) = *(s++);
      }
   return;
   }
       
/* copy a word image to a word image */

void MoveWord2Word( S, sX, sY, soX, soY, D, dX, dY, doX, doY, X, Y )
   word *S;         /* source image */
   int  sX,sY;      /* source image dimensions */
   int  soX,soY;    /* source image offsets */
   word *D;         /* destination image */
   int  dX,dY;      /* destination image dimensions */
   int  doX,doY;    /* destination image offsets */
   int  X,Y;        /* number of bytes per row / number of rows to move */
   {
   int i,j;
   word *s;
   word *d;

   for (j=0; j<Y; ++j)
      {
      s = S + ((soY+j) * sX) + soX;
      d = D + ((doY+j) * dX) + doX;
      for (i=0; i<X; ++i ) 
         *(d++) = *(s++);
      }
   return;
   }
       

/* copy a word image to a byte image and scale or clip */

void ScaleWord2Byte( S, sX, sY, soX, soY, D, dX, dY, doX, doY, X, Y, 
                     noscale, dsp )

   word *S;         /* source image */
   int  sX,sY;      /* source image dimensions */
   int  soX,soY;    /* source image offsets */
   byte *D;         /* destination image */
   int  dX,dY;      /* destination image dimensions */
   int  doX,doY;    /* destination image offsets */
   int  X,Y;        /* number of bytes per row / number of rows to move */
   int noscale;     /* T => clip output */
   int dsp;         /* if nonzero, display extreme values */
   {
   int i,j;
   word *s;
   byte *d;
   int min =  32767;
   int max = -32768;
   int dif;
   int r;

   /* find extrema not including offset area */
   for (j=0; j<Y; ++j)
      {
      s = S + ((soY+j) * sX) + soX;
      for (i=0; i<X; ++i ) 
         {
         r = (int)*(s++);
         min = (int)MIN(min,r);
         max = (int)MAX(max,r);  
         }
      }

   dif = abs(max-min);
   if ( dsp )
      fprintf(stderr, "Extrema of result:  min - %d  max - %d  dif - %d\n",
         min,max,dif);

   if ( noscale )
      {
      for (j=0; j<Y; ++j)
         {
         s = S + ((soY+j) * sX) + soX;
         d = D + ((doY+j) * dX) + doX;
         for ( i=0; i<X; ++i )
            {
            r = (int)*(s++);
            *(d++) = (byte)( r > WHITE ? WHITE : ( r < BLACK ? BLACK : r ) );
            }
         }
      }
   else  /* scale */
      {
      /* if the range of values is less than a byte, simply translate */
      if ( dif < WHITE+1 )
         {
         for (j=0; j<Y; ++j)
            {
            s = S + ((soY+j) * sX) + soX;
            d = D + ((doY+j) * dX) + doX;
            for (i=0; i<X; ++i ) 
               *(d++) = (byte)(((int)*(s++)) - min);
            }
          }
      else  /* otherwise translate and scale */
         for (j=0; j<Y; ++j)
            {
            s = S + ((soY+j) * sX) + soX;
            d = D + ((doY+j) * dX) + doX;
            for (i=0; i<X; ++i ) 
               *(d++) = (byte)(((((int)(*(s++))) - min) * WHITE) / dif);
            }
      }

  return;
  }


/* zero the border of a byte image */

void ZeroPadByte( I, X, Y, bX, bY )  /* v2.0 */
   byte *I;
   int X,Y;
   int bX,bY;
   {
   int i,j;
   int xul = bX;      /* v2.0 */
   int yul = bY;      /* v2.0 */
   int xlr = X - bX;  /* v2.0 */
   int ylr = Y - bY;  /* v2.0 */

   for ( j=0; j<yul; ++j )    
      for ( i=0; i<X; ++i ) 
         *(I + j*X + i) = 0;
   for ( j=ylr; j<Y; ++j ) 
      for ( i=0; i<X; ++i ) 
         *(I + j*X + i) = 0;
   for ( i=0; i<xul; ++i )    
      for ( j=yul; j<ylr; ++j ) 
         *(I + j*X + i) = 0;
   for ( i=xlr; i<X; ++i )    
      for ( j=yul; j<ylr; ++j ) 
         *(I + j*X + i) = 0; 
   return;
   }



/* zero the border of a word image */

void ZeroPadWord( I, X, Y, bX, bY )  /* v2.0 */
   word *I;
   int X,Y;
   int bX,bY;
   {
   int i,j;
   int xul = bX;      /* v2.0 */
   int yul = bY;      /* v2.0 */
   int xlr = X - bX;  /* v2.0 */
   int ylr = Y - bY;  /* v2.0 */

   for ( j=0; j<yul; ++j )    
      for ( i=0; i<X; ++i ) 
         *(I + j*X + i) = 0;
   for ( j=ylr; j<Y; ++j ) 
      for ( i=0; i<X; ++i ) 
         *(I + j*X + i) = 0;
   for ( i=0; i<xul; ++i )    
      for ( j=yul; j<ylr; ++j ) 
         *(I + j*X + i) = 0;
   for ( i=xlr; i<X; ++i )    
      for ( j=yul; j<ylr; ++j ) 
         *(I + j*X + i) = 0; 
   return;
   }




void SubtractImg( A, B, C, x, y )
   word *A,*B,*C;
   int x,y;
   {
   word *a=A;
   word *b=B;
   word *c=C;
   int n=x*y;
   int i;

   for ( i=0; i<n; ++i ) *(a++) = *(b++) - *(c++);

   return;
   }




void SubtrByteImg( A, B, C, x, y )
   byte *A,*B,*C;
   int x,y;
   {
   byte *a=A;
   byte *b=B;
   byte *c=C;
   int n=x*y;
   int i;

   for ( i=0; i<n; ++i ) *(a++) = *(b++) - *(c++);

   return;
   }
