
/* mrank.c  --  rank filter (order-statistic) operators for morph */

/* contains: GetSupport(), GrayRankFilt(), BinRankFilt(),   */
/*           LUMFilt(), LUMSmooth(), LUMSharp()             */

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


/* we do not define a hit-or-miss rank filter. Therefore, we change the */
/* zeros in a binary stelem into -1's (DON'T CARES). Then all the rank  */
/* routines can consider the support to be all pixels >= ZERO */

int GetSupport( s, sX, sY, SEisBIN )
   int *s;
   int sX, sY;
   int SEisBIN;
   {
   int i;
   int sup = 0;

   if ( SEisBIN )  for ( i=0; i<sX*sY; ++i ) s[i] = s[i] ? s[i] : -1 ;

   for ( i=0; i<sX*sY; ++i ) if ( s[i] >= 0 ) ++sup;

   return( sup );
   }


/* note: If you know that you are going to use only rectangular nbhds, */
/* there are faster ways to do a rank filter.  This implementation is, */
/* however, completely general to simply and multiply connected nbhds  */
/* of any shape. */


void GrayRankFilt( In, Out, X, tX, tY, stelem, sX, sY, sorgx, sorgy, rank ) 
   byte *In, *Out;   /* input and output images */
   int X;            /* image horizontal dimension */
   int tX,tY;        /* transform scan horiz., vert. dimensions */
   int *stelem;      /* SE in row-major order */
   int sX,sY;        /* SE horizontal, vertical dimensions */
   int sorgx,sorgy;  /* SE origin offset from upper LH corner. */
   int rank;         /* rank */
   {
   byte *I,*O;          /* image pointers */
   int histo[WHITE+1];  /* histogram */
   int *h;              /* pointer to histogram; */
   int *s;              /* pointer to SE  */
   int i,j;             /* transform indices */
   int x,y;             /* image indices */
   int a;               /* histogram area */
   int N=(WHITE+1)*sizeof(int); /* histo size in bytes */
   
   for ( y=0; y<tY; ++y )      /* image row scan */ 
      {
      I = In + y*X;
      O = Out + (y + sorgy)*X + sorgx;
      for ( x=0; x<tX; ++x )   /* image column scan */ 
         {
         s = stelem; 
         h = histo;
         /* zero the histogram */
         memset( histo, '\000', N );

         /* make a histogram of the neighborhood */
         for ( j=0; j<sY; ++j )     /* SE row scan */
            for ( i=0; i<sX; ++i )  /* SE column scan */
               if ( *(s++) >= ZERO )
                  histo[*(I + j*X + i)]++;

         /* scan backwards from WHITE end of histogram */
         j = WHITE+1;
         h = histo+j;
         a = ZERO;
         do a += *(--h);
         while ( (--j > BLACK) && (a < rank) );
         *(O++) = j;
         ++I;
         }
      }
   return;
   }  




void BinRankFilt( In, Out, X, tX, tY, stelem, sX, sY, sorgx, sorgy, rank ) 
   byte *In, *Out;   /* input and output images */
   int X;            /* image horizontal dimension */
   int tX,tY;        /* transform scan horiz., vert. dimensions */
   int *stelem;      /* SE in row-major order */
   int sX,sY;        /* SE horizontal, vertical dimensions */
   int sorgx,sorgy;  /* SE origin offset from upper LH corner. */
   int rank;         /* rank */
   {
   byte *I,*O;  /* image pointers */
   int accum;   /* accumulator */
   int i,j;     /* transform indices */
   int x,y;     /* image indices */
   int *s;      /* pointer to SE  */
   
   for ( y=0; y<tY; ++y )      /* image row scan */  
      {
      I = In + y*X;
      O = Out + (y + sorgy)*X + sorgx;
      for ( x=0; x<tX; ++x )   /* image column scan */
         {
         s = stelem; 
         accum = 0;
         for ( j=0; j<sY; ++j )     /* SE row scan */
            {
            for ( i=0; i<sX; ++i )  /* SE column scan */
               {
               if ( (*(s++) >= ZERO)  &&  *(I + j*X + i) ) 
                  {
                  accum++;
                  if (accum >= rank) break;
                  }
               }
            if (accum >= rank) break;
            }
         *(O++) = (accum >= rank) ? WHITE : BLACK ;
         ++I;
         }
      }
   return;
   }  



