/* mgetargs.c  --  morph argument parser program */

/* contains:  GetArgs(),  OptErr()[static]       */

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

/* See changes.doc for an explanation of the changes v4.0. */

#include "morph_sub.h"

static void OptErr();

/* parse (argc, argv) argument list for morph */

void GetArgs(argc,argv,sname,autoSE,x,y,z,
            MorphOp,SorF,SEType,ImgType,
            rank,lthresh,uthresh,
            nopad,noscale,dsp) 
   unsigned int argc;   /* count of arguments parsed by operating system    */
   char *argv[];        /* pointers to arguments parsed by operating system */
   char **sname;        /* structuring element (SE) path name */
   int *autoSE;         /* use canned or auto generated SE */
   int *x,*y,*z;        /* SE horizontal, vertical dimensions, gray-level */
   int *MorphOp;        /* morphological operation to perform */
   int *SorF;           /* Set operation or Function operation */
   int *SEType;         /* binary or gray-level SE */
   int *ImgType;        /* gray-level or binary */
   int *rank;           /* rank for rank filter */
   int *lthresh;        /* lower binary threshold value */
   int *uthresh;        /* upper binary threshold value */
   int *nopad;          /* flag.  F => zeropadding of input */
   int *noscale;        /* flag.  T => do not scale output of IntMorph */
   int *dsp;            /* flag.  T => display some info */
   {
   int n = argc;
   int i;

   /* zero everything initially */
   *autoSE=0;
   *x=0;
   *y=0;
   *z=0;
   *MorphOp=0;
   *SorF=0;
   *SEType=0;
   *ImgType=0;
   *rank=0;
   *lthresh=0;
   *uthresh=0;
   *nopad=0;
   *noscale=0;
   *dsp=0;


   /* get arguments */
   while ( --n )
      {
      if ( argv[n][0] == '-' )          /* then this is a switch */
         {
         if ( argv[n][1] == 'k' )       /* SE file name follows */
            {
            if ( !strcasecmp( argv[n+1], "3x3" ) )
               {
               *autoSE = S3X3;
               *sname = NULL;
               }
            else if ( !strcasecmp( argv[n+1], "plus" ) )
               {
               *autoSE = PLUS;
               *sname = NULL;
               }
            else if ( !strcasecmp( argv[n+1], "5x5" ) )
               {
               *autoSE = S5X5;
               *sname = NULL;
               }
            else if ( !strcasecmp( argv[n+1], "auto" ) )
               {
               *autoSE = AUTO;
               *x = atoi( argv[n+2] );
               *y = atoi( argv[n+3] );
               if ( ((n+4) < argc) && isdigit( argv[n+4][0] ) )
                  *z = atoi( argv[n+4] );
               /* note if (*autoSE != 0), setupop sets SEType = GRASE */
               *sname = NULL;
               }
            else
               *sname = argv[n+1];
            }
         else if ( argv[n][1] == 'm' )  /* morph op flag follows */
            {
            switch (argv[n+1][0])
               {
               case 'e' : { *MorphOp = ERODE;  break; }
               case 'd' : { *MorphOp = DILATE; break; }
               case 'o' : { *MorphOp = OPEN;   break; }
               case 'c' : { *MorphOp = CLOSE;  break; }
               case 'r' : { *MorphOp = RANK;   break; }
               case 't' : { *MorphOp = TOPHAT; break; }
               case 'b' : { *MorphOp = BOTHAT; break; }
               case 'l' : { *MorphOp = LUM;    break; }
               case 'm' : { *MorphOp = LUMSMO; break; }
               case 'n' : { *MorphOp = LUMSHA; break; }
               case 'p' : { *MorphOp = ERODE  | NOTFLG; break; }
               case 'q' : { *MorphOp = DILATE | NOTFLG; break; }
               default  : { OptErr( argv[n], argv[n+1] ); }
               }
            }
         else if ( argv[n][1] == 'o' )  /* op type follows */
            {
            switch (argv[n+1][0])
               {
               case 's' : { *SorF = SET;   break; }
               case 'f' : { *SorF = FUNCT; break; }
               default  : { OptErr( argv[n], argv[n+1] ); }
               }
            }
         else if ( argv[n][1] == 's' )  /* SE type follows */
            {
            switch (argv[n+1][0])
               {
               case 'b' : { *SEType = BINSE; break; }
               case 'g' : { *SEType = GRASE; break; }
               default  : { OptErr( argv[n], argv[n+1] ); }
               }
            }
         else if ( argv[n][1] == 'i' )  /* image type follows */
            {
            switch (argv[n+1][0])
               {
               case 'g' : { *ImgType = GRAIMG; break; }
               case 'b' : { *ImgType = BINIMG; break; }
               default  : { OptErr( argv[n], argv[n+1] ); }
               }
            }
         else if ( argv[n][1] == 'r' )  /* rank value follows */
            {
            *rank = strcmp( argv[n+1], "med" );
            if ( *rank ) *rank = atoi( argv[n+1] );
            } 
         else if ( argv[n][1] == 'l' )  /* LUM value(s) follows */
            {
            if ( (n+1 < argc) )
               {
               if ( isdigit( argv[n+1][0] ) )
                  {
                  *lthresh = atoi( argv[n+1] );
                  if ( n+2 < argc  &&  isdigit( argv[n+2][0] ) )
                      *uthresh = atoi( argv[n+2] );
                  else
                      OptErr( argv[n], "LUM (-l) parameter(s)." );
                  }
               else
                  OptErr( argv[n], "LUM (-l) parameter(s)." );
               }
            else 
               OptErr( argv[n], "LUM (-l) parameter(s)." );
            }
         else if ( argv[n][1] == 't' )  /* threshold value(s) follows */
            {
            *lthresh = atoi( argv[n+1] );
            if ( n+2 < argc  &&  isdigit( argv[n+2][0] ) )
                *uthresh = atoi( argv[n+2] );
            else
                *uthresh = WHITE;
            if (*lthresh > *uthresh) 
                OptErr( argv[n], "lthresh > uthresh" );
            } 
         else if ( argv[n][1] == 'z' ) /* do not zeropad input */
            *nopad = TRUE;

         else if ( argv[n][1] == 'n' ) /* do not scale output */
            *noscale = TRUE;

         else if ( argv[n][1] == 'v' ) /* display some info */
            *dsp = TRUE;
         }    /* end of if ( argv[n][0] == '-' ) */
      }   /* end of  while ( --n ) "get arguments" loop */

   return;
   }
   


static void OptErr( s, t )
   char *s,*t;
   {
   fprintf(stderr,"Invalid Op: %s %s\n",s,t);
   exit( 0 );
   }


