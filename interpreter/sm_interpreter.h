/*
 * sm_interpreter.h --
 *
 *   Copyright (c) 1999 Nicolas Decoster
 *   Copyright (c) 1999 Centre de Recherche Paul Pascal, Bordeaux, France
 *
 *   Copyright (c) 1999-2007 Pierre Kestener.
 *   Copyright (c) 1999-2002 Centre de Recherche Paul Pascal, Bordeaux, France.
 *   Copyright (c) 2002-2003 Ecole Normale Superieure de Lyon, Lyon, France.
 *   Copyright (c) 2003-2007 CEA DSM/DAPNIA/SEDI, centre Saclay, France.
 *
 */

#ifndef __SM_INTERPRETER__
#define __SM_INTERPRETER__

/*
 * Commands concerning the main hash table :
 */

void hash_tables_init ();
int object_info_TclCmd_  (ClientData, Tcl_Interp *, int, char **);
int delete_entry_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int object_display_description_TclCmd_  (ClientData, Tcl_Interp *, int, char **);
int object_set_description_TclCmd_  (ClientData, Tcl_Interp *, int, char **);
int get_type_TclCmd_  (ClientData, Tcl_Interp *, int, char **);

/*
 * Commands concerning the signal library :
 */

int SigOpAddCmd_         (ClientData, Tcl_Interp *, int, char **);
int SigOpMultCmd_        (ClientData, Tcl_Interp *, int, char **);
int SigOpScamultCmd_     (ClientData, Tcl_Interp *, int, char **);
int SigOpExtendCmd_      (ClientData, Tcl_Interp *, int, char **);
int SigOpFour2cplxCmd_   (ClientData, Tcl_Interp *, int, char **);
int SigOpConvolutionCmd_ (ClientData, Tcl_Interp *, int, char **);
int SigOpCutCmd_         (ClientData, Tcl_Interp *, int, char **);
int SigOpShiftCmd_       (ClientData, Tcl_Interp *, int, char **);
int SigOpNRCmd_          (ClientData, Tcl_Interp *, int, char **);

int comb_signals_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int fit_signal_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int my_fit_signal_TclCmd_ (ClientData, Tcl_Interp *, int, char **);

int create_signal_TclCmd_ (ClientData, Tcl_Interp *, int, char **);

int SigTopCopyCmd_     (ClientData, Tcl_Interp *, int, char **);
int SigTopGetValueCmd_ (ClientData, Tcl_Interp *, int, char **);
int SigTopSetValueCmd_ (ClientData, Tcl_Interp *, int, char **);
int SigTopGetSizeCmd_  (ClientData, Tcl_Interp *, int, char **);
int SigTopGetFirstCmd_ (ClientData, Tcl_Interp *, int, char **);
int SigTopGetLastCmd_  (ClientData, Tcl_Interp *, int, char **);
int get_sig_type_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int SigTopGetx0Cmd_ (ClientData, Tcl_Interp *, int, char **);
int SigTopGetdxCmd_  (ClientData, Tcl_Interp *, int, char **);
int SigTopPutx0Cmd_ (ClientData, Tcl_Interp *, int, char **);
int SigTopPutdxCmd_  (ClientData, Tcl_Interp *, int, char **);
int sig_get_index_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int sig_get_extrema_TclCmd_ (ClientData, Tcl_Interp *, int, char **);

int SigGenExprrCmd_       (ClientData, Tcl_Interp *, int, char **);
int SigGenSinCmd_         (ClientData, Tcl_Interp *, int, char **);
int SigFlatCmd_           (ClientData, Tcl_Interp *, int, char **);
int SigGenDiracCmd_       (ClientData, Tcl_Interp *, int, char **);
/*int SigGenDoubleDiracCmd_ (ClientData, Tcl_Interp *, int, char **);*/
int SigGenBoxCmd_         (ClientData, Tcl_Interp *, int, char **);
/*int SigGenCplxCmd_        (ClientData, Tcl_Interp *, int, char **);*/
int SigGenGaussCmd_       (ClientData, Tcl_Interp *, int, char **);

int real_real_to_complex_TclCmd_ (ClientData, Tcl_Interp *, int, char **);

int create_xy_TclCmd_      (ClientData, Tcl_Interp *, int, char **);
int display_realxy_TclCmd_ (ClientData, Tcl_Interp *, int, char **);

