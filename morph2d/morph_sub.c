/* morph_sub.c  --  image morphology subroutine    */

/* contains: MorphSub() */

/* dependencies: MakeSE(), LoadSE(), SetUpOp(), MakeWordImg(),        */
/*               ZeroPadWord(), SubtractImg(), ScaleWord2Byte(),      */
/*               MakeByteImg(), MoveByte2Byte(), ZeroPadByte(),       */
/*               SubtrByteImg(), BinBinErode(), BinBinDilate(),       */
/*               BinGrayErode(), BinGrayDilate(), BinRankFilt(),      */
/*               GrayBinErode(), GrayBinDilate(), GrayGraySetErode(), */
/*               GrayGraySetDilate(), GrayGrayFctErode(),             */
/*               GrayGrayFctDilate(), GrayRankFilt(),                 */
/*               LUMFilt(), LUMSmooth(), LUMSharp()                   */


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

/* Adapted to xsmurf by Pierre KESTENER (2000/06/26).
   Centre de Recherche Paul Pascal, Pessac, FRANCE. */


#include "morph_sub.h"


/* canned structuring elements */

int SEplus[9] = {   -1, BLACK,    -1, 
                 BLACK, BLACK, BLACK, 
                    -1, BLACK,    -1};

int SE3x3[9]  = {BLACK, BLACK, BLACK, 
                 BLACK, BLACK, BLACK, 
                 BLACK, BLACK, BLACK};

int SE5x5[25] = {   -1, BLACK, BLACK, BLACK,    -1,
                 BLACK, BLACK, BLACK, BLACK, BLACK, 
                 BLACK, BLACK, BLACK, BLACK, BLACK, 
                 BLACK, BLACK, BLACK, BLACK, BLACK, 
                    -1, BLACK, BLACK, BLACK,    -1};


/* input "file" is expected on stdin. output "file" is to stdout. */

void MorphSub( MOp, In, Out, Ix, Iy, ImgType, NZPad, LThresh, UThresh,
              SEName, AutoSE, Sx, Sy, Sz, SEType, SorF, Rank, NoScale, Dsp) 

     int MOp;             /* Morphological operation to perform */
     
     byte *In;            /* input image as byte list in row major order */
     byte *Out;           /* output image as byte list in row major order */
     int Ix, Iy;          /* image horizontal, vertical dimensions */
     int ImgType;         /* image type (gray-level or binary) */
     int NZPad ;          /* flag.  F => zeropadding of input */
     int LThresh;         /* lower binary threshold value */
     int UThresh;         /* upper binary threshold value */
     
     char *SEName;        /* structuring element (SE) path name */
     int AutoSE;          /* use a canned or auto generated SE */
     int Sx,Sy,Sz;        /* SE x and y support dims and max gray-lev (z) */
     int SEType;          /* Binary SE or gray-level SE */
     int SorF;            /* Set operation or Function operation */
     int Rank;            /* rank for rank filter */
     int NoScale;         /* flag.  T => do not scale output of IntMorph */
     int Dsp;             /* flag.  T => display some info */
     
