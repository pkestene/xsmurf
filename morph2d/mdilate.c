/* mdilate.c  --  morph dilation operators */

/* contains: BinBinDilate(), BinGrayDilate(), GrayBinDilate(),  */
/*           GrayGraySetDilate(), GrayGrayFctDilate()           */

/* dependencies: none */

/* morph version 4.0    10 May 1993                */
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

/* See changes.doc for an explanation of the changes in v4.0 */

#include "morph_sub.h"

/* Dilation via hit-or-miss transform of a binary image with a binary SE.   */
/* Binary is interpreted as zero and positive (not zero). Images are        */
/* unsigned. A negative SE element is treated as a DON'T CARE. Positive,    */
/* non-zero SE elements are hit-points and zero SE elements are miss-points */

/* note: at hit occurs when the SE element is >ZERO and the corresponding   */
/* pixel is >ZERO. a miss occurs when the SE element is ZERO and the        */
/* corresponding pixel is ZERO. Most SE's will contain only 1's and -1's    */
/* (don't cares)                                                            */

/* New in v3.0: when the "not" flag is true, the pixel under the SE origin  */
/* in the input image is passed to the output on a TRUE condition and BLACK */
/* is passed on a FALSE condition. If I is the original (binary) image and  */
/* D is the dilated image, then the "not" flag causes (NOT D) OR I          */
/* pixelwise to be output.   This option when used with a hit-or-miss       */
/* transform can delete black features with a specific shape from an image. */


void BinBinDilate( In, Out, X, tX, tY, stelem, sX, sY, sorgx, sorgy, not ) 
   byte *In, *Out;   /* input and output images */
   int X;            /* image horizontal dimension */
   int tX,tY;        /* transform scan horiz., vert. dimensions */
   int *stelem;      /* SE in row-major order */
   int sX,sY;        /* SE horizontal, vertical dimensions */
   int sorgx,sorgy;  /* SE origin offset from upper LH corner. */
   int not;          /* T => pass pixel on dilation true, black on false */
   {
   byte *I,*O;    /* image pointers */
   int i,j;       /* transform indices */
   int x,y;       /* image indices */    
   int p;         /* pixel value */
   int op;        /* pixel value at SE origin */
   int r;         /* transform result */
   int *s;        /* pointer to SE  */

   for ( y=0; y<tY; ++y )      /* image row scan */ 
      {
      I = In + y*X;
      O = Out + (y+sY-sorgy-1)*X + sX-sorgx-1;  
      for ( x=0; x<tX; ++x )   /* image column scan */
         {
         r = ZERO;
         s = stelem + sX*sY -1;     
         op = *(I + (sY-sorgy-1)*X + sX-sorgx-1); /* pixel at SE origin */ 
         for ( j=0; j<sY; ++j )     /* SE row scan */
            {
            for ( i=0; i<sX; ++i )  /* SE column scan */
               {
               if (*s >= ZERO)  
                  {
                  p = *(I + j*X + i); /* get pixel   */
                  r = *s ? p : !p ;   /* hit-or-miss */
                  if (r)  /* TRUE--for dilation, succeed once and we're done */
                     {
                     *(O++) = not ? op : WHITE;
                     break; /* out of SE column scan */
                     }
                  }
               --s;
               }                    /* end of SE column scan */
            if (r) break; /* out of SE row scan */
            }
         ++I;
         if (!r) /* FALSE --every hit missed and every miss hit */
            *(O++) = not ? WHITE : BLACK;  
         }                          /* end of image column scan */
      }                             /* end of image row scan */
   return;
  }                                 /* end of BinBinDilate */




/* Morphological dilation of a binary image with a gray-level SE.           */
/* Binary is interpreted as zero and positive (not zero). Images are        */
/* unsigned. A negative SE element is treated as a DON'T CARE. The dilation */
/* is performed over the support of nonnegative SE elements.                */

/* New in v3.0: when the "not" flag is true, the pixel under the SE origin  */
/* in the input image is passed to the output on a TRUE condition and BLACK */
/* is passed on a FALSE condition. If I is the original (binary) image and  */
/* D is the dilated image, then the "not" flag causes (NOT D) OR I          */
/* pixelwise to be output. When used with a convex, symmetric,              */
/* non-hit-or-miss SE, this causes an interior delete of black regions.     */


