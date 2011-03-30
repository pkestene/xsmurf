/*
 * myFftw2.c --
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: myFftw2.c,v 1.2 1999/05/15 15:45:33 decoster Exp $
 */

#include "../cv1d/myFftw.h"

#include <stdlib.h> // for malloc

int
my_is_good_fftw2_size (int size)
{
  while ((size % 2) == 0) {
    size /= 2;
  }
  while ((size % 3) == 0) {
    size /= 3;
  }
  while ((size % 5) == 0) {
    size /= 5;
  }
  while ((size % 7) == 0) {
    size /= 7;
  }

  return (size == 1);
}

int
my_next_good_fftw2_size (int size)
{

  while (!my_is_good_fftw_size(size)) {
    size++;
  }

  return size;
}

#ifdef USE_FFTW

#include "../image/fftw3_inc.h"

#include "sys/times.h"
#include "time.h"
#include "myFftw2.h"

#ifndef _REAL_
#define _REAL_
typedef float real;
#endif

#ifndef _COMPLEX_
#define _COMPLEX_
typedef struct
{
  real real;
  real imag;
} complex;
#endif

/*static int    begin;
  static int    end;
  static struct tms ttime;
  static int zeT;*/

/*int zeNb;*/

enum {
  NO,
  YES
};

static int _wizdom_init_ = NO;
#define _wizdom_str_ "(FFTW-2.0.1 (854 17 -1 1 1 1 6 35) (122 17 -1 1 1 1 6 35) (427 17 -1 1 1 1 6 115) (61 17 -1 1 1 1 7 0) (4320 17 1 1 1 1 6 107) (864 17 1 1 1 1 6 107) (1440 17 1 1 1 1 6 107) (288 17 1 1 1 1 6 155) (480 17 1 1 1 1 6 43) (96 17 1 1 1 1 6 139) (160 17 1 1 1 1 6 91) (2160 17 1 1 1 1 6 267) (432 17 1 1 1 1 6 107) (720 17 1 1 1 1 6 107) (144 17 1 1 1 1 6 155) (240 17 1 1 1 1 6 267) (48 17 1 1 1 1 6 75) (80 17 1 1 1 1 6 91) (1080 17 1 1 1 1 6 139) (216 17 1 1 1 1 6 43) (360 17 1 1 1 1 6 75) (72 17 1 1 1 1 6 107) (120 17 1 1 1 1 6 139) (24 17 1 1 1 1 6 43) (40 17 1 1 1 1 6 75) (540 17 1 1 1 1 6 107) (108 17 1 1 1 1 6 155) (180 17 1 1 1 1 6 43) (36 17 1 1 1 1 6 59) (60 17 1 1 1 1 6 75) (12 17 1 1 1 1 5 204) (20 17 1 1 1 1 6 43) (270 17 1 1 1 1 6 43) (54 17 1 1 1 1 6 107) (90 17 1 1 1 1 6 107) (18 17 1 1 1 1 6 43) (30 17 1 1 1 1 6 43) (6 17 1 1 1 1 5 108) (4320 17 -1 1 1 1 6 99) (864 17 -1 1 1 1 6 51) (1440 17 -1 1 1 1 6 83) (288 17 -1 1 1 1 6 147) (480 17 -1 1 1 1 6 83) (96 17 -1 1 1 1 6 51) (160 17 -1 1 1 1 6 83) (2160 17 -1 1 1 1 6 67) (432 17 -1 1 1 1 6 99) (720 17 -1 1 1 1 6 99) (144 17 -1 1 1 1 6 147) (240 17 -1 1 1 1 6 259) (48 17 -1 1 1 1 6 67) (80 17 -1 1 1 1 6 83) (1080 17 -1 1 1 1 6 147) (216 17 -1 1 1 1 6 51) (360 17 -1 1 1 1 6 67) (72 17 -1 1 1 1 6 99) (120 17 -1 1 1 1 6 131) (24 17 -1 1 1 1 6 35) (40 17 -1 1 1 1 6 67) (540 17 -1 1 1 1 6 99) (108 17 -1 1 1 1 6 147) (180 17 -1 1 1 1 6 35) (36 17 -1 1 1 1 6 51) (60 17 -1 1 1 1 6 67) (12 17 -1 1 1 1 4 194) (20 17 -1 1 1 1 6 35) (270 17 -1 1 1 1 6 51) (54 17 -1 1 1 1 6 99) (90 17 -1 1 1 1 6 99) (18 17 -1 1 1 1 6 35) (30 17 -1 1 1 1 6 35) (6 17 -1 1 1 1 4 98) (3430 17 1 1 1 1 6 91) (490 17 1 1 1 1 6 91) (70 17 1 1 1 1 6 91) (10 17 1 1 1 1 5 172) (686 17 1 1 1 1 6 123) (98 17 1 1 1 1 6 123) (14 17 1 1 1 1 5 236) (3430 17 -1 1 1 1 6 83) (490 17 -1 1 1 1 6 83) (70 17 -1 1 1 1 6 83) (10 17 -1 1 1 1 4 162) (686 17 -1 1 1 1 6 115) (98 17 -1 1 1 1 6 115) (14 17 -1 1 1 1 4 226) (8575 17 1 1 1 1 6 123) (1225 17 1 1 1 1 6 91) (175 17 1 1 1 1 6 91) (25 17 1 1 1 1 6 91) (1715 17 1 1 1 1 6 91) (245 17 1 1 1 1 6 91) (35 17 1 1 1 1 6 91) (8575 17 -1 1 1 1 6 115) (1225 17 -1 1 1 1 6 83) (175 17 -1 1 1 1 6 83) (1715 17 -1 1 1 1 6 83) (245 17 -1 1 1 1 6 83) (35 17 -1 1 1 1 6 83) (654 17 -1 1 1 1 6 99) (218 17 -1 1 1 1 6 35) (327 17 -1 1 1 1 6 51) (109 17 -1 1 1 1 7 0) (1 17 -1 1 1 1 4 18) (8192 17 1 1 1 1 6 43) (4096 17 1 1 1 1 6 43) (2048 17 1 1 1 1 6 139) (1024 17 1 1 1 1 6 267) (512 17 1 1 1 1 6 267) (256 17 1 1 1 1 6 139) (128 17 1 1 1 1 6 75) (64 17 1 1 1 1 5 1036) (32 17 1 1 1 1 5 524) (16 17 1 1 1 1 5 268) (8 17 1 1 1 1 5 140) (4 17 1 1 1 1 5 76) (2 17 1 1 1 1 5 44) (1323 17 1 1 1 1 6 59) (189 17 1 1 1 1 6 59) (441 17 1 1 1 1 6 123) (63 17 1 1 1 1 6 123) (147 17 1 1 1 1 6 59) (21 17 1 1 1 1 6 59) (1323 17 -1 1 1 1 6 51) (189 17 -1 1 1 1 6 51) (441 17 -1 1 1 1 6 115) (63 17 -1 1 1 1 6 115) (147 17 -1 1 1 1 6 51) (21 17 -1 1 1 1 6 51) (135 17 1 1 1 1 6 155) (27 17 1 1 1 1 6 59) (45 17 1 1 1 1 6 59) (9 17 1 1 1 1 5 156) (15 17 1 1 1 1 5 252) (3 17 1 1 1 1 5 60) (5 17 1 1 1 1 5 92) (135 17 -1 1 1 1 6 147) (45 17 -1 1 1 1 6 51) (15 17 -1 1 1 1 4 242) (343 17 1 1 1 1 6 123) (49 17 1 1 1 1 6 123) (7 17 1 1 1 1 5 124) (117649 17 -1 1 1 1 6 115) (390625 17 -1 1 1 1 6 83) (78125 17 -1 1 1 1 6 83) (177147 17 -1 1 1 1 6 51) (59049 17 -1 1 1 1 6 147) (131072 17 -1 1 1 1 6 67) (65536 17 -1 1 1 1 6 35) (32768 17 -1 1 1 1 6 515) (16807 17 -1 1 1 1 6 115) (15625 17 -1 1 1 1 6 83) (19683 17 -1 1 1 1 6 147) (6561 17 -1 1 1 1 6 147) (16384 17 -1 1 1 1 6 515) (8192 17 -1 1 1 1 6 35) (4096 17 -1 1 1 1 6 35) (2048 17 -1 1 1 1 6 515) (2401 17 -1 1 1 1 6 115) (343 17 -1 1 1 1 6 115) (49 17 -1 1 1 1 6 115) (7 17 -1 1 1 1 4 114) (3125 17 -1 1 1 1 6 83) (625 17 -1 1 1 1 6 83) (125 17 -1 1 1 1 6 83) (25 17 -1 1 1 1 6 83) (5 17 -1 1 1 1 4 82) (2187 17 -1 1 1 1 6 51) (729 17 -1 1 1 1 6 147) (243 17 -1 1 1 1 6 51) (81 17 -1 1 1 1 6 147) (27 17 -1 1 1 1 6 51) (9 17 -1 1 1 1 4 146) (3 17 -1 1 1 1 4 50) (1024 17 -1 1 1 1 6 259) (512 17 -1 1 1 1 6 131) (256 17 -1 1 1 1 6 131) (128 17 -1 1 1 1 6 35) (64 17 -1 1 1 1 4 1026) (32 17 -1 1 1 1 4 514) (16 17 -1 1 1 1 4 258) (8 17 -1 1 1 1 4 130) (4 17 -1 1 1 1 4 66) (2 17 -1 1 1 1 4 34))"