int save_signal_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int load_signal_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int SigFileLoadCmd_     (ClientData, Tcl_Interp *, int, char **);

int sig_fft_nr_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int sig_gfft_TclCmd_   (ClientData, Tcl_Interp *, int, char **);
int sig_srfft_TclCmd_   (ClientData, Tcl_Interp *, int, char **);

int sig_zoom_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int sig_get_value_lst_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int sig_2_coords_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int sig_gfft_to_2real_TclCmd_(ClientData, Tcl_Interp *, int, char **); 
int sig_2real_to_gfft_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int sig_integration_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int sig_get_info_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int sig_get_info_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int fct_signal_TclCmd_(ClientData, Tcl_Interp *, int, char **); 
int foreachs_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int sig_derivative_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int sig_2_sig_extrema_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int apply_fct_to_s_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int s_swap_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int sig_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int s_rmdisc_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int s_symfit_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int s_asymfit_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int s_thresh_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int sig_colle2sig_TclCmd_   (ClientData, Tcl_Interp *, int, char **);
int sig_merge2sig_TclCmd_   (ClientData, Tcl_Interp *, int, char **);

int get_rule_TclCmd_ (ClientData, Tcl_Interp *, int, char **);

int s_cplx_to_2_real_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int sig_pol_interp_derivative_TclCmd_(ClientData, Tcl_Interp *, int, char **);

/*
 * Commands concerning the stats library :
 */
 
int StatSigHistoCmd_ (ClientData, Tcl_Interp *, int, char **);
int StatImHistoCmd_ (ClientData, Tcl_Interp *, int, char **);
int StatImHisto2DCmd_ (ClientData, Tcl_Interp *, int, char **);
int StatIm3DHistoCmd_ (ClientData, Tcl_Interp *, int, char **);
int StatExtHistoCmd_ (ClientData, Tcl_Interp *, int, char **);
int StatExt3DHistoCmd_ (ClientData, Tcl_Interp *, int, char **);
int StatExtHisto2Cmd_ (ClientData, Tcl_Interp *, int, char **);
int StatLineHistoCmd_ (ClientData, Tcl_Interp *, int, char **);
int stat_x_line_size_histo_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int stat_n_line_histo_TclCmd_ (ClientData, Tcl_Interp *, int, char **);

int s_histo_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int i_histo_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int e_histo_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
 
/*
 * Commands concerning the image library :
 */

int im_real_to_complex_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int im_mult_TclCmd_            (ClientData, Tcl_Interp *, int, char **);
int im_toto_TclCmd_            (ClientData, Tcl_Interp *, int, char **);
int im_scalar_mult_TclCmd_     (ClientData, Tcl_Interp *, int, char **);
int im_scalar_add_TclCmd_      (ClientData, Tcl_Interp *, int, char **);
int im_add_TclCmd_             (ClientData, Tcl_Interp *, int, char **);
int im_max_TclCmd_             (ClientData, Tcl_Interp *, int, char **);
int im_filter_TclCmd_          (ClientData, Tcl_Interp *, int, char **);
int im_conv_TclCmd_            (ClientData, Tcl_Interp *, int, char **);
int ImaCrt2GradModCmd_         (ClientData, Tcl_Interp *, int, char **);
int ImaCrt2GradArgCmd_         (ClientData, Tcl_Interp *, int, char **);
int ImaCvlCutEdgeCmd_          (ClientData, Tcl_Interp *, int, char **);

int im_create_TclCmd_          (ClientData, Tcl_Interp *, int, char **);
int im_insert_TclCmd_          (ClientData, Tcl_Interp *, int, char **);
int im_set_border_TclCmd_      (ClientData, Tcl_Interp *, int, char **);

