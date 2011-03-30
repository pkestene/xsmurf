#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * These routines is usefull to fit data into the 
 * format used by rfftwnd_one_real_to_complex 
 */ 
/* recall memmove prototype :
 *   void *memmove(void *dest, const void *src, size_t n);
 */
  void Shift_Buffer_2D(float *indata, int M, int N) ;

  void Shift_Inv_Buffer_2D(float *indata, int M, int N);

  void im_fftw_filter_gradx (float  *data,
			     int Lx,
			     int Ly,
			     float   scale);

  int SaveBuffer4xsmurf2D_extimage(void *bufferOut, void *bufferOut2,
				   int lx, int ly, char *extImageFilename,
				   float coef);

  
#ifdef __cplusplus
}
#endif /* __cplusplus */
