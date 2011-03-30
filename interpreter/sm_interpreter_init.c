/*
 * sm_interpreter_init.c --
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

#include <tcl.h>
#include "sm_interpreter.h"
#include "arguments.h"

/*#include <defunc.h>*/
/*#include <matheval.h>*/
#include <math.h>


/*
 * Time counter.
 */

float smTimeBegin;
float smTimeEnd;


/*
 * Functions from the cv1d package.
 */

extern int Cv1d_pkgInit (Tcl_Interp * interp);

/*
 * Functions from the cv2d package.
 */

extern int Cv2d_pkgInit (Tcl_Interp * interp);


/*
 * Functions from the steph package.
 */

extern int Steph_pkgInit (Tcl_Interp * interp);


/*
 * Functions from the dyadique package.
 */

extern int dyad_pkgInit (Tcl_Interp * interp);


/*
 */

/*static void _math_expr_init_ ();*/
int cmds_list_TclCmd_ (ClientData clientData, Tcl_Interp *interp,
		       int argc, char **argv);


/*
 * The following structure defines all of the commands in the smurf core,
 * and the C procedures that execute them.
 */

typedef struct {
  char        *name;		/* Name of command. */
  Tcl_CmdProc *proc;		/* Procedure that executes command. */
} cmd_info;


/*
 * Built-in commands, and the procedures associated with them:
 */

static cmd_info smurf_cmds[] = {

  {"cmdlist",    (Tcl_CmdProc *) cmds_list_TclCmd_},
  {"ntime",      (Tcl_CmdProc *) ntime_TclCmd_},

  /*
   * Commands concerning the hash tables :
   */

  {"ginfo",      (Tcl_CmdProc *) object_info_TclCmd_},
  {"delete",     (Tcl_CmdProc *) delete_entry_TclCmd_},
  /* the 3 following commands are for backwards compability */
  /*  {"idelete",     delete_entry_TclCmd_},
  {"edelete",     delete_entry_TclCmd_},
  {"sdelete",     delete_entry_TclCmd_},*/

  {"describe",   (Tcl_CmdProc *) object_display_description_TclCmd_},
  {"setdesc",    (Tcl_CmdProc *) object_set_description_TclCmd_},
  {"gettype",    (Tcl_CmdProc *) get_type_TclCmd_},

  /*
   * End of the smurf comands.
   */
  {NULL, (Tcl_CmdProc *) NULL}
};


/*
 * Commands concerning the signal library :
 */

