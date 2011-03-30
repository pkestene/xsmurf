/* mthresh.c  --  image threshold routine for morph */

/* contains: ThresholdImg() */

/* dependencies: none */

/* morph version 4.0  10 May 1993                  */
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


void ThresholdImg( Img, X, Y, low, high )
   byte *Img;
   int  X,Y;
   int  low;
   int  high;
   {
   byte *I = Img;
   int  x,y;

   for ( y=0; y<Y; ++y )
      for (x=0; x<X; ++x )
         {
         *I = ( *I >= low  &&  *I <= high )  ?  WHITE  :  BLACK ;
         ++I;
         }
   return;
   }
