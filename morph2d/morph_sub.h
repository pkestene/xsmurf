/* morph_sub.h  --  header file for image morphology subroutine MorphSub() */

/* morph version 4.0  1 May 1993                   */
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

/* This subroutine performs many (if not all) possible 2D morphological     */
/* operations on gray-level or binary images. This includes hit-or-miss     */
/* transforms, order-statistic filters, function-processing or set-         */
/* processing erosion,  dilation, opening and closing, tophat (image minus  */
/* opening),  bothat (closing minus image), and isolated delete functions.  */

/* For a nice tutorial on image morphology, see                             */
/* Haralick, R.M, S.R. Sternberg, and X. Zhuang, "Image analysis using      */
/* mathematical morphology," IEEE Trans. PAMI, vol. 9, No. 4, July 1987,    */
/* pp. 532-550.                                                             */
/* or                                                                       */
/* Maragos, P. and R. Shaffer,                                              */
/* "Morphological systems for multidimensional Signal Processing",          */
/* Proc. IEEE, vol. 78, no. 4, April 1990.                                  */
/* or                                                                       */
/* For additional info on the hit-or-miss transform, see J. Serra, Image    */
/* Analysis and Mathematical Morphology, Academic Press, London, 1982.      */

/*

To use:

void MorphSub( MOp, In, Out, Ix, Iy, ImgType, NZPad, LThresh, UThresh,
              SEName, AutoSE, Sx, Sy, Sz, SEType, SorF, Rank, NoScale, Dsp) 

   int MOp;       Morphological operation to perform 

   byte *In;      input image as byte list in row major order 
   byte *Out;     output image as byte list in row major order 
   int Ix, Iy;    image horizontal, vertical dimensions 
   int ImgType;   image type (gray-level or binary) 
   int NZPad ;    flag.  F => zeropadding of input 
   int LThresh;   lower binary threshold value 
   int UThresh;   upper binary threshold value 

   char *SEName;  structuring element (SE) path name 
   int AutoSE;    use a canned or auto generated SE 
   int Sx,Sy,Sz;  SE x and y support dims and max gray-lev (z) 
   int SEType;    Binary SE or gray-level SE 
   int SorF;      Set operation or Function operation 
   int Rank;      rank for rank filter 
   int NoScale;   flag.  T => do not scale output of IntMorph 
   int Dsp;       flag.  T => display some info 


Detailed explanation of parameters:

    MOp     Morphological Operation. This integer parameter can have the
            following values:

            mnemon  hex       dec        action
            
            ERODE   0x0001      1        erosion
            DILATE  0x0002      2        dilation
            OPEN    0x0004      4        opening
            CLOSE   0x0008      8        closing
            RANK    0x0010     16        rank filter (order statistic filter)
            LUM     0x0011     17        Lower-Upper-Middle filter
            LUMSMO  0x0012     18        LUM smoothing filter
            LUMSHA  0x0014     20        LUM sharpening filter
            TOPHAT  0x0020     32        tophat (image minus opening)
            BOTHAT  0x0040     64        bothat (closing minus image)
            
            Moreover, the erode and dilate functions can be "negated" using:

            NOTFLG  0x0080  128 "not" flag for binary erodes and dilates

            Let I = original image; E = eroded image; D = dilated image.
            The following are valid parameters:
 
            mnemon            hex       dec        action
            
            ERODE  | NOTFLG   0x0081    129        I && !E (pixelwise)
            DILATE | NOTFLG   0x0082    130        D && !I (pixelwise)

            ERODE | NOTFLG with a binary hit-or-miss structuring element will 
            delete in a binary image, white features with the shape of the 
            "hit" portion of the SE. (e.g. one can easily devise a SE to 
            delete isolated white pixels). With a gray-level SE it will delete 
            the "interiors" from sets of white pixels in a binary image.
            DILATE | NOTFLG with a binary hit-or-miss structuring element will 
            delete in a binary image, black features with the shape of the 
            "hit" portion of the SE. (e.g. one can easily devise a SE to delete 
            isolated black pixels). With a gray-level SE it will delete the 
            "interiors" from sets of black pixels in a binary image.

            The Mop LUM is the lower-upper-middle filter as defined by
            Hardie, R. C., and C. G. Boncelet, "LUM filters: A class of 
            rank-order-based filters for smoothing and sharpening," 
            IEEE Trans. Signal Processing, vol. SP-41, No. 3, March 1993. 
            LUMSMO is the LUM smoothing filter defined therein, and LUMSHA 
            is the LUM sharpening filter.
            These filters compare the center pixel in a neighborhood defined
            by an SE to upper and lower order statistics in the neighborhood.
            Depending on the ordering either the center pixel or one of
            the order stats is output.

    In      The input image. A pointer to an Ix-times-Iy long list of bytes.
            The byte list is a one-byte per pixel grayscale image that is
            Iy rows by Ix columns in row major order. (First row of pixels,
            followed by second row, followed by third, ...).

    Out     The output image. A pointer to an Ix-times-Iy long list of
            bytes. The byte list is a one-byte per pixel grayscale image 
            that is Iy rows by Ix columns in row major order. This is where 
            morph will write its output. The space must be allocated by the 
            calling program.  

            Out and In may point to the same space without error. However,
            morph will then overwrite the input image with the output.

    Ix      Image horizontal dimension. Number of pixels per line (or
            number of columns per row) in image.

    Iy      Image vertical dimension. Number of lines (or rows) in the image.

    ImgType Image type: This specifies whether the input image is to be 
            treated like a gray level or binary image. The values in
            a binary image are zero and not-zero (it does not matter
            which of the values in [BLACK,WHITE]).
            Parameter values:

            mnemon    hex        dec        meaning
            
            GRAIMG    0x0000      0        gray-level image
            BINIMG    0x0200    512        binary image

    NZPad   If zero (FALSE) MorphSub will extend the size of the input
            image internally and create a border of zeros around it.
            If nonzero (TRUE) MorphSub will not do the zero padding.
            Note: this is all internal to MorphSub. The input and
            output images are both Ix by Iy independent of this flag.

            There are image border effects with each option. The effects
            differ depending on the option.
            With the option FALSE, morph-sub does what it can to "color in" 
            the image near its perimeter.
            With this option TRUE, the output image has a border of zeros 
            inside it the width and height of the SE. That is, the 
            transformed area of the output image is smaller than the actual
            image dimensions. This is, in a sense, a more accurate result than
            the zero padded default. To zero pad the input permits the program
            to transform the border region, but it does this on the assumption
            that the original scene was black outside the image. This, of 
            course, is almost never true. Thus, the border region is 
            inaccurately transformed. Use this switch if accuracy is more 
            important than having an image that is "colored in" out to the 
            boundary.

    LThresh lower binary threshold or lower LUM parameter.
    UTHresh upper binary threshold or upper LUM parameter.

            These two parameters will extract a binary image from a grayscale 
            image via thresholding if: 
            (1) MOp is not one of LUM, LUMSMO, or LUMSHA, and
            (2) UThresh is non zero, and 
            (3) SorF is SET. 
            Then all pixels with grey-levels in [Lthresh,UThresh] will be set 
            to WHITE, all others to BLACK. 
            Thresholding occurs before any morphology. This makes it possible 
            to specify binary morphology on a graylevel image. If UThresh > 0 
            AND LThresh > Uthresh, MorphSub aborts with an error. If UThresh 
            is zero no thresholding is performed.
            Note that one can use MorphSub to perform a simple threshold on an
            image by specifying MOp = ERODE, ImgType = GRAIMG, AutoSE = AUTO
            Sx = 1, Sy = 1, Sz = 0, and the appropriate LThresh and UThresh.

            When Mop == LUM, LUMSMO, or LUMSHA, then (LThresh,Uthresh) are 
            the lower (k) and upper (l) LUM parameters respectively as 
            defined in Hardie, R. C., and C. G. Boncelet, "LUM filters: A 
            class of rank-order-based filters for smoothing and sharpening," 
            IEEE Trans. Signal Processing, vol. SP-41, No. 3, March 1993. 
            If Mop == LUM, the general LUM filter is selected, then 
            (LThresh,Uthresh) must satisfy
            1 <= LThresh <= Uthresh <= med == int(SESupport / 2) + 1, 
            where SESupport is the number of active pixels in the 
            structuring element.
            If Mop == LUMSMO, the LUM smoothing filter is selected, then 
            (LThresh,Uthresh) must satisfy
            1 <= LThresh <= med == int(SESupport / 2) + 1.
            The value of UThresh is ignored.
            If Mop == LUMSHA, the LUM sharpening filter is selected, then 
            (LThresh,Uthresh) must satisfy
            1 <= UThresh <= med == int(SESupport / 2) + 1.
            The value of LThresh is ignored.
            
    SEName  This is a string pointer to the file (or path) name of a 
            structuring element (SE) file. If this pointer is not NULL and 
            AutoSE==0, then SEName is catenated to the end of the value of 
            environment variable SEPATH to get the SE file pathname. (If 
            SEPATH is not defined, then SEName is used alone.)
            See below for more complete info on SE's.

    AutoSE  If this flag is non zero, it signifies: use a canned or auto 
            generated SE. Its possible values are 

            mnemon    dec        meaning

            ZERO    0        do not use auto SE
            AUTO    1        generate disk shaped SE
            PLUS    2        use 3 by 3 "+" shaped SE
            S3X3    3        use 3 by 3 square SE
            S5X5    4        use 5 by 5 quasi disk SE

            See below for more complete info on SE's.

    Sx,Sy   Horizontal and vertical dimensions of program generated SE.
            Ignored if AutoSE != AUTO.

    Sz      Gray level of origin pixel in program generated SE. This
            is used if AutoSE == AUTO. Ignored if AutoSE != AUTO.

    SEType  specifies whether the structuring element is to be interpreted
            as a gray-level SE or a binary SE. (See below for the distinction.) 
            Its possible values are:

            mnemon    hex        dec        meaning

            GRASE    0x0000      0        gray-level SE
            BINSE    0x0100    256        binary SE

    SorF    Specifies whether the morphological operation is to be of the
            "set" type or "function" type. (See below for the 
            distinction.) Its possible values are:

            mnemon    hex        dec        meaning

            SET        0x0000       0    set operation
            FUNCT    0x0400    1024    function operation


            Structuring element specifications:

            SEFile: A structuring element file is an ASCII file of integers 
            separated by spaces. The first two numbers, x, y, are the 
            horizontal and vertical dimensions in pixels of the smallest 
            rectangle that will cover the structuring element. Both x and y 
            must be > 0.  The next two numbers, i, j are the horizontal and 
            vertical coordinates, respectively, of the SE origin.  
            IMPORTANT: The origin is expected to be in the covering 
            rectangle. If not, MorphSub aborts.
            The upper left hand corner of  the rectangle has coordinates 
            (0,0); the lower right is (x-1,y-1). Following the first four 
            integers are x*y integers separated by 
            spaces. These numbers are the SE elements. Their interpretation 
            depends on the morphological operation being performed.

            Negative SE elements are ALWAYS treated as logical DON'T CAREs. 
            That is, when the operation is in progress, image pixels under 
            negative SE elements are ignored. Thus, the support of the SE 
            is limited to those elments that are nonnegative. This permits 
            the creation of odd-shaped and multiply connected SE's or the
            placement of the SE origin outside the body of the SE.
            If ImgType is binary, (i.e. pixels grouped as zero and not 
            zero), and if SEType is binary, then the SE is used to perform 
            a hit-or-miss transform. In this case, zero SE elements cover 
            the "miss" support and positive (nonzero) elements cover the 
            "hit" support. The actual gray-levels are ignored.

            If ImgType is binary, and SEType is gray then the nonnegative 
            (both zero and greater than zero) SE elements determine the 
            support of a "hit-only" transform.  That is, the nonnegative 
            suport is used as a standard set-type SE for set (binary) 
            morphology. (Of course, the other gray-level info is ignored.)
            Note: if ImgType is binary, then a set operation is performed
            by default (SorF is ignored).

            The interpretation of the SE elements for ImgType gray depends
            on the flags SEType and SorF:

            SEType    SorF    action

            binary    set        Function-set morphology on support of strictly
                            greater than zero SE elements.
            binary    funct    Same as above.

            gray    set        Function-set morphology on support of nonnegative  
                            (greater than or equal to zero) SE elements.
            gray    funct    Function-function morphology on support of 
                            nonnegative SE elements.

            AUTO:  the program makes an SE. The self-made SE is a disk with 
            a diameter of Sx pixels horizontally and Sy pixels vertically.  
            Sx and Sy must be odd and greater than or equal to 1. 
            If AutoSE == AUTO, SEType is set to gray (SEType == GRASE).
            Sz is the gray level of the center pixel. 
            If Sz > 0: the SE will have a "curved" top. (Use Sz > 0 for a 
            rolling ball transform.) A function operation (SorF == FUNCT) is 
            performed.
            If Sz == 0, the SE is flat-topped. A set operation is performed 
            (SorF == SET).
            
            3x3:  specifies the SE to be a 3 by 3 square of pixels.

            plus: specifies the SE to be a 3 by 3 "+" shaped set of pixels.

            5x5:  specifies the SE to be a 5x5 quasi disk (square without
                  corners) of pixels.

            If one of the canned SE's (3x3, plus, or 5x5) is chosen, then
            SorF is set to SET and SEType is set to GRASE.

            Note that when AutoSE != 0, the values of SEType and SorF are
            ignored.

    Rank    This is meaningful only if Mop == RANK. Then the parameter 
            indicates the order of the filter. The rank option of MorphSub
            is actually an order statistic filter.  To compute the order,
            MorphSub counts the number of pixels in the support of
            the SE. In a gray-level SE that is all values greater than
            or equal to zero. We did not define a hit-or-miss rank filter. 
            Therefore, to compute the support of a binary SE, MorphSub 
            changes the zeros in a binary SE into -1's (DON'T CARES). 
            Then all the rank routine considers the support to be all SE 
            elements >= ZERO.

            If Rank == 0, MorphSub applies a median filter. If Rank == 1 
            it does a dilation. If Rank == support of SE, then MorphSub
            does an erosion. Otherwise, MorphSub does an order statistic
            filter of order Rank.


    NoScale When a function processing operation is performed on a gray-level
            image with a gray-level SE, the operation is performed with
            16-bit-precision signed arithmetic. When the operation is through,
            if NoScale == 0, MorphSub scales the result to fit into one byte 
            per pixel. If NoScale != 0, morph limits the results at BLACK
            and WHITE.

            Scaling is performed as follows: if the range of the result is
            less than one byte (i.e MaxPixVal - MinPixVal < or = 256) the
            result is translated so that the minimum is zero. (i.e. each
            resultant pixel is replaced with pixel - MinPixVal) 
            Otherwise, the minimum is subtracted from each pixel and the
            the difference is scaled by the ratio WHITE/(MaxPixVal-MinPixVal).

    Dsp     If this is true (nonzero) some info about what is going on is 
            displayed.
        
*/