void 
myFftw2_real(real *in, complex *out, int sizex, int sizey)
{
  FFTW_REAL    *w_in  = (FFTW_REAL    *) in;
  FFTW_COMPLEX *w_out = (FFTW_COMPLEX *) out;
  FFTW_PLAN p;

  /*  if (_wizdom_init_ == NO) {
    my_fftw_import_wisdom_from_string(_wizdom_str_);
    _wizdom_init_ = YES;
    }*/

  /* hope there will be no malloc error */
  /*  tmp = (FFTW_REAL *) malloc(sizeof(FFTW_REAL)*size);*/

  p = my_fftw_plan_dft_r2c_2d(sizex, sizey, 
			      w_in, w_out,
			      FFTW_ESTIMATE);
  
  my_fftw_execute(p);
  
  my_fftw_destroy_plan(p);
}

void
myFftw2I_real(complex *in, real *out, int sizex, int sizey)
{
  FFTW_COMPLEX *w_in  = (FFTW_COMPLEX *) in;
  FFTW_REAL    *w_out = (FFTW_REAL    *) out;

  FFTW_PLAN p;

  p = my_fftw_plan_dft_c2r_2d(sizex, sizey, 
			      w_in, w_out,
			      FFTW_ESTIMATE);

  my_fftw_execute(p);

  my_fftw_destroy_plan(p);
}

