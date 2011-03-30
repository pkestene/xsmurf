/* merode.c  --  morph erosion operators */

/* contains: BinBinErode(), BinGrayErode(), GrayBinErode(),  */
/*           GrayGraySetErode(), GrayGrayFctErode()          */

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

/* See changes.doc for an explanation of the changes in v4.0. */

#include "morph_sub.h"


/* Erosion via hit-or-miss transform of a binary image with a binary SE.     */
/* Binary is interpreted as zero and positive (not zero). Images are         */
/* unsigned. A negative SE element is treated as a DON'T CARE. Positive,     */
/* non-zero SE elements are hit-points and zero SE elements are miss-points  */

/* note: at hit occurs when the SE element is >0 and the corresponding pixel */
/* is >0. a miss occurs when the SE element is 0 and the corresponding pixel */
/* is 0. Most SE's will contain only 1's and -1's (don't cares)              */

/* New in v3.0: when the "not" flag is true, the pixel under the SE origin   */
/* in input image is passed to the output on a FALSE condition and BLACK is  */
/* passed on a TRUE condition. If I is the original (binary) image and E is  */
/* the eroded image, then the "not" flag causes I AND (NOT E) pixelwise to   */
/* be output.  This option when used with a hit-or-miss transform can delete */ 
/* features with a specific shape from an image. */


void BinBinErode( In, Out, X, tX, tY, stelem, sX, sY, sorgx, sorgy, not ) 
   byte *In, *Out;   /* input and output images */
   int X;            /* image horizontal dimension */
   int tX,tY;        /* transform scan horiz., vert. dimensions */
   int *stelem;      /* SE in row-major order */
   int sX,sY;        /* SE horizontal, vertical dimensions */
   int sorgx,sorgy;  /* SE origin offset from upper LH corner. */
   int not;          /* T => pass pixel through unless erosion is true */
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
      O = Out + (y + sorgy)*X + sorgx;
      for ( x=0; x<tX; ++x )   /* image column scan */  
         {
         r = 1;
         s = stelem;   
         op = *(I + sorgy*X + sorgx); /* pixel at SE origin */
         for ( j=0; j<sY; ++j )     /* SE row scan */
            {
            for ( i=0; i<sX; ++i )  /* SE column scan */
               {
               if (*s >= ZERO)  
                  {
                  p = *(I + j*X + i); /* get pixel   */
                  r = *s ? p : !p ;   /* hit-or-miss */
                  if (!r)  /* FALSE -- for erosion, fail once and we're done */
                     {
                     *(O++) = not ? op : BLACK;
                     break; /* out of SE column scan */
                     }
                  }
               ++s;
               }                    /* end of SE column scan */ 
            if (!r) break; /* out of SE row scan */
            }                       /* end of SE row scan */
         ++I;
         if (r) /* TRUE -- every hit hit and every miss missed */
             *(O++) = not ? BLACK : WHITE; 
         }                          /* end of image column scan */
      }                             /* end of image row scan */
  return;
  }                                 /* end of BinBinErode */





/* Morphological erosion of a binary image with a gray-level SE.            */
/* Binary is interpreted as zero and positive (not zero). Images are        */
/* unsigned. A negative SE element is treated as a DON'T CARE. The erosion  */
/* is performed over the support of nonnegative SE elements.                */

/* New in v3.0: when the "not" flag is true, the pixel under the SE origin  */
/* in input image is passed to the output on a FALSE condition and BLACK is */
/* passed on a TRUE condition. If I is the original (binary) image and E is */
/* the eroded image, then the "not" flag causes I AND (NOT E) pixelwise to  */
/* be output.  When used with a convex, symmetric, non-hit-or-miss SE,      */ 
/* this causes an interior delete of white regions */