extern void  GetArgs();


/* includes */

#include <ctype.h>
#include <stdlib.h>            
#include <math.h>
#include <memory.h>
#include <stdio.h>    
#include <strings.h>


/* defs */

#define SFMASK    0x0400      /* set / function mask */
#define SET       0x0000      /* set operation */
#define FUNCT     0x0400      /* function operation */

#define IMMASK    0x0200      /* image type bit of MorphOp */
#define GRAIMG    0x0000      /* gray-level image flag */
#define BINIMG    0x0200      /* binary image flag */

#define SEMASK    0x0100      /* structuring element (SE) type bit */
#define GRASE     0x0000      /* gray-level SE */
#define BINSE     0x0100      /* binary SE */

#define OPMASK    0x00FF      /* morph operation type bits of MorphOp */
#define LLMASK    0x001F      /* low-level morph ops mask */
#define LUMASK    0x0007      /* LUM bits mask */
#define ERODE     0x0001      /* erode flag  */
#define DILATE    0x0002      /* dilate flag */
#define OPEN      0x0004      /* open flag   */
#define CLOSE     0x0008      /* close flag  */
#define RANK      0x0010      /* rank filter flag */
#define LUM       0x0011      /* Lower-Upper-Middle filter */
#define LUMSMO    0x0012      /* LUM smoothing filter */
#define LUMSHA    0x0014      /* LUM sharpening filter */
#define TOPHAT    0x0020      /* tophat (image minus open ) flag */
#define BOTHAT    0x0040      /* tophat (close minus image) flag */
#define NOTFLG    0x0080      /* "not" flag for binary erodes and dilates */

