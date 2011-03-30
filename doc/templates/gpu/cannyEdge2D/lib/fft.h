#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void im_fftw2d_real (float *in, float *out, int Lx, int Ly, int direction, int inplace);
void
im_fftw_filter (float  *data,
		int Lx,
		int Ly,
		float   scale,
		void *r_fct,
		void *i_fct);


#ifdef __cplusplus
}
#endif /* __cplusplus */
