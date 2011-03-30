/* morph.h  --  header file for image morphology program morph */

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


/* For a nice tutorial on image morphology, see                             */
/* Haralick, R.M, S.R. Sternberg, and X. Zhuang, "Image analysis using      */
/* mathematical morphology," IEEE Trans. PAMI, vol. 9, No. 4, July 1987,    */
/* pp. 532-550.                                                             */
/* or                                                                       */
/* Maragos, P. and R. Shaffer,                                              */
/* "Morphological systems for multidimensional Signal Processing",          */
/* Proc. IEEE, vol. 78, no. 4, April 1990.                                  */

/* For additional info on the hit-or-miss transform, see J. Serra, Image    */
/* Analysis and Mathematical Morphology, Academic Press, London, 1982.      */

/* usage:

morph <In >Out -m e|d|o|c|r|t|b|l|m|n|p|q [-i g|b] [-s g|b] [-o s|f] 
      [-t nnn [mmm]] [-l kkk lll] [-r med|nnn] [-z] [-n] [-v] 
      -k SEFile | 3x3 | 5x5 | plus | auto xxx yyy [zzz]

where:    

    In is the path name of the Sun rasterfile input image. 
    (Type <In so the file is read in through stdin.)

    Out is the pathname of the Sun rasterfile output image.
    (Type >Out so the file is output through stdout.)

    morph extracts and operates on the luminance component of the image.
    Thus the output of morph is grayscale even if the input is color.

    The letter following -m indicates the morphological operation:
        e - erode
        d - dilate
        o - open
        c - close
        r - rank filter
        t - top hat transform (image minus opening)
        b - bot hat transform (closing minus image)
        l - general LUM (lower-upper-middle) filter
        m - LUM smoothing filter
        n - LUM sharpening filter
        p - I && !E (pixelwise)  where I = original image; E = eroded image;
        q - D && !I (pixelwise)        D = dilated image;
    one of these letters must be specified; there is no default.

    l is the lower-upper-middle filter as defined by Hardie, R. C., and 
    C. G. Boncelet, "LUM filters: A class of rank-order-based filters for 
    smoothing and sharpening," IEEE Trans. Signal Processing, vol. SP-41, 
    No. 3, March 1993.    
    m is the LUM smoothing filter defined therein, and n is the LUM 
    sharpening filter.                                    
    These filters compare the center pixel in a neighborhood defined by 
    an SE to upper and lower order statistics in the neighborhood.
    Depending on the ordering either the center pixel or one of the order 
    stats is output.                                       
    Note that we define order statistics opposite Hardie and Boncelet
    Whereas OS(1) is the minimum for them OS(1) is the maximum in     
    these routines.   

    p with a binary hit-or-miss structuring element will delete in a binary 
    image, white features with the shape of the "hit" portion of the SE.
    (e.g. one can easily devise a SE to delete isolated pixels).
    p with a gray-level SE will delete the "interiors" from sets of white
    pixels in a binary image.
    q with a binary hit-or-miss structuring element will delete in a binary 
    image, black features with the shape of the "hit" portion of the SE.
    (e.g. one can easily devise a SE to delete isolated pixels).
    q with a gray-level SE will delete the "interiors" from sets of black
    pixels in a binary image.

    Switch -i indicates that the next letter tells the image type:
    either g for a gray-level image or b for a binary image. 
    If -i is not included, the default is gray-level.

    Switch -s  indicates that the next letter tells the structuring 
    element type: either b for a binary SE or g for a gray-level SE. 
    If -s is not included, the default is binary.

    The letter following -o, either s or f, indicates that the
    operation is either a set operation or a function operation.
    (See reference.) If -o is not included, the default is set op.

    -t nnn [mmm] indicates that a threshold of value nnn from below (and 
    mmm from above; if unspecified mmm == 255) will be used on the
    input if the following 2 criteria are true: 
    the input is a gray-level image AND the operation is a set operation. 
    If the two criteria are true and -t nnn [mmm] is not included, 
    the operation is treated as a function and set processing (FSP) 
    operation (See ref.). If the criteria are not true and -t nnn is 
    specified anyway, it is ignored.
    Note that you can do a simple threshold of a gray level image with:

    morph < in.ras > out.ras  -m e -t 128 -k auto 1 1

    Switch -r is meaningful only if -m r is specified. Then the
    field following -r indicates the order of the filter. If the
    letters "med" are in the field, a median filter is used. If the
    field contains a number, nnn, then that value is used. If -op r
    is specified and -r med|nnn is not, the rank filter defaults to
    a median filter.

    Switch -l is meaningful only if -m l, -m m, or -m n is specified.
    Then kkk and lll are integers that correspond to the the values in 
    Hardie, R. C., and C. G. Boncelet, 
    "LUM filters: A class of rank-order-based filters for smoothing and 
    sharpening," IEEE Trans. Signal Processing, vol. SP-41, No. 3, 
    March 1993. 
    .
    If -m l is specified, the general LUM filter is selected, then 
    integers kkk and lll in the statement -l kkk lll must satisfy
    1 <= kkk <= lll <= med == int(SESupport / 2) + 1, where SESupport is
    the number of active pixels in the structuring element.
    .
    If -m m is specified, the LUM smoothing filter is selected, then 
    integer kkk in the statement -l kkk lll must satisfy
    1 <= kkk <= med == int(SESupport / 2) + 1. The value of lll is ignored.
    .
    If -m n is specified, the LUM sharpening filter is selected, then 
    integer lll in the statement -l kkk lll must satisfy
    1 <= lll <= med == int(SESupport / 2) + 1. The value of kkk is ignored.
    .
    To have morph tell you the value of int(SESupport / 2) + 1,
    execute the program with -l 0 0 as well as the -k SE spec.

    The presence of switch -z tells the program NOT to zero-pad the
    boundary of the image. With this option, the output image has a
    border of zeros inside it the width and height of the SE. That is,
    the transformed area of the output image is smaller than the actual
    image dimensions. This is, in a sense, a more accurate result than
    the zero padded default. To zero pad the input permits the program
    to transform the border region, but it does this on the assumption
    that the original scene was black outside the image. This, of course,
    is almost nver true. Thus, the border region is inaccurately 
    transformed. Use this switch if accuracy is more important than having
    an image that is "colored in" out to the boundary.

    Switch -n tells the program NOT to scale the output of the operation.
    Such scaling happens by default for a function operation (-o f) on
    a gray-scale image (-i g) with a gray-scale SE (-s g). This switch
    is ignored by other operations.

    Switch -v tells the program to display some info as it computes.

    Switch -k indicates that the next field is one of 5 things:

    (1) "3x3"  --  specifies the structuring element to be a 3 by 3 square 
    of pixels. (Don't use the quotes "" on the actual command line.)

    (2) "plus"  --  specifies the SE to be a 3 by 3 "+" shaped set of 
    pixels.

    (3) "5x5"  --  specifies the SE to be a 5x5 quasi disk (square without
    corners) of pixels.

    If one of the canned SE's (3x3, plus, or 5x5) is chosen, the -s and -o
    flags are forced to be -s g  -o s.

    (4) "auto x y [z]"  --  the program makes an SE. The self-made SE is
    a disk with support covering x pixels horizontally and y pixels
    vertically.  x and y must be odd. If specified, z is the gray level of 
    the center pixel. If z > 0, a function operation (-o f) is performed. 
    If z is not given or z == 0, the level defaults to BLACK (0) and a set
    operation is performed (-o s). If "-k auto" is specified the SE type is 
    forced to gray (-s g).

    (5) If the field following -k is not one of the above, then the 
    string, called SEFILE in the usage example, is taken as the pathname 
    of a structuring element file. If the user has an environment 
    variable called SEPATH, the program appends SEFILE to it for the
    complete pathname.

    A structuring element file is an ASCII file of integers separated
    by spaces. The first two numbers, x, y, are the horizontal and 
    vertical dimensions in pixels of the smallest rectangle that will 
    cover the structuring element. Both x and y must be > 0.
    The next two numbers, i, j are the horizontal and vertical 
    coordinates, respectively, of the SE origin.  
    IMPORTANT: The origin is expected to be in the covering rectangle. 
    The upper left hand corner of  the rectangle has coordinates (0,0); 
    the lower right is (x-1,y-1).
    Following the first four integers are x*y integers separated by 
    spaces. These numbers are the SE elements. Their interpretation 
    depends on the morphological operation being performed.

    Negative SE elements are ALWAYS treated as logical DON'T CAREs. That
    is, when the operation is in progress, image pixels under negative SE
    elements are ignored. Thus, the support of the SE is limited to
    those elments that are nonnegative. This permits the creation
    of odd-shaped and multiply connected SE's.

    If the input image is flagged binary, -i b, (i.e. pixels grouped as 
    zero and not zero), and if the SE is flaged binary, -s b, then the SE 
    is used to perform a hit-or-miss transform. In this case, zero SE 
    elements cover the "miss" support and positive (nonzero) elements 
    cover the "hit" support. The actual gray-levels are ignored.

    If the input image is flagged -i b and the SE is flagged gray (-s g)
    then the nonnegative (both zero and greater than zero) SE elements
    determine the support of a "hit-only" transform.  That is, the
    nonnegative suport is used as a standard "set"-type SE for set
    (binary) morphology. (Of course, the other gray-level info is ignored.)

    Note: If the input is flagged -i b, then the flag [-o s|f] is ignored.

    Here are the possible gray-level image flags and the resulting
    interpretation of the SE elements:

    -i g  -s b  -o s  Function-set morphology on support of strictly
        greater than zero SE elements.

    -i g  -s b  -o f  Same as above.

    -i g  -s g  -o s  Function-set morphology on support of nonnegative
        (greater than or equal to zero) SE elements.

    -i g  -s g  -o f  Function-function morphology on support of 
        nonnegative SE elements.

*/

extern void  GetArgs();