{
  
  int MorphOp;         /* morphological operation specification variable */
  
  byte *BI1,*BI2,*BI3; /* byte image pointers */
  word *WI1,*WI2,*WI3; /* word image pointers */
  byte *bp;            /* byte pointer*/
  word *wp;            /* word pointer */
  int iX,iY;           /* image dimensions (w/ zero padding) */
  int tX,tY;           /* transform scan dimensions */
  int bX,bY;           /* x and y image border (zeropad) widths */ /* m v2.0 */
  int oX,oY;           /* image offsets */
  
  int *stelem;         /* SE in row-major order */
  int ssize;           /* SE support size */
  int sX,sY,sZ;        /* SE horizontal, vertical dimensions, max gray lev */
  int sorgx,sorgy;     /* SE origin offset from upper LH corner. */
  
  void (*morph1)();    /* addresses of morphological function 1 */
  void (*morph2)();    /* addresses of morphological function 2 */
  
  int Opt;             /* optional argument */
  int LUMparam[3];     /* LUM filter parameters (k,l,med) */
  
  
  /* begin */
  
  /* initialize everything */
  
  BI1=BI2=BI3=(byte *)NULL;
  WI1=WI2=WI3=(word *)NULL;
  stelem=NULL;
  morph1=morph2=NULL;
  iX=iY=tX=tY=oX=oY=ssize=sorgx=sorgy=ZERO;
  sX = Sx;
  sY = Sy;
  sZ = Sz;
  
  /* If requsted, make a structuring element. Otherwise, load one */
  if ( !( AutoSE || SEName ) ) 
    OptErr( 3, NULL, NULL );
  else if( AutoSE )
    stelem = MakeSE( SEName, AutoSE, &sX, &sY, sZ, &sorgx, &sorgy);
  else
    stelem = LoadSE( SEName, &sX, &sY, &sorgx, &sorgy);
  if ( sX<=0 || sY<=0 || sZ < 0 )
    OptErr( 6, NULL, NULL );
  if ( sorgx>=sX || sorgy>=sY )
    OptErr( 8, NULL, NULL );
  
  /* select appropriate functions given Op specification */
  MorphOp = SetUpOp( &morph1,&morph2,MOp,SorF,SEType,ImgType,stelem,sX,sY,sZ,
		     AutoSE,&ssize,Rank,&LThresh,&UThresh,&Opt,LUMparam );
  
  /* calculate image zeropad border widths */  /* m v2.0 */
  bX = sX - 1;                                 /* m v2.0 */
  bY = sY - 1;                                 /* m v2.0 */
  
  /* calculate working image dimensions and transform scan dimensions */
  if ( NZPad )
    {
      iX = Ix;            /* dimensions of image actually scanned by prog.  */
      iY = Iy;            
      tX = iX - bX;       /* actual number of neighborhoods scanned per row */
      tY = iY - bY;       /* actual number of neighborhoods scanned per col */
      oX = oY = 0;        /* offset between i/o images and working images   */
    }
   else
     {
       iX = Ix + 2*bX;    /* dimensions of image actually scanned by prog.  */
       iY = Iy + 2*bY;    /* including zero padding */
       tX = Ix + bX;      /* actual number of neighborhoods scanned per row */
       tY = Iy + bY;      /* actual number of neighborhoods scanned per col */
       oX = bX;           /* offset between i/o images and working images   */
       oY = bY;
     }
  
  /* select appropriate wordlength for ops and perform */
  
  if ( (MorphOp & SFMASK) == FUNCT   &&   (MorphOp & SEMASK) == GRASE   &&
       (MorphOp & IMMASK) == GRAIMG  )  
    /* Do 16-bit (word) morphological operations */
    {
      WI1 = MakeWordImg( iX, iY );
      WI2 = MakeWordImg( iX, iY );
      
      MoveByte2Word( In, Ix, Iy, 0, 0, WI1, iX, iY, oX, oY, Ix, Iy );
      
      morph1( WI1,WI2,iX,tX,tY,stelem,sX,sY,sorgx,sorgy,FALSE );
      
      if ( morph2 )        /* then this is an open or a close */
	{
	  WI3 = MakeWordImg( iX, iY );
	  
	  ZeroPadWord( WI2, iX, iY, bX, bY );  /* m v2.0 */
	  morph2( WI2,WI3,iX,tX,tY,stelem,sX,sY,sorgx,sorgy,FALSE );
	  
	  if ( (MorphOp & TOPHAT) || (MorphOp & BOTHAT) )
            {
	      ZeroPadWord( WI3, iX, iY, bX, bY );  /* m v2.0 */
	      if ( MorphOp & TOPHAT )
		SubtractImg( WI2, WI1, WI3, iX, iY );  /* WI2 = WI1 - WI3 */
	      else
		SubtractImg( WI2, WI3, WI1, iX, iY );  /* WI2 = WI3 - WI1 */
            }
	  else  /* move result to WI2 */
            {
	      wp  = WI2;
	      WI2 = WI3;
	      WI3 = wp;
            }
	}
      
      ZeroPadWord( WI2, iX, iY, bX, bY );            
      ScaleWord2Byte( WI2,iX,iY,oX,oY,Out,Ix,Iy,0,0,Ix,Iy,NoScale,Dsp); 
      
      free( WI1 );
      free( WI2 );
      if ( WI3 ) free ( WI3 );
      
    }        /* end of 16-bit (word) morphological operations */
  
  else     /* do 8-bit (byte) morphological operations */
    {
      
      BI1 = MakeByteImg( iX, iY );
      BI2 = MakeByteImg( iX, iY );
      
      MoveByte2Byte( In, Ix, Iy, 0, 0, BI1, iX, iY, oX, oY, Ix, Iy );
      
      if ( ((MorphOp & SFMASK) == SET)  &&  UThresh )
	{
	  if (LThresh > UThresh) OptErr( 7, NULL, NULL );
	  ThresholdImg( BI1, iX, iY, LThresh, UThresh );
	}
      
      morph1( BI1,BI2,iX,tX,tY,stelem,sX,sY,sorgx,sorgy,Opt );
      
      if ( morph2 )   /* then this is an open or a close */
	{
	  BI3 = MakeByteImg( iX, iY );
	  
	  morph2( BI2,BI3,iX,tX,tY,stelem,sX,sY,sorgx,sorgy,FALSE );
	  
	  if ( (MorphOp & TOPHAT) || (MorphOp & BOTHAT) )
            {
	      ZeroPadByte( BI2, iX, iY, bX, bY );  /* m v2.0 */
	      ZeroPadByte( BI3, iX, iY, bX, bY );  /* m v2.0 */
	      if ( MorphOp & TOPHAT )
		SubtrByteImg( BI2, BI1, BI3, iX, iY );  /* BI2 = BI1 - BI3 */
	      else
		SubtrByteImg( BI2, BI3, BI1, iX, iY );  /* BI2 = BI3 - BI1 */
            }
	  else
            {
	      bp  = BI2;
	      BI2 = BI3;
	      BI3 = bp;
            }
	}
      
      ZeroPadByte( BI2, iX, iY, bX, bY );
      MoveByte2Byte( BI2, iX, iY, oX, oY, Out, Ix, Iy, 0, 0, Ix, Iy );
      
      free( BI1 );
      free( BI2 );
      if ( BI3 ) free ( WI3 );
    }        /* end of 8-bit (byte) morphological operations */
  
  /*free( stelem );*/
  return;
}