static cmd_info signal_cmds[] = {

  {"sfft",      (Tcl_CmdProc *) sig_fft_nr_TclCmd_},
  {"gfft",      (Tcl_CmdProc *) sig_gfft_TclCmd_},
  {"srfft",     (Tcl_CmdProc *) sig_srfft_TclCmd_},

  {"ssave",     (Tcl_CmdProc *) save_signal_TclCmd_},
  {"sload",     (Tcl_CmdProc *) load_signal_TclCmd_},

  {"screate",   (Tcl_CmdProc *) create_signal_TclCmd_},

  {"scopy",     (Tcl_CmdProc *) SigTopCopyCmd_},
  {"sget",      (Tcl_CmdProc *) SigTopGetValueCmd_},
  {"sset",      (Tcl_CmdProc *) SigTopSetValueCmd_},
  {"ssize",     (Tcl_CmdProc *) SigTopGetSizeCmd_},
  {"sfirst",    (Tcl_CmdProc *) SigTopGetFirstCmd_},
  {"slast",     (Tcl_CmdProc *) SigTopGetLastCmd_},
  {"sgettype",  (Tcl_CmdProc *) get_sig_type_TclCmd_},
  {"sgetdx",       (Tcl_CmdProc *) SigTopGetdxCmd_},
  {"sgetx0",       (Tcl_CmdProc *) SigTopGetx0Cmd_},
  {"sputdx",       (Tcl_CmdProc *) SigTopPutdxCmd_},
  {"sputx0",       (Tcl_CmdProc *) SigTopPutx0Cmd_},
  {"sgetindex", (Tcl_CmdProc *) sig_get_index_TclCmd_},
  {"sgetextr", (Tcl_CmdProc *) sig_get_extrema_TclCmd_},

  {"sshift",    (Tcl_CmdProc *) SigOpShiftCmd_},
  {"scut",      (Tcl_CmdProc *) SigOpCutCmd_},
  {"snr",       (Tcl_CmdProc *) SigOpNRCmd_},
  {"sadd",      (Tcl_CmdProc *) SigOpAddCmd_},
  {"smult",     (Tcl_CmdProc *) SigOpMultCmd_},
  {"sscamult",  (Tcl_CmdProc *) SigOpScamultCmd_},
  {"extend",    (Tcl_CmdProc *) SigOpExtendCmd_},
  {"four2cplx", (Tcl_CmdProc *) SigOpFour2cplxCmd_},
  {"sconv",     (Tcl_CmdProc *) SigOpConvolutionCmd_},

  {"scomb",     (Tcl_CmdProc *) comb_signals_TclCmd_},

  {"sfit",      (Tcl_CmdProc *) fit_signal_TclCmd_},
  {"my_sfit",   (Tcl_CmdProc *) my_fit_signal_TclCmd_},
  {"rr2c",      (Tcl_CmdProc *) real_real_to_complex_TclCmd_},

  {"sinus",     (Tcl_CmdProc *) SigGenSinCmd_},
  {"exprr",     (Tcl_CmdProc *) SigGenExprrCmd_},
  {"sdirac",    (Tcl_CmdProc *) SigGenDiracCmd_},
  {"sflat",     (Tcl_CmdProc *) SigFlatCmd_},
  /*  {"sddirac",   (Tcl_CmdProc *) SigGenDoubleDiracCmd_},*/
  {"box",       (Tcl_CmdProc *) SigGenBoxCmd_},
  /*  {"cplx",      (Tcl_CmdProc *) SigGenCplxCmd_},*/
  {"gauss",     (Tcl_CmdProc *) SigGenGaussCmd_},

  {"crxy",      (Tcl_CmdProc *) create_xy_TclCmd_},
  {"xy",        (Tcl_CmdProc *) display_realxy_TclCmd_},

  {"szoom",     (Tcl_CmdProc *) sig_zoom_TclCmd_},

  {"sgetlst",   (Tcl_CmdProc *) sig_get_value_lst_TclCmd_},

  {"sig2coords", (Tcl_CmdProc *) sig_2_coords_TclCmd_},

  {"sgfft2ri",   (Tcl_CmdProc *) sig_gfft_to_2real_TclCmd_},
  {"sri2gfft",   (Tcl_CmdProc *) sig_2real_to_gfft_TclCmd_},

  {"sintegrate", (Tcl_CmdProc *) sig_integration_TclCmd_},

  {"sinfo",      (Tcl_CmdProc *) sig_get_info_TclCmd_},

  {"s2fs",       (Tcl_CmdProc *) fct_signal_TclCmd_},

  {"foreachs",   (Tcl_CmdProc *) foreachs_TclCmd_},
  {"sigloop",   (Tcl_CmdProc *) foreachs_TclCmd_},

  {"sderiv",     (Tcl_CmdProc *) sig_derivative_TclCmd_},

  {"s2ext",      (Tcl_CmdProc *) sig_2_sig_extrema_TclCmd_},

  {"sfct",       (Tcl_CmdProc *) apply_fct_to_s_TclCmd_},

  {"sswap",      (Tcl_CmdProc *) s_swap_TclCmd_},

  {"sig",        (Tcl_CmdProc *) sig_TclCmd_},

  {"rmdisc",     (Tcl_CmdProc *) s_rmdisc_TclCmd_},

  {"ssymfit",     (Tcl_CmdProc *) s_symfit_TclCmd_},

  {"sasymfit",     (Tcl_CmdProc *) s_asymfit_TclCmd_},

  {"sthresh",     (Tcl_CmdProc *) s_thresh_TclCmd_},

  {"scolle",    (Tcl_CmdProc *) sig_colle2sig_TclCmd_}, 
  {"smerge",    (Tcl_CmdProc *) sig_merge2sig_TclCmd_}, 

  {"getrule",    (Tcl_CmdProc *) get_rule_TclCmd_}, 

  {"sc2ri",	  (Tcl_CmdProc *) s_cplx_to_2_real_TclCmd_}, 
  {"sderiv_int",  (Tcl_CmdProc *) sig_pol_interp_derivative_TclCmd_}, 

  /*
   * End of the signal comands.
   */
  {NULL, (Tcl_CmdProc *) NULL}
};


/*
 * Commands concerning the stats library :
 */

static cmd_info stats_cmds[] = {
  
  {"shisto",     (Tcl_CmdProc *) StatSigHistoCmd_},
  {"ihisto",     (Tcl_CmdProc *) StatImHistoCmd_},
  {"ihisto2d",   (Tcl_CmdProc *) StatImHisto2DCmd_},
  {"i3Dhisto",   (Tcl_CmdProc *) StatIm3DHistoCmd_},
  {"ehisto",     (Tcl_CmdProc *) StatExtHistoCmd_},
  {"ehisto3D",   (Tcl_CmdProc *) StatExt3DHistoCmd_},
  {"e2histo",    (Tcl_CmdProc *) StatExtHisto2Cmd_},
  {"lhisto",     (Tcl_CmdProc *) StatLineHistoCmd_},
  {"elshisto",   (Tcl_CmdProc *) stat_x_line_size_histo_TclCmd_},
  {"el2shisto",  (Tcl_CmdProc *) stat_n_line_histo_TclCmd_},

  {"shisto2",    (Tcl_CmdProc *) s_histo_TclCmd_},
  {"ihisto2",    (Tcl_CmdProc *) i_histo_TclCmd_},
  {"ehisto2",    (Tcl_CmdProc *) e_histo_TclCmd_},

  /*
   * End of the stats comands.
   */
  {NULL, (Tcl_CmdProc *) NULL}
};


