/* mloadse.c  --  structuring element loaders for morph */

/* contains: LoadSE, MakeSE, MakeDisk              */

/* dependencies: none                              */

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

#include <stdlib.h>
#include "morph_sub.h"


/* Get the strcututring element from a file. Allocate the space */
/* for it and return a pointer to it.                           */

int *LoadSE( sname, sX, sY, sorgx, sorgy )
   char *sname;        /* structuring element file name */
   int *sX,*sY;        /* x,y dimensions of bounding box */
   int *sorgx, *sorgy; /* x,y position of SE origin */
   {    
   int i,n;            /* indicies */
   int *SE;            /* SE array */
   char *prefix;       /* leading component of pathnmae */
   char pathname[256]; /* pathname string */
   FILE *sefp;         /* structuring element file pointer */

   /* if environment variable SEPATH is set, get it and prepend to path */
   pathname[0] = '\0';
   prefix = getenv( SEPATH );
   if ( prefix && (n = strlen(prefix))  ) 
      {
      strcpy(pathname,prefix);
      if ( pathname[n-1] != '/' )
         {
         pathname[n] = '/';
         pathname[n+1] = '\0';
         }
      }
   strcat( pathname, sname );

   /* try to open the SE file */
   if ( !(sefp = fopen( pathname, "r")) )
      {
      fprintf(stderr,"Unable to open %s\n", pathname);
      exit(0);
      }

   /* get the xy dimensions and origin coordinates */
   fscanf(sefp,"%d %d %d %d",sX,sY,sorgx,sorgy);

   /* make sure origin is within bounding box */
   if ( *sorgx>=*sX || *sorgy>=*sY )
      {
      fprintf(stderr,"The SE origin must be inside the bounding rectangle.\n");
      exit(0);
      }

   /* number of pixels covered by bounding box */
   n = (*sX) * (*sY);

   /* allocate the space for it */
   if ( !(SE = (int *)calloc( n, sizeof(int) )) )
      {
      fprintf(stderr,"Unable to allocate memory for SE %s\n", sname);
      exit(0);
      }

   /* read it in from the file */
   for ( i=0; i<n; ++i ) fscanf(sefp, "%d", SE+i);

   /* close the file */
   fclose( sefp );

   /* pass the pointer to the SE array */
   return( SE );
   }




/* create a stucturing element per command SEtype */

int *MakeSE( sname, SEtype, sX, sY, smax, sorgx, sorgy )
   char *sname;         /* SE path name */
   int SEtype;          /* type of SE to get or make */
   int *sX,*sY;         /* x,y, dims of SE bpounding box */
   int smax;            /* peak value of SE of type auto */
   int *sorgx, *sorgy;  /* x,y location of SE origin */
   {
   int *SE;


   /* act according to type */
   switch ( SEtype )
      {
      case S3X3 :  /* 3x3 square */
         {
         SE = SE3x3;
         *sX = *sY = 3; 
         *sorgx = *sorgy = 1;
         break;
         }
      case S5X5 : /* 5x5 "fat plus": 5x5 square with corners deleted */
         {
         SE = SE5x5;
         *sX = *sY = 5; 
         *sorgx = *sorgy = 2;
         break;
         }
      case PLUS : /* 3x3 plus: 3x3 square with corners deleted */
         {
         SE = SEplus;
         *sX = *sY = 3; 
         *sorgx = *sorgy = 1;
         break;
         }
      case AUTO : /* create disk-shaped SE of arbitrary size */
         {
         SE = MakeDisk( sname, *sX, *sY, smax, sorgx, sorgy );
         break;
         }
      default : /* this should never happen */
         {
         fprintf(stderr, "Catastrophic failure!\n");
         fprintf(stderr,
         "   The kaelstrom directory has overwritten the lamoid pointer.\n");
         exit( 0 );
         }
      }
   return( SE );
   }



/* create a structuring element of type AUTO. */ 
/* That is, a digital approximation to a disk */

int *MakeDisk( sname, sX, sY, smax, sorgx, sorgy )
   char *sname;
   int sX,sY;
   int smax;
   int *sorgx, *sorgy;
   {    
   int i,j,n;
   int *s,*SE;
   double a,b,r,t,z;

   /* check for zero dims */
   if ( !( sX && sY )  )
      {
      fprintf(stderr,
         "-k auto x y z :  neither x nor y may be zero.\n");
      exit(0);
      }

   /* make sure peak value is legal */ 
   if ( smax < BLACK  ||  smax > WHITE )
      {
      fprintf(stderr,
         "-k auto x y z :  z must be in [0,255]\n");
      exit(0);
      }

   /* compute the origin. differs depending on parity of dimension */
   *sorgx = (sX % 2) ? sX/2 : (sX/2)-1;
   *sorgy = (sY % 2) ? sY/2 : (sY/2)-1;

   /* number of pixels covered by bounding box */
   n = sX * sY;

   /* allocate space for it */
   if ( !(SE = (int *)calloc( n, sizeof(int) )) )
      {
      fprintf(stderr,"Unable to allocate memory for auto SE.\n");
      exit(0);
      }

   if ( n > 1 ) /* then SE is not a single point */
      {
      a = (sX>1) ? 2/(double)(sX-1) : 0.0 ;
      b = (sY>1) ? 2/(double)(sY-1) : 0.0 ;
      z = (double)smax;

      /* t is the distance from the center of the disk out to the edge */
      /* t is one within a tolerance of max(a,b) */
      t = MAX(a,b);
      t = sqrt( 1.0 + t*t + 0.0001*t );

      /* compute disk */
      s = SE;
      if ( sX>1 && sY>1 )
         for ( j=0; j<sY; ++j )
            for ( i=0; i<sX; ++i )
               {
               r = sqrt( pow(-1.0+(double)i*a,2.0) + pow(1.0-(double)j*b,2.0) );
               if ( r*r > t ) *(s++) = -1;
               else *(s++) = (int)( z*sqrt( t - r*r ) + 0.5 );
               }
      else  /* either sX==1 or sY==1 */
         for ( j=0; j<n; ++j )
            {
            r = sqrt( pow(-1.0+(double)j*(a+b),2.0) );
            r = ( r < 1 ) ? r : 1.0 ;
            *(s++) = (int)( z*sqrt( 1.0 - r*r ) + 0.5 );
            }
      }
   else
      *SE = smax; /* SE is a single point */

   return( SE );
   }


