#ifndef FFTW3_INC_H_
#define FFTW3_INC_H_

#include <fftw3.h>

typedef float FFTW_REAL;
typedef fftwf_complex FFTW_COMPLEX;
typedef fftwf_plan    FFTW_PLAN;
#define my_fftw_plan_dft_r2c_1d fftwf_plan_dft_r2c_1d
#define my_fftw_plan_dft_c2r_1d fftwf_plan_dft_c2r_1d
#define my_fftw_plan_dft_r2c_2d fftwf_plan_dft_r2c_2d
#define my_fftw_plan_dft_c2r_2d fftwf_plan_dft_c2r_2d
#define my_fftw_plan_dft_r2c_3d fftwf_plan_dft_r2c_3d
#define my_fftw_plan_dft_c2r_3d fftwf_plan_dft_c2r_3d

#define my_fftw_plan_dft_1d fftwf_plan_dft_1d
#define my_fftw_plan_dft_2d fftwf_plan_dft_2d

#define my_fftw_destroy_plan fftwf_destroy_plan
#define my_fftw_execute fftwf_execute
#define my_fftw_cleanup fftwf_cleanup

#define my_fftw_plan_with_nthreads fftwf_plan_with_nthreads
#define my_fftw_init_threads fftwf_init_threads 

#define FFTW_WISDOM_FILENAME ("fftwf_wisdom.txt")
#define my_fftw_import_wisdom_from_file fftwf_import_wisdom_from_file 
#define my_fftw_export_wisdom_to_file fftwf_export_wisdom_to_file 
#define my_fftw_import_wisdom_from_string fftwf_import_wisdom_from_string
#define my_fftw_export_wisdom_to_string fftwf_export_wisdom_to_string
#define my_fftw_free fftwf_free

/* in case we switch to double some day */
/*
typedef double FFTW_REAL;
typedef fftw_complex FFTW_COMPLEX;
typedef fftw_plan    FFTW_PLAN;
#define my_fftw_plan_dft_r2c_1d fftw_plan_dft_r2c_1d
#define my_fftw_plan_dft_c2r_1d fftw_plan_dft_c2r_1d
#define my_fftw_plan_dft_r2c_2d fftw_plan_dft_r2c_2d
#define my_fftw_plan_dft_c2r_2d fftw_plan_dft_c2r_2d
#define my_fftw_plan_dft_r2c_3d fftw_plan_dft_r2c_3d
#define my_fftw_plan_dft_c2r_3d fftw_plan_dft_c2r_3d

#define my_fftw_plan_dft_1d fftw_plan_dft_1d
#define my_fftw_plan_dft_2d fftw_plan_dft_2d

#define my_fftw_destroy_plan fftw_destroy_plan
#define my_fftw_execute fftw_execute
#define my_fftw_cleanup fftw_cleanup

#define my_fftw_plan_with_nthreads fftw_plan_with_nthreads
#define my_fftw_init_threads fftw_init_threads 

#define FFTW_WISDOM_FILENAME ("fftw_wisdom.txt")
#define my_fftw_import_wisdom_from_file fftw_import_wisdom_from_file 
#define my_fftw_export_wisdom_to_file fftw_export_wisdom_to_file
#define my_fftw_import_wisdom_from_string fftw_import_wisdom_from_string
#define my_fftw_export_wisdom_to_string fftw_export_wisdom_to_string
#define my_fftw_free fftw_free
 */


#endif /* FFTW3_INC_H_ */