/*
 * Commands concerning the image library :
 */

static cmd_info image_cmds[] = {

  {"ir2c",       (Tcl_CmdProc *) im_real_to_complex_TclCmd_},
  {"imult",      (Tcl_CmdProc *) im_mult_TclCmd_},
  {"itoto",      (Tcl_CmdProc *) im_toto_TclCmd_},
  {"iscamult",   (Tcl_CmdProc *) im_scalar_mult_TclCmd_},
  {"iscaadd",    (Tcl_CmdProc *) im_scalar_add_TclCmd_},
  {"iadd",       (Tcl_CmdProc *) im_add_TclCmd_},
  {"ifilter",    (Tcl_CmdProc *) im_filter_TclCmd_},
  {"iconv",      (Tcl_CmdProc *) im_conv_TclCmd_},
  {"icreate",    (Tcl_CmdProc *) im_create_TclCmd_},
  {"iinsert",    (Tcl_CmdProc *) im_insert_TclCmd_},
  {"isetborder", (Tcl_CmdProc *) im_set_border_TclCmd_},

  {"gmod",       (Tcl_CmdProc *) ImaCrt2GradModCmd_},
  {"garg",       (Tcl_CmdProc *) ImaCrt2GradArgCmd_},
  {"cutedge",    (Tcl_CmdProc *) ImaCvlCutEdgeCmd_},

  {"diamond",    (Tcl_CmdProc *) im_diamond_TclCmd_},
  {"idirac",     (Tcl_CmdProc *) im_dirac_TclCmd_},
  {"igauss",     (Tcl_CmdProc *) im_gauss_TclCmd_},
  {"icell",      (Tcl_CmdProc *) im_cell_TclCmd_},
  {"icellelli",  (Tcl_CmdProc *) im_cell_elli_TclCmd_},
  {"icell3Dproj",(Tcl_CmdProc *) im_cell3Dproj_TclCmd_},
  {"istep",      (Tcl_CmdProc *) im_step_TclCmd_},
  {"icircle",    (Tcl_CmdProc *) im_circle_TclCmd_},
  {"iellipse",   (Tcl_CmdProc *) im_ellipse_TclCmd_},
  {"ibro",       (Tcl_CmdProc *) im_brownian_TclCmd_},
  {"ibro2Dfield",(Tcl_CmdProc *) im_brownian_2D_field_TclCmd_},
  {"ibro3D",     (Tcl_CmdProc *) im_brownian3D_TclCmd_},
  {"idirac3D",   (Tcl_CmdProc *) im_dirac3D_TclCmd_},
  {"itest3D",    (Tcl_CmdProc *) im_test3D_TclCmd_},
  {"itest3Dvector",    (Tcl_CmdProc *) im_test3D_vector_TclCmd_},
  {"igauss3D",   (Tcl_CmdProc *) im_gauss3D_TclCmd_},
  {"isingul3D",  (Tcl_CmdProc *) im_singul3D_TclCmd_},
  {"ivortex",    (Tcl_CmdProc *) im_vortex_TclCmd_},
  {"iUrand",     (Tcl_CmdProc *) im_Urand_TclCmd_},
  {"iGrand",     (Tcl_CmdProc *) im_Grand_TclCmd_},
  {"i3DGrand",   (Tcl_CmdProc *) im3D_Grand_TclCmd_},
  {"i3DvectorGrand",   (Tcl_CmdProc *) im3D_vector_Grand_TclCmd_},
  {"iprim",      (Tcl_CmdProc *) im_Prim_TclCmd_},
  {"dla",        (Tcl_CmdProc *) im_dla_TclCmd_},
  {"iwhite",     (Tcl_CmdProc *) im_white_noise_TclCmd_},
  {"snowflake",  (Tcl_CmdProc *) ImaSnfSnowFlakeCmd_},
  {"sflocon",    (Tcl_CmdProc *) ImaSnfSuperSnowFlakeCmd_},
  {"inull",      (Tcl_CmdProc *) im_null_TclCmd_},

  {"icopy",      (Tcl_CmdProc *) ImaTopCopyCmd_},
  {"i3Dcopy",    (Tcl_CmdProc *) Ima3DTopCopyCmd_},
  {"igetvalue",  (Tcl_CmdProc *) ImaTopGetValueCmd_},
  {"value",      (Tcl_CmdProc *) ImaTopGetValueCmd_},
  {"im_size",    (Tcl_CmdProc *) ImaTopGetSizeCmd_},
  {"im_extrema", (Tcl_CmdProc *) ImaTopGetminmaxCmd_},
  {"i3D_extrema",(Tcl_CmdProc *) Ima3DTopGetminmaxCmd_},
  {"igetlx",     (Tcl_CmdProc *) ImaGetlxCmd_},
  {"igetly",     (Tcl_CmdProc *) ImaGetlyCmd_},
  {"ifftfilter", (Tcl_CmdProc *) ImaTopFftFilterCmd_},

  {"nuage",      (Tcl_CmdProc *) ImaNuaLoadCmd_},
  {"sim",        (Tcl_CmdProc *) ImaSimNuaLoadCmd_},

  {"iconvol",    (Tcl_CmdProc *) ImaCvlConvolutionCmd_},
  {"ifftwfilter",(Tcl_CmdProc *) ImaFftwFilterCmd_},
  {"ifftw3dfilter",(Tcl_CmdProc *) ImaFftw3DFilterCmd_},

  {"itofft",     (Tcl_CmdProc *) ImaFftImageToFftCmd_},
  {"itofft2",    (Tcl_CmdProc *) ImaFftImageToFft2Cmd_},
  {"ffttoi",     (Tcl_CmdProc *) ImaFftFftToImageCmd_},
  {"fft",        (Tcl_CmdProc *) ImaFftFftCmd_},
  {"igfft",      (Tcl_CmdProc *) im_gfft2d_real_TclCmd_},
  {"ifftw2d",    (Tcl_CmdProc *) im_fftw2d_real_TclCmd_},
  {"ifftw3d",    (Tcl_CmdProc *) im_fftw3d_real_TclCmd_},

  {"isave",      (Tcl_CmdProc *) ImaFileSaveCmd_},
  {"iload",      (Tcl_CmdProc *) ImaFileLoadCmd_},
  {"i3Dload",    (Tcl_CmdProc *) Ima3DFileLoadCmd_},
  {"i3Dsave",    (Tcl_CmdProc *) Ima3DSaveFileCmd_},

  {"icolorizerect", (Tcl_CmdProc *) ImaColorizeRectangleCmd_},
  {"icolorizerect2", (Tcl_CmdProc *) ImaColorizeRectangle2Cmd_},
  {"igrondf",    (Tcl_CmdProc *) Ima_g_rond_f_Cmd_},
  {"iaddtraj",   (Tcl_CmdProc *) AddTrajBillesLatexCmd_},

  {"ladn",       (Tcl_CmdProc *) ImaAdnLoadCmd_},

  {"ifct",       (Tcl_CmdProc *) apply_fct_to_i_TclCmd_},
  {"ifct_vector",(Tcl_CmdProc *) apply_fct_to_i_vector_TclCmd_},
  {"i3Dfct",     (Tcl_CmdProc *) apply_fct_to_i3D_TclCmd_},
  {"i3Dfct_vector",(Tcl_CmdProc *) apply_fct_to_i3D_vector_TclCmd_},

  {"izoom",      (Tcl_CmdProc *) im_zoom_TclCmd_},
  {"iswapraw",   (Tcl_CmdProc *) im_swap_raws_TclCmd_},
  {"iswapcol",   (Tcl_CmdProc *) im_swap_cols_TclCmd_},
  {"imirror",    (Tcl_CmdProc *) im_mirror_TclCmd_},

  {"icut",       (Tcl_CmdProc *) im_cut_TclCmd_},

  {"icomb",      (Tcl_CmdProc *) comb_images_TclCmd_},

  {"myifilter",  (Tcl_CmdProc *) myifilter_image_TclCmd_},

  {"itracerect",  (Tcl_CmdProc *) itrace_rectangle_TclCmd_},

  {"itracesegment",  (Tcl_CmdProc *) itrace_segment_TclCmd_},

  {"itracecurve",  (Tcl_CmdProc *) itrace_curve_TclCmd_},

  {"iblackbox",  (Tcl_CmdProc *) iblackbox_TclCmd_},

  {"icombline",  (Tcl_CmdProc *) icomb_line_TclCmd_},

  {"iaddima",     (Tcl_CmdProc *) iadd_ima_TclCmd_},

  {"iaddborder",  (Tcl_CmdProc *) iadd_border_TclCmd_},

  {"icopyraw",   (Tcl_CmdProc *) icopy_raw_TclCmd_},

  {"iconvertsize",   (Tcl_CmdProc *) iconvert_size_TclCmd_},

  {"igfft2ri",   (Tcl_CmdProc *) im_gfft_to_2real_TclCmd_},

  {"inr2ri",     (Tcl_CmdProc *) im_nr_to_2real_TclCmd_},

  {"iswap",      (Tcl_CmdProc *) im_swap_part_TclCmd_},

  {"iinssig",    (Tcl_CmdProc *) im_insert_sig_TclCmd_},

  {"iinfo",      (Tcl_CmdProc *) im_get_info_TclCmd_},

  {"foreachi",   (Tcl_CmdProc *) foreachi_TclCmd_},
  {"imloop",   (Tcl_CmdProc *) foreachi_TclCmd_},

  {"iexpr",      (Tcl_CmdProc *) create_im_TclCmd_},

  {"idboxconv", (Tcl_CmdProc *) dxbox_conv_TclCmd_},

  {"isp2ssp",   (Tcl_CmdProc *) isp_2_ssp_TclCmd_},

  {"myisp2ssp", (Tcl_CmdProc *) my_isp_2_ssp_TclCmd_},

  {"ipowspec",  (Tcl_CmdProc *) ipowspec_TclCmd_},

  {"i3Dpowspec",  (Tcl_CmdProc *) i3Dpowspec_TclCmd_},

  {"iicut",     (Tcl_CmdProc *) iicut_TclCmd_},

  {"ithresh",    (Tcl_CmdProc *) im_thresh_TclCmd_},

  {"ithresh3D",    (Tcl_CmdProc *) im_thresh_TclCmd_},

  {"isubsample",    (Tcl_CmdProc *)  isubsample_TclCmd_},

  {"iipa",     (Tcl_CmdProc *) i_apply_ipa_table_TclCmd_},

  {"iinvert",    (Tcl_CmdProc *) i_inverse_TclCmd_},

  {"iangularmean",(Tcl_CmdProc *)  i_angular_mean_TclCmd_},

  {"vector2D_2_vtk", (Tcl_CmdProc *) vector2D_2_vtk_structured_points_TclCmd_},

  {"vector2D_2_jvx", (Tcl_CmdProc *) vector2D_2_jvx_vectorField_TclCmd_},

  {"vector2D_2_flowvis", (Tcl_CmdProc *) vector2D_2_flow_vis_TclCmd_},

  {"i3Dcomb",      (Tcl_CmdProc *) comb_images_3D_TclCmd_},

  {"iicut3D",      (Tcl_CmdProc *) iicut_3D_TclCmd_},

  {"iappodisation",      (Tcl_CmdProc *) im_appodisation_TclCmd_},

  {"i3Dappodisation",    (Tcl_CmdProc *) im_3D_appodisation_TclCmd_},

  {"imlogincr",      (Tcl_CmdProc *) image_moment_of_log_increments_TclCmd_},



  /*
   * End of the image comands.
   */
  {NULL, (Tcl_CmdProc *) NULL}
};