void BinGrayDilate( In, Out, X, tX, tY, stelem, sX, sY, sorgx, sorgy, not ) 
   byte *In, *Out;   /* input and output images */
   int X;            /* image horizontal dimension */
   int tX,tY;        /* transform scan horiz., vert. dimensions */
   int *stelem;      /* SE in row-major order */
   int sX,sY;        /* SE horizontal, vertical dimensions */
   int sorgx,sorgy;  /* SE origin offset from upper LH corner. */
   int not;          /* T => pass pixel complement on dilation true */

   {
   byte *I,*O;    /* image pointers */
   int i,j;       /* transform indices */
   int x,y;       /* image indices */
   int op;        /* pixel value at SE origin */
   int r;         /* transform result */
   int *s;        /* pointer to SE  */

   for ( y=0; y<tY; ++y )      /* image row scan */  
      {
      I = In + y*X;
      O = Out + (y+sY-sorgy-1)*X + sX-sorgx-1;   
      for ( x=0; x<tX; ++x )   /* image column scan */  
         {
         r = ZERO;
         s = stelem + sX*sY -1;     
         op = *(I + (sY-sorgy-1)*X + sX-sorgx-1); /* pixel at SE origin */ 
         for ( j=0; j<sY; ++j )     /* SE row scan */
            {
            for ( i=0; i<sX; ++i )  /* SE column scan */
               {
               if (*s >= ZERO)  
                  {
                  r = *(I + j*X + i);
                  if (r)  /* TRUE--for dilation, succeed once and we're done */
                     {
                     *(O++) = not ? op : WHITE;
                     break; /* out of SE column scan */
                     }
                  }
               --s;
               }                    /* end of SE column scan */
            if (r) break; /* out of SE row scan */
            }   /* end of SE row scan */
         ++I;
         if (!r) /* FALSE --every hit missed and every miss hit */
            *(O++) = not ? WHITE : BLACK;  
         }                          /* end of image column scan */
      }                             /* end of image row scan */
   return;
  }                                 /* end of BinGrayDilate */




/* Morphological (function-set) dilation of a gray-level image with a binary */
/* SE. Binary is interpreted as zero and positive (not zero). Images are     */
/* unsigned. A negative SE element is treated as a DON'T CARE. The erosion   */
/* is perfomed over the positive (>ZERO) support of of the SE.               */

void GrayBinDilate( In, Out, X, tX, tY, stelem, sX, sY, sorgx, sorgy, unused ) 
   byte *In, *Out;   /* input and output images */
   int X;            /* image horizontal dimension */
   int tX,tY;        /* transform scan horiz., vert. dimensions */
   int *stelem;      /* SE in row-major order */
   int sX,sY;        /* SE horizontal, vertical dimensions */
   int sorgx,sorgy;  /* SE origin offset from upper LH corner. */
   int unused;       /* to conform with other routines' param list */
   {
   byte *I,*O;    /* image pointers */
   int i,j;       /* transform indices */
   int x,y;       /* image indices */
   int p;         /* pixel value */
   int r;         /* transform result */
   int *s;        /* pointer to SE  */
   
   for ( y=0; y<tY; ++y )      /* image row scan */  
      {
      I = In + y*X;
      O = Out + (y+sY-sorgy-1)*X + sX-sorgx-1; 
      for ( x=0; x<tX; ++x )   /* image column scan */  
         {
         s = stelem + sX*sY -1;     
         r = BLACK;
         for ( j=0; j<sY; ++j )     /* SE row scan */
            {
            for ( i=0; i<sX; ++i )  /* SE column scan */
               {
               if (*s > ZERO)  
                  {
                  p = *(I + j*X + i);  /* get pixel   */
                  r = MAX(r,p);
                  if ( r == WHITE ) break; /* out of col. scan */
                  }
               --s;
               }                    /* end of SE column scan */ 
            if ( r == WHITE ) break;    /* out of SE row scan */
            }                       /* end of SE row scan */
         ++I;
         *(O++) = r; /* output result */
         }                          /* end of image column scan */
      }                             /* end of image row scan */
  return;
  }                                 /* end of GrayBinDilate */



/* Morphological function-set dilation of a gray-level image with a      */
/* gray-level SE. Images are unsigned. A negative SE element is treated  */
/* as a DON'T CARE. The dilation is perfomed as a function-set operation */
/* over the nonnegative (>= ZERO) support of of the SE. */

void GrayGraySetDilate( In, Out, X, tX, tY, stelem, sX, sY, sorgx, sorgy, 
                        unused ) 
   byte *In, *Out;   /* input and output images */
   int X;            /* image horizontal dimension */
   int tX,tY;        /* transform scan horiz., vert. dimensions */
   int *stelem;      /* SE in row-major order */
   int sX,sY;        /* SE horizontal, vertical dimensions */
   int sorgx,sorgy;  /* SE origin offset from upper LH corner. */
   int unused;       /* to conform with other routines' param list */
   {
   byte *I,*O;    /* image pointers */
   int i,j;       /* transform indices */
   int x,y;       /* image indices */
   int p;         /* pixel value */
   int r;         /* transform result */
   int *s;        /* pointer to SE  */
   
   for ( y=0; y<tY; ++y )      /* image row scan */  
      {
      I = In + y*X;
      O = Out + (y+sY-sorgy-1)*X + sX-sorgx-1;   
      for ( x=0; x<tX; ++x )   /* image column scan */  
         {
         s = stelem + sX*sY -1;     
         r = BLACK;
         for ( j=0; j<sY; ++j )     /* SE row scan */
            {
            for ( i=0; i<sX; ++i )  /* SE column scan */
               {
               if (*s >= ZERO)  
                  {
                  p = *(I + j*X + i);      /* get pixel   */
                  r = MAX(r,p);
                  if ( r == WHITE ) break; /* out of col. scan */
                  }
               --s;
               }                    /* end of SE column scan */ 
            if ( r == WHITE ) break;    /* out of SE row scan */
            }                       /* end of SE row scan */
         ++I;
         *(O++) = r; /* output result */
         }                          /* end of image column scan */
      }                             /* end of image row scan */
  return;
  }                                 /* end of GrayGraySetDilate */

