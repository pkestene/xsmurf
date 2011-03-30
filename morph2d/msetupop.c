/* msetupop.c  --  configure morph program for requested operation */

/* contains: SetUpOp(), OptErr() */

/* dependencies: GetSupport(), BinBinErode(), BinBinDilate(),         */
/*               BinGrayErode(), BinGrayDilate(), BinRankFilt(),      */
/*               GrayBinErode(), GrayBinDilate(), GrayGraySetErode(), */
/*               GrayGraySetDilate(), GrayGrayFctErode(),             */
/*               GrayGrayFctDilate(), GrayRankFilt()                  */

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

void OptErr();

int SetUpOp( morph1, morph2, MOp, SorF, SEType, ImgType, stelem, sX, sY, sZ, 
             AutoSE, ssize, rank, LThresh, UThresh, opt, LUMparam )
   void (**morph1)();    /* morph routine 1 */
   void (**morph2)();    /* morph routine 2 (for 2 pass procs. e.g. open) */
   int  MOp;             /* basic morphological operation */
   int  SorF;            /* set or function operation */
   int  SEType;          /* structuring element type (gray or binary) */
   int  ImgType;         /* image type (gray or binary) */
   int  *stelem;         /* SE data list */
   int  sX,sY,sZ;        /* SE dimensions and max gray-level */
   int  AutoSE;          /* T => automatic SE -- always grayscale SE */
   int  *ssize;          /* SE support area */
   int  rank;            /* rank for rank filter */
   int  *LThresh;        /* lower threshold */
   int  *UThresh;        /* upper threshold */
   int  *opt;            /* optional argument (either 0 or rank or NOTFLG) */
   int  *LUMparam;       /* LUM filter parameters (array of 3 int's) */
   {
   int MorphOp;
   int lum;
   char msg[16];


   /* build operation flag MorphOp */
   MorphOp = MOp | SorF | SEType | ImgType ;

   /* must specify a morphological operation to perform */
   if ( !( MorphOp & OPMASK ) )  OptErr( 4, NULL, NULL );

   /* automatic SE's (includes canned SE's) are necessarily gray-scale */
   if ( AutoSE )
      {
      MorphOp = (MorphOp & ~SEMASK) | GRASE;
      if ( sZ == 0 ) /* and if gray-level is constant, do a set op */
         MorphOp = (MorphOp & ~SFMASK) | SET;
      else 
         MorphOp = (MorphOp & ~SFMASK) | FUNCT;
      }

   /* is this an LUM filter? */
   lum = (MOp == LUM) || (MOp == LUMSMO) || (MOp == LUMSHA);

   /* init option flag */
   *opt = FALSE;

   if ( !lum && (((MorphOp & IMMASK) == BINIMG) || *UThresh) )  
      /* treat input as a binary image */
      {
      MorphOp = (MorphOp & ~SFMASK) | SET;
      if ( !*UThresh ) 
         {
         *UThresh = WHITE;
         if (!*LThresh) *LThresh = WHITE/2 + 1;
         }
      if ( (MorphOp & SEMASK) == BINSE ) /* binary SE (Hit-or-miss trans) */
         switch ( MorphOp & OPMASK & (~NOTFLG) )    
            {
            case ERODE  : { *morph1 = BinBinErode;  break; }
            case DILATE : { *morph1 = BinBinDilate; break; }
            case OPEN   : { *morph1 = BinBinErode; 
                            *morph2 = BinBinDilate; break; }
            case CLOSE  : { *morph1 = BinBinDilate; 
                            *morph2 = BinBinErode;  break; }
            case RANK   : { *morph1 = BinRankFilt;  break; }
            default     : { OptErr( 2, NULL, NULL ); }
            }
      else                             /* gray-lev SE; hit on support >= 0 */
         switch ( MorphOp & OPMASK & (~NOTFLG) )   
            {
            case ERODE  : { *morph1 = BinGrayErode;  break; }
            case DILATE : { *morph1 = BinGrayDilate; break; }
            case OPEN   : { *morph1 = BinGrayErode; 
                            *morph2 = BinGrayDilate; break; }
            case CLOSE  : { *morph1 = BinGrayDilate; 
                            *morph2 = BinGrayErode;  break; }
            case RANK   : { *morph1 = BinRankFilt;   break; }
            default     : { OptErr( 2, NULL, NULL ); }
            }
      if ( MorphOp & NOTFLG ) *opt = TRUE;
      }
   else  /* treat input as a gray-level image; pixels are intensities */
      {
      if ( MorphOp & NOTFLG ) { OptErr( 5, NULL, NULL ); }

      if      ((MorphOp & OPMASK) == TOPHAT) { MorphOp |= OPEN; }
      else if ((MorphOp & OPMASK) == BOTHAT) { MorphOp |= CLOSE; }

      if ( MOp == RANK ) { *morph1 = GrayRankFilt; }
      else if ( lum )
         switch ( MorphOp & LLMASK )    
            {
            case LUM    : { *morph1 = LUMfilt;   break; }
            case LUMSMO : { *morph1 = LUMsmooth; break; }
            case LUMSHA : { *morph1 = LUMsharp;  break; }
            }
      else if ( (MorphOp & SEMASK) == BINSE ) 
                                      /* binary SE; set morph on >0 supp.  */
         switch ( MorphOp & LLMASK )    
            {
            case ERODE  : { *morph1 = GrayBinErode;  break; }
            case DILATE : { *morph1 = GrayBinDilate; break; }
            case OPEN   : { *morph1 = GrayBinErode; 
                            *morph2 = GrayBinDilate; break; }
            case CLOSE  : { *morph1 = GrayBinDilate; 
                            *morph2 = GrayBinErode;  break; }
            }
      else 
         {                             /* gray-level SE */
         if ( (MorphOp & SFMASK) == SET ) /* set morph on >=0 support   */
            switch ( MorphOp & LLMASK )    
               {
               case ERODE  : { *morph1 = GrayGraySetErode;  break; }
               case DILATE : { *morph1 = GrayGraySetDilate; break; }
               case OPEN   : { *morph1 = GrayGraySetErode; 
                               *morph2 = GrayGraySetDilate; break; }
               case CLOSE  : { *morph1 = GrayGraySetDilate; 
                               *morph2 = GrayGraySetErode;  break; }
               }
         else                          /* functional morph operation */
            switch ( MorphOp & LLMASK )    
               {
               case ERODE  : { *morph1 = GrayGrayFctErode;  break; }
               case DILATE : { *morph1 = GrayGrayFctDilate; break; }
               case OPEN   : { *morph1 = GrayGrayFctErode; 
                               *morph2 = GrayGrayFctDilate; break; }
               case CLOSE  : { *morph1 = GrayGrayFctDilate; 
                               *morph2 = GrayGrayFctErode;  break; }
               }
         }
      }

   if ( lum )
      {
      if ( !(*ssize = GetSupport( stelem, sX, sY, (MorphOp & SEMASK)==BINSE )) )
         {
         fprintf(stderr,"Error: Structuring element has zero support\n");
         exit( 0 );
         }
      *(LUMparam) = *LThresh;   /* this is k in Hardie and Boncelet */
      *(LUMparam+1) = *UThresh; /* this is l in Hardie and Boncelet */
      *(LUMparam+2) = *ssize;   /* this is N in Hardie and Boncelet */
      /* note that I am forcing an int to contain an address that will be */
      /* used as a pointer by the LUM routines. This is bad programming   */
      /* practice and might cause problems for some compilers.            */
      *opt = (int)LUMparam; 

      if ( (MorphOp & LLMASK) == LUM )
         {
         if ( *UThresh == ((*ssize / 2) + 1) )
            {
            MorphOp = (MorphOp & ~OPMASK) | LUMSMO;
            *morph1 = LUMsmooth;
            *(LUMparam+1) = 0;
            }
         else if ( *LThresh == 1 )
            {
            MorphOp = (MorphOp & ~OPMASK) | LUMSHA;
            *morph1 = LUMsharp;
            *LUMparam = 0;
            }
         else if ( !(*LThresh) ||
                   (*LThresh > ((*ssize / 2) + 1)) ||
                   (*LThresh > *UThresh) ||
                   (*UThresh > (*ssize - *UThresh))
                 )
            {
            sprintf( msg, "%d", (*ssize / 2) + 1 );
            OptErr( 9, msg, NULL );
            }
         }

      if ( (MorphOp & LLMASK) == LUMSMO )
         {
         if ( !(*LThresh) )
            {
            sprintf( msg, "%d", (*ssize / 2) + 1 );
            OptErr( 10, msg, NULL );
            }
         else if ( *LThresh == ((*ssize / 2) + 1) )
            {
            MorphOp = (MorphOp & ~OPMASK) | RANK;
            *morph1 = GrayRankFilt;
            rank = (*ssize / 2) + 1;
            *LUMparam = *(LUMparam+1) = *(LUMparam+2) = 0;
            }
         }
      else if ( (MorphOp & LLMASK) == LUMSHA )
         {
         if ( !(*UThresh) )
            {
            sprintf( msg, "%d", (*ssize / 2) + 1 );
            OptErr( 11, msg, NULL );
            }
         }

      *LThresh = 0;
      *UThresh = 0;

      }


   if ( (MorphOp & OPMASK) == RANK )
      {
      if ( !(*ssize = GetSupport( stelem, sX, sY, (MorphOp & SEMASK)==BINSE )) )
         {
         fprintf(stderr,"Error: Structuring element has zero support\n");
         exit( 0 );
         }

      if ( rank > *ssize )
         {
         fprintf(stderr,"Error: rank (%d) is greater than support (%d)\n",
            rank,*ssize);
         exit( 0 );
         }
      else if ( rank == 0 ) /* default to median filter */
         {
         *opt = (*ssize / 2) + 1;
         }
      else if ( rank < 1 )
         {
         fprintf(stderr,"Error: rank (%d) is less than 1\n",rank);
         exit( 0 );
         }
      else if ( rank == 1 ) /* identical to dilate */
         {
         if ( (MorphOp & IMMASK) == GRAIMG )
            *morph1 = GrayGraySetDilate;
         else
            *morph1 = BinGrayDilate;
         MorphOp = DILATE | (MorphOp & IMMASK);
         }
      else if ( rank == *ssize ) /* identical to erode */
         {
         if ( (MorphOp & IMMASK) == GRAIMG )
            *morph1 = GrayGraySetErode;
         else
            *morph1 = BinGrayErode;
         MorphOp = ERODE | (MorphOp & IMMASK);
         }
      else *opt = rank; /* pass requested rank */
      }

   return(MorphOp);
   }