/*
 * Commands concerning the mathematical morphology library :
 */

static cmd_info morph_cmds[] = {

  {"morphtoto",   (Tcl_CmdProc *) morph_toto_TclCmd_},
  {"morph",       (Tcl_CmdProc *) morph_main_TclCmd_},

  /*
   * End of the mathematical morphology comands.
   */
  {NULL, (Tcl_CmdProc *) NULL}
};

/*
 * Commands concerning the wt1d library :
 */

static cmd_info wt1d_cmds[] = {

  {"wvcreate",   (Tcl_CmdProc *) create_wavelet_TclCmd_},
  {"wv2sig",     (Tcl_CmdProc *) wavelet_to_signal_TclCmd_},
  {"wt1d",       (Tcl_CmdProc *) wt1d_single_TclCmd_},
  {"wvtab",      (Tcl_CmdProc *) tab_scales_TclCmd_},
  {"awt1d",      (Tcl_CmdProc *) wt1d_all_TclCmd_},
  {"getscales",  (Tcl_CmdProc *) get_scales_TclCmd_},
  {"wvget",      (Tcl_CmdProc *) get_wavelet_from_collection_TclCmd_},

  /*
   * End of the wt1d comands.
   */
  {NULL, (Tcl_CmdProc *) NULL}
};