/* Morphological function-set dilation of a gray-level image with a      */
/* gray-level SE. Images are unsigned. A negative SE element is treated  */
/* as a DON'T CARE. The dilation is perfomed as a function-set operation */
/* over the nonnegative (>= ZERO) support of of the SE. */

void GrayGraySetDilate2( In, Out, X, tX, tY, stelem, sX, sY, sorgx, sorgy, 
                        unused ) 
   word *In, *Out;   /* input and output images */
   int X;            /* image horizontal dimension */
   int tX,tY;        /* transform scan horiz., vert. dimensions */
   int *stelem;      /* SE in row-major order */
   int sX,sY;        /* SE horizontal, vertical dimensions */
   int sorgx,sorgy;  /* SE origin offset from upper LH corner. */
   int unused;       /* to conform with other routines' param list */
   {
   word *I,*O;    /* image pointers */
   int i,j;       /* transform indices */
   int x,y;       /* image indices */
   int p;         /* pixel value */
   int r;         /* transform result */
   int *s;        /* pointer to SE  */
   
   for ( y=0; y<tY; ++y )      /* image row scan */  
      {
      I = In + y*X;
      O = Out + (y+sY-sorgy-1)*X + sX-sorgx-1;   
      for ( x=0; x<tX; ++x )   /* image column scan */  
         {
         s = stelem + sX*sY -1;     
         r = BLACK2;
         for ( j=0; j<sY; ++j )     /* SE row scan */
            {
            for ( i=0; i<sX; ++i )  /* SE column scan */
               {
               if (*s >= ZERO)  
                  {
                  p = *(I + j*X + i);      /* get pixel   */
                  r = MAX(r,p);
                  if ( r == WHITE2 ) break; /* out of col. scan */
                  }
               --s;
               }                    /* end of SE column scan */ 
            if ( r == WHITE2 ) break;    /* out of SE row scan */
            }                       /* end of SE row scan */
         ++I;
         *(O++) = r; /* output result */
         }                          /* end of image column scan */
      }                             /* end of image row scan */
  return;
  }                                 /* end of GrayGraySetDilate2 */




/* Morphological function dilation of a gray-level image with a          */
/* gray-level SE. Images are unsigned. A negative SE element is treated  */
/* as a DON'T CARE. The dilation is perfomed as a function operation     */
/* over the nonnegative (>= ZERO) support of of the SE. */

void GrayGrayFctDilate( In, Out, X, tX, tY, stelem, sX, sY, sorgx, sorgy, 
                        unused ) 
   word *In, *Out;   /* input and output images */
   int X;            /* image horizontal dimension */
   int tX,tY;        /* transform scan horiz., vert. dimensions */
   int *stelem;      /* SE in row-major order */
   int sX,sY;        /* SE horizontal, vertical dimensions */
   int sorgx,sorgy;  /* SE origin offset from upper LH corner. */
   int unused;       /* to conform with other routines' param list */
   {
   word *I,*O;    /* image pointers */
   int i,j;       /* transform indices */
   int x,y;       /* image indices */
   int t,r;       /* transform result */
   int *s;        /* pointer to SE  */
   
   for ( y=0; y<tY; ++y )      /* image row scan */ 
      {
      I = In + y*X;
      O = Out + (y+sY-sorgy-1)*X + sX-sorgx-1;  
      for ( x=0; x<tX; ++x )   /* image column scan */
         {
         s = stelem + sX*sY -1;     
         r = BLACK;
         for ( j=0; j<sY; ++j )     /* SE row scan */
            {
            for ( i=0; i<sX; ++i )  /* SE column scan */
               {
               if (*s >= ZERO)  
                  {
                  t = *(I + j*X + i) + *s; /* add pixel to SE value */
                  r = MAX(t,r);
                  }
               --s;
               }                    /* end of SE column scan */ 
            }                       /* end of SE row scan */
         ++I;
         *(O++) = r; /* output result */
         }                          /* end of image column scan */
      }                             /* end of image row scan */
  return;
  }                                 /* end of GrayGrayFctDilate */