void
myFftw2I_real2(complex *in, real *out, int size)
{
  FFTW_COMPLEX *w_in  = (FFTW_COMPLEX *) in;
  FFTW_REAL    *w_out = (FFTW_REAL    *) out;

  FFTW_PLAN p;
  int i;

  if (_wizdom_init_ == NO) {
    my_fftw_import_wisdom_from_string(_wizdom_str_);
    _wizdom_init_ = YES;
  }

  p = my_fftw_plan_dft_c2r_1d(size, w_in, w_out, 
			      FFTW_ESTIMATE);
  my_fftw_execute(p);

  for (i = 0; i < size; i++) {
    w_out[i] /=size;
  }

  my_fftw_destroy_plan(p);
}

void
myFftw2_cplx(complex *in, complex *out, int sizex, int sizey)
{
  FFTW_COMPLEX *w_in  = (FFTW_COMPLEX *) in;
  FFTW_COMPLEX *w_out = (FFTW_COMPLEX *) out;
  FFTW_PLAN p;

  p = my_fftw_plan_dft_2d(sizex, sizey, 
			  w_in, w_out,
			  FFTW_FORWARD, FFTW_ESTIMATE);

  my_fftw_execute(p);

  my_fftw_destroy_plan(p);
}

void
myFftw2I_cplx(complex *in, complex *out, int sizex, int sizey)
{
  FFTW_COMPLEX *w_in  = (FFTW_COMPLEX *) in;
  FFTW_COMPLEX *w_out = (FFTW_COMPLEX *) out;
  FFTW_PLAN p;

  p = my_fftw_plan_dft_2d(sizex, sizey, 
			  w_in, w_out,
			  FFTW_BACKWARD, FFTW_ESTIMATE);
  my_fftw_execute(p);

  my_fftw_destroy_plan(p);
}

/* These functions are taken from the fftw lib. */
/*
 * Timer facility. Inspired by fftw.
 */

#define myFftw2GetTime() clock()
#define myFftw2TimeDiff(t1,t2) ((t1) - (t2))
#define myFftw2Time2Sec(t) (((double) (t)) / CLOCKS_PER_SEC)

#define myFftw2SetBeginTime() (myFftw2TimeBegin = myFftw2Time2Sec(myFftw2GetTime()))
#define myFftw2SetEndTime() (myFftw2TimeEnd = myFftw2Time2Sec(myFftw2GetTime()))
#define myFftw2GetEllapseTime() (myFftw2TimeDiff(myFftw2TimeEnd, myFftw2TimeBegin))

/*
 * ***VERY*** conservative constant: this says that a
 * measurement must run for 200ms in order to be valid.
 * You had better check the manual of your machine
 * to discover if it can do better than this
 */
#define myFftw2_TIME_MIN (20.0e-1)	/* for default clock() timer */

/* take myFftw2_TIME_REPEAT measurements... */
#define myFftw2_TIME_REPEAT 1