int im_diamond_TclCmd_     (ClientData, Tcl_Interp *, int, char **);
int im_dirac_TclCmd_       (ClientData, Tcl_Interp *, int, char **);
int im_gauss_TclCmd_       (ClientData, Tcl_Interp *, int, char **);
int im_gauss_ellipse_TclCmd_   (ClientData, Tcl_Interp *, int, char **);
int im_cell_TclCmd_        (ClientData, Tcl_Interp *, int, char **);
int im_cell_elli_TclCmd_        (ClientData, Tcl_Interp *, int, char **);
int im_cell3Dproj_TclCmd_        (ClientData, Tcl_Interp *, int, char **);
int im_step_TclCmd_        (ClientData, Tcl_Interp *, int, char **);
int im_circle_TclCmd_      (ClientData, Tcl_Interp *, int, char **);
int im_ellipse_TclCmd_     (ClientData, Tcl_Interp *, int, char **);
int my_im_ellipse_TclCmd_  (ClientData, Tcl_Interp *, int, char **);
int my_im_ellipse_3Dproj_TclCmd_  (ClientData, Tcl_Interp *, int, char **);
int im_cell_ellipsoid_proj_TclCmd_  (ClientData, Tcl_Interp *, int, char **);
int im_cell_ellipsoid_slice_TclCmd_  (ClientData, Tcl_Interp *, int, char **);
int im_brownian_TclCmd_    (ClientData, Tcl_Interp *, int, char **);
int im_brownian_2D_field_TclCmd_    (ClientData, Tcl_Interp *, int, char **);
int im_brownian3D_TclCmd_  (ClientData, Tcl_Interp *, int, char **);
int im_dirac3D_TclCmd_     (ClientData, Tcl_Interp *, int, char **);
int im_test3D_TclCmd_      (ClientData, Tcl_Interp *, int, char **);
int im_test3D_vector_TclCmd_      (ClientData, Tcl_Interp *, int, char **);
int im_gauss3D_TclCmd_     (ClientData, Tcl_Interp *, int, char **);
int im_singul3D_TclCmd_    (ClientData, Tcl_Interp *, int, char **);
int im_Urand_TclCmd_       (ClientData, Tcl_Interp *, int, char **);
int im_vortex_TclCmd_      (ClientData, Tcl_Interp *, int, char **);
int im_Grand_TclCmd_       (ClientData, Tcl_Interp *, int, char **);
int im3D_Grand_TclCmd_     (ClientData, Tcl_Interp *, int, char **);
int im3D_vector_Grand_TclCmd_     (ClientData, Tcl_Interp *, int, char **);
int im_Prim_TclCmd_        (ClientData, Tcl_Interp *, int, char **);
int im_dla_TclCmd_         (ClientData, Tcl_Interp *, int, char **);
int im_white_noise_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int ImaSnfSnowFlakeCmd_    (ClientData, Tcl_Interp *, int, char **);
int ImaSnfSuperSnowFlakeCmd_ (ClientData, Tcl_Interp *, int, char **);
int im_null_TclCmd_        (ClientData, Tcl_Interp *, int, char **);

int ImaTopCopyCmd_      (ClientData,Tcl_Interp*,int,char**);
int Ima3DTopCopyCmd_    (ClientData,Tcl_Interp*,int,char**);
int ImaTopInfoCmd_      (ClientData,Tcl_Interp*,int,char**);
int ImaTopGetValueCmd_  (ClientData,Tcl_Interp*,int,char**);
int ImaTopGetSizeCmd_   (ClientData,Tcl_Interp*,int,char**);
int ImaTopGetminmaxCmd_ (ClientData,Tcl_Interp*,int,char**);
int ImaTopGetavgCmd_    (ClientData,Tcl_Interp*,int,char**);
int Ima3DTopGetminmaxCmd_ (ClientData,Tcl_Interp*,int,char**);
int ImaGetlxCmd_        (ClientData,Tcl_Interp*,int,char**);
int ImaGetlyCmd_        (ClientData,Tcl_Interp*,int,char**);

int ImaTopFftFilterCmd_ (ClientData,Tcl_Interp*,int,char**);

int ImaNuaLoadCmd_    (ClientData,Tcl_Interp*,int,char**);
int ImaSimNuaLoadCmd_ (ClientData,Tcl_Interp*,int,char**);

int ImaCvlConvolutionCmd_ (ClientData,Tcl_Interp*,int,char**);
int ImaFftwFilterCmd_     (ClientData,Tcl_Interp*,int,char**);
int ImaFftw3DFilterCmd_   (ClientData,Tcl_Interp*,int,char**);

int ImaFftImageToFftCmd_ (ClientData,Tcl_Interp*,int,char**);
int ImaFftImageToFft2Cmd_ (ClientData,Tcl_Interp*,int,char**);
int ImaFftFftToImageCmd_ (ClientData,Tcl_Interp*,int,char**);
int ImaFftFftCmd_        (ClientData,Tcl_Interp*,int,char**);