/* LUMfilt is the lower-upper-middle filter as defined by            */
/* Hardie, R. C., and C. G. Boncelet, "LUM filters: A class of       */
/* rank-order-based filters for smoothing and sharpening,"           */
/* IEEE Trans. Signal Processing, vol. SP-41, No. 3, March 1993.     */
/* LUMSMO is the LUM smoothing filter defined therein, and LUMSHA    */
/* is the LUM sharpening filter.                                     */
/* These filters compare the center pixel in a neighborhood defined  */
/* by an SE to upper and lower order statistics in the neighborhood. */
/* Depending on the ordering either the center pixel or one of       */
/* the order stats is output.                                        */
/* Note that we define order statistics opposite Hardie and Boncelet */
/* Whereas OS(1) is the minimum for them OS(1) is the maximum in     */ 
/* these routines.  This has no effect, as far as the user is        */
/* concerned, on the effects of parameters k and l.                  */

void LUMfilt( In, Out, X, tX, tY, stelem, sX, sY, sorgx, sorgy, param ) 
   byte *In, *Out;   /* input and output images */
   int X;            /* image horizontal dimension */
   int tX,tY;        /* transform scan horiz., vert. dimensions */
   int *stelem;      /* SE in row-major order */
   int sX,sY;        /* SE horizontal, vertical dimensions */
   int sorgx,sorgy;  /* SE origin offset from upper LH corner. */
   int *param;       /* parameter list */
   {
   byte *I,*O,*p;       /* image pointers */
   int histo[WHITE+1];  /* histogram */
   int *h;              /* pointer to histogram; */
   int *s;              /* pointer to SE  */
   int i,j;             /* transform indices */
   int x,y;             /* image indices */
   int a;               /* histogram area */
   int N=(WHITE+1)*sizeof(int); /* histo size in bytes */
   int k,K,l,L,n;       /* LUM parameters */
   int xk,xl,xK,xL,tl;  /* LUM parameters */

   k = param[0];
   l = param[1];
   n = param[2];
   K = n - k + 1;
   L = n - l + 1;
   
   for ( y=0; y<tY; ++y )      /* image row scan */ 
      {
      I = In  + y*X;
      p = In  + (y + sorgy)*X + sorgx; /* center pixel in input nbhd */
      O = Out + (y + sorgy)*X + sorgx; /* output pixel */
      for ( x=0; x<tX; ++x )   /* image column scan */ 
         {
         s = stelem; 
         h = histo;
         /* zero the histogram */
         memset( histo, '\000', N );

         /* make a histogram of the neighborhood */
         for ( j=0; j<sY; ++j )     /* SE row scan */
            for ( i=0; i<sX; ++i )  /* SE column scan */
               if ( *(s++) >= ZERO )
                  histo[*(I + j*X + i)]++;

         j = WHITE+1;
         h = histo+j;
         a = ZERO;
         while ( (j > BLACK) && (a < k) )
            {
            a += *(--h);
            --j;
            }
         xk = j;
         if ( k == l )
            {
            xl = j;
            }
         else
            {
            while ( (j > BLACK) && (a < l) )
               {
               a += *(--h);
               --j;
               }
            xl = j;
            }
         while ( (j > BLACK) && (a < L) )
            {
            a += *(--h);
            --j;
            }
         xL = j;
         if ( K == L )
            {
            xK = j;
            }
         else
            {
            while ( (j > BLACK) && (a < K) )
               {
               a += *(--h);
               --j;
               }
            xK = j;
            }

         tl = (xl + xL) / 2;

         if ( *p > xk )
            *(O++) = xk;
         else if ( *p > xl )
            *(O++) = *p;
         else if ( *p > tl )
            *(O++) = xl;
         else if ( *p > xL )
            *(O++) = xL;
         else if ( *p > xK )
            *(O++) = *p;
         else
            *(O++) = xK;

         ++p;
         ++I;
         }
      }
   return;
   }  