/* but do not run for more than myFftw2_TIME_LIMIT seconds while measuring something */
#ifndef myFftw2_TIME_LIMIT
#define myFftw2_TIME_LIMIT 2.0
#endif


static double
_my_measure_(int n)
{
  int begin, end;// start;
  double t = 0;
  double tmax, tmin;
  int i, iter;
  int repeat;
  FFTW_REAL *in;
  FFTW_COMPLEX *tmp;
  FFTW_PLAN p;

  double min_time = 60.0;

  iter = 100;

  in  = (FFTW_REAL    *) malloc (sizeof(FFTW_REAL)   * n     );
  tmp = (FFTW_COMPLEX *) malloc (sizeof(FFTW_COMPLEX)*(n/2+1));

  p = my_fftw_plan_dft_r2c_1d(n, in, tmp, FFTW_MEASURE);

  while (t < min_time) {
    begin = myFftw2GetTime();
    for (i = 0; i < iter; ++i) {
      my_fftw_execute(p);
    }
    end = myFftw2GetTime();
    t = myFftw2Time2Sec(myFftw2TimeDiff(end, begin));
    iter *= 2;
  }
  t /= (double) iter;

  free(in);
  free(tmp);
  my_fftw_destroy_plan(p);

  return t;

  p = my_fftw_plan_dft_r2c_1d(n, in, tmp, FFTW_MEASURE);
  in  = (FFTW_REAL    *) malloc (sizeof(FFTW_REAL)   * n     );
  tmp = (FFTW_COMPLEX *) malloc (sizeof(FFTW_COMPLEX)*(n/2+1));
  for (;;) {
    tmin = 1.0E10;
    tmax = -1.0E10;

    /*    start = myFftw2GetTime();*/
    /* repeat the measurement myFftw2_TIME_REPEAT times */
    for (repeat = 0; repeat < myFftw2_TIME_REPEAT; ++repeat) {
      begin = myFftw2GetTime();
      for (i = 0; i < iter; ++i) {
	my_fftw_execute(p);
      }
      end = myFftw2GetTime();

      t = myFftw2Time2Sec(myFftw2TimeDiff(end, begin));
      if (t < tmin){
	tmin = t;
      }
      if (t > tmax) {
	tmax = t;
      }

      /* do not run for too long */
      /*      t = myFftw2Time2Sec(myFftw2TimeDiff(end, start));*/
      if (tmin > myFftw2_TIME_LIMIT)
	break;
    }

    if (tmin >= myFftw2_TIME_MIN)
      break;

    iter *= 2;
  }

  tmin /= (double) iter;
  tmax /= (double) iter;

  free(in);
  free(tmp);
  my_fftw_destroy_plan(p);

  return tmin;
}

void
my_fftw2_init (int nMin, int maxN, FILE *file)
{
  int n = nMin;
  double t;
  int nb = 0;

  while (n <= maxN) {
    if (my_is_good_fftw_size(n)) {
      t =_my_measure_(n);
      fprintf(file, "  {%d, %g},\n", n, t);
      nb++;
    }
    n++;
  }
  fprintf(file, "\n#define ARR_SIZE %d\n", nb);
  fprintf(file, "\n#define _wisdom_str_ ");
  my_fftw_export_wisdom_to_file(file);

  return;
}

void
my_fftw2_init2 (int nMin, int maxN, FILE *file)
{
  int n = nMin;
  FFTW_PLAN   p;
  FFTW_PLAN  cp;

  FFTW_REAL    *r;
  FFTW_COMPLEX *c;

  while (n <= maxN) {
    if (my_is_good_fftw_size(n)) {

      p = my_fftw_plan_dft_r2c_1d (n, r, c, FFTW_MEASURE);
      my_fftw_destroy_plan(p);
      
      p = my_fftw_plan_dft_c2r_1d (n, c, r, FFTW_MEASURE);
      my_fftw_destroy_plan(p);
      
      cp = my_fftw_plan_dft_1d    (n, c, c, FFTW_FORWARD,  FFTW_MEASURE);
      my_fftw_destroy_plan(cp);
      
      cp = my_fftw_plan_dft_1d    (n, c, c, FFTW_BACKWARD, FFTW_MEASURE);
      my_fftw_destroy_plan(cp);

    }
    n++;
  }
  fprintf(file, "\n#define _wisdom_str_ \"");
  my_fftw_export_wisdom_to_file(file);
  fprintf(file, "\"");

  return;
}

real
cv2d_get_fftw_factor (int nx, int ny)
{
  real factor;

  factor = (1.0/(nx*ny));

  return factor;
}

#endif /* USE_FFTW */