void OptErr( num, s, t )
   int num;
   char *s,*t;
   {
   switch ( num )
      {
      case 1 : 
         {   
         fprintf(stderr,"Invalid Op: %s %s\n",s,t);
         break;
         }
      case 2 :
         {
         fprintf(stderr,
                 "Cannot perform a tophat transform on a binary image.\n");
         break;
         }
      case 3 :
         {
         fprintf(stderr,
                 "You must specify a structuring element.\n");
         break;
         }
      case 4 :
         {
         fprintf(stderr,
                 "You must specify a morphological operation to perform.\n");
         break;
         }
      case 5 :
         {
         fprintf(stderr,
                 "Must specify ImgType binary, or SorF set and thresholds when using -p or -q.\n");
         break;
         }
      case 6 :
         {
         fprintf(stderr,
               "SE dimensions < 1 or negative maximum gray-level requested.\n");
         break;
         }
      case 7 :
         {
         fprintf(stderr,
               "Lower threshold greater than upper threshold.\n");
         break;
         }
      case 8 :
         {
         fprintf(stderr,
               "SE origin is outside enclosing rectangle\n");
         break;
         }
      case 9 :
         {
         fprintf(stderr,
               "LUM: Must have 0 < k < l < %s.\n",s);
         break;
         }
      case 10 :
         {
         fprintf(stderr,
               "LUMSmooth: Must have 0 < k < %s.\n",s);
         break;
         }
      case 11 :
         {
         fprintf(stderr,
               "LUMSharp: Must have 0 < l < %s.\n",s);
         break;
         }

      }
   exit( 0 );
   }