/*
 * Commands concerning the wt2d library :
 */

static cmd_info wt2d_cmds[] = {

  {"chain2",          (Tcl_CmdProc *) ExtChnChainCmd_},
  {"ethresh",         (Tcl_CmdProc *) ExtChnThreshCmd_},

  {"esave",           (Tcl_CmdProc *) ExtFileSaveCmd_},
  {"eload",           (Tcl_CmdProc *) ExtFileLoadCmd_},
  {"eload3D_display", (Tcl_CmdProc *) ExtFileLoad3DDisplayCmd_},
  {"ext3ddecimation", (Tcl_CmdProc *) ExtFile3DDecimationCmd_},
  {"save3d",          (Tcl_CmdProc *) ExtFileChainSaveCmd_},

  {"eupdate",     (Tcl_CmdProc *) update_ext_image_TclCmd_},

  {"hsearch",     (Tcl_CmdProc *) search_lines_TclCmd_},
  {"hsearch_with_diam",  (Tcl_CmdProc *)   search_lines_with_diam_TclCmd_},
  {"eibending",   (Tcl_CmdProc *) compute_bending_TclCmd_},
  {"l2c",         (Tcl_CmdProc *) lines_to_chains_TclCmd_},
  {"vchainold",   (Tcl_CmdProc *) vertical_chain_TclCmd_},

  {"extrema",     (Tcl_CmdProc *) ExtExtComputeCmd_},
  {"carac",       (Tcl_CmdProc *) ExtExtCaracCmd_},

  {"locmax",      (Tcl_CmdProc *) w2_local_maxima_TclCmd_},
  {"sslocmax",    (Tcl_CmdProc *) w2_local_space_scale_maxima_TclCmd_},
  {"emerge",      (Tcl_CmdProc *) merge_ext_image_TclCmd_},
  {"egather",     (Tcl_CmdProc *) gather_ext_image_TclCmd_},
  {"follow",      (Tcl_CmdProc *) w2_follow_contour_TclCmd_},
  {"follow2",     (Tcl_CmdProc *) w2_follow2_contour_TclCmd_},
  {"wtmm2d",      (Tcl_CmdProc *) w2_wtmm2d_TclCmd_},  
  {"eiset",       (Tcl_CmdProc *) w2_set_with_images_TclCmd_},


  {"unlink",      (Tcl_CmdProc *) ExtDicUnlinkCmd_},
  {"getnamefrom", (Tcl_CmdProc *) ExtDicNameCmd_},

  {"efct",         (Tcl_CmdProc *) apply_fct_to_e_TclCmd_},
  {"efct_tsallis", (Tcl_CmdProc *) apply_fct_to_e_tsallis_TclCmd_},
  {"efct3D",       (Tcl_CmdProc *) apply_fct_to_extima3D_TclCmd_},
  {"ifct3D",       (Tcl_CmdProc *) apply_fct_to_data3D_TclCmd_},

  {"gkapa",       (Tcl_CmdProc *) gkapa_TclCmd_},
  {"gkapap",      (Tcl_CmdProc *) gkapap_TclCmd_},

  {"linestats",   (Tcl_CmdProc *) get_line_stats_TclCmd_},

  {"rm_ext",           (Tcl_CmdProc *) w2_remove_extrema_TclCmd_},
  {"ekeep_isolated",   (Tcl_CmdProc *) w2_keep_isolated_TclCmd_},
  {"ekeep_circle",     (Tcl_CmdProc *) w2_keep_circle_TclCmd_},
  {"elog",             (Tcl_CmdProc *) w2_log_TclCmd_},
  {"rm_1line",         (Tcl_CmdProc *) w2_remove_line_TclCmd_},
  {"rm_lines",         (Tcl_CmdProc *) w2_remove_lines_TclCmd_},
  {"rm_by_size",       (Tcl_CmdProc *) w2_remove_lines_by_size_TclCmd_},
  {"rm_by_mod",        (Tcl_CmdProc *) w2_remove_lines_by_mean_mod_TclCmd_},
  {"rm_by_arg",        (Tcl_CmdProc *) w2_remove_lines_by_arg_TclCmd_},

  {"ekeep",       (Tcl_CmdProc *) w2_keep_by_value_TclCmd_},

  {"emask",       (Tcl_CmdProc *) mask_ext_image_TclCmd_},
  {"emask2",      (Tcl_CmdProc *) mask2_ext_image_TclCmd_},

  {"l2m",         (Tcl_CmdProc *) line_2_max_TclCmd_},
  {"ssm",         (Tcl_CmdProc *) search_single_max_TclCmd_},

  {"vchain",      (Tcl_CmdProc *) mlm_vert_chain_TclCmd_},
  {"vchain2",     (Tcl_CmdProc *) pt_to_pt_vert_chain_TclCmd_},
  {"epurge",      (Tcl_CmdProc *) purge_chains_TclCmd_},

  {"vc2s",        (Tcl_CmdProc *) vert_chain_to_s_TclCmd_},

  {"egbm",        (Tcl_CmdProc *) get_best_max_on_vc_TclCmd_},

  {"einfo",       (Tcl_CmdProc *) ext_get_info_TclCmd_},

  {"foreache",    (Tcl_CmdProc *) foreache_TclCmd_},
  {"eiloop",      (Tcl_CmdProc *) foreache_TclCmd_},
  {"eigrloop",    (Tcl_CmdProc *) gr_loop_TclCmd_},
  {"eilineloop",  (Tcl_CmdProc *) line_loop_TclCmd_},

  {"l2s",         (Tcl_CmdProc *) line_to_s_TclCmd_},

  {"ecut",        (Tcl_CmdProc *) e_cut_TclCmd_},

  {"l2name",      (Tcl_CmdProc *) name_line_TclCmd_},

  {"s2ei",	  (Tcl_CmdProc *) xysig_2_ei_TclCmd_},

  {"mys2ei",	  (Tcl_CmdProc *) my_xysig_2_ei_TclCmd_},
 
  {"eisavelines",  (Tcl_CmdProc *) ei_save_lines_TclCmd_},
  
  {"eisavelines2",  (Tcl_CmdProc *) ei_save_lines_2_TclCmd_},
  
  {"eisave_skel4vtk",  (Tcl_CmdProc *) ei_save_skel_vtk_TclCmd_},
  {"eisave_skel4jvx",  (Tcl_CmdProc *) ei_save_skel_jvx_TclCmd_},
  {"eisave_skel4m3d",  (Tcl_CmdProc *) ei_save_skel_m3d_TclCmd_},

  {"eisave_skelchain4vtk",  (Tcl_CmdProc *) ei_save_skel_chain_vtk_TclCmd_},
  {"e2vtk", (Tcl_CmdProc *) ei_save_extimage_4_vtk_TclCmd_ },

  {"sw_s2ext",	  (Tcl_CmdProc *) sw_sig_2_sig_extrema_TclCmd_},

  {"egetextr",	(Tcl_CmdProc *) egetextr_TclCmd_},
  {"egetmin",	(Tcl_CmdProc *) egetmin_TclCmd_},
  {"egetmax",	(Tcl_CmdProc *) egetmax_TclCmd_},

  {"eimult",	(Tcl_CmdProc *) e_mult_by_i_TclCmd_},

  {"eidrawcv",	(Tcl_CmdProc *) e_draw_in_canvas_TclCmd_},

  {"eilinetag",	(Tcl_CmdProc *) special_tag_lines_TclCmd_},

  {"eicorr",	(Tcl_CmdProc *) e_correlation_TclCmd_},

  {"eicorr2",	(Tcl_CmdProc *) e_correlation2_TclCmd_},

  {"eitagvc",	(Tcl_CmdProc *) e_tag_vc_TclCmd_},

  {"eivchisto",   (Tcl_CmdProc *) e_vc_histo_TclCmd_},

  {"eivchisto2",   (Tcl_CmdProc *) e_vc_histo2_TclCmd_},

  {"eivcgerbe",   (Tcl_CmdProc *) e_vc_gerbe_TclCmd_},

  {"eisavetag",  (Tcl_CmdProc *) e_save_tagged_TclCmd_},

  {"eicounttag",  (Tcl_CmdProc *) e_count_tag_TclCmd_},

  {"skelsave",	(Tcl_CmdProc *) e_save_skeleton_TclCmd_},

  /*
   * End of the wt2d comands.
   */
  {NULL, (Tcl_CmdProc *) NULL}
};