#define SEPATH    "SEPATH"    /* structuring element directory env. var */

#define AUTO    1        /* make disk SE */
#define PLUS    2        /* use 3 by 3 "+" shaped SE */
#define S3X3    3        /* use 3 by 3 square SE */
#define S5X5    4        /* use 5 by 5 quasi disk SE */

#define BLACK    0
#define BLACK2 -32766
#define WHITE  255
#define WHITE2 32767
/*#define WHITE2 65535*/
#define TRUE     1
#define FALSE    0
#define ZERO     0
#define NSIZE  256
#define ALLOC TRUE

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

/* typedefs */

#ifndef BYTE
typedef unsigned char byte;
#define BYTE 1
#endif

#ifndef WORD
typedef short int word;
/*typedef unsigned short int word;*/
#define WORD
#endif

/* global functions */

extern int  *LoadSE();
extern int  *MakeSE();
extern int  *MakeDisk();
extern byte *MakeByteImg();
extern word *MakeWordImg();
extern void  MorphSub();
extern void  MorphSub2();      /* receive directly word images ! */
extern void  MoveByte2Byte();
extern void  MoveWord2Word();
extern void  MoveByte2Word();
extern void  ScaleWord2Byte();
extern int   SetUpOp();
extern void  SubtractImg();
extern void  ZeroPadByte();
extern void  ZeroPadWord();

extern void BinBinErode();
extern void BinGrayErode();
extern void GrayBinErode();
extern void GrayGraySetErode();
extern void GrayGrayFctErode();

extern void BinBinDilate();
extern void BinGrayDilate();
extern void GrayBinDilate();
extern void GrayGraySetDilate();
extern void GrayGraySetDilate2();
extern void GrayGrayFctDilate();

extern int  GetSupport();
extern void BinRankFilt();
extern void GrayRankFilt();
extern void LUMfilt();
extern void LUMsmooth();
extern void LUMsharp();

/* Global Variables */

extern int SEplus[];
extern int SE3x3[];
extern int SE5x5[];