int im_gfft2d_real_TclCmd_ (ClientData,Tcl_Interp*,int,char**);
int im_fftw2d_real_TclCmd_ (ClientData,Tcl_Interp*,int,char**);
int im_fftw3d_real_TclCmd_ (ClientData,Tcl_Interp*,int,char**);

int ImaFileSaveCmd_ (ClientData,Tcl_Interp*,int,char**);
int ImaFileLoadCmd_ (ClientData,Tcl_Interp*,int,char**);
int ImaFileLoadHdf5Cmd_ (ClientData,Tcl_Interp*,int,char**);
int Ima3DFileLoadCmd_ (ClientData,Tcl_Interp*,int,char**);
int Ima3DSaveFileCmd_ (ClientData,Tcl_Interp*,int,char**);

int ImaColorizeRectangleCmd_ (ClientData,Tcl_Interp*,int,char**);
int ImaColorizeRectangle2Cmd_ (ClientData,Tcl_Interp*,int,char**);
int Ima_g_rond_f_Cmd_ (ClientData,Tcl_Interp*,int,char**);
int AddTrajBillesLatexCmd_ (ClientData,Tcl_Interp*,int,char**);

int ImaAdnLoadCmd_ (ClientData, Tcl_Interp *, int, char **);

int apply_fct_to_i_TclCmd_(ClientData, Tcl_Interp *, int, char **); 
int apply_fct_to_i_vector_TclCmd_(ClientData, Tcl_Interp *, int, char **); 
int apply_fct_to_i3D_TclCmd_(ClientData, Tcl_Interp *, int, char **); 
int apply_fct_to_i3D_vector_TclCmd_(ClientData, Tcl_Interp *, int, char **); 
int im_zoom_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int im_swap_raws_TclCmd_(ClientData, Tcl_Interp *, int, char **); 
int im_swap_cols_TclCmd_(ClientData, Tcl_Interp *, int, char **); 
int im_mirror_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int im_cut_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int comb_images_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int myifilter_image_TclCmd_(ClientData, Tcl_Interp *, int, char **);

int itrace_rectangle_TclCmd_(ClientData, Tcl_Interp *, int, char **);
int itrace_segment_TclCmd_(ClientData, Tcl_Interp *, int, char **);
int itrace_curve_TclCmd_(ClientData, Tcl_Interp *, int, char **);
int iblackbox_TclCmd_(ClientData, Tcl_Interp *, int, char **);

int icomb_line_TclCmd_(ClientData, Tcl_Interp *, int, char **);

int iadd_ima_TclCmd_(ClientData, Tcl_Interp *, int, char **);
int imax_ima_TclCmd_(ClientData, Tcl_Interp *, int, char **);

int iadd_border_TclCmd_(ClientData, Tcl_Interp *, int, char **);
int icopy_raw_TclCmd_(ClientData, Tcl_Interp *, int, char **);
int iconvert_size_TclCmd_(ClientData, Tcl_Interp *, int, char **);

int im_gfft_to_2real_TclCmd_(ClientData, Tcl_Interp *, int, char **);
 
int im_nr_to_2real_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int im_swap_part_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int im_insert_sig_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int im_get_info_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int foreachi_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int create_im_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int dxbox_conv_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int isp_2_ssp_TclCmd_(ClientData, Tcl_Interp *, int, char **);

int my_isp_2_ssp_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int ipowspec_TclCmd_ (ClientData, Tcl_Interp *, int, char **);

int i3Dpowspec_TclCmd_ (ClientData, Tcl_Interp *, int, char **);

int iicut_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int im_thresh_TclCmd_          (ClientData, Tcl_Interp *, int, char **);

int im_thresh_3D_TclCmd_          (ClientData, Tcl_Interp *, int, char **);

int isubsample_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int i_apply_ipa_table_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int i_inverse_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int i_angular_mean_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int vector2D_2_vtk_structured_points_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int vector2D_2_jvx_vectorField_TclCmd_ (ClientData, Tcl_Interp *, int, char **);int vector2D_2_flow_vis_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int comb_images_3D_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int iicut_3D_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int im_appodisation_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int im_3D_appodisation_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int image_moment_of_log_increments_TclCmd_ (ClientData, Tcl_Interp *, int, char **);