/*
 * Commands concerning the wt3d library :
 */

static cmd_info wt3d_cmds[] = {

  {"unlink3D",            (Tcl_CmdProc *) Ext3DDicUnlinkCmd_},
  {"eload3Dsmall",        (Tcl_CmdProc *) ExtFileLoadCmd_3Dsmall_},
  {"esave3Dsmall",        (Tcl_CmdProc *) ExtFileSaveCmd_3Dsmall_},
  {"efct3Dsmall",         (Tcl_CmdProc *) apply_fct_to_e3Dsmall_TclCmd_},
  {"efct3Dsmall2",        (Tcl_CmdProc *) apply_fct_to_e3Dsmall2_TclCmd_},
  {"ei3Dpercent_identity",(Tcl_CmdProc *) percent_of_identity_e3Dsmall_TclCmd_},
  {"vchain3Dsmall",       (Tcl_CmdProc *) pt_to_pt_vert_chain_3Dsmall_TclCmd_},
  {"e3Dsmall_getextr",    (Tcl_CmdProc *) e3Dsmall_getextr_TclCmd_},
  {"ehisto3Dsmall",       (Tcl_CmdProc *) StatExt3DsmallHistoCmd_},
  {"ei3Dsmallsave_4_vtk", (Tcl_CmdProc *) ei_3Dsmall_save_4_vtk_TclCmd_},
  {"buffer3D_2_vtk",      (Tcl_CmdProc *) buffer3D_2_vtk_structured_points_TclCmd_},
  {"i3D_2_vtk",           (Tcl_CmdProc *) i3D_2_vtk_structured_points_TclCmd_},
  {"vector3D_2_vtk",      (Tcl_CmdProc *) vector3D_2_vtk_structured_points_TclCmd_},
  {"ei3Dsmallsave_4_vtkpolydata", (Tcl_CmdProc *) ei_3Dsmall_save_4_vtk_polydata_TclCmd_},
  {"ei3Dsmallsave_skel4vtk", (Tcl_CmdProc *) ei3Dsmall_save_skel_vtk_TclCmd_},
  {"ecut3Dsmall",         (Tcl_CmdProc *) e_cut_3Dsmall_TclCmd_},
  {"vc2s3Dsmall",         (Tcl_CmdProc *) vert_chain_3Dsmall_to_s_TclCmd_},
  {"eivcgerbe3Dsmall",    (Tcl_CmdProc *) e_vc_gerbe_3Dsmall_TclCmd_},
  {"ei3Dsmallcorr2",      (Tcl_CmdProc *) ei3Dsmall_correlation2_TclCmd_},
  {"ei3Dsmallloop",       (Tcl_CmdProc *) ei3Dsmall_loop_TclCmd_},
  {"eitagvc3Dsmall",      (Tcl_CmdProc *) e_tag_vc_3Dsmall_TclCmd_ },
  {"wtmm3d",              (Tcl_CmdProc *) w3_wtmm3d_TclCmd_},  

  /*
   * End of the wt3d comands.
   */
  {NULL, (Tcl_CmdProc *) NULL}
};