void LUMsmooth( In, Out, X, tX, tY, stelem, sX, sY, sorgx, sorgy, param ) 
   byte *In, *Out;   /* input and output images */
   int X;            /* image horizontal dimension */
   int tX,tY;        /* transform scan horiz., vert. dimensions */
   int *stelem;      /* SE in row-major order */
   int sX,sY;        /* SE horizontal, vertical dimensions */
   int sorgx,sorgy;  /* SE origin offset from upper LH corner. */
   int *param;       /* parameter list */
   {
   byte *I,*O,*p;       /* image pointers */
   int histo[WHITE+1];  /* histogram */
   int *h;              /* pointer to histogram; */
   int *s;              /* pointer to SE  */
   int i,j;             /* transform indices */
   int x,y;             /* image indices */
   int a;               /* histogram area */
   int N=(WHITE+1)*sizeof(int); /* histo size in bytes */
   int k,K,n;  /* LUM parameters */
   int xk,xK;  /* LUM parameters */

   k = param[0];  /* upper smoothing parameter, 1 <= k <= (n+1)/2 */
   n = param[2];  /* support of filter */
   K = n - k + 1; /* lower smoothing parameter */
   
   for ( y=0; y<tY; ++y )      /* image row scan */ 
      {
      I = In  + y*X;
      p = In  + (y + sorgy)*X + sorgx; /* center pixel in input nbhd */
      O = Out + (y + sorgy)*X + sorgx; /* output pixel */
      for ( x=0; x<tX; ++x )   /* image column scan */ 
         {
         s = stelem; 
         h = histo;
         /* zero the histogram */
         memset( histo, '\000', N );

         /* make a histogram of the neighborhood */
         for ( j=0; j<sY; ++j )     /* SE row scan */
            for ( i=0; i<sX; ++i )  /* SE column scan */
               if ( *(s++) >= ZERO )
                  histo[*(I + j*X + i)]++;

         j = WHITE+1;
         h = histo+j;
         a = ZERO;
         while ( (j > BLACK) && (a < k) )
            {
            a += *(--h);
            --j;
            }
         xk = j;
         while ( (j > BLACK) && (a < K) )
            {
            a += *(--h);
            --j;
            }
         xK = j;

         if ( *p > xk )
            *(O++) = xk;
         else if ( *p > xK )
            *(O++) = *p;
         else
            *(O++) = xK;

         ++p;
         ++I;
         }
      }
   return;
   }  




void LUMsharp( In, Out, X, tX, tY, stelem, sX, sY, sorgx, sorgy, param ) 
   byte *In, *Out;   /* input and output images */
   int X;            /* image horizontal dimension */
   int tX,tY;        /* transform scan horiz., vert. dimensions */
   int *stelem;      /* SE in row-major order */
   int sX,sY;        /* SE horizontal, vertical dimensions */
   int sorgx,sorgy;  /* SE origin offset from upper LH corner. */
   int *param;       /* parameter list */
   {
   byte *I,*O,*p;       /* image pointers */
   int histo[WHITE+1];  /* histogram */
   int *h;              /* pointer to histogram; */
   int *s;              /* pointer to SE  */
   int i,j;             /* transform indices */
   int x,y;             /* image indices */
   int a;               /* histogram area */
   int N=(WHITE+1)*sizeof(int); /* histo size in bytes */
   int l,L,n;     /* LUM parameters */
   int xl,xL,tl;  /* LUM parameters */

   l = param[1];
   n = param[2];
   L = n - l + 1;

   if ( l == L )
      {
      memcpy(Out,In,X*(tY+sY-1));
      return;
      }
   
   for ( y=0; y<tY; ++y )      /* image row scan */ 
      {
      I = In  + y*X;
      p = In  + (y + sorgy)*X + sorgx; /* center pixel in input nbhd */
      O = Out + (y + sorgy)*X + sorgx; /* output pixel */
      for ( x=0; x<tX; ++x )   /* image column scan */ 
         {
         s = stelem; 
         h = histo;
         /* zero the histogram */
         memset( histo, '\000', N );

         /* make a histogram of the neighborhood */
         for ( j=0; j<sY; ++j )     /* SE row scan */
            for ( i=0; i<sX; ++i )  /* SE column scan */
               if ( *(s++) >= ZERO )
                  histo[*(I + j*X + i)]++;

         j = WHITE+1;
         h = histo+j;
         a = ZERO;
         while ( (j > BLACK) && (a < l) )
            {
            a += *(--h);
            --j;
            }
         xl = j;
         while ( (j > BLACK) && (a < L) )
            {
            a += *(--h);
            --j;
            }
         xL = j;

         tl = (xl + xL) / 2;

         if ( *p > xl )
            *(O++) = *p;
         else if ( *p > tl )
            *(O++) = xl;
         else if ( *p > xL )
            *(O++) = xL;
         else
            *(O++) = *p;

         ++p;
         ++I;
         }
      }
   return;
   }  