/*
 * Commands concerning the math morphology library :
 */

int morph_toto_TclCmd_    (ClientData, Tcl_Interp *, int, char **);
int morph_main_TclCmd_    (ClientData, Tcl_Interp *, int, char **);

/*
 * Commands concerning the wt1d library :
 */

int create_wavelet_TclCmd_    (ClientData, Tcl_Interp *, int, char **);
int wavelet_to_signal_TclCmd_    (ClientData, Tcl_Interp *, int, char **);
int tab_scales_TclCmd_    (ClientData, Tcl_Interp *, int, char **);
int wt1d_single_TclCmd_    (ClientData, Tcl_Interp *, int, char **);
int wt1d_all_TclCmd_    (ClientData, Tcl_Interp *, int, char **);
int get_scales_TclCmd_    (ClientData, Tcl_Interp *, int, char **);
int get_wavelet_from_collection_TclCmd_    (ClientData, Tcl_Interp *, int, char **);

/*
 * Commands concerning the wt2d library :
 */

int ExtChnChainCmd_  (ClientData, Tcl_Interp *, int, char **);
int ExtChnThreshCmd_ (ClientData, Tcl_Interp *, int, char **);


void      ExtDicUnlinkStamp_(int);
int       ExtDicUnlinkCmd_  (ClientData, Tcl_Interp *, int, char **);
int       ExtDicNameCmd_    (ClientData,Tcl_Interp *,int ,char **);      

int ExtFileSaveCmd_       (ClientData, Tcl_Interp *, int, char **);
int ExtFileLoadCmd_       (ClientData, Tcl_Interp *, int, char **);
int ExtFileLoad3DDisplayCmd_ (ClientData, Tcl_Interp *, int, char **);
int ExtFile3DDecimationCmd_ (ClientData, Tcl_Interp *, int, char **);
int ExtFileChainSaveCmd_  (ClientData, Tcl_Interp *, int, char **);

int search_lines_TclCmd_    (ClientData, Tcl_Interp *, int, char **);
int search_lines_with_diam_TclCmd_    (ClientData, Tcl_Interp *, int, char **);
int compute_bending_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int lines_to_chains_TclCmd_ (ClientData, Tcl_Interp *, int, char **);

int vertical_chain_TclCmd_  (ClientData,Tcl_Interp*,int,char**);

int lscreate_TclCmd_  (ClientData, Tcl_Interp *, int, char**);
int lsdestroy_TclCmd_ (ClientData, Tcl_Interp *, int, char**);
int lsadd_TclCmd_     (ClientData, Tcl_Interp *, int, char**);
int lsget_TclCmd_     (ClientData, Tcl_Interp *, int, char**);
int lsremove_TclCmd_  (ClientData, Tcl_Interp *, int, char**);
int lsempty_TclCmd_   (ClientData, Tcl_Interp *, int, char**);
int lsaff_TclCmd_     (ClientData, Tcl_Interp *, int, char**);

int ExtExtComputeCmd_ (ClientData, Tcl_Interp *, int, char **);      
int ExtExtCaracCmd_   (ClientData, Tcl_Interp *, int, char **);      
int w2_local_maxima_TclCmd_ (ClientData, Tcl_Interp *, int, char **);      
int w2_local_space_scale_maxima_TclCmd_ (ClientData, Tcl_Interp *, int, char **);      
int merge_ext_image_TclCmd_ (ClientData, Tcl_Interp *, int, char **);      
int gather_ext_image_TclCmd_ (ClientData, Tcl_Interp *, int, char **);      

int w2_follow_contour_TclCmd_ (ClientData, Tcl_Interp *, int, char **);      
int w2_follow2_contour_TclCmd_ (ClientData, Tcl_Interp *, int, char **);      
int w2_wtmm2d_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int w3_wtmm3d_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int w2_set_with_images_TclCmd_ (ClientData, Tcl_Interp *, int, char **);      

int gkapa_TclCmd_      (ClientData,Tcl_Interp *,int,char**);
int gkapap_TclCmd_     (ClientData,Tcl_Interp *,int,char**);

int get_line_stats_TclCmd_     (ClientData,Tcl_Interp *,int,char**);