/*
 * Commands concerning the files manipulation :
 */

static cmd_info file_cmds[] = {

  {"xv_file",      (Tcl_CmdProc *) xv_file_TclCmd_},
  {"i2eps",        (Tcl_CmdProc *) i_to_ps_TclCmd_},
  {"e2eps",        (Tcl_CmdProc *) e_to_ps_TclCmd_},
  {"r2eps",        (Tcl_CmdProc *) rectangle_to_ps_TclCmd_},

  /*
   * End of the files manipulation comands.
   */
  {NULL, (Tcl_CmdProc *) NULL}
};

/*
 * Commands concerning viewer task :
 */

/*static cmd_info viewer_cmds[] = {

  {"iconvert",     ViewConvImageCmd_},*/
  
  /*
   * End of viewer comands.
   */
/*  {NULL, (Tcl_CmdProc *) NULL}
};*/






static cmd_info * all_cmds_list[] = {
  smurf_cmds,
  signal_cmds,
  stats_cmds,
  image_cmds,
  morph_cmds,
  wt1d_cmds,
  wt2d_cmds,
  wt3d_cmds,
  file_cmds,
  /*viewer_cmds,*/
  NULL
};


/*
 */

int
sm_interpreter_init (Tcl_Interp * interp)
{
  register cmd_info *cmd_info_ptr;
  register cmd_info *cmd_info_list_ptr;
  int i;

  hash_tables_init();
  /*_math_expr_init_ ();*/

  /*
   * Create the built-in commands.
   */

  for (cmd_info_list_ptr = all_cmds_list[0], i=0;
       cmd_info_list_ptr != NULL;
       cmd_info_list_ptr = all_cmds_list[i], i++) {
    for (cmd_info_ptr = cmd_info_list_ptr;
	 cmd_info_ptr -> name != NULL;
	 cmd_info_ptr++) {
      Tcl_CreateCommand(interp,
			cmd_info_ptr -> name,
			cmd_info_ptr -> proc,
			(ClientData) 0, (void (*)()) NULL);
    }
  }

  Cv1d_pkgInit(interp);
  Cv2d_pkgInit(interp);
  Steph_pkgInit(interp);
  dyad_pkgInit(interp);

  //cmd_info_list_ptr = all_cmds_list[0];
  //cmd_info_ptr = cmd_info_list_ptr;
  //printf("mon cul %s\n",cmd_info_ptr -> name);

  return TCL_OK;
}