/* MorphSub2 receive In and return Out word (short int) directly
   without using MoveByte2Word() function !! */


void MorphSub2( MOp, In, Out, Ix, Iy, ImgType, NZPad, LThresh, UThresh,
              SEName, AutoSE, Sx, Sy, Sz, SEType, SorF, Rank, NoScale, Dsp) 

     int MOp;             /* Morphological operation to perform */
     
     word *In;            /* input image as byte list in row major order */
     word *Out;           /* output image as byte list in row major order */
     int Ix, Iy;          /* image horizontal, vertical dimensions */
     int ImgType;         /* image type (gray-level or binary) */
     int NZPad ;          /* flag.  F => zeropadding of input */
     int LThresh;         /* lower binary threshold value */
     int UThresh;         /* upper binary threshold value */
     
     char *SEName;        /* structuring element (SE) path name */
     int AutoSE;          /* use a canned or auto generated SE */
     int Sx,Sy,Sz;        /* SE x and y support dims and max gray-lev (z) */
     int SEType;          /* Binary SE or gray-level SE */
     int SorF;            /* Set operation or Function operation */
     int Rank;            /* rank for rank filter */
     int NoScale;         /* flag.  T => do not scale output of IntMorph */
     int Dsp;             /* flag.  T => display some info */
     