void BinGrayErode( In, Out, X, tX, tY, stelem, sX, sY, sorgx, sorgy, not ) 
   byte *In, *Out;   /* input and output images */
   int X;            /* image horizontal dimension */
   int tX,tY;        /* transform scan horiz., vert. dimensions */
   int *stelem;      /* SE in row-major order */
   int sX,sY;        /* SE horizontal, vertical dimensions */
   int sorgx,sorgy;  /* SE origin offset from upper LH corner. */
   int not;          /* T => pass pixel through unless erosion is true */
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
      O = Out + (y + sorgy)*X + sorgx;
      for ( x=0; x<tX; ++x )   /* image column scan */  
         {
         r = 1;
         s = stelem;     
         op = *(I + sorgy*X + sorgx); /* pixel at SE origin */ 
         for ( j=0; j<sY; ++j )     /* SE row scan */
            {
            for ( i=0; i<sX; ++i )  /* SE column scan */
               {
               if (*s >= ZERO)  
                  {
                  r = *(I + j*X + i);  
                  if (!r)  /* FALSE -- for erosion, fail once and we're done */
                     {
                     *(O++) = not ? op : BLACK;
                     break; /* out of SE column scan */
                     }
                  }
               ++s;
               }                    /* end of SE column scan */ 
            if (!r) break; /* out of SE row scan */
            }                       /* end of SE row scan */
         ++I;
         if (r) /* TRUE -- every hit hit and every miss missed */
             *(O++) = not ? BLACK : WHITE; 
         }                          /* end of image column scan */
      }                             /* end of image row scan */
  return;
  }                                 /* end of GrayBinErode */




/* Morphological (function-set) erosion of a gray-level image with a binary */
/* SE. Binary is interpreted as zero and positive (not zero). Images are    */
/* unsigned. A negative SE element is treated as a DON'T CARE. The erosion  */
/* is perfomed over the positive (>ZERO) support of of the SE.              */

void GrayBinErode( In, Out, X, tX, tY, stelem, sX, sY, sorgx, sorgy, unused ) 
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
      O = Out + (y + sorgy)*X + sorgx;
      for ( x=0; x<tX; ++x )   /* image column scan */  
         {
         r = WHITE;
         s = stelem;     
         for ( j=0; j<sY; ++j )     /* SE row scan */
            {
            for ( i=0; i<sX; ++i )  /* SE column scan */
               {
               if (*s > ZERO)  
                  {
                  p = *(I + j*X + i);  /* get pixel   */
                  r = MIN(r,p);
                  if ( r == BLACK ) break; /* out of col. scan */
                  }
               ++s;
               }                    /* end of SE column scan */ 
            if ( r == BLACK ) break;    /* out of SE row scan */
            }                       /* end of SE row scan */
         ++I;
         *(O++) = r; /* output result */
         }                          /* end of image column scan */
      }                             /* end of image row scan */
  return;
  }                                 /* end of GrayBinErode */




/* Morphological function-set erosion of a gray-level image with a      */
/* gray-level SE. Images are unsigned. A negative SE element is treated */
/* as a DON'T CARE. The erosion is perfomed as a function-set operation */
/* over the nonnegative (>= ZERO) support of of the SE. */

void GrayGraySetErode( In, Out, X, tX, tY, stelem, sX, sY, sorgx, sorgy, 
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
      O = Out + (y + sorgy)*X + sorgx;
      for ( x=0; x<tX; ++x )   /* image column scan */
         {
         r = WHITE;
         s = stelem;     
         for ( j=0; j<sY; ++j )     /* SE row scan */
            {
            for ( i=0; i<sX; ++i )  /* SE column scan */
               {
               if (*s >= ZERO)  
                  {
                  p = *(I + j*X + i);  /* get pixel   */
                  r = MIN(r,p);
                  if ( r == BLACK ) break; /* out of col. scan */
                  }
               ++s;
               }                    /* end of SE column scan */ 
            if ( r == BLACK ) break;    /* out of SE row scan */
            }                       /* end of SE row scan */
         ++I;
         *(O++) = r; /* output result */
         }                          /* end of image column scan */
      }                             /* end of image row scan */
  return;
  }                                 /* end of GrayGraySetErode */




/* Morphological function erosion of a gray-level image with a          */
/* gray-level SE. Images are unsigned. A negative SE element is treated */
/* as a DON'T CARE. The erosion is perfomed as a function operation     */
/* over the nonnegative (>= ZERO) support of of the SE. */

void GrayGrayFctErode( In, Out, X, tX, tY, stelem, sX, sY, sorgx, sorgy, 
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
      O = Out + (y + sorgy)*X + sorgx;
      for ( x=0; x<tX; ++x )   /* image column scan */  
         {
         r = WHITE;
         s = stelem;     
         for ( j=0; j<sY; ++j )     /* SE row scan */
            {
            for ( i=0; i<sX; ++i )  /* SE column scan */
               {
               if (*s >= ZERO)  
                  {
                  t = *(I + j*X + i) - *s;
                  r = MIN(t,r);
                  }
               ++s;
               }                    /* end of SE column scan */ 
            }                       /* end of SE row scan */
         ++I;
         *(O++) = r; /* output result */
         }                          /* end of image column scan */
      }                             /* end of image row scan */
  return;
  }                                 /* end of GrayGrayFctErode */