int w2_remove_extrema_TclCmd_     (ClientData,Tcl_Interp *,int,char**);
int w2_keep_isolated_TclCmd_     (ClientData,Tcl_Interp *,int,char**);
int w2_keep_circle_TclCmd_     (ClientData,Tcl_Interp *,int,char**);
int w2_keep_circle_simple_TclCmd_     (ClientData,Tcl_Interp *,int,char**);
int w2_log_TclCmd_     (ClientData,Tcl_Interp *,int,char**);
int w2_remove_line_TclCmd_     (ClientData,Tcl_Interp *,int,char**);
int w2_remove_lines_TclCmd_     (ClientData,Tcl_Interp *,int,char**);
int w2_remove_lines_by_size_TclCmd_     (ClientData,Tcl_Interp *,int,char**);
int w2_remove_lines_by_mean_mod_TclCmd_     (ClientData,Tcl_Interp *,int,char**);
int w2_remove_lines_by_arg_TclCmd_     (ClientData,Tcl_Interp *,int,char**);

int mask_ext_image_TclCmd_     (ClientData,Tcl_Interp *,int,char**);
int mask2_ext_image_TclCmd_     (ClientData,Tcl_Interp *,int,char**);
int w2_keep_by_value_TclCmd_     (ClientData,Tcl_Interp *,int,char**);

int PartFqaqtqCmd_(ClientData, Tcl_Interp *, int, char **); 
int PartFqaqtqIICmd_(ClientData, Tcl_Interp *, int, char **); 
int PartFqaqtqIIICmd_(ClientData, Tcl_Interp *, int, char **); 
int PartSavePartCmd_(ClientData, Tcl_Interp *, int, char **); 
int PartFqaqtqIVCmd_(ClientData, Tcl_Interp *, int, char **); 
int apply_fct_to_e_TclCmd_(ClientData, Tcl_Interp *, int, char **); 
int apply_fct_to_e_tsallis_TclCmd_(ClientData, Tcl_Interp *, int, char **); 
int apply_fct_to_extima3D_TclCmd_(ClientData, Tcl_Interp *, int, char **); 
int apply_fct_to_data3D_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int update_ext_image_TclCmd_ (ClientData, Tcl_Interp *, int, char **); 

int line_2_max_TclCmd_ (ClientData, Tcl_Interp *, int, char **); 

int search_single_max_TclCmd_ (ClientData, Tcl_Interp *, int, char **); 

int mlm_vert_chain_TclCmd_ (ClientData, Tcl_Interp *, int, char **); 
int pt_to_pt_vert_chain_TclCmd_ (ClientData, Tcl_Interp *, int, char **); 
int purge_chains_TclCmd_ (ClientData, Tcl_Interp *, int, char **); 

int vert_chain_to_s_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int get_best_max_on_vc_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int ext_get_info_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int foreache_TclCmd_(ClientData, Tcl_Interp *, int, char **); 
int gr_loop_TclCmd_(ClientData, Tcl_Interp *, int, char **); 
int line_loop_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int line_to_s_TclCmd_(ClientData, Tcl_Interp *, int, char **); 
int e_cut_TclCmd_(ClientData, Tcl_Interp *, int, char **); 

int name_line_TclCmd_(ClientData,Tcl_Interp *, int, char **);

int xysig_2_ei_TclCmd_ (ClientData,Tcl_Interp *, int, char **);
int my_xysig_2_ei_TclCmd_ (ClientData,Tcl_Interp *, int, char **);
int ei_save_lines_TclCmd_ (ClientData,Tcl_Interp *, int, char **);
int ei_save_lines_2_TclCmd_ (ClientData,Tcl_Interp *, int, char **);
int ei_save_skel_vtk_TclCmd_ (ClientData,Tcl_Interp *, int, char **);
int ei_save_skel_jvx_TclCmd_ (ClientData,Tcl_Interp *, int, char **);
int ei_save_skel_m3d_TclCmd_ (ClientData,Tcl_Interp *, int, char **);
int ei_save_skel_chain_vtk_TclCmd_ (ClientData,Tcl_Interp *, int, char **);
int ei_save_extimage_4_vtk_TclCmd_ (ClientData,Tcl_Interp *, int, char **);

int sw_sig_2_sig_extrema_TclCmd_ (ClientData,Tcl_Interp *, int, char **);