{
  
  int MorphOp;         /* morphological operation specification variable */
  
  byte *BI1,*BI2,*BI3; /* byte image pointers */
  word *WI1,*WI2,*WI3; /* word image pointers */
  byte *bp;            /* byte pointer*/
  word *wp;            /* word pointer */
  int iX,iY;           /* image dimensions (w/ zero padding) */
  int tX,tY;           /* transform scan dimensions */
  int bX,bY;           /* x and y image border (zeropad) widths */ /* m v2.0 */
  int oX,oY;           /* image offsets */
  
  int *stelem;         /* SE in row-major order */
  int ssize;           /* SE support size */
  int sX,sY,sZ;        /* SE horizontal, vertical dimensions, max gray lev */
  int sorgx,sorgy;     /* SE origin offset from upper LH corner. */
  
  void (*morph1)();    /* addresses of morphological function 1 */
  void (*morph2)();    /* addresses of morphological function 2 */
  
  int Opt;             /* optional argument */
  int LUMparam[3];     /* LUM filter parameters (k,l,med) */
  
  
  /* begin */
  
  /* initialize everything */
  
  BI1=BI2=BI3=(byte *)NULL;
  WI1=WI2=WI3=(word *)NULL;
  stelem=NULL;
  morph1=morph2=NULL;
  iX=iY=tX=tY=oX=oY=ssize=sorgx=sorgy=ZERO;
  sX = Sx;
  sY = Sy;
  sZ = Sz;
  
  /* If requsted, make a structuring element. Otherwise, load one */
  if ( !( AutoSE || SEName ) ) 
    OptErr( 3, NULL, NULL );
  else if( AutoSE )
    stelem = MakeSE( SEName, AutoSE, &sX, &sY, sZ, &sorgx, &sorgy);
  else
    stelem = LoadSE( SEName, &sX, &sY, &sorgx, &sorgy);
  if ( sX<=0 || sY<=0 || sZ < 0 )
    OptErr( 6, NULL, NULL );
  if ( sorgx>=sX || sorgy>=sY )
    OptErr( 8, NULL, NULL );
  
  /* select appropriate functions given Op specification */
  MorphOp = SetUpOp( &morph1,&morph2,MOp,SorF,SEType,ImgType,stelem,sX,sY,sZ,
		     AutoSE,&ssize,Rank,&LThresh,&UThresh,&Opt,LUMparam );
  
  /* calculate image zeropad border widths */  /* m v2.0 */
  bX = sX - 1;                                 /* m v2.0 */
  bY = sY - 1;                                 /* m v2.0 */
  
  /* calculate working image dimensions and transform scan dimensions */
  if ( NZPad )
    {
      iX = Ix;            /* dimensions of image actually scanned by prog.  */
      iY = Iy;            
      tX = iX - bX;       /* actual number of neighborhoods scanned per row */
      tY = iY - bY;       /* actual number of neighborhoods scanned per col */
      oX = oY = 0;        /* offset between i/o images and working images   */
    }
   else
     {
       iX = Ix + 2*bX;    /* dimensions of image actually scanned by prog.  */
       iY = Iy + 2*bY;    /* including zero padding */
       tX = Ix + bX;      /* actual number of neighborhoods scanned per row */
       tY = Iy + bY;      /* actual number of neighborhoods scanned per col */
       oX = bX;           /* offset between i/o images and working images   */
       oY = bY;
     }
  
  /* select appropriate wordlength for ops and perform */
  
  if (1)
    /* (MorphOp & SFMASK) == FUNCT   &&   (MorphOp & SEMASK) == GRASE   &&
       (MorphOp & IMMASK) == GRAIMG */
    /* Do 16-bit (word) morphological operations */
    {
      WI1 = MakeWordImg( iX, iY );
      WI2 = MakeWordImg( iX, iY );
      
      MoveWord2Word( In, Ix, Iy, 0, 0, WI1, iX, iY, oX, oY, Ix, Iy );
      
      morph1( WI1,WI2,iX,tX,tY,stelem,sX,sY,sorgx,sorgy,FALSE );
      
      if ( morph2 )        /* then this is an open or a close */
	{
	  WI3 = MakeWordImg( iX, iY );
	  
	  ZeroPadWord( WI2, iX, iY, bX, bY );  /* m v2.0 */
	  morph2( WI2,WI3,iX,tX,tY,stelem,sX,sY,sorgx,sorgy,FALSE );
	  
	  if ( (MorphOp & TOPHAT) || (MorphOp & BOTHAT) )
            {
	      ZeroPadWord( WI3, iX, iY, bX, bY );  /* m v2.0 */
	      if ( MorphOp & TOPHAT )
		SubtractImg( WI2, WI1, WI3, iX, iY );  /* WI2 = WI1 - WI3 */
	      else
		SubtractImg( WI2, WI3, WI1, iX, iY );  /* WI2 = WI3 - WI1 */
            }
	  else  /* move result to WI2 */
            {
	      wp  = WI2;
	      WI2 = WI3;
	      WI3 = wp;
            }
	}
      
      ZeroPadWord( WI2, iX, iY, bX, bY );            
      /*ScaleWord2Byte( WI2,iX,iY,oX,oY,Out,Ix,Iy,0,0,Ix,Iy,NoScale,Dsp);*/ 
      MoveWord2Word( WI2, iX, iY, oX, oY, Out, Ix, Iy, 0, 0, Ix, Iy);
     
      free( WI1 );
      free( WI2 );
      if ( WI3 ) free ( WI3 );
      
    }        /* end of 16-bit (word) morphological operations */

  /* do 8-bit (byte) morphological operations */
  
  /*else  
    {
      
      BI1 = MakeByteImg( iX, iY );
      BI2 = MakeByteImg( iX, iY );
      
      MoveByte2Byte( In, Ix, Iy, 0, 0, BI1, iX, iY, oX, oY, Ix, Iy );
      
      if ( ((MorphOp & SFMASK) == SET)  &&  UThresh )
	{
	  if (LThresh > UThresh) OptErr( 7, NULL, NULL );
	  ThresholdImg( BI1, iX, iY, LThresh, UThresh );
	}
      
      morph1( BI1,BI2,iX,tX,tY,stelem,sX,sY,sorgx,sorgy,Opt );
      
      if ( morph2 )   
	{
	  BI3 = MakeByteImg( iX, iY );
	  
	  morph2( BI2,BI3,iX,tX,tY,stelem,sX,sY,sorgx,sorgy,FALSE );
	  
	  if ( (MorphOp & TOPHAT) || (MorphOp & BOTHAT) )
            {
	      ZeroPadByte( BI2, iX, iY, bX, bY );
	      ZeroPadByte( BI3, iX, iY, bX, bY );
	      if ( MorphOp & TOPHAT )
		SubtrByteImg( BI2, BI1, BI3, iX, iY );
	      else
		SubtrByteImg( BI2, BI3, BI1, iX, iY );
            }
	  else
            {
	      bp  = BI2;
	      BI2 = BI3;
	      BI3 = bp;
            }
	}
      
      ZeroPadByte( BI2, iX, iY, bX, bY );
      MoveByte2Byte( BI2, iX, iY, oX, oY, Out, Ix, Iy, 0, 0, Ix, Iy );
      
      free( BI1 );
      free( BI2 );
      if ( BI3 ) free ( WI3 );
      } */       /* end of 8-bit (byte) morphological operations */
  
  /*free( stelem );*/
  return;
}