/*
 * Command name in xsmurf : cmdlist
 */

int
cmds_list_TclCmd_ (ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "",
    NULL
  };

  char * help_msg =
  {
    (" List the C-defined commands.\n"
     "\n"
     "Parameters :\n"
     "  none.")
  };

  /* Command's parameters */

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  register cmd_info *cmd_info_ptr;
  register cmd_info *cmd_info_list_ptr;
  int i;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  for (cmd_info_list_ptr = all_cmds_list[0], i=0;
       cmd_info_list_ptr != NULL;
       i++, cmd_info_list_ptr = all_cmds_list[i]) {
    if (i != 0) {
      Tcl_AppendResult(interp, " {", (char *) NULL);
    } else {
      Tcl_AppendResult(interp, "{", (char *) NULL);
    }
    cmd_info_ptr = cmd_info_list_ptr;
    Tcl_AppendResult(interp, cmd_info_ptr -> name, (char *) NULL);
    for (cmd_info_ptr++;
	 cmd_info_ptr -> name != NULL;
	 cmd_info_ptr++) {
      Tcl_AppendResult(interp, " ", cmd_info_ptr -> name, (char *) NULL);
    }
    Tcl_AppendResult(interp, "}", (char *) NULL);
  }

  return TCL_OK;
}



/* emplacement a revoir */

double step (double x, double step)
{
  return (x < step ? 0 : 1);
}

/*static void
_math_expr_init_ ()
{
  namefnct("sin", sin);
  namefnct("exp", exp);
  namefnct("cos", cos);
  namefnct("fabs", fabs);
  namefnct("mod", fmod);
  namefnct("atan2", atan2);
  namefnct("log", log);
  namefnct("log10", log10);
  namefnct("pow", pow);
  namefnct("sqrt", sqrt);
  namefnct("tan", tan);
  namefnct("atan", atan);
  namefnct("acos", acos);
  namefnct("asin", asin);
  namefnct("ceil", ceil);
  namefnct("floor", floor);
  namefnct("step", step);
}*/





