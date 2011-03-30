################################################################################
#
# Build script for project
#
################################################################################

# Add source files here
EXECUTABLE	:= cannyEdge2D_cuda
# Cuda source files (compiled with cudacc)
CUFILES		:= cannyEdge2D_cuda.cu
# CUDA dependency files
CU_DEPS         := cannyEdge2D_cuda_kernel.cu reduction.cu reduction_kernel.cu
# C/C++ source files (compiled with gcc / c++)
CCFILES        :=
CFILES		:= cmd_cannyEdge2D_cuda.c fft_utils.c

verbose:=1
nvcc_warn_verbose:=1

# Additional libraries needed by the project
USECUFFT        := 1


################################################################################
# Rules and targets

include ../../common/common.mk