int egetextr_TclCmd_ (ClientData,Tcl_Interp *, int, char **);
int egetmin_TclCmd_ (ClientData,Tcl_Interp *, int, char **);
int egetmax_TclCmd_ (ClientData,Tcl_Interp *, int, char **);

int e_mult_by_i_TclCmd_ (ClientData,Tcl_Interp *, int, char **);

int e_draw_in_canvas_TclCmd_ (ClientData,Tcl_Interp *, int, char **);

int special_tag_lines_TclCmd_ (ClientData,Tcl_Interp *, int, char **);

int e_correlation_TclCmd_ (ClientData,Tcl_Interp *, int, char **);
int e_correlation2_TclCmd_ (ClientData,Tcl_Interp *, int, char **);

int e_tag_vc_TclCmd_ (ClientData,Tcl_Interp *, int, char **);

int e_vc_histo_TclCmd_ (ClientData,Tcl_Interp *, int, char **);

int e_vc_histo2_TclCmd_ (ClientData,Tcl_Interp *, int, char **);

int e_vc_gerbe_TclCmd_ (ClientData,Tcl_Interp *, int, char **);

int e_save_tagged_TclCmd_ (ClientData,Tcl_Interp *, int, char **);

int e_count_tag_TclCmd_ (ClientData,Tcl_Interp *, int, char **);

int e_save_skeleton_TclCmd_ (ClientData,Tcl_Interp *, int, char **);

/*
 * Commands concerning the wt3d library :
 */

int     Ext3DDicUnlinkCmd_        (ClientData, Tcl_Interp *, int, char **);
int     ExtFileLoadCmd_3Dsmall_   (ClientData, Tcl_Interp *, int, char **);
int     ExtFileSaveCmd_3Dsmall_   (ClientData, Tcl_Interp *, int, char **);
int     apply_fct_to_e3Dsmall_TclCmd_   (ClientData, Tcl_Interp *, int, char **);
int     apply_fct_to_e3Dsmall2_TclCmd_   (ClientData, Tcl_Interp *, int, char **);
int     percent_of_identity_e3Dsmall_TclCmd_   (ClientData, Tcl_Interp *, int, char **);
int     pt_to_pt_vert_chain_3Dsmall_TclCmd_   (ClientData, Tcl_Interp *, int, char **);
int     e3Dsmall_getextr_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int     StatExt3DsmallHistoCmd_  (ClientData, Tcl_Interp *, int, char **);
int     ei_3Dsmall_save_4_vtk_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int     ei_3Dsmall_save_4_vtk_polydata_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int     ei3Dsmall_save_skel_vtk_TclCmd_ (ClientData, Tcl_Interp *, int, char **); 
int     e_cut_3Dsmall_TclCmd_ (ClientData, Tcl_Interp *, int, char **); 
int     vert_chain_3Dsmall_to_s_TclCmd_ (ClientData, Tcl_Interp *, int, char **); 
int     e_vc_gerbe_3Dsmall_TclCmd_ (ClientData, Tcl_Interp *, int, char **); 
int     ei3Dsmall_correlation2_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int     buffer3D_2_vtk_structured_points_TclCmd_ (ClientData, Tcl_Interp *, int, char **); 
int     i3D_2_vtk_structured_points_TclCmd_ (ClientData, Tcl_Interp *, int, char **); 
int     vector3D_2_vtk_structured_points_TclCmd_ (ClientData, Tcl_Interp *, int, char **); 
int     ei3Dsmall_loop_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
int     e_tag_vc_3Dsmall_TclCmd_ (ClientData, Tcl_Interp *, int, char **);
/*
 * Commands concerning the files manipulation :
 */

int xv_file_TclCmd_ (ClientData, Tcl_Interp *, int, char **); 
int i_to_ps_TclCmd_ (ClientData, Tcl_Interp *, int, char **); 
int e_to_ps_TclCmd_ (ClientData, Tcl_Interp *, int, char **); 
int rectangle_to_ps_TclCmd_ (ClientData, Tcl_Interp *, int, char **); 
int ntime_TclCmd_ (ClientData, Tcl_Interp *, int, char **); 

/*
 * Commands concerning lw :
 */

/*int Lw_ExecCmd_TclCmd_ (ClientData, Tcl_Interp *, int, char **); 
int Lw_Init_TclCmd_ (ClientData, Tcl_Interp *, int, char **); */

#endif /* __SM_INTERPRETER__ */
